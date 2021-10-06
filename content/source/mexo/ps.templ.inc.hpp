#ifndef PS_TEMPLATE_NAME
#define PS_TEMPLATE_NAME ps
#define POWER_SUPPLY_PREFIX(name)  _POWER_SUPPLY_PREFIX(name,PS_TEMPLATE_NAME)
#define _POWER_SUPPLY_PREFIX(name,prfx)  __POWER_SUPPLY_PREFIX(name,prfx)
#define __POWER_SUPPLY_PREFIX(name,prfx) prfx##_##name
#define ps_CURRENT_MEASSURY_ENABLED 1
#define ps_CURRENT_DIFF_ENABLED 1
#define ps_VOLTAGE_REGULATOR_ENABLED 1
#define ps_CURRENT_REGULATOR_ENABLED 1
#define ps_CURRENT_LIMMITER_ENABLED 1
#define ps_CURRENT_FILTER_ENABLED 1
#define ps_CURRENT_DIFF_FILTER_ENABLED 1
#endif

#define POWER_SUPPLY_CURRENT_MEASSURY_ENABLED POWER_SUPPLY_PREFIX(CURRENT_MEASSURY_ENABLED)
#define POWER_SUPPLY_VOLTAGE_REGULATOR_ENABLED POWER_SUPPLY_PREFIX(VOLTAGE_REGULATOR_ENABLED)
#if POWER_SUPPLY_CURRENT_MEASSURY_ENABLED==1 
#define POWER_SUPPLY_CURRENT_REGULATOR_ENABLED POWER_SUPPLY_PREFIX(CURRENT_REGULATOR_ENABLED)
#define POWER_SUPPLY_CURRENT_LIMMITER_ENABLED POWER_SUPPLY_PREFIX(CURRENT_LIMMITER_ENABLED)
#define POWER_SUPPLY_CURRENT_DIFF_ENABLED POWER_SUPPLY_PREFIX(CURRENT_DIFF_ENABLED)
#define POWER_SUPPLY_CURRENT_FILTER_ENABLED POWER_SUPPLY_PREFIX(CURRENT_FILTER_ENABLED)
#define POWER_SUPPLY_CURRENT_FAST_FILTER_ENABLED POWER_SUPPLY_PREFIX(CURRENT_FAST_FILTER_ENABLED)
#define POWER_SUPPLY_CURRENT_DIFF_FILTER_ENABLED POWER_SUPPLY_PREFIX(CURRENT_DIFF_FILTER_ENABLED)
#endif

