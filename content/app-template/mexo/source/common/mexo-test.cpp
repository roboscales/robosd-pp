#include "mexo-test.hpp"
#include "freemaster/robosd_fm.hpp"
//голая инфраструктура занимает ровно 3 мкс

//1.	Соединение между блоками , управляющими переферией и системой управления
struct hardware_link {
			static dc_power_supply_b & pwm_block(void);
			static current_sensor_b & sence_block(void);
			static ::mexo::prioritet_subsystem & subsystem(void);
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

#include "mexo/actuator.inc.hpp"
#endif

typedef  joint_t<types, hardware_link> joint;

//2. Конфигурируем систему управления мотором и собираем шаблон для мотора
// собираем все сигналы и настройки в одно место- легко сохранять, легко отлаживаться, 
//2.1. тут храним все актуальные сигналы в одном месте
struct {
	dc_power_supply_b::present_s dc_power_supply;
	current_sensor_b::present_s current_sensor;
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
::mexo::prioritet_subsystem hardware_subsystem(RT("hardware"), false); 

//3.2. подключаем к системе силовой преобразователь + 0.5 мкс
dc_power_supply_b dc_power_supply_(hardware_subsystem, RT("dc"), dc_power_supply_config, present.dc_power_supply ); 

//3.3.  подключаем к подсистеме датчик тока  + 0.5 мкс
current_sensor_b current_sensor_(hardware_subsystem,RT("current sensor"),current_sensor_config, present.current_sensor);

//3.4.  делигируем (to do коряво)
dc_power_supply_b & hardware_link::pwm_block(void){
	return dc_power_supply_;
}
current_sensor_b & hardware_link::sence_block(void){
	return current_sensor_;
}
void hardware_link::reconfig(void){
	dc_power_supply_.reconfig();
}
::mexo::prioritet_subsystem & hardware_link::subsystem(void){
	return hardware_subsystem;
}

//3.5. сам моторчик +1мкс - инфраструкутура и быстрый фильтр  +1мкс - напряженческий, +2,5мкс -токовый
joint joint_(hardware_link_, RT("joint"), action.joint, config.joint, present.joint);

//4. дополнителные сервисы для mexo //не добавляет времени к обрабготке прерывания
::mexo::machine::slot::simple frontend_pool_ (
	::mexo::machine::slot::kind::frontend
	,	[]{
static volatile robo::time_us_t g_time_us_t = 0;
		robo::freemaster::poll();
		g_time_us_t = robo::system::env::realtime_us();
	}
);

//+0.4мкс
::mexo::machine::slot::simple backend_(
	::mexo::machine::slot::kind::backend
	,	[]{
		robo::freemaster::recorder();
	}
);	

