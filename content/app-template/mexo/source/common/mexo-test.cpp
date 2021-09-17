#include "mexo-test.hpp"
#include "freemaster/robosd_fm.hpp"

typedef mexo::filter_b<types >current_filter_b;
typedef mexo::quazzy_adapt_b<types > current_regulator_b;

struct all_config{
	current_filter_b::config_s current_filter;
	current_regulator_b::config_s current_regulator;
} config =
/* {
	{//mexo::ps::current_filter_b::config_s
		{//fb
			{0} //ref
			,0 //standalone input
			,0 //стартовое значение
		}
		, 10000
	}
	,{ //mexo::ps::current_regulator_b::config_s
		{ //control block
			{0} //ref
			,{ //standalone
				{ // range
					-32767 //мин
					, 32767 //макс
				}
				,0 //input
				,mexo::iblock::satstate::none //master_satstate
			}
		}
		, 1 //пропорцилнальный
		, 1 //интегральный
		, 0	//диффиренциальный

		,0 //standalone actual
		, 0 //standalone actual_diff
	}
	,{ //mexo::ps::current_regulator_b::config_s
		{ //control block
			{0} //ref
			,{ //standalone
				{ // range
					-32767 //мин
					, 32767 //макс
				}
				,0 //input
				,mexo::iblock::satstate::none //master_satstate
			}
		}
		, 1 //пропорцилнальный
		, 1 //интегральный
		, 0	//диффиренциальный

		,0 //standalone actual
		, 0 //standalone actual_diff
	}
	,{ //mexo::ps::current_regulator_b::config_s
		{ //control block
			{0} //ref
			,{ //standalone
				{ // range
					-32767 //мин
					, 32767 //макс
				}
				,0 //input
				,mexo::iblock::satstate::none //master_satstate
			}
		}
		, 1 //пропорцилнальный
		, 1 //интегральный
		, 0	//диффиренциальный

		,0 //standalone actual
		, 0 //standalone actual_diff
	}
	,{ //mexo::ps::current_regulator_b::config_s
		{ //control block
			{0} //ref
			,{ //standalone
				{ // range
					-32767 //мин
					, 32767 //макс
				}
				,0 //input
				,mexo::iblock::satstate::none //master_satstate
			}
		}
		, 1 //пропорцилнальный
		, 1 //интегральный
		, 0	//диффиренциальный

		,0 //standalone actual
		, 0 //standalone actual_diff
	}
};*/

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
};

struct {
	dc_power_supply_b::present_s dc_power_supply;
	current_sensor_b::present_s current_sensor;
	current_filter_b::present_s current_filter;
	current_regulator_b::present_s current_regulator;
} present;

//1. конфигурируем приоритетную подсистему,саязывающую все блоки с аппаратурой
//подсистема - работает в контексте prioritet ( будет срабатывать по прерыванию от АЦП)
::mexo::prioritet_subsystem hardware_subsystem(RT("hardware"), false); 

//1.1. подключаем к системе силовой преобразователь
dc_power_supply_b dc_power_supply_(hardware_subsystem, RT("dc"), dc_power_supply_config, present.dc_power_supply ); //2us

//1.2.  подключаем к подсистеме датчик тока
current_sensor_b current_sensor_(hardware_subsystem,RT("current sensor"),current_sensor_config, present.current_sensor);

//2. подсистема управления током
::mexo::backend_subsystem current_control_subsystem(RT("current_control"), false); 

//2.1 подключаем к подсистеме управления током фильтр тока
current_filter_b current_filter_(current_control_subsystem, RT("current filter"), config.current_filter,present.current_filter); 

//2.2 подключаем к подсистеме регулятор тока
current_regulator_b current_regulator_(current_control_subsystem, RT("current_regulator"), config.current_regulator ,present.current_regulator ); 

//делегат в слот "begin" - сработает при инициализации - собираем все вместе
::mexo::machine::slot::simple begin(
	::mexo::machine::slot::kind::begin
	,	[] {	
		//стыкуем фильтр тока с датчиком тока
		current_filter_.input.link_to(&current_sensor_.output);
		
		//стыкуем фильтр тока и регулятор тока
		current_regulator_.actual.link_to(&current_filter_.output);
		//стыкуем регулятор тока  и силовой преобразователь
		dc_power_supply_.link_to(current_regulator_);
		
		::mexo::tp::set_verb(::mexo::tp_verb::priority);
	}
);

	
//делегат в слот "start" - сработает при старте
::mexo::machine::slot::simple start(
	::mexo::machine::slot::kind::start
, [] {
	config.current_regulator.cb.standalone.input = 1.f;	//заданный ток
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
		config.current_regulator.cb.standalone.input = -1.f;
	} else if ( current_regulator_.output.value < - 11.9f){
		config.current_regulator.cb.standalone.input = 1.f;
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
	
	
static volatile robo::time_us_t g_time_us_t = 0;
	
::mexo::machine::slot::simple frontend_pool_ (
	::mexo::machine::slot::kind::frontend
	,	[]{
		robo::freemaster::poll();
		g_time_us_t = robo::system::env::realtime_us();
	}

);