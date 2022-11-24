#define TEMPL_BEGIN
#include "mexo/pmsm.templ.prepare.hpp"
#define ACTUATOR_TEMPLATE_NAME PMSM_ACTUATOR_TEMPLATE_SUB_NAME
#define ACTUATOR_PS_TEMPLATE_NAME PMSM_PS_CROSS_TEMPLATE_NAME
#include "mexo/actuator.templ.inc.hpp"
namespace PMSM_TEMPLATE_NAME {

	template <typename types, typename hardwaresys_t>  class dev_t: public PMSM_ACTUATOR_TEMPLATE_SUB_NAME:: dev_t<types,hardwaresys_t> {

	public:
		typedef ::mexo::front::PMSM_TEMPLATE_NAME::action_t<types> action_s;
		typedef ::mexo::front::PMSM_TEMPLATE_NAME::feedback_t<types> feedback_s;
		typedef ::mexo::front::PMSM_TEMPLATE_NAME::mode mode;

		typedef PMSM_ACTUATOR_TEMPLATE_SUB_NAME::dev_t<types, hardwaresys_t> actuator_t;

	private:
		friend class synchro_voltage_mode_t;

		#if LAT_CURRENT_REGULATOR_ENABLED == 1
		typedef ::mexo::controller_task_t <
			::mexo::quazzy_adapt<types>
			, ::mexo::backend_subsystem
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
				::mexo::range_s<typename types::signal_t> voltage_range_desired;
				#endif		

			} lat_current;
		};
	protected:
		#if LAT_CURRENT_MEASSURY_ENABLED == 1
		#if LAT_CURRENT_FILTER_ENABLED==1 || LAT_CURRENT_FAST_FILTER_ENABLED==1
		#if LAT_CURRENT_DIFF_ENABLED==1
		#if LAT_CURRENT_DIFF_FILTER_ENABLED==1
		
		#define LAT_ACTUAL_SIGNALS _present.lat_current.filter.fb.output,_present.lat_current.diff_filter.fb.output
		#else
		#define LAT_ACTUAL_SIGNALS _present.lat_current.filter.fb.output,actuator_t::hardwaresys.current_sence_block.lat_current_delta_ref()
		#endif
		#else
		#define LAT_ACTUAL_SIGNALS _present.lat_current.filter.fb.output,_present.actuator.ps.dummy
		#endif
		#else
		#if LAT_CURRENT_DIFF_ENABLED==1
		#if LAT_CURRENT_DIFF_FILTER_ENABLED==1
		#define LAT_ACTUAL_SIGNALS actuator_t::hardwaresys.current_sence_block.lat_current_ref(),_present.lat_current.diff_filter.fb.output
		#else
		#define LAT_ACTUAL_SIGNALS actuator_t::hardwaresys.current_sence_block.lat_current_ref(),actuator_t::hardwaresys.current_sence_block.lat_current_delta_ref()
		#endif
		#else
	#define LAT_ACTUAL_SIGNALS actuator_t::hardwaresys.current_sence_block.lat_current_ref(),_present.actuator.ps.dummy
		#endif	
		#endif
		#endif


		typedef ::mexo::machine::slot::member<dev_t> inverter_controller_t;
		inverter_controller_t	inverter_controller;
		int old_mode_id_ = mode::idle;
		void inverter_controller_run(void) {
			
			present_s& present = actuator_t::template present<dev_t>();
			
			if (old_mode_id_ != present.actuator.ps.dev.mode) {
				switch (present.actuator.ps.dev.mode) {
				case mode::idle:
					switch (old_mode_id_) {
					#if PMSM_SYNC_CURRENT_MODE_ENABLED ==1
					case mode::sync_current:
					break;
					#endif
					#if PMSM_SYNC_VOLTAGE_MODE_ENABLED ==1
					case mode::sync_voltage:
					break;
					#endif
					#if LAT_CURRENT_MEASSURY_ENABLED == 1
					default:
					lat_current_regulator.stop();
					lat_current_regulator.set_output(nullptr);
					lat_current_regulator.set_input(nullptr);
					#endif
					};
				break;
				#if PMSM_SYNC_CURRENT_MODE_ENABLED ==1
				case mode::sync_current:
				break;
				#endif
				#if PMSM_SYNC_VOLTAGE_MODE_ENABLED ==1
				case mode::sync_voltage:
				break;
				#endif
				default:
				#if LAT_CURRENT_MEASSURY_ENABLED == 1
				present.lat_current.deseired = 0;
				lat_current_regulator.set_output(&actuator_t::hardwaresys.power_supply_block.inverter.lat_voltage_ref());
				lat_current_regulator.set_input(&present.lat_current.deseired);
				lat_current_regulator.reconfig();
				lat_current_regulator.start();
				present.lat_current.voltage_range_desired.hi = types::max;
				present.lat_current.voltage_range_desired.low = -types::max;

				#endif
				break;
				};
				old_mode_id_ = present.actuator.ps.dev.mode;
			}

			switch (present.actuator.ps.dev.mode) {
			case mode::idle:
			break;
			#if PMSM_SYNC_VOLTAGE_MODE_ENABLED ==1
			case mode::sync_voltage:
				present.angle_req += present.freq_req;
				actuator_t::hardwaresys.rotator_block.angle_set(types::scale_l(present.angle_req));
			break;
			#endif
			#if PMSM_SYNC_CURRENT_MODE_ENABLED ==1
			case mode::sync_current:
				present.angle_req += present.freq_req;
				actuator_t::hardwaresys.rotator_block.angle_set(types::scale_l(present.angle_req));
				break;
			#endif
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

		#if PMSM_SYNC_VOLTAGE_MODE_ENABLED ==1
		void synchro_voltage_mode_start(void) {
		present_s& present = actuator_t:: template present<dev_t>();
			actuator_t::hardwaresys.power_supply_block.set_input(&present.actuator.ps.voltage_deseired);
			actuator_t::hardwaresys.reconfig();
			actuator_t::on();
			actuator_t::hardwaresys.rotator_block.off();
		}

		virtual void synchro_voltage_mode_stop(void) {
			actuator_t::off();
			actuator_t::hardwaresys.rotator_block.on();
			actuator_t::hardwaresys.power_supply_block.set_input(nullptr);
		}
		virtual void synchro_voltage_mode_applay_action(void) {
			const action_s& action = actuator_t::template action<dev_t>();
			present_s& present = actuator_t::template present<dev_t>();
			present.actuator.ps.voltage_deseired = action.actuator.ps.voltage;
			present.freq_req = action.freq;
			present.angle_req = action.angle;
			actuator_t::hardwaresys.power_supply_block.inverter.lat_voltage_set(action.lateral.voltage);
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
		#endif

		#if LAT_CURRENT_REGULATOR_ENABLED == 1
	protected:
		#if PMSM_SYNC_CURRENT_MODE_ENABLED ==1
		void synchro_current_mode_start(void) {
			present_s& present = actuator_t::template present<dev_t>();

			actuator_t::hardwaresys.power_supply_block.set_input(&present.actuator.ps.voltage_required);
			actuator_t::current_regulator.set_output(&present.actuator.ps.voltage_required);
			actuator_t::current_regulator.set_input(&present.actuator.ps.current_deseired);

			lat_current_regulator.set_output(&actuator_t::hardwaresys.power_supply_block.inverter.lat_voltage_ref());
			lat_current_regulator.set_input(&present.lat_current.deseired);

			actuator_t::hardwaresys.reconfig();

			actuator_t::current_regulator.reconfig();
			actuator_t::current_regulator.start();
			lat_current_regulator.reconfig();
			lat_current_regulator.start();

			actuator_t::on();
			actuator_t::hardwaresys.rotator_block.off();

		}

		void synchro_current_mode_stop(void) {
			actuator_t::off();
			actuator_t::hardwaresys.rotator_block.on();
			actuator_t::current_regulator.stop();
			actuator_t::hardwaresys.power_supply_block.set_input(nullptr);
			actuator_t::current_regulator.set_output(nullptr);
			actuator_t::current_regulator.set_input(nullptr);

			lat_current_regulator.stop();
			lat_current_regulator.set_output(nullptr);
			lat_current_regulator.set_input(nullptr);
		}

		virtual void synchro_current_mode_action(void) {
			const action_s& action = actuator_t::template action<dev_t>();
			const config_s& config = actuator_t::template config<dev_t>();
			present_s& present =  actuator_t::template present<dev_t>();
			present.actuator.ps.current_deseired = 0;


			present.actuator.ps.voltage_range_desired.hi = action.actuator.ps.voltage;
			present.actuator.ps.voltage_range_desired.low = -action.actuator.ps.voltage;
			present.lat_current.voltage_range_desired.hi = action.lateral.voltage;
			present.lat_current.voltage_range_desired.low = -action.lateral.voltage;

			present.lat_current.deseired = action.lateral.current;
			present.actuator.ps.current_deseired = action.actuator.ps.current;
			
			if (config.actuator.ps.invers) {
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
			dev_t& owner(void) { return mode::owner_cast<dev_t>(); }
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
		#endif
		protected:
			
		#if ROBO_APP_MEXO_VAR_ENABLED == 1
		void do_create_vars(void) {
			actuator_t::do_create_vars();

			#if PMSM_SYNC_VOLTAGE_MODE_ENABLED ==1 || PMSM_SYNC_CURRENT_MODE_ENABLED ==1
			if (::mexo::var::machine::actual_mode() >= ::mexo::var::machine::mode::action) {
				const action_s& act = action<dev_t>();
				::mexo::var::record::create(types::var::long_signal, act.angle, RT("act.angle"), key(), vars);
				::mexo::var::record::create(types::var::long_signal, act.freq, RT("act.freq"), key(), vars);
				::mexo::var::record::create(types::var::signal, act.lateral.voltage, RT("act.lat.v"), key(), vars);
				::mexo::var::record::create(types::var::signal, act.lateral.current, RT("act.lat.c"), key(), vars);
				const present_s& prsnt = present<dev_t>();
				::mexo::var::record::create(types::var::long_signal, prsnt.angle_req, RT("angle_req"), key(), vars);
				::mexo::var::record::create(types::var::long_signal, prsnt.freq_req, RT("freq_req"), key(), vars);
			}
			#endif
		}
		#endif
		
	public:

		dev_t (hardwaresys_t &  _hardwaresys, cstr _name, action_s & _action, feedback_s& _feedback, config_s& _config, present_s& _present, int _slot_index)
			: actuator_t(_hardwaresys, _name, _action.actuator, _feedback.actuator, _config.actuator, _present.actuator, _slot_index)
			, inverter_controller(::mexo::machine::slot::kind::backend ,*this, &dev_t::inverter_controller_run)
			#if PMSM_SYNC_VOLTAGE_MODE_ENABLED ==1
			, synchro_voltage_mode_(mode::sync_voltage,*this)
			#endif
			#if LAT_CURRENT_REGULATOR_ENABLED == 1
			#if PMSM_SYNC_CURRENT_MODE_ENABLED ==1
			, synchro_current_mode_(mode::sync_current, *this)
			#endif
			#endif
			#if LAT_CURRENT_FILTER_ENABLED==1
			, lat_current_filter(RT("lc_f"), &actuator_t::hardwaresys.prioritet_subsystem, _config.lat_current.filter, _present.lat_current.filter, actuator_t::hardwaresys.current_sence_block.lat_current_ref())
			#endif
			#if LAT_CURRENT_FAST_FILTER_ENABLED==1
			, lat_current_filter(RT("lc_f"), &actuator_t::hardwaresys.prioritet_subsystem, _config.lat_current.filter, _present.lat_current.filter, actuator_t::hardwaresys.current_sence_block.lat_current_ref())
			#endif
			#if LAT_CURRENT_DIFF_FILTER_ENABLED==1
			, lat_current_diff_filter(RT("lc_dif_f"), &actuator_t::hardwaresys.prioritet_subsystem, _config.lat_current.diff_filter, _present.lat_current.diff_filter, actuator_t::hardwaresys.current_sence_block.lat_current_delta_ref())
			#endif
			#if LAT_CURRENT_REGULATOR_ENABLED == 1
			, lat_current_regulator(
				RT("lc_re")
				, this
				, _config.lat_current.regulator
				, _present.lat_current.regulator
				, _present.lat_current.voltage_range_desired
				, actuator_t::hardwaresys.power_supply_block.actual_satstate()
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
