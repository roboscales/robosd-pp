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

		struct config_s {
			typename actuator_t::config_s actuator;
		};

		
		struct present_s {
			typename actuator_t::present_s actuator;
			typename types::long_signal_t angle_req;
			typename types::long_signal_t freq_req;
		};
		enum { mode_sync = 32, mode_sync_voltage = 33, mode_sync_current = 34 };
	private:
		friend class synchro_voltage_mode_t;

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
				hardwaresys.pwm_block().inverter.angle_set(types::scale_l(present.angle_req));
			break;
			case mode_sync_current:
			break;
				present.angle_req += present.freq_req;
				hardwaresys.pwm_block().inverter.angle_set(types::scale_l(present.angle_req));
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

		void voltage_freq_mode_start(void) {
			present_s& present = present_cast<present_s>();
			hardwaresys.pwm_block().set_input(&present.actuator.ps.voltage_deseired);
			hardwaresys.reconfig();
			on();
		}

		virtual void voltage_mode_stop(void) {
			off();
			hardwaresys.pwm_block().set_input(nullptr);
		}
		virtual void voltage_mode_applay_action(void) {
			const action_s& action = action_cast<action_s>();
			present_s& present = present_cast<present_s>();
			present.actuator.ps.voltage_deseired = action.actuator.ps.voltage;
			present.freq_req = action.freq;
			present.angle_req = action.angle;
			hardwaresys.pwm_block().inverter.lateral_voltage_set(action.voltage_lateral);
		}

		class synchro_voltage_mode_t :public ::mexo::ps::dev::mode {
		protected:
			dev_t& owner(void) { return owner_cast<dev_t>(); }
			virtual void applay_action(void) {
				owner().voltage_mode_applay_action();
			}

			virtual void do_start(void) {
				owner().voltage_freq_mode_start();
			}

			virtual void do_stop(void) {
				owner().voltage_mode_stop();
			}

		public:
			synchro_voltage_mode_t(int _index, dev_t& _owner) :
				::mexo::ps::dev::mode(_index, RT("mod_sv"), _owner) {}
		} synchro_voltage_mode_;
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

		dev_t (hardwaresys_t& _hardwaresys, cstr _name, action_s & _action, config_s& _config, present_s& _present, int _slot_index)
			: actuator_t(_hardwaresys, _name, _action.actuator, _config.actuator, _present.actuator, _slot_index)
			, inverter_controller(::mexo::machine::slot::kind::backend ,this, &dev_t::inverter_controller_run)
			, synchro_voltage_mode_(mode_sync_voltage,*this)
		{

		}
	};
}

#define TEMPL_FINISH
#include "mexo/pmsm.templ.prepare.hpp"
