#include "mexo-test.hpp"
#include "freemaster/robosd_fm.hpp"
//голая инфраструктура занимает ровно 3 мкс

//1.	Соединение между блоками , управляющими переферией и системой управления
struct hardware_link {
			static dc_power_supply & pwm_block(void);
			static current_sensor & sence_block(void);
			static motor_quadr_enco & enco_block(void);
			static ::mexo::prioritet_subsystem & prioritet_subsystem(void);
			static ::mexo::periodic_subsystem & periodic_subsystem(void);
			static void reconfig(void);	
} hardware_link_;

#if 0
#define ACTUATOR_TEMPLATE_NAME joint
#define  joint_ps_VOLTAGE_REGULATOR_ENABLED 1
#define  joint_ps_CURRENT_REGULATOR_ENABLED 1
#define  joint_ps_CURRENT_MEASSURY_ENABLED 1
#define  joint_ps_CURRENT_DIFF_ENABLED 0
#define  joint_ps_CURRENT_FILTER_ENABLED 1
#define joint_ps_VOLTAGE_MIN_LIM -12.f
#define joint_ps_VOLTAGE_MAX_LIM 12.f
#define joint_ps_VOLTAGE_RAMP_GAIN 0.001f
#define joint_ps_CURRENT_FILTER_GAIN 0.9f
#define joint_ps_CURRENT_FILTER_SHIFT_GAIN 0
#define joint_ps_CURRENT_FILTER_SHIFT_PRESC 0
#define joint_ps_CURRENT_FILTER_SHIFT_VALUE 0

#define joint_ps_CURRENT_PROP_GAIN 0.f
#define joint_ps_CURRENT_MODEL_GAIN 0.f
#define joint_ps_CURRENT_DIFF_GAIN 0
#define joint_ps_CURRENT_CONTROL_SHIFT 0
#define joint_ps_CURRENT_MODEL_SHIFT 0
#include "mexo/actuator.inc.hpp"
#else

#define ACTUATOR_TEMPLATE_NAME joint
#define  joint_ps_VOLTAGE_REGULATOR_ENABLED 1
#define  joint_ps_CURRENT_REGULATOR_ENABLED 1
#define  joint_ps_CURRENT_LIMMITER_ENABLED 1
#define  joint_ps_CURRENT_MEASSURY_ENABLED 1
#define  joint_ps_CURRENT_DIFF_ENABLED 0
#define  joint_ps_CURRENT_FILTER_ENABLED 0
#define  joint_ps_CURRENT_FAST_FILTER_ENABLED 1
#define joint_ps_VOLTAGE_MIN_LIM -32767
#define joint_ps_VOLTAGE_MAX_LIM 32767
#define joint_ps_VOLTAGE_RAMP_GAIN 300

#if joint_ps_CURRENT_FILTER_ENABLED ==1
#define joint_ps_CURRENT_FILTER_GAIN 7
#define joint_ps_CURRENT_FILTER_SHIFT_GAIN 3
#define joint_ps_CURRENT_FILTER_SHIFT_PRESC 0
#define joint_ps_CURRENT_FILTER_SHIFT_VALUE 0
#endif

#if joint_ps_CURRENT_FAST_FILTER_ENABLED ==1
#define joint_ps_CURRENT_FAST_FILTER_SHIFT_VALUE 3
#endif

#define joint_ps_CURRENT_PROP_GAIN 50
#define joint_ps_CURRENT_MODEL_GAIN 700
#define joint_ps_CURRENT_DIFF_GAIN 0
#define joint_ps_CURRENT_CONTROL_SHIFT 7
#define joint_ps_CURRENT_MODEL_SHIFT 10

#define joint_ps_CURRENT_MIN_LIM -200
#define joint_ps_CURRENT_MAX_LIM 200
#define joint_ps_CURRENT_LIMMITER_RAMP_STEP 500

#define joint_MOTOR_POSTITION_MEASSURY_ENABLED 1
#define joint_MOTOR_SPEED_FILTER_ENABLED 1
#define joint_SPEED_FILTER_GAIN 31
#define joint_SPEED_FILTER_SHIFT_GAIN 5
#define joint_SPEED_FILTER_SHIFT_PRESC 5
#define joint_SPEED_FILTER_SHIFT_VALUE 0

#define joint_SPEED_OV_CURRENT_MODE_ENABLED 1
#define joint_SPEED_OV_CURRENT_PROP_GAIN 0
#define joint_SPEED_OV_CURRENT_MODEL_GAIN 0
#define joint_SPEED_OV_CURRENT_CONTROL_SHIFT 0
#define joint_SPEED_OV_CURRENT_MODEL_SHIFT 0
#define joint_SPEED_OV_CURRENT_FORCE_GAIN 0
#define joint_SPEED_OV_CURRENT_FORCE_LIM 0
#define joint_SPEED_OV_CURRENT_REF_GAIN 0
#define joint_SPEED_OV_CURRENT_REF_PRESC_SHIFT 0

#define joint_SPEED_OV_VOLTAGE_CL_MODE_ENABLED 1
#define joint_SPEED_OV_VOLTAGE_CL_PROP_GAIN 45
#define joint_SPEED_OV_VOLTAGE_CL_MODEL_GAIN 30
#define joint_SPEED_OV_VOLTAGE_CL_CONTROL_SHIFT 0
#define joint_SPEED_OV_VOLTAGE_CL_MODEL_SHIFT 10
#define joint_SPEED_OV_VOLTAGE_CL_FORCE_GAIN 50
#define joint_SPEED_OV_VOLTAGE_CL_FORCE_LIM 7000

#define joint_POSITION_OV_CURRENT_MODE_ENABLED 1
#define joint_POSITION_OV_VOLTAGE_CL_MODE_ENABLED 1


