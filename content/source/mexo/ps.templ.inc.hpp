#define TEMPL_BEGIN
#include "mexo/ps.templ.prepare.hpp"
#include "mexo/mexo.hpp"
#include "mexo/math.hpp"
#include "mexo/ps.hpp"
#include "mexo/regulator.hpp"
namespace PS_TEMPLATE_NAME {
	template <typename types, typename hardwaresys_t>  class dev_t : public ::mexo::ps::dev {
	public:
		hardwaresys_t& hardwaresys;
		typedef ::mexo::front::PS_TEMPLATE_NAME::action_t<types> action_s;
		typedef ::mexo::front::PS_TEMPLATE_NAME::feedback_t<types> feedback_s;
		typedef ::mexo::front::PS_TEMPLATE_NAME::mode mode;

		#if POWER_SUPPLY_VOLTAGE_REGULATOR_ENABLED == 1
		typedef ::mexo::controller_task_t <
			::mexo::ramp<types>
			, ::mexo::control_task
		> voltage_regulator_b;
		voltage_regulator_b voltage_regulator;
		#endif

		#if POWER_SUPPLY_CURRENT_REGULATOR_ENABLED == 1
		typedef ::mexo::controller_task_t <
			::mexo::quazzy_adapt<types>
			, ::mexo::control_task
			, const typename types::signal_t&
			, const typename types::signal_t&
		> current_regulator_b;
		current_regulator_b current_regulator;
		#endif

		#if POWER_SUPPLY_CURRENT_LIMMITER_ENABLED == 1
		typedef ::mexo::controller_task_t <
			::mexo::limmiter<types>
			, ::mexo::control_task
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
			::mexo::ps::dev::config_s dev;
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
			bool invers;
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
		#if ROBO_APP_MEXO_VAR_ENABLED == 1
		void do_create_vars(void) {
			::mexo::ps::dev::do_create_vars();
			if (::mexo::var::machine::actual_mode() >= ::mexo::var::machine::mode::action) {
				action_s& act = action<dev_t>();
				const config_s& conf = config<dev_t>();
				::mexo::var::record::create(::mexo::var::uint8, conf.invers, RT("act.invers"), key(), vars);
				::mexo::var::record::create(types::var::signal, act.voltage, RT("act.v"), key(), vars);
				#if POWER_SUPPLY_CURRENT_MEASSURY_ENABLED == 1
				::mexo::var::record::create(types::var::signal, act.current, RT("act.c"), key(), vars);
				#endif
			}
			if (::mexo::var::machine::actual_mode() >= ::mexo::var::machine::mode::full) {
				const present_s& psnt = present<dev_t>();

				#if POWER_SUPPLY_VOLTAGE_REGULATOR_ENABLED == 1 ||  POWER_SUPPLY_CURRENT_REGULATOR_ENABLED == 1 || POWER_SUPPLY_CURRENT_LIMMITER_ENABLED == 1
				::mexo::var::record::create(types::var::signal, psnt.voltage_required, RT("req.v"), key(), vars);
				#endif		

				#if POWER_SUPPLY_VOLTAGE_REGULATOR_ENABLED == 1 ||  POWER_SUPPLY_CURRENT_LIMMITER_ENABLED == 1
				::mexo::var::record::create(types::var::signal, psnt.voltage_deseired, RT("desrd.v"), key(), vars);
				#endif

				#if POWER_SUPPLY_CURRENT_REGULATOR_ENABLED == 1 || POWER_SUPPLY_CURRENT_LIMMITER_ENABLED == 1
				::mexo::var::record::create(types::var::signal, psnt.current_deseired, RT("desrd.c"), key(), vars);
				#endif
			}

		}
		#endif
		#if POWER_SUPPLY_VOLTAGE_REGULATOR_ENABLED == 1
	protected:
		virtual void do_update_feedback(void) {
			::mexo::ps::dev::do_update_feedback();			
			feedback< dev_t >().voltage = present< dev_t >().voltage_required;
			#if POWER_SUPPLY_CURRENT_MEASSURY_ENABLED == 1
			#if POWER_SUPPLY_CURRENT_FILTER_ENABLED==1 || POWER_SUPPLY_CURRENT_FAST_FILTER_ENABLED==1
			feedback< dev_t >().current = present< dev_t >().current_filter.fb.output;
			#else
			feedback< dev_t >().current = hardwaresys.current_sence_block.current_delta_ref();			
			#endif
			#else
			feedback< dev_t >().current = 0;
			#endif
		}

		void voltage_mode_start(void) {

			present_s& prsnt = present<dev_t>();

			hardwaresys.power_supply_block.set_input(&prsnt.voltage_required);
			voltage_regulator.set_output(&prsnt.voltage_required);
			voltage_regulator.set_input(&prsnt.voltage_deseired);

			hardwaresys.reconfig();
			voltage_regulator.reconfig();
			voltage_regulator.start();
			on();
		}
		virtual void voltage_mode_stop(void) {
			off();
			voltage_regulator.stop();
			hardwaresys.power_supply_block.set_input(nullptr);
			voltage_regulator.set_output(nullptr);
			voltage_regulator.set_input(nullptr);
		}
		friend class voltage_mode_t;

	public:
		class voltage_mode_t :public ::mexo::ps::dev::mode {
		protected:
			dev_t& owner(void) { return owner_cast<dev_t>(); }

			virtual void applay_action(void) {
				const action_s& act= owner().template action<dev_t>();
				const config_s& conf = owner().template config<dev_t>();
				present_s& present = owner().template present<dev_t>();

				if (conf.invers) {
					present.voltage_deseired = -act.voltage;
				}
				else {
					present.voltage_deseired = act.voltage;
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
		#define POWER_SUPPLY_ACTUAL_SIGNALS _present.current_filter.fb.output,hardwaresys.current_sence_block.current_delta_ref()
		#endif
		#else
		#define POWER_SUPPLY_ACTUAL_SIGNALS _present.current_filter.fb.output,_present.dummy
		#endif
		#else
		#if POWER_SUPPLY_CURRENT_DIFF_ENABLED==1
		#if POWER_SUPPLY_CURRENT_DIFF_FILTER_ENABLED==1
		#define POWER_SUPPLY_ACTUAL_SIGNALS hardwaresys.current_sence_block.current_ref(),_present.current_diff_filter.fb.output
		#else
		#define POWER_SUPPLY_ACTUAL_SIGNALS hardwaresys.current_sence_block.current_ref(),hardwaresys.current_sence_block.current_delta_ref()
		#endif
		#else
		#define POWER_SUPPLY_ACTUAL_SIGNALS hardwaresys.current_sence_block.current_ref(),_present.dummy
		#endif	
		#endif
		#endif

		#if POWER_SUPPLY_CURRENT_REGULATOR_ENABLED == 1
	protected:
		void mode_current_start(void) {
			present_s& prsnt = present<dev_t>();

			hardwaresys.power_supply_block.set_input(&prsnt.voltage_required);
			current_regulator.set_output(&prsnt.voltage_required);
			current_regulator.set_input(&prsnt.current_deseired);

			hardwaresys.reconfig();
			current_regulator.reconfig();
			current_regulator.start();
			on();
		}
		void mode_current_stop(void) {
			off();
			current_regulator.stop();
			hardwaresys.power_supply_block.set_input(nullptr);
			current_regulator.set_output(nullptr);
			current_regulator.set_input(nullptr);
		}
		friend class current_mode_t;
	public:
		class current_mode_t :public ::mexo::ps::dev::mode {
		protected:
			dev_t& owner(void) { return owner_cast<dev_t>(); }
			virtual void applay_action(void) {
				const action_s& act= owner().template action<dev_t>();
				const config_s& config = owner().template config<dev_t>();
				present_s& present = owner().template present<dev_t>();
				if (config.invers) {
					present.current_deseired = -act.current;
				}
				else {
					present.current_deseired = act.current;
				}

				present.voltage_range_desired.hi = act.voltage;
				present.voltage_range_desired.low = -act.voltage;

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
			present_s& prsnt = present<dev_t>();

			hardwaresys.power_supply_block.set_input(&prsnt.voltage_required);
			current_limmiter.set_output(&prsnt.voltage_required);
			current_limmiter.set_input(&prsnt.voltage_deseired);
			prsnt.voltage_range_desired = hardwaresys.power_supply_block.pwm_voltage_limits();
			hardwaresys.power_supply_block.reconfig();
			current_limmiter.reconfig();
			current_limmiter.start();
			on();
		}
		void mode_limmiter_stop(void) {
			off();
			current_limmiter.stop();
			hardwaresys.power_supply_block.set_input(nullptr);
			current_limmiter.set_output(nullptr);
			current_limmiter.set_input(nullptr);
		}
		friend class current_limmiter_mode_t;
	public:
		class current_limmiter_mode_t :public ::mexo::ps::dev::mode {
		protected:
			dev_t& owner(void) { return owner_cast<dev_t>(); }
			virtual void applay_action(void) {
				const action_s& act= owner().template action<dev_t>();
				const config_s& conf= owner().template config<dev_t>();
				present_s& present = owner().template present<dev_t>();

				if (conf.invers) {
					present.voltage_deseired = -act.voltage;
				}
				else {
					present.voltage_deseired = act.voltage;
				}

				present.current_range_desired.hi = act.current;
				present.current_range_desired.low = -act.current;
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
		dev_t(hardwaresys_t& _hardwaresys, cstr _name, action_s& _action, feedback_s& _feedback, config_s& _config, present_s& _present)
			: ::mexo::ps::dev(_name, _action.dev, _feedback.dev, _present.dev, _config.dev, _hardwaresys.power_supply_block )
			, hardwaresys(_hardwaresys)
			#if POWER_SUPPLY_VOLTAGE_REGULATOR_ENABLED == 1
			, voltage_regulator(RT("v_re"), this, _config.voltage_regulator, _present.voltage_regulator, hardwaresys.power_supply_block.pwm_voltage_limits(), hardwaresys.power_supply_block.actual_satstate())
			, voltage_mode(mode::voltage, *this) 
			#endif

			#if POWER_SUPPLY_CURRENT_FILTER_ENABLED==1
			, current_filter(RT("c_f"), &_hardwaresys.current_sence_block, _config.current_filter, _present.current_filter, hardwaresys.current_sence_block.current_ref())
			#endif
			#if POWER_SUPPLY_CURRENT_FAST_FILTER_ENABLED==1
			, current_filter(RT("c_f"), &_hardwaresys.current_sence_block, _config.current_filter, _present.current_filter, hardwaresys.current_sence_block.current_ref())
			#endif
			#if POWER_SUPPLY_CURRENT_DIFF_FILTER_ENABLED==1
			, current_diff_filter(RT("c_dif_f"), &_hardwaresys.current_sence_block, _config.current_diff_filter, _present.current_diff_filter, hardwaresys.current_sence_block.current_delta_ref())
			#endif
			#if POWER_SUPPLY_CURRENT_REGULATOR_ENABLED == 1
			, current_regulator(
				RT("c_re")
				, this
				, _config.current_regulator
				, _present.current_regulator
				, _present.voltage_range_desired
				, hardwaresys.power_supply_block.actual_satstate()
				, POWER_SUPPLY_ACTUAL_SIGNALS
			)
			, current_mode(mode::current, *this)
			#endif
			#if POWER_SUPPLY_CURRENT_LIMMITER_ENABLED == 1
			, current_limmiter(
				RT("c_lm_r")
				, this
				, _config.current_limmiter
				, _present.current_limmiter
				, hardwaresys.power_supply_block.pwm_voltage_limits()
				, hardwaresys.power_supply_block.actual_satstate()
				, POWER_SUPPLY_ACTUAL_SIGNALS
				, _present.current_range_desired
			)
			, current_limmiter_mode(mode::voltage_cl, *this)
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

