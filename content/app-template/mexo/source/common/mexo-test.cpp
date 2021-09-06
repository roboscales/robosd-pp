#include "mexo-test.hpp"


//300 ns
//подсистема - работает в контексте prioritet ( будет срабатывать по прерыванию от АЦП)
::mexo::prioritet_subsystem hardware_subsystem(RT("hardware"), true); //500 ns

//подключаем к системе регулятор напряжения (рампа)
voltage::config_s dcv_config = {
	{0} //block
	, ::mexo::signal_t(0.0001)
	,{ //
		-::mexo::signal_t(12)
		, ::mexo::signal_t(12)
	}
	, ::mexo::signal_t(0)
};
voltage dcv_(hardware_subsystem, RT("dcv"), dcv_config); //1,5 us

//подключаем к системе источник напряжения
dc dc_(hardware_subsystem, RT("dc"), dc_config); //2us

//делегат в слот "begin" - сработает при инициализации
::mexo::machine::slot::simple begin(
	::mexo::machine::slot::kind::begin
	,	[] {	
//стыкуем источник напряжения к регулятору напряжения
	dc_.link_to(dcv_);
		::mexo::tp::set_verb(::mexo::tp_verb::priority);
	}
);

	
//делегат в слот "start" - сработает при старте
::mexo::machine::slot::simple start(
	::mexo::machine::slot::kind::start
, [] {
	dcv_.standalone_deseired = 12.f;
	dc_.on(); //2.5 us
});

//делегат в слот "0" - сработает каждый 16-ый такт
::mexo::machine::slot::simple restart(
	0
, [] {
	//гоняем ШИМ туда обратно
	if(dcv_.actual.value > 11.9f){
		dcv_.standalone_deseired = 0.f;//-12.f;
	} else if ( dcv_.actual.value < - 11.9f){
		dcv_.standalone_deseired = 12.f;
		
	} //2.5us
	ROBO_APP_ASSERT(0)
});


	
