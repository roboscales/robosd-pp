/*
 * stspin32_ps.hpp
 *
 *  Created on: Aug 19, 2026
 *      Author: a.boiko
 */

#ifndef STSPIN32_PS_HPP_
#define STSPIN32_PS_HPP_


#include "burst++/modules/ps.hpp"
#include "burst++/modules/actor.hpp"

namespace burst
{
	namespace ps
	{
		template <typename A> struct stspin32_driver_t
		{
			static void boot_begin() {};
			static bool do_boot() { return true; };
			static void boot_complete(void) {}
			static satstates set_pwm(const A & /*_pwm*/) { return satstates::none; }
			static void shutdown_begin(void) {}
			static bool do_shutdown(void) { return true; }
			static void shutdown_complete(void) {}
			static bool state_query(uint8_t &);
			static bool fault_query(uint8_t &);
			static int & get_state_instance();
		};
		template <class driver> class stspin32_t : public machine_t<driver>
		{
		public:

			struct present_s
			{
				typename machine_t<driver>::present_s super;
				uint8_t driver_state;
				uint8_t fault_state;
			};

			using duty_s = typename driver::duty_t;
			using super = machine_t<driver>;
			stspin32_t(const typename super::config_s& _config, present_s& _present)
			: super(_config, _present) {}
			stspin32_t(const typename super::config_s& _config, present_s& _present, subsystem& _subsystem)
			: super(_config, _present.super, _subsystem) {}

			virtual void run(void)
			{
				ACTOR_PRESENT_S(p);
				driver::state_query(p.driver_state);
				driver::fault_query(p.fault_state);
				super::run();
			}

			void connect(duty_s * _pwm)
			{
				super::connect(_pwm);
			}

			virtual void begin(void)
			{
				super::begin();
			}

			virtual void finish(void)
			{
				super::finish();
			}

		};
	}
}




#endif /* STSPIN32_PS_HPP_ */
