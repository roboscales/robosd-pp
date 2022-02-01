#define TEMPL_BEGIN
#include "mexo/ps.templ.prepare.hpp"
#include "mexo/mexo.hpp"
#include "mexo/math.hpp"
#include "mexo/ps.hpp"
#include "mexo/regulator.hpp"
namespace PS_TEMPLATE_NAME {
	template <typename types, typename hardwaresys_t>  class dev_t : public ::mexo::ps::dev {
	public:
		typedef action_t<types> action_s;
		typedef feedback_t<types> feedback_s;

		#if POWER_SUPPLY_VOLTAGE_REGULATOR_ENABLED == 1
		typedef ::mexo::controller_task_t <
			::mexo::ramp<types>
			, ::mexo::control_subsystem
		> voltage_regulator_b;
		voltage_regulator_b voltage_regulator;
		#endif

		#if POWER_SUPPLY_CURRENT_REGULATOR_ENABLED == 1
		typedef ::mexo::controller_task_t <
			::mexo::quazzy_adapt<types>
			, ::mexo::control_subsystem
			, const typename types::signal_t&
			, const typename types::signal_t&
		> current_regulator_b;
		current_regulator_b current_regulator;
		#endif

		#if POWER_SUPPLY_CURRENT_LIMMITER_ENABLED == 1
		typedef ::mexo::controller_task_t <
			::mexo::limmiter<types>
			, ::mexo::control_subsystem
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
			typename current_regulator_b::config_s current_regulator;
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
			typename current_regulator_b::present_s current_regulator;
			#endif
			#if POWER_SUPPLY_CURRENT_LIMMITER_ENABLED == 1
			typename current_limmiter_b::present_s current_limmiter;
			#endif
			
			#if POWER_SUPPLY_VOLTAGE_REGULATOR_ENABLED == 1 ||  POWER_SUPPLY_CURRENT_REGULATOR_ENABLED == 1 || POWER_SUPPLY_CURRENT_LIMMITER_ENABLED == 1
			//typename types::discret_t pwm_duty;
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
protected:
		void do_create_vars(void) {
			::mexo::ps::dev::do_create_vars();
			if (::mexo::var::machine::actual_mode() >= ::mexo::var::machine::mode::action) {
				const action_s& action = action_cast<action_s>();
				::mexo::var::record::create(::mexo::var::uint8, action.invers, RT("act.invers"), key(), vars);
				::mexo::var::record::create(typename types::var::signal, action.voltage, RT("act.v"), key(), vars);
				#if POWER_SUPPLY_CURRENT_MEASSURY_ENABLED == 1
				::mexo::var::record::create(typename types::var::signal, action.current, RT("act.c"), key(), vars);
				#endif
			}
			if (::mexo::var::machine::actual_mode() >= ::mexo::var::machine::mode::full) {
				const present_s& present = present_cast<present_s>();

				#if POWER_SUPPLY_VOLTAGE_REGULATOR_ENABLED == 1 ||  POWER_SUPPLY_CURRENT_REGULATOR_ENABLED == 1 || POWER_SUPPLY_CURRENT_LIMMITER_ENABLED == 1
				::mexo::var::record::create(typename types::var::signal, present.voltage_required, RT("req.v"), key(), vars);
				#endif		

				#if POWER_SUPPLY_VOLTAGE_REGULATOR_ENABLED == 1 ||  POWER_SUPPLY_CURRENT_LIMMITER_ENABLED == 1
				::mexo::var::record::create(typename types::var::signal, present.voltage_deseired, RT("desrd.v"), key(), vars);
				#endif
			}

		}

		#if POWER_SUPPLY_VOLTAGE_REGULATOR_ENABLED == 1
	protected:
		void voltage_mode_start(void) {

			present_s& present = present_cast<present_s>();

			hardwaresys_t::power_supply_block.set_input(&present.voltage_required);
			voltage_regulator.set_output(&present.voltage_required);
			voltage_regulator.set_input(&present.voltage_deseired);

			hardwaresys_t::reconfig();
			voltage_regulator.reconfig();
			voltage_regulator.start();
			on();
		}

		virtual void voltage_mode_stop(void) {
			off();
			voltage_regulator.stop();
			hardwaresys_t::power_supply_block.set_input(nullptr);
			voltage_regulator.set_output(nullptr);
			voltage_regulator.set_input(nullptr);
		}
		friend class voltage_mode_t;

	public:
		class voltage_mode_t :public ::mexo::ps::dev::mode {
		protected:
			dev_t& owner(void) { return owner_cast<dev_t>(); }

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
				owner().voltage_mode_start();
			}

			virtual void do_stop(void) {
				owner().voltage_mode_stop();
			}

		public:
			voltage_mode_t(int _index, dev_t& _owner) :
				::mexo::ps::dev::mode(_index, RT("mod_v"), _owner) {}
		};
		#endif
		#if POWER_SUPPLY_CURRENT_MEASSURY_ENABLED == 1
		#if POWER_SUPPLY_CURRENT_FILTER_ENABLED==1 || POWER_SUPPLY_CURRENT_FAST_FILTER_ENABLED==1
		#if POWER_SUPPLY_CURRENT_DIFF_ENABLED==1
		#if POWER_SUPPLY_CURRENT_DIFF_FILTER_ENABLED==1
		#define POWER_SUPPLY_ACTUAL_SIGNALS _present.current_filter.fb.output,_present.current_diff_filter.fb.output
		#else
		#define POWER_SUPPLY_ACTUAL_SIGNALS _present.current_filter.fb.output,hardwaresys_t::current_sence_block.current_delta_ref()
		#endif
		#else
		#define POWER_SUPPLY_ACTUAL_SIGNALS _present.current_filter.fb.output,_present.dummy
		#endif
		#else
		#if POWER_SUPPLY_CURRENT_DIFF_ENABLED==1
		#if POWER_SUPPLY_CURRENT_DIFF_FILTER_ENABLED==1
		#define POWER_SUPPLY_ACTUAL_SIGNALS hardwaresys_t::current_sence_block.current_ref(),_present.current_diff_filter.fb.output
		#else
		#define POWER_SUPPLY_ACTUAL_SIGNALS hardwaresys_t::current_sence_block.current_ref(),hardwaresys_t::current_sence_block.current_delta_ref()
		#endif
		#else
		#define POWER_SUPPLY_ACTUAL_SIGNALS hardwaresys_t::current_sence_block.current_ref(),_present.dummy
		#endif	
		#endif
		#endif