#include "mexo/mexo.hpp"
#include "mexo/math.hpp"
#include "mexo/ps.hpp"
#include "mexo/regulator.hpp"
template <typename types, typename hardwaresys_t>  class PS_TEMPLATE_NAME : public ::mexo::ps::dev {
public:
	
	#if POWER_SUPPLY_VOLTAGE_REGULATOR_ENABLED == 1
	typedef ::mexo::controller_task_t <
		::mexo::ramp<types>
		, ::mexo::backend_subsystem
	> voltage_regulator_b;
	voltage_regulator_b voltage_regulator;
	#endif

	#if POWER_SUPPLY_CURRENT_REGULATOR_ENABLED == 1
	typedef ::mexo::controller_task_t <
		::mexo::quazzy_adapt<types>
		, ::mexo::backend_subsystem
		, const typename types::signal_t&
		, const typename types::signal_t&
	> current_regulaor_b;
	current_regulaor_b current_regulator;
	#endif

	#if POWER_SUPPLY_CURRENT_LIMMITER_ENABLED == 1
	typedef ::mexo::controller_task_t <
		::mexo::limmiter<types>
		, ::mexo::prioritet_subsystem
		, const typename types::signal_t&
		, const typename types::signal_t&
		, const ::mexo::range_s<typename types::signal_t>&
	> current_limmiter_b;
	current_limmiter_b current_limmiter;
	#endif

	#if POWER_SUPPLY_CURRENT_FILTER_ENABLED==1 || POWER_SUPPLY_CURRENT_DIFF_ENABLED==1
	typedef ::mexo::function_task_t <
			::mexo::filter<types>
			, ::mexo::prioritet_subsystem
		>  filter_b;
	#endif 

	#if POWER_SUPPLY_CURRENT_FAST_FILTER_ENABLED==1
	typedef ::mexo::function_block_t <
		::mexo::fast_filter<types>
		, ::mexo::prioritet_subsystem
	>fast_filter_b;
	#endif 

	#if POWER_SUPPLY_CURRENT_FILTER_ENABLED == 1
	filter_b current_filter;
	#endif 

	#if POWER_SUPPLY_CURRENT_FAST_FILTER_ENABLED == 1
	fast_filter_b current_filter;
	#endif 

	#if POWER_SUPPLY_CURRENT_DIFF_FILTER_ENABLED == 1
	filter_b current_diff_filter;
	#endif 


	struct action_s {
		::mexo::ps::dev::action_s dev;
		bool invers;
		typename types::signal_t voltage;
		typename types::signal_t current;
	};

	struct config_s {
		int tag;
		#if POWER_SUPPLY_VOLTAGE_REGULATOR_ENABLED == 1
		typename voltage_regulator_b::config_s voltage_regulator;
		#endif
		#if POWER_SUPPLY_CURRENT_FILTER_ENABLED==1
		typename filter_b::config_s current_filter;
		#endif
		#if POWER_SUPPLY_CURRENT_FAST_FILTER_ENABLED==1
		typename fast_filter_b::config_s current_filter;
		#endif
		#if POWER_SUPPLY_CURRENT_DIFF_FILTER_ENABLED == 1
		typename filter_b::config_s current_diff_filter;
		#endif
		#if POWER_SUPPLY_CURRENT_REGULATOR_ENABLED == 1
		typename current_regulaor_b::config_s current_regulaor;
		#endif
		#if POWER_SUPPLY_CURRENT_LIMMITER_ENABLED == 1
		typename current_limmiter_b::config_s current_limmiter;
		#endif
	};
	
	struct present_s {		
		::mexo::ps::dev::present_s dev;
		#if POWER_SUPPLY_VOLTAGE_REGULATOR_ENABLED == 1
		typename voltage_regulator_b::present_s voltage_regulator;
		#endif
		#if POWER_SUPPLY_CURRENT_FILTER_ENABLED==1
		typename filter_b::present_s current_filter;
		#endif
		#if POWER_SUPPLY_CURRENT_FAST_FILTER_ENABLED==1
		typename fast_filter_b::present_s current_filter;
		#endif
		#if POWER_SUPPLY_CURRENT_DIFF_FILTER_ENABLED == 1
		typename filter_b::present_s current_diff_filter;
		#endif
		#if POWER_SUPPLY_CURRENT_REGULATOR_ENABLED == 1
		typename current_regulaor_b::present_s current_regulaor;
		#endif
		#if POWER_SUPPLY_CURRENT_LIMMITER_ENABLED == 1
		typename current_limmiter_b::present_s current_limmiter;
		#endif

		#if POWER_SUPPLY_VOLTAGE_REGULATOR_ENABLED == 1 ||  POWER_SUPPLY_CURRENT_REGULATOR_ENABLED == 1 || POWER_SUPPLY_CURRENT_LIMMITER_ENABLED == 1
		typename types::discret_t pwm_duty;
		typename types::signal_t voltage_required;
		#endif		

		#if POWER_SUPPLY_VOLTAGE_REGULATOR_ENABLED == 1 ||  POWER_SUPPLY_CURRENT_LIMMITER_ENABLED == 1
		typename types::signal_t voltage_deseired;
		#endif
		
		#if  POWER_SUPPLY_CURRENT_REGULATOR_ENABLED == 1
		typename types::signal_t current_deseired;
		#endif		
		
		#if POWER_SUPPLY_VOLTAGE_REGULATOR_ENABLED == 1 \
		||  POWER_SUPPLY_CURRENT_REGULATOR_ENABLED == 1 \
		|| POWER_SUPPLY_CURRENT_LIMMITER_ENABLED == 1
		::mexo::range_s<typename types::signal_t> voltage_range_desired;
		#endif
		
		#if POWER_SUPPLY_CURRENT_REGULATOR_ENABLED == 1 \
		|| POWER_SUPPLY_CURRENT_LIMMITER_ENABLED == 1
		::mexo::range_s<typename types::signal_t> current_range_desired;
		#endif
		typename types::signal_t dummy;
	};

	
	#if POWER_SUPPLY_VOLTAGE_REGULATOR_ENABLED == 1
	class voltage_mode_t :public ::mexo::ps::dev::mode {
	protected:
		PS_TEMPLATE_NAME& owner(void) { return owner_cast<PS_TEMPLATE_NAME>(); }

		virtual void applay_action(void) {
			const action_s& action = owner().template action_cast<action_s>();
			present_s& present = owner().template present_cast<present_s>();

			if (action.invers) {
				present.voltage_deseired = -action.voltage;
			}
			else {
				present.voltage_deseired = action.voltage;
			}
		}

		virtual void do_start(void) {
			
			present_s& present = owner().template present_cast<present_s>();

			owner().hardwaresys.pwm_block().set_output(&present.pwm_duty);
			owner().hardwaresys.pwm_block().set_input(&present.voltage_required);
			owner().voltage_regulator.set_output(&present.voltage_required);
			owner().voltage_regulator.set_input(&present.voltage_deseired);

			owner().hardwaresys.reconfig();
			owner().voltage_regulator.reconfig();
			owner().voltage_regulator.start();
			owner().on();
		}

		virtual void do_stop(void) {
			owner().off();
			owner().voltage_regulator.stop();
			owner().hardwaresys.pwm_block().set_output(nullptr);
			owner().hardwaresys.pwm_block().set_input(nullptr);
			owner().voltage_regulator.set_output(nullptr);
			owner().voltage_regulator.set_input(nullptr);
		}

	public:
		voltage_mode_t(int _index, PS_TEMPLATE_NAME& _owner) :
			::mexo::ps::dev::mode(_index, RT("mod_v"), _owner) {}
	};
	#endif
	#if POWER_SUPPLY_CURRENT_MEASSURY_ENABLED == 1
	#if POWER_SUPPLY_CURRENT_FILTER_ENABLED==1 || POWER_SUPPLY_CURRENT_FAST_FILTER_ENABLED==1
		#if POWER_SUPPLY_CURRENT_DIFF_ENABLED==1
			#if POWER_SUPPLY_CURRENT_DIFF_FILTER_ENABLED==1
				#define POWER_SUPPLY_ACTUAL_SIGNALS _present.current_filter.fb.output,_present.current_diff_filter.fb.output
			#else
				#define POWER_SUPPLY_ACTUAL_SIGNALS _present.current_filter.fb.output,hardwaresys.sence_block().output_diff()
			#endif
		#else
			#define POWER_SUPPLY_ACTUAL_SIGNALS _present.current_filter.fb.output,_present.dummy
		#endif
	#else
		#if POWER_SUPPLY_CURRENT_DIFF_ENABLED==1
			#if POWER_SUPPLY_CURRENT_DIFF_FILTER_ENABLED==1
				#define POWER_SUPPLY_ACTUAL_SIGNALS hardwaresys.sence_block().output(),_present.current_diff_filter.fb.output
			#else
				#define POWER_SUPPLY_ACTUAL_SIGNALS hardwaresys.sence_block().output(),hardwaresys.sence_block().output_diff()
			#endif
		#else
			#define POWER_SUPPLY_ACTUAL_SIGNALS hardwaresys.sence_block().output(),_present.dummy
		#endif	
	#endif
	#endif
	
	#if POWER_SUPPLY_CURRENT_REGULATOR_ENABLED == 1
	class current_mode_t :public ::mexo::ps::dev::mode {
	protected:
		PS_TEMPLATE_NAME& owner(void) { return owner_cast<PS_TEMPLATE_NAME>(); }
		virtual void applay_action(void) {
			const action_s& action = owner().template action_cast<action_s>();
			present_s& present = owner().template present_cast<present_s>();
	
			if (action.invers) {
				present.current_deseired = -action.current;
			}
			else {
				present.current_deseired = action.current;
			}
			
			present.voltage_range_desired.hi = action.voltage;
			present.voltage_range_desired.low = -action.voltage;
			
		}
		virtual void do_start(void) {
			
			
			present_s& present = owner().template present_cast<present_s>();

			owner().hardwaresys.pwm_block().set_output(&present.pwm_duty);
			owner().hardwaresys.pwm_block().set_input(&present.voltage_required);
			owner().current_regulator.set_output(&present.voltage_required);
			owner().current_regulator.set_input(&present.current_deseired);

			owner().hardwaresys.reconfig();
			owner().current_regulator.reconfig();
			owner().current_regulator.start();
			owner().on();			

		}
		virtual void do_stop(void) {
			owner().off();
			owner().current_regulator.stop();
			owner().hardwaresys.pwm_block().set_output(nullptr);
			owner().hardwaresys.pwm_block().set_input(nullptr);
			owner().current_regulator.set_output(nullptr);
			owner().current_regulator.set_input(nullptr);
		}
	public:
		current_mode_t(int _index, PS_TEMPLATE_NAME& _owner) :
			::mexo::ps::dev::mode(_index, RT("mod_c"), _owner) {}
	};
	#endif
	#if POWER_SUPPLY_CURRENT_LIMMITER_ENABLED == 1
	class current_limmiter_mode_t :public ::mexo::ps::dev::mode {
	protected:
		PS_TEMPLATE_NAME& owner(void) { return owner_cast<PS_TEMPLATE_NAME>(); }
		virtual void applay_action(void) {
			const action_s& action = owner().template action_cast<action_s>();
			present_s& present = owner().template present_cast<present_s>();
	
			if (action.invers) {
				present.voltage_deseired = -action.voltage;
			}
			else {
				present.voltage_deseired = action.voltage;
			}
			
			present.current_range_desired.hi = action.current;
			present.current_range_desired.low = -action.current;
		}

		virtual void do_start(void) {
			present_s& present = owner().template present_cast<present_s>();

			owner().hardwaresys.pwm_block().set_output(&present.pwm_duty);
			owner().hardwaresys.pwm_block().set_input(&present.voltage_required);
			owner().current_limmiter.set_output(&present.voltage_required);
			owner().current_limmiter.set_input(&present.voltage_deseired);
			present.voltage_range_desired =  owner().hardwaresys.pwm_block().pwm_voltage_limits();
			owner().hardwaresys.reconfig();
			owner().current_limmiter.reconfig();
			owner().current_limmiter.start();
			owner().on();			
		}
		virtual void do_stop(void) {
			owner().off();
			owner().current_limmiter.stop();
			owner().hardwaresys.pwm_block().set_output(nullptr);
			owner().hardwaresys.pwm_block().set_input(nullptr);
			owner().current_limmiter.set_output(nullptr);
			owner().current_limmiter.set_input(nullptr);
		}
	public:
		current_limmiter_mode_t(int _index, PS_TEMPLATE_NAME& _owner) :
			::mexo::ps::dev::mode(_index, RT("mod_c_lm"), _owner) {}
	};
	#endif
	hardwaresys_t& hardwaresys;
	#if POWER_SUPPLY_VOLTAGE_REGULATOR_ENABLED == 1
	voltage_mode_t voltage_mode;
	#endif
	#if POWER_SUPPLY_CURRENT_REGULATOR_ENABLED == 1
	current_mode_t current_mode;
	#endif
	#if POWER_SUPPLY_CURRENT_LIMMITER_ENABLED == 1
	current_limmiter_mode_t current_limmiter_mode;
	#endif
	PS_TEMPLATE_NAME(hardwaresys_t& _hardwaresys, cstr _name, action_s& _action, config_s& _config, present_s& _present)
		: ::mexo::ps::dev(_name, _action.dev, _present.dev, _hardwaresys.pwm_block())
		, hardwaresys(_hardwaresys)

		#if POWER_SUPPLY_VOLTAGE_REGULATOR_ENABLED == 1
		, voltage_regulator(RT("v_re"),this,_config.voltage_regulator, _present.voltage_regulator, _hardwaresys.pwm_block().pwm_voltage_limits(),  _hardwaresys.pwm_block().actual_satstate() )
		, voltage_mode(1, *this)
		#endif
		
		#if POWER_SUPPLY_CURRENT_FILTER_ENABLED==1
		, current_filter(RT("cf"), &_hardwaresys.subsys(), _config.current_filter, _present.current_filter,_hardwaresys.sence_block().output())
		#endif
		#if POWER_SUPPLY_CURRENT_FAST_FILTER_ENABLED==1
		, current_filter( RT("cf"), &_hardwaresys.subsys(), _config.current_filter, _present.current_filter,_hardwaresys.sence_block().output())
		#endif
		#if POWER_SUPPLY_CURRENT_DIFF_FILTER_ENABLED==1
		, current_diff_filter(RT("cdf"),&_hardwaresys.subsys(), _config.current_diff_filter, _present.current_diff_filter,_hardwaresys.sence_block().output_diff())
		#endif
		#if POWER_SUPPLY_CURRENT_REGULATOR_ENABLED == 1
		, current_regulator(
				RT("c_re")
				,this
				,	_config.current_regulaor
				, _present.current_regulaor
				, _present.voltage_range_desired
				, _hardwaresys.pwm_block().actual_satstate()
				, POWER_SUPPLY_ACTUAL_SIGNALS
			)
		, current_mode(2, *this)
		#endif
		#if POWER_SUPPLY_CURRENT_LIMMITER_ENABLED == 1
		, current_limmiter( 
			RT("c_lm_r")
			, this
			, _config.current_limmiter
			, _present.current_limmiter
			, _hardwaresys.pwm_block().pwm_voltage_limits()
			, _hardwaresys.pwm_block().actual_satstate()
			, POWER_SUPPLY_ACTUAL_SIGNALS
			, _present.current_range_desired
			)
		, current_limmiter_mode(16, *this)
		#endif
	{

		_config =
		#include "mexo/ps.templ.settings.inc.hpp"
		;
	}
};



#undef  POWER_SUPPLY_VOLTAGE_REGULATOR_ENABLED
#if POWER_SUPPLY_CURRENT_MEASSURY_ENABLED==1 
#undef  POWER_SUPPLY_CURRENT_REGULATOR_ENABLED
#undef  POWER_SUPPLY_CURRENT_MEASSURY_ENABLED
#undef  POWER_SUPPLY_CURRENT_DIFF_ENABLED
#undef  POWER_SUPPLY_CURRENT_FILTER_ENABLED
#undef  POWER_SUPPLY_CURRENT_DIFF_FILTER_ENABLED
#undef POWER_SUPPLY_ACTUAL_SIGNALS
#endif

