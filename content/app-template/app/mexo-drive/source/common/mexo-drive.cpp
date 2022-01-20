#include "mexo-drive.hpp"
		
namespace mexo_drive{

//1.1	Соединение между блоками , управляющими переферией и системой управления
	struct hardware_link {
			static dc_power_supply & pwm_block(void);
			static current_sensor & sence_block(void);
			static motor_quadr_enco & enco_block(void);
			static ::mexo::prioritet_subsystem & prioritet_subsystem(void);
			static ::mexo::periodic_subsystem & periodic_subsystem(void);
			static void reconfig(void);	
	};	
//1.2 Сам актуатор
typedef MEXO_DRIVE_ACTUATOR_NAME::dev_t<types,hardware_link>  joint;

//2.1. тут храним все актуальные сигналы в одном месте
struct {
	dc_power_supply::present_s dc_power_supply;
	current_sensor::present_s current_sensor;
	motor_enco::present_s motor_enco;
	motor_quadr_enco::present_s motor_quadr_enco;
	joint::present_s joint;
} present ={};

//2.2. тут храним все настройки в одном месте
struct {
	joint::config_s joint;
} config ={};

//2.3. тут храним все сигналы управления в одном месте
struct {
	joint::action_s joint;
} action={};

hardware_link hardware_link_;

//3 готовим железо
//3.1. конфигурируем приоритетную подсистему,саязывающую все блоки с аппаратурой
// подсистема - работает в контексте prioritet ( будет срабатывать по прерыванию от АЦП)
::mexo::prioritet_subsystem hardware_prioritet_subsystem(RT("hw-A"), false); 

//3.2. подключаем к системе силовой преобразователь + 0.5 мкс
dc_power_supply dc_power_supply_(RT("dc"), &hardware_prioritet_subsystem, perephery_config.dc_power_supply, present.dc_power_supply ); 

//3.3.  подключаем к подсистеме датчик тока  + 0.5 мкс
current_sensor current_sensor_(RT("current sensor"), &hardware_prioritet_subsystem, perephery_config.current_sensor, present.current_sensor);

//4. еще один системный сервис - будет работать каждый четвертый период - в слоте 0
::mexo::periodic_subsystem hardware_periodic_subsystem(RT("hardware-B"), false,{0}); 
//4.1. подключаем датчик положения мотора
motor_enco motor_enco_(RT("motor enco"),&hardware_periodic_subsystem, perephery_config.motor_enco,present.motor_enco );


//4.2. подключаем квадратурный датчик положения мотора
motor_quadr_enco motor_quadr_enco_(RT("motor quadro enco"),&hardware_periodic_subsystem, perephery_config.motor_quadr_enco,present.motor_quadr_enco );


//5. сам моторчик +1мкс - инфраструкутура и быстрый фильтр  +1мкс - напряженческий, +2,5мкс -токовый
joint joint_(hardware_link_, RT("joint"), action.joint, config. joint, present.joint,0);

//6.3. +0.3мкс от hardware_subsystem
//делегат в слот "start" - сработает при старте 
::mexo::machine::slot::simple start(
	::mexo::machine::slot::kind::start
, [] {
	hardware_prioritet_subsystem.start();									//активируем подсистему аппаратуры
	hardware_periodic_subsystem.start();									//активируем подсистему аппаратуры
});


//7.  делигируем (to do коряво)
dc_power_supply & hardware_link::pwm_block(void){
	return dc_power_supply_;
}
current_sensor & hardware_link::sence_block(void){
	return current_sensor_;
}

motor_quadr_enco & hardware_link::enco_block(void){
	return motor_quadr_enco_;
}

void hardware_link::reconfig(void){
	dc_power_supply_.reconfig();
}
::mexo::prioritet_subsystem & hardware_link::prioritet_subsystem(void){
	return hardware_prioritet_subsystem;
}
::mexo::periodic_subsystem & hardware_link::periodic_subsystem(void){
	return hardware_periodic_subsystem;
}

#if ROBO_APP_NET_FLOW_ENABLED == 1

FLOW_PERFORMER_RAND_RECORD(echo,can0,
{
		static uint8_t old_data[8];
		static size_t old_sz;
		if(in_msg){
			put_answer(in_msg->data(),in_msg->size());
			old_sz = in_msg->size();
			std::copy_n(in_msg->data(),old_sz,old_data);
		}	else{
			put_answer(old_data,old_sz);
		}	
	}
)

FLOW_SERIAL_PERFORMER_RECORD(serial0,can0,10,4);

void decode_action_(const uint8_t * _data, int _sz){
	if(_sz>=0){
		action.joint.ps.dev.mode = *_data; _data++; _sz--;
		action.joint.ps.dev.actual = true;
	}
	if(_sz>=2){
		action.joint.speed = *(int16_t *)_data;_data+=2;_sz-=2;
	}
	if(_sz>=4){
		action.joint.position = *(int32_t *)_data;
	}
}
void encode_feetback_(uint8_t * _data){
	*_data = (uint8_t ) present.joint.ps.dev.mode; _data++;
	*(uint16_t *)_data = (uint16_t ) present.joint.speed_filter.fb.output; _data+=2;
	*(uint32_t *)_data = (uint32_t ) present.motor_quadr_enco.sb.output; 
}

FLOW_PERFORMER_RAND_RECORD(exchange,can0,
	static uint8_t snapshot[7];
	if(in_msg){
		decode_action_(in_msg->data(),in_msg->size());
		encode_feetback_(snapshot);
		put_answer(snapshot,7);
	}	else{
		encode_feetback_(snapshot);
		put_answer(snapshot,7);
	}	
)

FLOW_PERFORMER_RAND_RECORD(var,can0,
	static uint8_t answer[8];
	if(in_msg){
		put_answer(answer,::mexo::var::machine::proto(in_msg->data(),answer) );
	}
)

//8. делегат в слот "begin" - сработает при инициализации - собираем все вместе
::mexo::machine::slot::simple begin(
	::mexo::machine::slot::kind::begin
	,	[] {	
		::mexo::tp.set_verb(::mexo::tp_verb::loop);
		action.joint.ps.voltage = 32767;
		action.joint.ps.current = 1500;
	}
);

#endif
}


#include "mexo/mexo.board.common.hpp"
#define BOARD_SWITCH_PORT_PATH serial0_PATH
//#define BOARD_FREEMASTER_CONNECT_TYPE BOARD_FREEMASTER_CONNECT_TYPE_NONE
//#define BOARD_TERMO_CONNECT_TYPE BOARD_TERMO_CONNECT_TYPE_NONE
#define BOARD_FREEMASTER_CONNECT_TYPE BOARD_FREEMASTER_CONNECT_TYPE_ABONENT
#define BOARD_TERMO_CONNECT_TYPE BOARD_TERMO_CONNECT_TYPE_ABONENT
#include "mexo/actuator.board.inc.hpp"


