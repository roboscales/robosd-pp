#include "mexo-test.hpp"


//300 ns
//подсистема - работает в контексте prioritet ( будет срабатывать по прерыванию от АЦП)
::mexo::prioritet_subsystem hardware_subsystem(RT("hardware"), true); //500 ns
::mexo::frontend_subsystem control_subsystem(RT("control"), true); 

::mexo::backend_subsystem dirrect_pwm_subsystem(RT("dirrect_pwm"), false); 


//подключаем к системе регулятор напряжения (рампа)
mexo::ps::voltage::config_s dcv_config = {
	{0} //block
	,{ //
		-::mexo::signal_t(12)
		, ::mexo::signal_t(12)
	}
	, ::mexo::signal_t(0.0001)
	, ::mexo::signal_t(0)
};


mexo::ps::current::config_s dcc_config = {
	{0} //block
	,{ //
		-::mexo::signal_t(12)
		, ::mexo::signal_t(12)
	}
	, ::mexo::signal_t(0.1)
	, ::mexo::signal_t(0.001)
	, ::mexo::signal_t(0)
	, ::mexo::signal_t(0)
};

mexo::ps::filter::config_s current_filter_config = {
	{0} //block
	, ::mexo::signal_t(0.99)
	, ::mexo::signal_t(0)
};



mexo::ps::voltage dcv_(dirrect_pwm_subsystem, RT("dcv"), dcv_config); //1,5 us


//подключаем к системе источник напряжения
dc dc_(hardware_subsystem, RT("dc"), dc_config); //2us

//подключаем к системе датчик тока
current_sensor_t current_sensor_(hardware_subsystem,RT("current sensor"),current_sensor_config);

//конфигурируем cbcntve eghfdktybz njrf
::mexo::backend_subsystem current_control_subsystem(RT("current_control"), false); 

//подключаем к системе управления током фильтр тока
mexo::ps::filter current_filter_(current_control_subsystem, RT("current filter"), current_filter_config); //

//подключаем к системе регулятор тока
mexo::ps::current current_regulator_(current_control_subsystem, RT("current_regulator"), dcc_config); //

//делегат в слот "begin" - сработает при инициализации
::mexo::machine::slot::simple begin(
	::mexo::machine::slot::kind::begin
	,	[] {	
		//стыкуем фильтр тока с датчиком тока
		current_filter_.input.link_to(&current_sensor_.output);
		
		//стыкуем фильтр тока ирегулятор тока
		current_regulator_.actual.link_to(&current_filter_.output);

		//стыкуем регулятор тока  и силовой преобразователь
		dc_.link_to(current_regulator_);
		
		::mexo::tp::set_verb(::mexo::tp_verb::priority);
	}
);

	
//делегат в слот "start" - сработает при старте
::mexo::machine::slot::simple start(
	::mexo::machine::slot::kind::start
, [] {
	current_regulator_.standalone_input = 1.f;
	current_control_subsystem.start();
	dc_.on(); //2.5 us
});

//делегат в слот "0" - сработает каждый 16-ый такт
::mexo::machine::slot::simple restart(
	0
, [] {
/*
	//гоняем ШИМ туда обратно
	if(dcv_.output.value > 11.9f){
		dcv_.standalone_input = -12.f;
	} else if ( dcv_.output.value < - 11.9f){
		dcv_.standalone_input = 12.f;
		
	} //2.5us
*/
	//гоняем ток туда обратно
	if(current_regulator_.output.value > 11.9f){
		current_regulator_.standalone_input = -1.f;
	} else if ( current_regulator_.output.value < - 11.9f){
		current_regulator_.standalone_input = 1.f;
		
	} //2.5us
});