	//+0.3мкс от hardware_subsystem
//делегат в слот "start" - сработает при старте 
::mexo::machine::slot::simple start(
	::mexo::machine::slot::kind::start
, [] {
	hardware_subsystem.start();									//активируем подсистему аппаратуры
});

//делегат в слот "begin" - сработает при инициализации - собираем все вместе
::mexo::machine::slot::simple begin(
	::mexo::machine::slot::kind::begin
	,	[] {	
		::mexo::tp::set_verb(::mexo::tp_verb::loop);
	}
);
	
#if 0
typedef mexo::filter_b<types >current_filter_b;
typedef mexo::quazzy_adapt_b<types > current_regulator_b;


struct all_config{
	current_filter_b::config_s current_filter;
	current_regulator_b::config_s current_regulator;
	current_regulator_b::config_s current_regulator2;
} real_config =
{
	{//mexo::ps::current_filter_b::config_s
		{//fb
			{0} //ref
			,0.f //standalone input
			,0.f //стартовое значение
		}
		, types::parameter_t(0.99)	//gain y = y*gain+(1-gain)*x;
		, 0 //shift
	}
	,{ //mexo::ps::current_regulator_b::config_s
		{ //control block
			{0} //ref
			,{ //standalone
				{ // range
					-types::signal_t(12) //мин
					, types::signal_t(12) //макс
				}
				,0.f //input
				,mexo::iblock::satstate::none //master_satstate
			}
		}
		,{ //qa
			types::parameter_t(0.1) //пропорцилнальный
			, types::parameter_t(0.01) //интегральный
			, types::parameter_t(0)	//диффиренциальный
			, 0
			, 0
		}
		, types::signal_t(0) //standalone actual
		, types::signal_t(0) //standalone actual_diff
	}
	,{ //mexo::ps::current_regulator_b::config_s
		{ //control block
			{0} //ref
			,{ //standalone
				{ // range
					-types::signal_t(12) //мин
					, types::signal_t(12) //макс
				}
				,0.f //input
				,mexo::iblock::satstate::none //master_satstate
			}
		}
		,{ //qa
			types::parameter_t(0.1) //пропорцилнальный
			, types::parameter_t(0.01) //интегральный
			, types::parameter_t(0)	//диффиренциальный
			, 0
			, 0
		}
		, types::signal_t(0) //standalone actual
		, types::signal_t(0) //standalone actual_diff
	}
};

struct all_present{
	dc_power_supply_b::present_s dc_power_supply;
	current_sensor_b::present_s current_sensor;
	current_filter_b::present_s current_filter;
	current_regulator_b::present_s current_regulator;
	current_regulator_b::present_s current_regulator2;
} real_present ={};

//1. конфигурируем приоритетную подсистему,саязывающую все блоки с аппаратурой
//подсистема - работает в контексте prioritet ( будет срабатывать по прерыванию от АЦП)
::mexo::prioritet_subsystem hardware_subsystem(RT("hardware"), false); 

//1.1. подключаем к системе силовой преобразователь
dc_power_supply_b dc_power_supply_(hardware_subsystem, RT("dc"), dc_power_supply_config, real_present.dc_power_supply ); //2us

//1.2.  подключаем к подсистеме датчик тока
current_sensor_b current_sensor_(hardware_subsystem,RT("current sensor"),current_sensor_config, real_present.current_sensor);

//2. подсистема управления током
::mexo::backend_subsystem current_control_subsystem(RT("current_control"), false); 

//2.1 подключаем к подсистеме управления током фильтр тока
current_filter_b current_filter_(current_control_subsystem, RT("current filter"), real_config.current_filter,real_present.current_filter); 

//2.2 подключаем к подсистеме регулятор тока
current_regulator_b current_regulator_(current_control_subsystem, RT("current_regulator"), real_config.current_regulator ,real_present.current_regulator ); 
//2.2 подключаем к подсистеме регулятор тока
current_regulator_b current_regulator2_(current_control_subsystem, RT("current_regulator2"), real_config.current_regulator2 ,real_present.current_regulator2 ); 

//делегат в слот "begin" - сработает при инициализации - собираем все вместе
::mexo::machine::slot::simple begin(
	::mexo::machine::slot::kind::begin
	,	[] {	
		//стыкуем фильтр тока с датчиком тока
		current_filter_.input.link_to(&current_sensor_.output);
		
		//стыкуем фильтр тока и регулятор тока
		current_regulator_.actual.link_to(&current_filter_.output);
		current_regulator2_.actual.link_to(&current_filter_.output);
		//стыкуем регулятор тока  и силовой преобразователь
		dc_power_supply_.link_to(current_regulator_);
		
		::mexo::tp::set_verb(::mexo::tp_verb::loop);
	}
);

	
//делегат в слот "start" - сработает при старте
::mexo::machine::slot::simple start(
	::mexo::machine::slot::kind::start
, [] {
	real_config.current_regulator.cb.standalone.input = 1.f;	//заданный ток
	real_config.current_regulator2.cb.standalone.input = -1.f;	//заданный ток
	hardware_subsystem.start();									//активируем подсистему аппаратуры
	current_control_subsystem.start();					//активируем подсистему управления током
	dc_power_supply_.on(); 						
});

//делегат в слот "0" - сработает каждый 16-ый такт
::mexo::machine::slot::simple restart(
	0
, [] {
	//гоняем ток туда обратно
	if(current_regulator_.output.value > 11.9f){
		real_config.current_regulator.cb.standalone.input = -1.f;
	} else if ( current_regulator_.output.value < - 11.9f){
		real_config.current_regulator.cb.standalone.input = 1.f;
	} //2.5us
});


::mexo::machine::slot::simple frontend_(
	::mexo::machine::slot::kind::frontend
	,	[]{
		robo::freemaster::poll();
	}
);
	
::mexo::machine::slot::simple backend_(
	::mexo::machine::slot::kind::backend
	,	[]{
		static  robo::system::mem::stat st;
		st =robo::system::get_mem_statistic();
		volatile size_t tmp = st.total.count;
		robo::freemaster::recorder();
	}
);	
	
/*
::mexo::frontend_subsystem control_subsystem(RT("control"), true); 

//подключаем к системе регулятор напряжения (рампа)
mexo::ps::voltage::config_s dcv_config = {
	{0} //block
	,{ //
		-::mexo::signal_t(12) //мин, В
		, ::mexo::signal_t(12)//макс, В
	}
	, ::mexo::signal_t(0.0001)
	, ::mexo::signal_t(0)
};
::mexo::backend_subsystem dirrect_pwm_subsystem(RT("dirrect_pwm"), false); 
mexo::ps::voltage dcv_(dirrect_pwm_subsystem, RT("dcv"), dcv_config); 
*/
	
	
#endif


