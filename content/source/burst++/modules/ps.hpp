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
			#if ROBO_APP_ULTRACOMPACT == 0
			control(const config_s& _config, present_s& _present, subsystem& _subsystem)
				: actor(_config.tag, _present.tag, _subsystem) {};
			#endif
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
		template< typename A >  class dummy_wrapper_driver_t {
			protected:
			void boot_begin() {};
			bool do_boot() { return true; };
			void boot_complete(void) {}
			satstates set_pwm(const A & /*_pwm*/) { return satstates::none; }
			void shutdown_begin(void) {}
			bool do_shutdown(void) { return true; }
			void shutdown_complete(void) {}
		};
		
		template< typename A >  class driver_t{
		public:
			driver_t(void){};
			driver_t(const driver_t & _driver)= delete;
			void operator=( const driver_t& ) = delete;
			using duty_s = A;
			virtual void boot_begin() {};
			virtual bool do_boot() { return true; };
			virtual void boot_complete(void) {}
			virtual satstates set_pwm(const duty_s & /*_pwm*/) { return satstates::none; }
			virtual void shutdown_begin(void) {}
			virtual bool do_shutdown(void) { return true; }
			virtual void shutdown_complete(void) {}
		};

		template< typename A >   class driver_ref_t{
		private:
			driver_t<A> & instance_;
		public:
			using duty_t = A ;
			void boot_begin() { instance_.boot_begin(); };
			bool do_boot() { return instance_.do_boot(); };
			void boot_complete(void) {instance_.boot_complete();}
			satstates set_pwm(const duty_t & _pwm) { return instance_.set_pwm(_pwm); }
			void shutdown_begin(void) { instance_.shutdown_begin();}
			bool do_shutdown(void) { return instance_.do_shutdown(); }
			void shutdown_complete(void) { instance_.shutdown_complete();}
			driver_ref_t(driver_t<A>  & _instance):instance_(_instance) {}
		};
		
		template<typename driver >  class machine_t : public control, public  driver{
		public:
			using duty_s = typename driver::duty_t;
		protected:
			#if ROBO_APP_ULTRACOMPACT == 0
			duty_s * pwm = &standby<duty_s>();
			#else
			duty_s & pwm;
			#endif
		public:
			#if ROBO_APP_ULTRACOMPACT == 0
			machine_t(const config_s& _config, present_s& _present, Arg... arg)
				: driver(arg...)
				, control(_config, _present) {};
			machine_t(const config_s& _config, present_s& _present, subsystem& _subsystem, Arg... arg)
				: driver(arg...)
				, control(_config, _present, _subsystem) {};
			#else
			template<typename...Arg> machine_t(const config_s& _config, present_s& _present, duty_s & _pwm, Arg... arg)
				: control(_config, _present)
				, driver(arg...)
				, pwm(_pwm) {};
			#endif
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
						#if ROBO_APP_ULTRACOMPACT == 0
						p.satstate = driver::set_pwm(*pwm);
						#else
						p.satstate = driver::set_pwm(pwm);
						#endif
						p.status = statuses::on;
						return;
					}
					else {
						break;
					}
					case statuses::on:
					if (p.command == commands::on) {
						#if ROBO_APP_ULTRACOMPACT == 0
						p.satstate = driver::set_pwm(*pwm);
						#else
						p.satstate = driver::set_pwm(pwm);
						#endif
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
			#if ROBO_APP_ULTRACOMPACT == 0
			void connect(duty_s * _pwm) {
				connectto(pwm, _pwm);
			}
			#endif
			virtual void begin(void) {
				present<present_s>().status = statuses::off;
			}

			virtual void finish(void) {
				#if ROBO_APP_ULTRACOMPACT == 0
				pwm = nullptr;
				#endif
				present<present_s>().status = statuses::unknown;
			}

		};

	}
	template <typename A> class ps_t: public  ps::machine_t<ps::driver_ref_t<A> >{
		using B =  ps::machine_t<ps::driver_ref_t<A> >;		
		public:
			using instance_s = ps::driver_t<A>; 
			#if ROBO_APP_ULTRACOMPACT == 0
			ps_t(const typename B::config_s& _config, typename  B::present_s& _present, driver_s & _driver)
				: B(_config, _present,_driver) {};
			ps_t(const typename B::config_s& _config, typename  B::present_s& _present, subsystem& _subsystem, driver_s & _driver)
				: B(_config, _present,_subsystem, _driver) {};
			#else
		ps_t(const typename B::config_s& _config, typename  B::present_s& _present, typename B::duty_s & _duty , instance_s & _instance)
				: B(_config, _present,_duty,std::ref(_instance)) {};
			#endif
	};
	
}
#endif