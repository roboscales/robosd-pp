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
	::mexo::backend_subsystem voltage_control;
	#endif

	#if POWER_SUPPLY_CURRENT_REGULATOR_ENABLED == 1
	::mexo::backend_subsystem current_control;
	#endif

	#if POWER_SUPPLY_CURRENT_LIMMITER_ENABLED == 1
	::mexo::backend_subsystem current_limmiter_control;
	#endif

	#if POWER_SUPPLY_VOLTAGE_REGULATOR_ENABLED == 1
	typedef ::mexo::ramp_b<types> voltage_regulator_b;
	voltage_regulator_b voltage_regulator;
	#endif

	#if POWER_SUPPLY_CURRENT_REGULATOR_ENABLED == 1
	typedef ::mexo::quazzy_adapt_b<types> current_regulaor_b;
	current_regulaor_b current_regulator;
	#endif

	#if POWER_SUPPLY_CURRENT_LIMMITER_ENABLED == 1
	typedef ::mexo::limmiter_b<types> current_limmiter_b;
	current_limmiter_b current_limmiter;
	#endif

	#if POWER_SUPPLY_CURRENT_FILTER_ENABLED==1 || POWER_SUPPLY_CURRENT_DIFF_ENABLED==1
	typedef ::mexo::filter_b<types> filter_b;
	#endif 

	#if POWER_SUPPLY_CURRENT_FAST_FILTER_ENABLED==1
	typedef ::mexo::fast_filter_b<types> fast_filter_b;
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
	};

	#if POWER_SUPPLY_VOLTAGE_REGULATOR_ENABLED == 1
	class voltage_mode_t :public ::mexo::ps::dev::mode {
	protected:
		PS_TEMPLATE_NAME& owner(void) { return owner_cast<PS_TEMPLATE_NAME>(); }

		virtual void applay_action(void) {
			const action_s& action = owner().template action_cast<action_s>();

			if (action.invers) {
				owner().voltage_regulator.set_input(-action.voltage);
			}
			else {
				owner().voltage_regulator.set_input(action.voltage);
			}
		}

		virtual void do_start(void) {
			owner().hardwaresys.reconfig();
			owner().voltage_control.reconfig();
			owner().voltage_control.start();
			owner().hardwaresys.pwm_block().link_to(owner().voltage_regulator);
			owner().on();
		}

		virtual void do_stop(void) {
			owner().voltage_control.stop();
			owner().off();
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
			if (action.invers) {
				owner().current_regulator.set_input(-action.current);
			}
			else {
				owner().current_regulator.set_input(action.current);
			}
			owner().current_regulator.set_min(-action.voltage);
			owner().current_regulator.set_max(action.voltage);
		}
		virtual void do_start(void) {
			owner().hardwaresys.pwm_block().link_to(owner().current_regulator);
			owner().hardwaresys.reconfig();
			owner().current_control.reconfig();
			owner().current_control.start();
			owner().on();
		}
		virtual void do_stop(void) {
			owner().current_control.stop();
			owner().off();
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
			if (action.invers) {
				owner().current_limmiter.set_input(-action.voltage);
			}
			else {
				owner().current_limmiter.set_input(action.voltage);
			}			
			owner().current_limmiter.set_signal_min(-action.current);
			owner().current_limmiter.set_signal_max(action.current);
		}
		
		virtual void do_start(void) {
			owner().hardwaresys.pwm_block().link_to(owner().current_limmiter);
			owner().hardwaresys.reconfig();
			owner().current_limmiter_control.reconfig();
			owner().current_limmiter_control.start();
			owner().on();
		}
		virtual void do_stop(void) {
			owner().current_limmiter_control.stop();
			owner().off();
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
		, voltage_control(RT("v_co"), false, this)
		, voltage_regulator(voltage_control, RT("v_re"), _config.voltage_regulator, _present.voltage_regulator)
		, voltage_mode(1, *this)
		#endif
		#if POWER_SUPPLY_CURRENT_MEASSURY_ENABLED == 1
		, current_control(RT("c_co"), false, this)
		#endif
		#if POWER_SUPPLY_CURRENT_MEASSURY_ENABLED == 1
		, current_limmiter_control(RT("c_lm_co"), false, this)
		#endif
		#if POWER_SUPPLY_CURRENT_FILTER_ENABLED==1
		, current_filter(_hardwaresys.subsystem(), RT("cf"), _config.current_filter, _present.current_filter)
		#endif
		#if POWER_SUPPLY_CURRENT_FAST_FILTER_ENABLED==1
		, current_filter(_hardwaresys.subsystem(), RT("cf"), _config.current_filter, _present.current_filter)
		#endif
		#if POWER_SUPPLY_CURRENT_DIFF_FILTER_ENABLED==1
		, current_diff_filter(current_control, RT("cdf"), _config.current_filter, _present.current_filter)
		#endif
		#if POWER_SUPPLY_CURRENT_REGULATOR_ENABLED == 1
		, current_regulator(current_control, RT("c_re"), _config.current_regulaor, _present.current_regulaor)
		, current_mode(2, *this)
		#endif
		#if POWER_SUPPLY_CURRENT_LIMMITER_ENABLED == 1
		, current_limmiter(current_limmiter_control, RT("c_lm_r"), _config.current_limmiter, _present.current_limmiter)
		, current_limmiter_mode(16, *this)
		#endif
	{

		//				prioritet_subsystem_.dc.link_to(current_regulator);
		#if POWER_SUPPLY_CURRENT_REGULATOR_ENABLED ==1
		#if POWER_SUPPLY_CURRENT_FILTER_ENABLED==1 || POWER_SUPPLY_CURRENT_FAST_FILTER_ENABLED==1
		current_filter.input.link_to(&hardwaresys.sence_block().output);
		current_regulator.actual.link_to(&current_filter.output);
		#else
		current_regulator.actual.link_to(&hardwaresys.sence_block().output);
		#endif

		#if POWER_SUPPLY_CURRENT_DIFF_ENABLED ==1
		#if POWER_SUPPLY_CURRENT_DIFF_FILTER_ENABLED==1
		current_diff_filter.input.link_to(&hardwaresys.sence_block().output_diff);
		current_regulator.actual_diff.link_to(&current_diff_filter.output);
		#else
		current_regulator.actual.link_to(&hardwaresys.sence_block().output_diff);
		#endif
		#endif
		#endif
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
#endif

