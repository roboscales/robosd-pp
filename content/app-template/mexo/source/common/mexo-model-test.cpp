#include "mexo-test.hpp"
#include "freemaster/robosd_fm.hpp"
#if 0 
namespace model{

	typedef  ::mexo::fixed_point<::mexo::int15> types;
	
	types::discret_t duty;
	float model = 0.f;
	class pwm  {
	public:
		
	protected:
		
		void boot_complete(types::discret_t _duty){
			duty = _duty;
		}
		static void shutdown_begin(void){
		}
		void do_run(types::discret_t _duty){
			duty = _duty;
		}

		static void boot_begin(void){};
		static bool do_boot(void) { return true; }

		static bool do_shutdown(void) { return true; }
		static void shutdown_complete(void) {  }
	};
	typedef ::mexo::ps::pwm_b<types, pwm>  dc_power_supply_b;

	struct current_adc{
		uint32_t sence[2];
		void query(void){
			model = 0.99 * model + 0.00001* duty;
			sence[0] = (12000.f*model);
			sence[1] = sence[0];
		};
		current_adc(void){
			sence[0] = 0;
			sence[1] = 0;
		}		
	};
	typedef ::mexo::adc_diff_b<current_adc, types>  current_sensor_b;

	typedef mexo::filter_b<types >current_filter_b;
	typedef mexo::quazzy_adapt_b<types > current_regulator_b;


	struct all_config{
		dc_power_supply_b::config_s dc_power_supply_config;
		current_sensor_b::config_s current_sensor_config;
		current_filter_b::config_s current_filter;
		current_regulator_b::config_s current_regulator;
		current_regulator_b::config_s current_regulator2;
		current_regulator_b::config_s current_regulator3;
	} model_config =

	{
		{
			{//controller block;
				{0} //ref
				,{//stabdalone
					{ //converter
					-1000
					, 1000
					}
					, 0 //defalt
				} 
			}
			, 31  //gain
			,10
		}
		,{
			{
				{0} //sence_block
			}
			,{0,1} //index
			,{-1, 1} //scale
			,10	// init_count_shift -1024 точек
		}
		, {//mexo::ps::current_filter_b::config_s
			{//fb
				{0} //ref
				,0 //standalone input
				,0 //стартовое значение
			}
			, (1<<14)-1
			, 15 //shift
		}
		,{ //mexo::ps::current_regulator_b::config_s
			{ //control block
				{0} //ref
				,{ //standalone
					{ // range
						-1000 //мин
						, 1000 //макс
					}
					,0 //input
					,mexo::iblock::satstate::none //master_satstate
				}
			}
			,{ //qa
				1 //пропорцилнальный
				, 1 //интегральный
				, 0	//диффиренциальный
				, 3
				, 10
			}
			, 0 //standalone actual
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
			,{ //qa
				1 //пропорцилнальный
				, 1 //интегральный
				, 0	//диффиренциальный
				, 3
				, 10
			}
			, 0 //standalone actual
			, 0 //standalone actual_diff
		}
		,{ //mexo::ps::current_regulator_b::config_s
			{ //control block
				{0} //ref
				,{ //standalone
					{ // range
						-1000 //мин
						, 1000 //макс
					}
					,0 //input
					,mexo::iblock::satstate::none //master_satstate
				}
			}
			,{ //qa
				1 //пропорцилнальный
				, 1 //интегральный
				, 0	//диффиренциальный
				, 10
				, 10
			}
			, 0 //standalone actual
			, 0 //standalone actual_diff
		}
	};

	struct {
		dc_power_supply_b::present_s dc_power_supply;
		current_sensor_b::present_s current_sensor;
		current_filter_b::present_s current_filter;
		current_regulator_b::present_s current_regulator;
		current_regulator_b::present_s current_regulator2;
		current_regulator_b::present_s current_regulator3;
		current_regulator_b::present_s current_regulator4;
		current_regulator_b::present_s current_regulator5;
		current_regulator_b::present_s current_regulator6;
	} model_present;

	//1. конфигурируем приоритетную подсистему,саязывающую все блоки с аппаратурой
	//подсистема - работает в контексте prioritet ( будет срабатывать по прерыванию от АЦП)
	::mexo::prioritet_subsystem hardware_subsystem(RT("model-hardware"), false); 

	//1.1. подключаем к системе силовой преобразователь
	dc_power_supply_b dc_power_supply_(hardware_subsystem, RT("dc"), model_config.dc_power_supply_config, model_present.dc_power_supply ); //2us

	//1.2.  подключаем к подсистеме датчик тока
	current_sensor_b current_sensor_(hardware_subsystem,RT("model-current sensor"),model_config.current_sensor_config, model_present.current_sensor);

	//2. подсистема управления током
	::mexo::backend_subsystem current_control_subsystem(RT("model-current_control"), false); 