#define joint_POSITIONER_OV_CURRENT_PROP_GAIN 1
#define joint_POSITIONER_OV_CURRENT_DIFF_GAIN 0
#define joint_POSITIONER_OV_CURRENT_CONTROL_SHIFT 0
#define joint_POSITIONER_OV_CURRENT_DIFF_QUADR_GAIN 0
#define joint_POSITIONER_OV_CURRENT_DIFF_QUADR_SHIFT 0
#define joint_POSITIONER_OV_CURRENT_DEAD_ZONE 1
#define joint_POSITIONER_OV_CURRENT_CRAWL_SPEED 1

#define joint_POSITIONER_OV_VOLTAGE_CL_PROP_GAIN 1
#define joint_POSITIONER_OV_VOLTAGE_CL_DIFF_GAIN 0
#define joint_POSITIONER_OV_VOLTAGE_CL_CONTROL_SHIFT 3
#define joint_POSITIONER_OV_VOLTAGE_CL_DIFF_QUADR_GAIN 0
#define joint_POSITIONER_OV_VOLTAGE_CL_DIFF_QUADR_SHIFT 0
#define joint_POSITIONER_OV_VOLTAGE_CL_DEAD_ZONE 16
#define joint_POSITIONER_OV_VOLTAGE_CL_CRAWL_SPEED 0


#include "mexo/actuator.inc.hpp"
#endif

typedef  joint_t<types, hardware_link> joint;

//2. Конфигурируем систему управления мотором и собираем шаблон для мотора
// собираем все сигналы и настройки в одно место- легко сохранять, легко отлаживаться, 
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

//3 готовим железо
//3.1. конфигурируем приоритетную подсистему,саязывающую все блоки с аппаратурой
// подсистема - работает в контексте prioritet ( будет срабатывать по прерыванию от АЦП)
::mexo::prioritet_subsystem hardware_prioritet_subsystem(RT("hardware-A"), false); 

//3.2. подключаем к системе силовой преобразователь + 0.5 мкс
dc_power_supply dc_power_supply_(RT("dc"), &hardware_prioritet_subsystem, perephery_config.dc_power_supply, present.dc_power_supply ); 

//3.3.  подключаем к подсистеме датчик тока  + 0.5 мкс
current_sensor current_sensor_(RT("current sensor"), &hardware_prioritet_subsystem, perephery_config.current_sensor, present.current_sensor);

//4. еще один системный скервис - будет работать каждый четвертый период - в слоте 0
::mexo::periodic_subsystem hardware_periodic_subsystem(RT("hardware-B"), false,{0}); 
//4.1. подключаем датчик положения мотора
motor_enco motor_enco_(RT("motor enco"),&hardware_periodic_subsystem, perephery_config.motor_enco,present.motor_enco );

//4.2. делегат в слот №0 - команда на расчет позиции.
::mexo::machine::slot::simple enco_put(
	0
	,	&AS5048A::put
);

//4.3. делегат в слот №2 - команда на чтение позиции . В нулевом слоте ( в данной настройке mexo имеет 4 слота ) 
// получим готовые данные
::mexo::machine::slot::simple enco_get(
	2
	,	&AS5048A::get
);

//4.2. подключаем квадратурный датчик положения мотора
motor_quadr_enco motor_quadr_enco_(RT("motor quadro enco"),&hardware_periodic_subsystem, perephery_config.motor_quadr_enco,present.motor_quadr_enco );


//5. сам моторчик +1мкс - инфраструкутура и быстрый фильтр  +1мкс - напряженческий, +2,5мкс -токовый
joint joint_(hardware_link_, RT("joint"), action.joint, config. joint, present.joint,0);

//6. дополнителные сервисы для mexo 
//6.1. не добавляет времени к обрабготке прерывания
::mexo::machine::slot::simple frontend_pool_ (
	::mexo::machine::slot::kind::frontend
	,	[]{
static volatile robo::time_us_t g_time_us_t = 0;
		robo::freemaster::poll();
		g_time_us_t = robo::system::env::realtime_us();
	}
);

//6.2. +0.4мкс
::mexo::machine::slot::simple backend_(
	::mexo::machine::slot::kind::backend
	,	&robo::freemaster::recorder	
);	

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

//uint16_t  as5040_driver::native(void){
	//return 0;
//}



void decode_action(const uint8_t * _data, int _sz){
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
void encode_feetback(uint8_t * _data){
	*_data = (uint8_t ) present.joint.ps.dev.mode; _data++;
	*(uint16_t *)_data = (uint16_t ) present.joint.speed_filter.fb.output; _data+=2;
	*(uint32_t *)_data = (uint32_t ) present.motor_quadr_enco.sb.output; 
}


//8. делегат в слот "begin" - сработает при инициализации - собираем все вместе
::mexo::machine::slot::simple begin(
	::mexo::machine::slot::kind::begin
	,	[] {	
		::mexo::tp::set_verb(::mexo::tp_verb::loop);
		::robo::freemaster::connect( ::robo::net::iserial::query(serial0_PATH) );
		action.joint.ps.voltage = 32767;
		action.joint.ps.current = 1500;
	}
);

// первый способ сразу  - в лоб
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
	
FLOW_SERIAL_PERFORMER_RECORD(serial0,can0,4,6);

FLOW_PERFORMER_RAND_RECORD(exchange,can0,
	static uint8_t snapshot[7];
	if(in_msg){
		decode_action(in_msg->data(),in_msg->size());
		encode_feetback(snapshot);
		put_answer(snapshot,7);
	}	else{
		encode_feetback(snapshot);
		put_answer(snapshot,7);
	}	
)