		#if POWER_SUPPLY_CURRENT_REGULATOR_ENABLED == 1
	protected:
		void mode_current_start(void) {
			present_s& present = present_cast<present_s>();

			hardwaresys_t::power_supply_block.set_input(&present.voltage_required);
			current_regulator.set_output(&present.voltage_required);
			current_regulator.set_input(&present.current_deseired);

			hardwaresys_t::reconfig();
			current_regulator.reconfig();
			current_regulator.start();
			on();
		}
		void mode_current_stop(void) {
			off();
			current_regulator.stop();
			hardwaresys_t::power_supply_block.set_input(nullptr);
			current_regulator.set_output(nullptr);
			current_regulator.set_input(nullptr);
		}
		friend class current_mode_t;
	public:
		class current_mode_t :public ::mexo::ps::dev::mode {
		protected:
			dev_t& owner(void) { return owner_cast<dev_t>(); }
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
				owner().mode_current_start();
			}
			virtual void do_stop(void) {
				owner().mode_current_stop();
			}
		public:
			current_mode_t(int _index, dev_t& _owner) :
				::mexo::ps::dev::mode(_index, RT("mod_c"), _owner) {}
		};
		#endif
		#if POWER_SUPPLY_CURRENT_LIMMITER_ENABLED == 1
	protected:
		void mode_limmiter_start(void) {
			present_s& present = present_cast<present_s>();

			hardwaresys_t::power_supply_block.set_input(&present.voltage_required);
			current_limmiter.set_output(&present.voltage_required);
			current_limmiter.set_input(&present.voltage_deseired);
			present.voltage_range_desired = hardwaresys_t::power_supply_block.pwm_voltage_limits();
			hardwaresys_t::power_supply_block.reconfig();
			current_limmiter.reconfig();
			current_limmiter.start();
			on();
		}
		void mode_limmiter_stop(void) {
			off();
			current_limmiter.stop();
			hardwaresys_t::power_supply_block.set_input(nullptr);
			current_limmiter.set_output(nullptr);
			current_limmiter.set_input(nullptr);
		}
		friend class current_limmiter_mode_t;
	public:
		class current_limmiter_mode_t :public ::mexo::ps::dev::mode {
		protected:
			dev_t& owner(void) { return owner_cast<dev_t>(); }
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
				owner().mode_limmiter_start();
			}
			virtual void do_stop(void) {
				owner().mode_limmiter_start();
			}
		public:
			current_limmiter_mode_t(int _index, dev_t& _owner) :
				::mexo::ps::dev::mode(_index, RT("mod_c_lm"), _owner) {}
		};
		#endif
		#if POWER_SUPPLY_VOLTAGE_REGULATOR_ENABLED == 1
		voltage_mode_t voltage_mode;
		#endif
		#if POWER_SUPPLY_CURRENT_REGULATOR_ENABLED == 1
		current_mode_t current_mode;
		#endif
		#if POWER_SUPPLY_CURRENT_LIMMITER_ENABLED == 1
		current_limmiter_mode_t current_limmiter_mode;
		#endif
		dev_t(cstr _name, action_s& _action, config_s& _config, present_s& _present)
			: ::mexo::ps::dev(_name, _action.dev, _present.dev, hardwaresys_t::power_supply_block )
			#if POWER_SUPPLY_VOLTAGE_REGULATOR_ENABLED == 1
			, voltage_regulator(RT("v_re"), this, _config.voltage_regulator, _present.voltage_regulator, hardwaresys_t::power_supply_block.pwm_voltage_limits(), hardwaresys_t::power_supply_block.actual_satstate())
			, voltage_mode(1, *this) 
			#endif

			#if POWER_SUPPLY_CURRENT_FILTER_ENABLED==1
			, current_filter(RT("c_f"), &hardwaresys_t::prioritet_subsystem, _config.current_filter, _present.current_filter, hardwaresys_t::current_sence_block.current_ref())
			#endif
			#if POWER_SUPPLY_CURRENT_FAST_FILTER_ENABLED==1
			, current_filter(RT("c_f"), &hardwaresys_t::prioritet_subsystem, _config.current_filter, _present.current_filter, hardwaresys_t::current_sence_block.current_ref())
			#endif
			#if POWER_SUPPLY_CURRENT_DIFF_FILTER_ENABLED==1
			, current_diff_filter(RT("c_dif_f"), &hardwaresys_t::prioritet_subsystem, _config.current_diff_filter, _present.current_diff_filter, hardwaresys_t::current_sence_block.current_delta_ref())
			#endif
			#if POWER_SUPPLY_CURRENT_REGULATOR_ENABLED == 1
			, current_regulator(
				RT("c_re")
				, this
				, _config.current_regulator
				, _present.current_regulator
				, _present.voltage_range_desired
				, hardwaresys_t::power_supply_block.actual_satstate()
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
				, hardwaresys_t::power_supply_block.pwm_voltage_limits()
				, hardwaresys_t::power_supply_block.actual_satstate()
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
}

#define TEMPL_FINISH
#include "mexo/ps.templ.prepare.hpp"