	//2.1 подключаем к подсистеме управления током фильтр тока
	current_filter_b current_filter_(current_control_subsystem, RT("model-current filter"), model_config.current_filter,model_present.current_filter); 

	//2.2 подключаем к подсистеме регулятор тока
	current_regulator_b current_regulator_(current_control_subsystem, RT("model-current_regulator"), model_config.current_regulator ,model_present.current_regulator ); 
	current_regulator_b current_regulator2_(current_control_subsystem, RT("model-current_regulator2"), model_config.current_regulator2 ,model_present.current_regulator2 ); 
	current_regulator_b current_regulator3_(current_control_subsystem, RT("model-current_regulator3"), model_config.current_regulator3 ,model_present.current_regulator3 ); 
	current_regulator_b current_regulator4_(current_control_subsystem, RT("model-current_regulator4"), model_config.current_regulator ,model_present.current_regulator4 ); 
	current_regulator_b current_regulator5_(current_control_subsystem, RT("model-current_regulator5"), model_config.current_regulator2 ,model_present.current_regulator5 ); 
	current_regulator_b current_regulator6_(current_control_subsystem, RT("model-current_regulator6"), model_config.current_regulator3 ,model_present.current_regulator6 ); 

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
			dc_power_supply_.link_to(current_regulator2_);
			
			::mexo::tp::set_verb(::mexo::tp_verb::loop);
		}
	);

	
	//делегат в слот "start" - сработает при старте
	::mexo::machine::slot::simple start(
		::mexo::machine::slot::kind::start
	, [] {
		model_config.current_regulator.cb.standalone.input = 1000;	//заданный ток
		model_config.current_regulator2.cb.standalone.input = 100;	//заданный ток
		model_config.current_regulator3.cb.standalone.input = -100;	//заданный ток
		//hardware_subsystem.start();									//активируем подсистему аппаратуры
		//current_control_subsystem.start();					//активируем подсистему управления током
		dc_power_supply_.on(); 						
	});

	//делегат в слот "0" - сработает каждый 16-ый такт
	::mexo::machine::slot::simple restart(
		0
	, [] {
		//гоняем ток туда обратно
		if(current_regulator_.output.value > 900){
			model_config.current_regulator.cb.standalone.input = -1000;
		} else if ( current_regulator_.output.value < - 900){
			model_config.current_regulator.cb.standalone.input = 1000;
		} //2.5us
	});
}

#include "mexo/adc.hpp"
#endif

#if 0
class fake_dc_periphery {
public:
	types::discret_t duty_;
protected:
	static void boot_begin(void) {}
	static bool do_boot(void) { return true; }
	void boot_complete(types::discret_t _duty) { duty_ = _duty; }

	static void shutdown_begin(void) {}
	static bool do_shutdown(void) { return true; }
	static void shutdown_complete(void) {}

	void do_run(types::discret_t _duty) {
		duty_ = _duty;
	}
};

typedef ::mexo::ps::pwm_b<types, fake_dc_periphery> fake_dc;
		
class fake_adc {
public:
	typedef uint16_t  native_t;
	typedef uint32_t  acc_t;
	native_t sence[3];
	void query(void) {};
};
		
		
		
	class prioritet_subsystem {
		public:

			::mexo::prioritet_subsystem hardware_subsystem;
			fake_dc::config_s dc_config;
			fake_dc::present_s dc_present;
			fake_dc dc;

			typedef ::mexo::adc_diff_b<fake_adc, types	> adcd;
			adcd::config_s adc_conf;
			adcd::present_s adc_present;
			adcd adc;

		public:
			::mexo::iblock::output_t< types::signal_t >& current(void) { return adc.output; }
			::mexo::iblock::output_t< types::signal_t >& current_diff(void) { return adc.output; }
			void reconfig(void) {
				hardware_subsystem.reconfig();
			}
			fake_dc& pwm() { return dc; }
			prioritet_subsystem()
				: hardware_subsystem(RT("hardware"), true)
				, adc(hardware_subsystem, RT("adc"), adc_conf, adc_present)
				, dc(hardware_subsystem, RT("dc"), dc_config, dc_present){
				adc_conf = { {22} };
				adc_present = { {23} };

				dc_config = {
					{
						{26} //ref
						,{
							{ //range
								-4095
								, 4095
							}
							, 0
						}
						, 0
					}
					, 128
					,10
				};
				dc_present = {
					{{27}}
				};
			}
		} prioritet_subsystem_;


		#define PS_TEMPLATE_NAME psdev
		#define  psdev_VOLTAGE_REGULATOR_ENABLED 1
		#define  psdev_CURRENT_REGULATOR_ENABLED 1
		#define  psdev_CURRENT_MEASSURY_ENABLED 1
		#define  psdev_CURRENT_DIFF_ENABLED 1
		#define  psdev_CURRENT_FILTER_ENABLED 1
		#define  psdev_CURRENT_DIFF_FILTER_ENABLED 1
		#define psdev_VOLTAGE_MIN_LIM -32767
		#define psdev_VOLTAGE_MAX_LIM 32767
		#define psdev_VOLTAGE_RAMP_GAIN 1
		#define psdev_CURRENT_FILTER_GAIN 200
		#define psdev_CURRENT_FILTER_SHIFT_GAIN 8
		#define psdev_CURRENT_FILTER_SHIFT_PRESC 0
		#define psdev_CURRENT_FILTER_SHIFT_VALUE 0

		#define psdev_CURRENT_DIFF_FILTER_GAIN 200
		#define psdev_CURRENT_DIFF_FILTER_SHIFT_GAIN 8
		#define psdev_CURRENT_DIFF_FILTER_SHIFT_PRESC 0
		#define psdev_CURRENT_DIFF_FILTER_SHIFT_VALUE 0

		#define psdev_CURRENT_PROP_GAIN 10
		#define psdev_CURRENT_MODEL_GAIN 10
		#define psdev_CURRENT_DIFF_GAIN 10
		#define psdev_CURRENT_CONTROL_SHIFT 7
		#define psdev_CURRENT_MODEL_SHIFT 10
		#include "mexo/ps.inc.hpp"
		typedef psdev<types, prioritet_subsystem> psdev_t;


