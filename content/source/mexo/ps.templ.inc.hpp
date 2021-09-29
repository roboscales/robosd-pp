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

	};


	#if POWER_SUPPLY_VOLTAGE_REGULATOR_ENABLED == 1
	class voltage_mode_t :public ::mexo::ps::dev::mode {
	protected:
		PS_TEMPLATE_NAME& owner(void) { return owner_cast<PS_TEMPLATE_NAME>(); }

		virtual void applay_action(void) {
			const action_s& action = owner().template action_cast<action_s>();
			const present_s& present = owner().template present_cast<present_s>();

			if (action.invers) {
				present.voltage_deseired = -action.voltage;
			}
			else {
				present.voltage_deseired = action.voltage;
			}
		}

		virtual void do_start(void) {
			
			const present_s& present = owner().template present_cast<present_s>();

			owner().pwm_block().set_output(&present.pwm_duty);
			owner().pwm_block().set_input(&present.voltage_required);
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
			owner().pwm_block().set_output(nullptr);
			owner().pwm_block().set_input(nullptr);
			owner().voltage_regulator.set_output(nullptr);
			owner().voltage_regulator.set_input(nullptr);
		}

	public:
		voltage_mode_t(int _index, PS_TEMPLATE_NAME& _owner) :
			::mexo::ps::dev::mode(_index, RT("mod_v"), _owner) {}
	};
	#endif
	
	
	#if POWER_SUPPLY_CURRENT_REGULATOR_ENABLED == 1
	class current_mode_t :public ::mexo::ps::dev::mode {
	protected:
		PS_TEMPLATE_NAME& owner(void) { return owner_cast<PS_TEMPLATE_NAME>(); }
		virtual void applay_action(void) {
			const action_s& action = owner().template action_cast<action_s>();
			const present_s& present = owner().template present_cast<present_s>();
	
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
			
			
			const present_s& present = owner().template present_cast<present_s>();

			owner().pwm_block().set_output(&present.pwm_duty);
			owner().pwm_block().set_input(&present.voltage_required);
			owner().current_regulator.set_output(&present.voltage_required);
			owner().current_regulator.set_input(&present.current_deseired);

			owner().hardwaresys.reconfig();
			owner().current_regulator.reconfig();
			owner().current_regulator.start();
			owner().on();			

		}
		virtual void do_stop(void) {
			owner().off();
			owner().v.stop();
			owner().pwm_block().set_output(nullptr);
			owner().pwm_block().set_input(nullptr);
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
			const present_s& present = owner().template present_cast<present_s>();
	
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
			const present_s& present = owner().template present_cast<present_s>();

			owner().pwm_block().set_output(&present.pwm_duty);
			owner().pwm_block().set_input(&present.voltage_required);
			owner().current_limmiter.set_output(&present.voltage_required);
			owner().current_limmiter.set_input(&present.voltage_deseired);
			present.voltage_range_desired =  owner().pwm_block().
			owner().hardwaresys.reconfig();
			owner().current_limmiter.reconfig();
			owner().current_limmiter.start();
			owner().on();			
		}
		virtual void do_stop(void) {
			owner().off();
			owner().current_limmiter.stop();
			owner().pwm_block().set_output(nullptr);
			owner().pwm_block().set_input(nullptr);
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
	PS_TEMPLATE_NAME(hardwaresys_t& _hardwaresys, cstr _name, action_s& _action, config_s& _config, standalone_s& _standalone, present_s& _present)
		: ::mexo::ps::dev(_name, _action.dev, _present.dev, _hardwaresys.pwm_handler())
		, hardwaresys(_hardwaresys)
		#if POWER_SUPPLY_VOLTAGE_REGULATOR_ENABLED == 1
		, voltage_control(RT("v_co"), false, this)
		#if POWER_SUPPLY_CURRENT_LIMMITER_ENABLED == 1
		, voltage_regulator(voltage_control, RT("v_re"), _config.voltage_regulator, _standalone.current_limmiter.cb, _present.voltage_regulator)
		#else
		, voltage_regulator(voltage_control, RT("v_re"), _config.voltage_regulator, _standalone.voltage_regulator, _present.voltage_regulator)
		#endif
		, voltage_mode(1, *this)
		#endif
		#if POWER_SUPPLY_CURRENT_REGULATOR_ENABLED == 1
		, current_control(RT("c_co"), false, this)
		#endif
		#if POWER_SUPPLY_CURRENT_LIMMITER_ENABLED == 1
		, current_limmiter_control(RT("c_lm_co"), false, this)
		#endif
		#if POWER_SUPPLY_CURRENT_FILTER_ENABLED==1
		, current_filter(_hardwaresys.subsys(), RT("cf"), _config.current_filter, _standalone.current_filter, _present.current_filter)
		#endif
		#if POWER_SUPPLY_CURRENT_FAST_FILTER_ENABLED==1
		, current_filter(_hardwaresys.subsys(), RT("cf"), _config.current_filter, _standalone.current_filter, _present.current_filter)
		#endif
		#if POWER_SUPPLY_CURRENT_DIFF_FILTER_ENABLED==1
		, current_diff_filter(current_control, RT("cdf"), _config.current_diff_filter, _standalone.current_diff_filter, _present.current_diff_filter)
		#endif
		#if POWER_SUPPLY_CURRENT_REGULATOR_ENABLED == 1
		, current_regulator(current_control, RT("c_re"), _config.current_regulaor, _standalone.current_regulaor, _present.current_regulaor)
		, current_mode(2, *this)
		#endif
		#if POWER_SUPPLY_CURRENT_LIMMITER_ENABLED == 1
		, current_limmiter(current_limmiter_control, RT("c_lm_r"), _config.current_limmiter, _standalone.current_limmiter, _present.current_limmiter)
		, current_limmiter_mode(16, *this)
		#endif
	{

		//				prioritet_subsystem_.dc.link_to(current_regulator);
		#if POWER_SUPPLY_CURRENT_REGULATOR_ENABLED ==1
		#if POWER_SUPPLY_CURRENT_FILTER_ENABLED==1 || POWER_SUPPLY_CURRENT_FAST_FILTER_ENABLED==1
		current_filter.input.link_to(&hardwaresys.sence_handler().output);
		current_regulator.actual.link_to(&current_filter.output);
		#else
		current_regulator.actual.link_to(&hardwaresys.sence_handler().output);
		#endif

		#if POWER_SUPPLY_CURRENT_DIFF_ENABLED ==1
		#if POWER_SUPPLY_CURRENT_DIFF_FILTER_ENABLED==1
		current_diff_filter.input.link_to(&hardwaresys.sence_handler().output_diff);
		current_regulator.actual_diff.link_to(&current_diff_filter.output);
		#else
		current_regulator.actual.link_to(&hardwaresys.sence_handler().output_diff);
		#endif
		#endif
		#endif
		_config =

		{
			0/*
			#if POWER_SUPPLY_VOLTAGE_REGULATOR_ENABLED == 1
			, {
				{
					{} //ref
				}
				, POWER_SUPPLY_PREFIX(VOLTAGE_RAMP_GAIN)
			}
			#endif
			#if POWER_SUPPLY_CURRENT_FILTER_ENABLED==1
			, {
				{}
				, POWER_SUPPLY_PREFIX(CURRENT_FILTER_GAIN)
				,{
					POWER_SUPPLY_PREFIX(CURRENT_FILTER_SHIFT_GAIN)
					, POWER_SUPPLY_PREFIX(CURRENT_FILTER_SHIFT_PRESC)
					, POWER_SUPPLY_PREFIX(CURRENT_FILTER_SHIFT_VALUE)
				}
			}
			#endif
			#if POWER_SUPPLY_CURRENT_FAST_FILTER_ENABLED==1
			, {
				{}
				, POWER_SUPPLY_PREFIX(CURRENT_FAST_FILTER_SHIFT_VALUE)
			}
			#endif
			#if POWER_SUPPLY_CURRENT_DIFF_FILTER_ENABLED == 1
			, {
				{}
				, POWER_SUPPLY_PREFIX(CURRENT_DIFF_FILTER_GAIN)
				,{
					POWER_SUPPLY_PREFIX(CURRENT_DIFF_FILTER_SHIFT_GAIN)
					, POWER_SUPPLY_PREFIX(CURRENT_DIFF_FILTER_SHIFT_PRESC)
					, POWER_SUPPLY_PREFIX(CURRENT_DIFF_FILTER_SHIFT_VALUE)
				}
			}
			#endif
			#if POWER_SUPPLY_CURRENT_REGULATOR_ENABLED == 1
			,
			#define REGULATOR_PROP_GAIN POWER_SUPPLY_PREFIX(CURRENT_PROP_GAIN)
			#define REGULATOR_MODEL_GAIN POWER_SUPPLY_PREFIX(CURRENT_MODEL_GAIN)
			#define REGULATOR_DIFF_GAIN POWER_SUPPLY_PREFIX(CURRENT_DIFF_GAIN)
			#define REGULATOR_CONTROL_SHIFT POWER_SUPPLY_PREFIX(CURRENT_CONTROL_SHIFT)
			#define REGULATOR_MODEL_SHIFT POWER_SUPPLY_PREFIX(CURRENT_MODEL_SHIFT)

			#include "mexo/qa.templ.settings.inc.hpp"
			#endif
			#if POWER_SUPPLY_CURRENT_LIMMITER_ENABLED == 1
			,
			#define REGULATOR_PROP_GAIN POWER_SUPPLY_PREFIX(CURRENT_PROP_GAIN)
			#define REGULATOR_MODEL_GAIN POWER_SUPPLY_PREFIX(CURRENT_MODEL_GAIN)
			#define REGULATOR_DIFF_GAIN POWER_SUPPLY_PREFIX(CURRENT_DIFF_GAIN)
			#define REGULATOR_CONTROL_SHIFT POWER_SUPPLY_PREFIX(CURRENT_CONTROL_SHIFT)
			#define REGULATOR_MODEL_SHIFT POWER_SUPPLY_PREFIX(CURRENT_MODEL_SHIFT)
			#define REGULATOR_RAMP_STEP POWER_SUPPLY_PREFIX(CURRENT_LIMMITER_RAMP_STEP)
			#define REGULATOR_SIGNAL_MIN POWER_SUPPLY_PREFIX(CURRENT_MIN_LIM)
			#define REGULATOR_SIGNAL_MAX POWER_SUPPLY_PREFIX(CURRENT_MAX_LIM)

			#include "mexo/lm.templ.settings.inc.hpp"
			#endif*/
		};
		/*			#include "mexo/ps.templ.settings.inc.hpp"
					;
				_standalone =
					#include "mexo/ps.templ.standalone.inc.hpp"
					;*/
	}
};



#undef  POWER_SUPPLY_VOLTAGE_REGULATOR_ENABLED
#if POWER_SUPPLY_CURRENT_MEASSURY_ENABLED==1 
#undef  POWER_SUPPLY_CURRENT_REGULATOR_ENABLED
#undef  POWER_SUPPLY_CURRENT_MEASSURY_ENABLED
#undef  POWER_SUPPLY_CURRENT_DIFF_ENABLED
#undef  POWER_SUPPLY_CURRENT_FILTER_ENABLED
#undef  POWER_SUPPLY_CURRENT_DIFF_FILTER_ENABLED
#endif

