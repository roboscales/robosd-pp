#ifndef burst_sp_hpp
#define burst_sp_hpp

#include "burst++/modules/actor.hpp"
#include "burst++/vartree.hpp"

namespace burst {
	namespace ps {
		enum class commands { off = 0, on = 1 };
		enum class statuses { unknown, off, boot, on, shutdown };

		class control : public actor {
		public:

			struct config_s {
				actor::config_s tag;
			};
			#define PS_CONFIG(a) PS_CONFIG_(a)
			#define PS_CONFIG_(a)\
			{\
				ACTOR_CONFIG(a)\
			}
			struct present_s {
				actor::present_s tag;
				commands command = commands::off;
				statuses status = statuses::unknown;
				satstates satstate = satstates::both;
			};

			control(const config_s& _config, present_s& _present)
				: actor(_config.tag, _present.tag) {};
			control(const config_s& _config, present_s& _present, subsystem& _subsystem)
				: actor(_config.tag, _present.tag, _subsystem) {};

			bool active(void) {
				return present<present_s>().status == statuses::on;
			}
			const satstates& satstate(void) {
				return present<present_s>().satstate;
			}
			void on(void) {
				present<present_s>().command = commands::on;
			}
			void off(void) {
				present<present_s>().command = commands::off;
			}

			virtual void finish(void) {
				present<present_s>().status = statuses::unknown;
			}
			virtual void begin(void) {
				actor::template present<present_s>().status = statuses::off;
			}
			#if ROBO_APP_BURST_VARTREE_ENABLED
			virtual void do_regvar_present(void) {
				using namespace burst::var;
				if (actual_mode >= mode::action) {
					ACTOR_PRESENT_S(p);
					reg(types::uint8, p.command, RT("cmd"));
					if (actual_mode >= mode::full) {
						reg(types::const_uint8, p.status, RT("stat"));
						reg(types::const_uint8, p.satstate, RT("sat"));
					}
				}
			}
			virtual void do_regvar_conf(void) {
			}
			#endif		
		};
		template< typename A >  struct dummy_driver_t {
			static void boot_begin() {};
			static bool do_boot() { return true; };
			static void boot_complete(void) {}
			static satstates set_pwm(const A & /*_pwm*/) { return satstates::none; }
			static void shutdown_begin(void) {}
			static bool do_shutdown(void) { return true; }
			static void shutdown_complete(void) {}
		};
		template<typename driver >  class machine_t : public control {
			using A = typename driver::duty_t;
		protected:
			A* pwm = &standby<A>();
		public:

			machine_t(const config_s& _config, present_s& _present)
				: control(_config, _present) {};
			machine_t(const config_s& _config, present_s& _present, subsystem& _subsystem)
				: control(_config, _present, _subsystem) {};

			virtual void run(void) {
				ACTOR_PRESENT_S(p);
				switch (p.status) {
					case statuses::unknown:
					break;
					case  statuses::off:
					if (p.command == commands::on) {
						driver::boot_begin();
						p.status = statuses::boot;
					}
					else {
						break;
					}

					case statuses::boot:
					if (driver::do_boot()) {
						p.satstate = satstates::none;
						driver::boot_complete(/*present.inverter.duty*/);
						p.satstate = driver::set_pwm(*pwm);
						p.status = statuses::on;
						return;
					}
					else {
						break;
					}
					case statuses::on:
					if (p.command == commands::on) {
						p.satstate = driver::set_pwm(*pwm);
						return;
					}
					else {
						p.satstate = satstates::both;
						p.status = statuses::shutdown;
						driver::shutdown_begin();
					}
					case statuses::shutdown:
					if (driver::do_shutdown()) {
						driver::shutdown_complete();
						p.status = statuses::off;
					}
					else {
						break;
					}
				}
			}
			void connect(A* _pwm) {
				connectto(pwm, _pwm);
			}
			virtual void begin(void) {
				present<present_s>().status = statuses::off;
			}

			virtual void finish(void) {
				pwm = nullptr;
				present<present_s>().status = statuses::unknown;
			}

		};

	}
	
}
#endif