#define PS_TEMPLATE_NAME psdev2
		#define  psdev2_VOLTAGE_REGULATOR_ENABLED 1
		#define  psdev2_CURRENT_REGULATOR_ENABLED 1
		#define  psdev2_CURRENT_MEASSURY_ENABLED 0
		#define psdev2_VOLTAGE_MIN_LIM -32767
		#define psdev2_VOLTAGE_MAX_LIM 32767
		#define psdev2_VOLTAGE_RAMP_GAIN 1
		#include "mexo/ps.inc.hpp"
		typedef psdev2<types, prioritet_subsystem> psdev2_t;

		#define ACTUATOR_TEMPLATE_NAME a1
		#define  a1_ps_POWER_SUPPLY_VOLTAGE_REGULATOR_ENABLED 1
		#define  a1_ps_POWER_SUPPLY_CURRENT_REGULATOR_ENABLED 1
		#define  a1_ps_POWER_SUPPLY_CURRENT_MEASSURY_ENABLED 1
		#define  a1_ps_POWER_SUPPLY_CURRENT_DIFF_ENABLED 1
		#define  a1_ps_POWER_SUPPLY_CURRENT_FILTER_ENABLED 1
		#define  a1_ps_POWER_SUPPLY_CURRENT_DIFF_FILTER_ENABLED 1
		#define a1_ps_VOLTAGE_MIN_LIM -32767
		#define a1_ps_VOLTAGE_MAX_LIM 32767
		#define a1_ps_VOLTAGE_RAMP_GAIN 1
		#define a1_ps_CURRENT_FILTER_GAIN 200
		#define a1_ps_CURRENT_FILTER_SHIFT_GAIN 8
		#define a1_ps_CURRENT_FILTER_SHIFT_PRESC 0
		#define a1_ps_CURRENT_FILTER_SHIFT_VALUE 0

		#define a1_ps_CURRENT_DIFF_FILTER_GAIN 200
		#define a1_ps_CURRENT_DIFF_FILTER_SHIFT_GAIN 8
		#define a1_ps_CURRENT_DIFF_FILTER_SHIFT_PRESC 0
		#define a1_ps_CURRENT_DIFF_FILTER_SHIFT_VALUE 0

		#define a1_ps_CURRENT_PROP_GAIN 10
		#define a1_ps_CURRENT_MODEL_GAIN 10
		#define a1_ps_CURRENT_DIFF_GAIN 10
		#define a1_ps_CURRENT_CONTROL_SHIFT 7
		#define a1_ps_CURRENT_MODEL_SHIFT 10
		#include "mexo/actuator.inc.hpp"
		typedef a1<types, prioritet_subsystem> a1_t;

namespace test1 {
	psdev_t::action_s action;
	psdev_t::present_s present;
	psdev_t::config_s config;
	psdev_t psdev_(prioritet_subsystem_, RT("ps"), action, config, present);
}

namespace test2 {
	psdev2_t::action_s action;
	psdev2_t::present_s present;
	psdev2_t::config_s config;
	psdev2_t psdev_(prioritet_subsystem_, RT("ps2"), action, config, present);
}
namespace test3 {
	a1_t::action_s action;
	a1_t::present_s present;
	a1_t::config_s config;
	a1_t a1_(prioritet_subsystem_, RT("ps3"), action, config, present);
}

#endif