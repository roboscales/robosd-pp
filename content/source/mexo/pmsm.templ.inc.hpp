#define TEMPL_BEGIN
#include "mexo/pmsm.templ.prepare.hpp"
#define ACTUATOR_TEMPLATE_NAME PMSM_ACTUATOR_TEMPLATE_SUB_NAME
#define ACTUATOR_PS_TEMPLATE_NAME PMSM_PS_CROSS_TEMPLATE_NAME
#include "mexo/actuator.templ.inc.hpp"
namespace PMSM_TEMPLATE_NAME {

	template <typename types, typename hardwaresys_t>  class dev_t: public PMSM_ACTUATOR_TEMPLATE_SUB_NAME:: dev_t<types,hardwaresys_t> {

	public:
		typedef action_t<types> action_s;
		typedef feedback_t<types> feedback_s;

		typedef PMSM_ACTUATOR_TEMPLATE_SUB_NAME::dev_t<types, hardwaresys_t> actuator_t;

		enum { mode_sync = 32, mode_sync_voltage = 33, mode_sync_current = 34 };
	private:
		friend class synchro_voltage_mode_t;

		#if LAT_CURRENT_REGULATOR_ENABLED == 1
		typedef ::mexo::controller_task_t <
			::mexo::quazzy_adapt<types>
			, ::mexo::control_subsystem
			, const typename types::signal_t&
			, const typename types::signal_t&
		> lat_current_regulator_b;
		lat_current_regulator_b lat_current_regulator;
		#endif

		#if LAT_CURRENT_FILTER_ENABLED==1 || LAT_CURRENT_DIFF_ENABLED==1
		typedef ::mexo::function_task_t <
			::mexo::filter<types>
			, ::mexo::prioritet_subsystem
		>  lat_filter_b;
		#endif 

		#if LAT_CURRENT_FAST_FILTER_ENABLED==1
		typedef ::mexo::function_block_t <
			::mexo::fast_filter<types>
			, ::mexo::prioritet_subsystem
		>lat_fast_filter_b;
		#endif 

		#if LAT_CURRENT_FILTER_ENABLED == 1
		lat_filter_b lat_current_filter;
		#endif 

		#if LAT_CURRENT_FAST_FILTER_ENABLED == 1
		lat_fast_filter_b lat_current_filter;
		#endif 

		#if LAT_CURRENT_DIFF_FILTER_ENABLED == 1
		lat_filter_b lat_current_diff_filter;
		#endif 
	public:
		struct config_s {
			typename actuator_t::config_s actuator;
			struct {
				#if LAT_CURRENT_FILTER_ENABLED==1
				typename lat_filter_b::config_s filter;
				#endif
				#if LAT_CURRENT_FAST_FILTER_ENABLED==1
				typename lat_fast_filter_b::config_s filter;
				#endif
				#if LAT_CURRENT_DIFF_FILTER_ENABLED == 1
				typename lat_filter_b::config_s diff_filter;
				#endif
				#if LAT_CURRENT_REGULATOR_ENABLED == 1
				typename lat_current_regulator_b::config_s regulator;
				#endif
			} lat_current;
		};

		struct present_s {
			typename actuator_t::present_s actuator;
			typename types::long_signal_t angle_req;
			typename types::long_signal_t freq_req;
			struct {
				#if LAT_CURRENT_FILTER_ENABLED==1
				typename lat_filter_b::present_s filter;
				#endif
				#if LAT_CURRENT_FAST_FILTER_ENABLED==1
				typename lat_fast_filter_b::present_s filter;
				#endif
				#if LAT_CURRENT_DIFF_FILTER_ENABLED == 1
				typename lat_filter_b::present_s diff_filter;
				#endif
				#if LAT_CURRENT_REGULATOR_ENABLED == 1
				typename lat_current_regulator_b::present_s regulator;
				#endif

				#if  LAT_CURRENT_REGULATOR_ENABLED == 1
				typename types::signal_t deseired;
				#endif		

			} lat_current;
			#if LAT_CURRENT_REGULATOR_ENABLED == 1 
			::mexo::range_s<typename types::signal_t> voltage_range_desired;
			#endif
		};
	protected:
		#if LAT_CURRENT_MEASSURY_ENABLED == 1
		#if LAT_CURRENT_FILTER_ENABLED==1 || LAT_CURRENT_FAST_FILTER_ENABLED==1
		#if LAT_CURRENT_DIFF_ENABLED==1
		#if LAT_CURRENT_DIFF_FILTER_ENABLED==1
		
		#define LAT_ACTUAL_SIGNALS _present.lat_current.filter.fb.output,_present.lat_current.diff_filter.fb.output
		#else
		#define LAT_ACTUAL_SIGNALS _present.lat_current.filter.fb.output,hardwaresys_t::current_sence_block.lat_current_delta_ref()
		#endif
		#else
		#define LAT_ACTUAL_SIGNALS _present.lat_current.filter.fb.output,_present.dummy
		#endif
		#else
		#if LAT_CURRENT_DIFF_ENABLED==1
		#if LAT_CURRENT_DIFF_FILTER_ENABLED==1
		#define LAT_ACTUAL_SIGNALS hardwaresys_t::current_sence_block.lat_current_ref(),_present.lat_current.diff_filter.fb.output
		#else
		#define LAT_ACTUAL_SIGNALS hardwaresys_t::current_sence_block.lat_current_ref(),hardwaresys_t::current_sence_block.lat_current_delta_ref()
		#endif
		#else
		#define LAT_ACTUAL_SIGNALS hardwaresys_t::current_sence_block.lat_current_ref(),_present.actuator.dummy
		#endif	
		#endif
		#endif


		typedef ::mexo::machine::slot::member<dev_t> inverter_controller_t;
		inverter_controller_t	inverter_controller;
		int old_mode_id_ = ::mexo::dev::idle_id;
		void inverter_controller_run(void) {
			present_s& present = present_cast<present_s>();
			
			if (old_mode_id_ != present.actuator.ps.dev.mode) {
				switch (::mexo::dev::idle_id) {
				case ::mexo::dev::idle_id:
				break;
				}
				old_mode_id_ = present.actuator.ps.dev.mode;
			}

			switch (present.actuator.ps.dev.mode) {
			case ::mexo::dev::idle_id:
			break;
			case mode_sync_voltage:
				present.angle_req += present.freq_req;
				hardwaresys_t::power_supply_block.inverter.angle_set(types::scale_l(present.angle_req));
			break;
			case mode_sync_current:
				present.angle_req += present.freq_req;
				hardwaresys_t::power_supply_block.inverter.angle_set(types::scale_l(present.angle_req));
				break;
			default:
			break;
			//
			}
			/*
			if ((((mexo_dev_p)_pmsm)->mode & MEXO_PMSM_MODE_SYNCHRO_ID) == MEXO_PMSM_MODE_SYNCHRO_ID) {
				PS_PREFIX(power3ph_rotate_dq)(ps, ps->phase.second);
			}
			else {
				static int _prev_mode = 0;
				PS_PREFIX(power3ph_rotate_dq)(ps, _phase);

				#if POWER_SUPPLY_CURRENT_REGULATOR_ENABLED == 1
				if (((mexo_dev_p)_pmsm)->mode != 0) {
					if (((mexo_dev_p)_pmsm)->mode != _prev_mode) {
						PS_PREFIX(ps3_lat_current_reset)(ps);
						ps->ps_lat.current_req = 0;
					}
					_prev_mode = ((mexo_dev_p)_pmsm)->mode;
					PS_PREFIX(ps3_lat_current_run(ps));
				}
				else {
					PS_PREFIX(ps3_lat_current_reset)(ps);
				}
				#endif
			}
			
		}*/
		}

		void synchro_voltage_mode_start(void) {
			present_s& present = present_cast<present_s>();
			hardwaresys_t::power_supply_block.set_input(&present.actuator.ps.voltage_deseired);
			hardwaresys_t::reconfig();
			on();
		}

		virtual void synchro_voltage_mode_stop(void) {
			off();
			hardwaresys_t::power_supply_block.set_input(nullptr);
		}
		virtual void synchro_voltage_mode_applay_action(void) {
			const action_s& action = action_cast<action_s>();
			present_s& present = present_cast<present_s>();
			present.actuator.ps.voltage_deseired = action.actuator.ps.voltage;
			present.freq_req = action.freq;
			present.angle_req = action.angle;
			hardwaresys_t::power_supply_block.inverter.lat_voltage_set(action.voltage_lateral);
		}

		class synchro_voltage_mode_t :public ::mexo::ps::dev::mode {
		protected:
			dev_t& owner(void) { return owner_cast<dev_t>(); }
			virtual void applay_action(void) {
				owner().synchro_voltage_mode_applay_action();
			}

			virtual void do_start(void) {
				owner().synchro_voltage_mode_start();
			}

			virtual void do_stop(void) {
				owner().synchro_voltage_mode_stop();
			}

		public:
			synchro_voltage_mode_t(int _index, dev_t& _owner) :
				::mexo::ps::dev::mode(_index, RT("mod_sv"), _owner) {}
		} synchro_voltage_mode_;


		#if LAT_CURRENT_REGULATOR_ENABLED == 1
	protected:
		void synchro_current_mode_start(void) {
			present_s& present = present_cast<present_s>();

			hardwaresys_t::power_supply_block.set_input(&present.actuator.ps.voltage_required);
			current_regulator.set_output(&present.actuator.ps.voltage_required);
			current_regulator.set_input(&present.actuator.ps.current_deseired);

			lat_current_regulator.set_output(&hardwaresys_t::power_supply_block.inverter.lat_voltage_ref());
			lat_current_regulator.set_input(&present.lat_current.deseired);

			hardwaresys_t::reconfig();

			current_regulator.reconfig();
			current_regulator.start();
			lat_current_regulator.reconfig();
			lat_current_regulator.start();

			on();

		}

		void synchro_current_mode_stop(void) {
			off();
			current_regulator.stop();
			hardwaresys_t::power_supply_block.set_input(nullptr);
			current_regulator.set_output(nullptr);
			current_regulator.set_input(nullptr);

			lat_current_regulator.stop();
			lat_current_regulator.set_output(nullptr);
			lat_current_regulator.set_input(nullptr);
		}

		virtual void synchro_current_mode_action(void) {
			const action_s& action = action_cast<action_s>();
			present_s& present = present_cast<present_s>();
			present.actuator.ps.current_deseired = 0;


			present.actuator.ps.voltage_range_desired.hi = action.actuator.ps.voltage;
			present.actuator.ps.voltage_range_desired.low = -action.actuator.ps.voltage;
			present.voltage_range_desired.hi = action.actuator.ps.voltage;
			present.voltage_range_desired.low = -action.actuator.ps.voltage;

			present.lat_current.deseired = action.actuator.ps.current;
			
			if (action.actuator.ps.invers) {
				present.freq_req = -action.freq;
				present.angle_req = -action.angle;
			}
			else {
				present.freq_req = action.freq;
				present.angle_req = action.angle;
			}


		}

		class synchro_current_mode_t :public ::mexo::ps::dev::mode {
		protected:
			dev_t& owner(void) { return owner_cast<dev_t>(); }
			virtual void applay_action(void) {
				owner().synchro_current_mode_action();
			}

			virtual void do_start(void) {
				owner().synchro_current_mode_start();
			}

			virtual void do_stop(void) {
				owner().synchro_current_mode_stop();
			}

		public:
			synchro_current_mode_t(int _index, dev_t& _owner) :
				::mexo::ps::dev::mode(_index, RT("mod_sc"), _owner) {}
		} synchro_current_mode_;
		#endif
		protected:
		void do_create_vars(void) {
			actuator_t::do_create_vars();

			if (::mexo::var::machine::actual_mode() >= ::mexo::var::machine::mode::action) {
				const action_s& action = action_cast<action_s>();
				::mexo::var::record::create(types::var::long_signal, action.angle, RT("act.angle"), key(), vars);
				::mexo::var::record::create(types::var::long_signal, action.freq, RT("act.freq"), key(), vars);
				::mexo::var::record::create(types::var::signal, action.voltage_lateral, RT("act.lat.v"), key(), vars);
				const present_s& present = present_cast<present_s>();
				::mexo::var::record::create(types::var::long_signal, present.angle_req, RT("angle_req"), key(), vars);
				::mexo::var::record::create(types::var::long_signal, present.freq_req, RT("freq_req"), key(), vars);
			}
		}


	public:

		dev_t (cstr _name, action_s & _action, config_s& _config, present_s& _present, int _slot_index)
			: actuator_t( _name, _action.actuator, _config.actuator, _present.actuator, _slot_index)
			, inverter_controller(::mexo::machine::slot::kind::control ,this, &dev_t::inverter_controller_run)
			, synchro_voltage_mode_(mode_sync_voltage,*this)
			#if LAT_CURRENT_REGULATOR_ENABLED == 1
			, synchro_current_mode_(mode_sync_current, *this)
			#endif
			#if LAT_CURRENT_FILTER_ENABLED==1
			, lat_current_filter(RT("lc_f"), &hardwaresys_t::prioritet_subsystem, _config.lat_current.filter, _present.lat_current.filter, hardwaresys_t::current_sence_block.lat_current_ref())
			#endif
			#if LAT_CURRENT_FAST_FILTER_ENABLED==1
			, lat_current_filter(RT("lc_f"), &hardwaresys_t::prioritet_subsystem, _config.lat_current.filter, _present.lat_current.filter, hardwaresys_t::current_sence_block.lat_current_ref())
			#endif
			#if LAT_CURRENT_DIFF_FILTER_ENABLED==1
			, lat_current_diff_filter(RT("lc_dif_f"), &hardwaresys_t::prioritet_subsystem, _config.lat_current.diff_filter, _present.lat_current.diff_filter, hardwaresys_t::current_sence_block.lat_current_delta_ref())
			#endif
			#if LAT_CURRENT_REGULATOR_ENABLED == 1
			, lat_current_regulator(
				RT("lc_re")
				, this
				, _config.lat_current.regulator
				, _present.lat_current.regulator
				, _present.voltage_range_desired
				, hardwaresys_t::power_supply_block.actual_satstate()
				, LAT_ACTUAL_SIGNALS
			) 
			#endif

		{
			#if LAT_CURRENT_FILTER_ENABLED==1
			_config.lat_current.filter =
				#define FILTER_GAIN LAT_PREFIX(CURRENT_FILTER_GAIN)
				#define FILTER_SHIFT_GAIN LAT_PREFIX(CURRENT_FILTER_SHIFT_GAIN)
				#define FILTER_SHIFT_PRESC LAT_PREFIX(CURRENT_FILTER_SHIFT_PRESC)
				#define FILTER_SHIFT_VALUE LAT_PREFIX(CURRENT_FILTER_SHIFT_VALUE)
				#include "mexo/filter.templ.settings.inc.hpp"
				;
			#endif
			#if LAT_CURRENT_FAST_FILTER_ENABLED==1
			_config.lat_current.filter =
				{
					{}
					, LAT_PREFIX(CURRENT_FAST_FILTER_SHIFT_VALUE)
				};
			#endif
			#if LAT_CURRENT_DIFF_FILTER_ENABLED == 1
			_config.lat_current.diff_filter =
				#define FILTER_GAIN LAT_PREFIX(CURRENT_DIFF_FILTER_GAIN)
				#define FILTER_SHIFT_GAIN LAT_PREFIX(CURRENT_DIFF_FILTER_SHIFT_GAIN)
				#define FILTER_SHIFT_PRESC LAT_PREFIX(CURRENT_DIFF_FILTER_SHIFT_PRESC)
				#define FILTER_SHIFT_VALUE LAT_PREFIX(CURRENT_DIFF_FILTER_SHIFT_VALUE)
				#include "mexo/filter.templ.settings.inc.hpp"
				;
			#endif
			#if LAT_CURRENT_REGULATOR_ENABLED == 1
			_config.lat_current.regulator =
				#define REGULATOR_PROP_GAIN LAT_PREFIX(CURRENT_PROP_GAIN)
				#define REGULATOR_MODEL_GAIN LAT_PREFIX(CURRENT_MODEL_GAIN)
				#define REGULATOR_DIFF_GAIN LAT_PREFIX(CURRENT_DIFF_GAIN)
				#define REGULATOR_CONTROL_SHIFT LAT_PREFIX(CURRENT_CONTROL_SHIFT)
				#define REGULATOR_MODEL_SHIFT LAT_PREFIX(CURRENT_MODEL_SHIFT)
				#include "mexo/qa.templ.settings.inc.hpp"
				;
			#endif

		}
	};
}

#define TEMPL_FINISH
#include "mexo/pmsm.templ.prepare.hpp"
