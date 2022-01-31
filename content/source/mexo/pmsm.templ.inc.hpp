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
			
		};

	protected:
		void voltage_freq_mode_start(void) {

			present_s& present = present_cast<present_s>();

			hardwaresys.pwm_block().set_input(&present.voltage_required);
			hardwaresys.reconfig();
			on();
		}

		virtual void voltage_mode_stop(void) {
			off();
			hardwaresys.pwm_block().set_input(nullptr);
		}
		friend class synchro_voltage_mode_t;

	public:
		class synchro_voltage_mode_t :public ::mexo::ps::dev::mode {
		protected:
			dev_t& owner(void) { return owner_cast<dev_t>(); }
			long_signal_t angle = (types::long_signal_t)0;
			long_signal_t freq = (types::long_signal_t)0;
			virtual void applay_action(void) {
				const action_s& action = owner().template action_cast<action_s>();
				present_s& present = owner().template present_cast<present_s>();
				action_s& action = owner().template action_cast<action_s>();
				present.voltage_deseired = action.voltage;
				freq = action.freq;
				angle = action.angle;
				hardwaresys.pwm_block().inverter.angle_set(types::scale_l(angle) );
				hardwaresys.pwm_block().inverter.lateral_voltage_set(action.voltage_lateral);
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
		};

	public:

		dev_t (hardwaresys_t& _hardwaresys, cstr _name, action_s & _action, config_s& _config, present_s& _present, int _slot_index)
			: actuator_t(_hardwaresys, _name, _action.actuator, _config.actuator, _present.actuator, _slot_index)
		{

		}
	};
}

#define TEMPL_FINISH
#include "mexo/pmsm.templ.prepare.hpp"
