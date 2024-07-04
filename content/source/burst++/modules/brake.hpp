#ifndef burst_modules_brake_hpp
#define burst_modules_brake_hpp
#include "burst++/burst.hpp"
#include "burst++/math.hpp"
#include "burst++/modules/filter.hpp"
#include "burst++/modules/ps.hpp"
#include "burst++/modules/brake.front.hpp"

#ifndef BURST_PANICS_ACTUATOR_TEMPER_ENABLED
#define BURST_PANICS_ACTUATOR_TEMPER_ENABLED 0
#endif

namespace burst{
	class brake {
	public:
		enum class statuses {
			fixed = 0,
			releasing = 1,
			released = 2,
			fixing = 3
		};

		enum class commands {
			set = 0,
			release = 1
		};

	public:
		virtual void set(void) = 0;
		virtual void release(void) = 0;
		virtual bool released(void) = 0;
		virtual bool fixed(void) = 0;
	};

	template <class number> class brake_time_machine_t : public brake
	{
	public:
		using parameter_t = typename number::parameter_t;
		using signal_t = typename number::signal_t;
		using long_signal_t = typename number::long_signal_t;
		typedef pi_t<number>  pi_t;

		struct config_s {
			struct {
				signal_t force;
				signal_t normal;
				range_s<signal_t> range;
			} pwm;
			struct {
				signal_t force;
				signal_t normal;
				typename pi_t::config_s pi;
			} current;

			struct {
				::robo::time_us_t relax_us;
				::robo::time_us_t force_us;
			} timeout;
			uint8_t mode;
		};

		#define BRAKE_CONFIG(a) BRAKE_CONFIG_(a)
		#define BRAKE_CONFIG_(a)\
		{\
			{\
				a##_PWM_FORCE\
				,a##_PWM_NORMAL\
				, BURST_RANGE_CONFIG(a##_PWM_RANGE)\
			}\
			,{\
				a##_CURRENT_FORCE\
				,a##_CURRENT_NORMAL\
				, PI_CONFIG(a##_PI)\
			}\
			,{\
				a##_TIMEOUT_RELAX_US\
				,a##_TIMEOUT_FORCE_US\
			}\
			, burst::front::brake::modes::a##_MODE\
		}

		struct present_s {
			commands command;
			statuses status;

			struct {
				signal_t req;
			} pwm;
			struct {
				signal_t req;
				typename pi_t::present_s pi;
			} current;
		};

		virtual void set(void) { present_.command = commands::set; }
		virtual void release(void) { present_.command = commands::release; }
		virtual bool released(void) { return present_.status == statuses::released; }
		virtual bool fixed(void) { return present_.status == statuses::fixed; }

	protected:
		ps::control& psc;

	private:
		pi_t pi_;
		const config_s& config_;
		present_s& present_;
		robo::time_us_t last_us_ = 0;
		void brake_force_(void) {
			switch (config_.mode) {
			case burst::front::brake::modes::dummy:
			break;
			case burst::front::brake::modes::pwm:
			present_.pwm.req = config_.pwm.force;
			break;
			case burst::front::brake::modes::current:
			present_.current.req = config_.current.force;
			break;
			}
		}
		void brake_normal_(void) {
			switch (config_.mode) {
			case burst::front::brake::modes::dummy:
			break;
			case burst::front::brake::modes::pwm:
			present_.pwm.req = config_.pwm.normal;
			break;
			case burst::front::brake::modes::current:
			present_.current.req = config_.current.normal;
			break;
			}
		}
	public:
		void poll(void) {
			#if ROBO_APP_ENV_ENABLED ==1
			::robo::time_us_t now_us_ = ::robo::system::time_us();

			switch (present_.status) {
			case statuses::fixed:
			if (present_.command == commands::release) {
				present_.status = statuses::releasing;
				brake_force_();
				psc.on();
				last_us_ = now_us_;
			}
			else {
				break;
			}

			case statuses::releasing:
			if (present_.command == commands::set) {
				psc.off();
				present_.status = statuses::fixing;
				last_us_ = now_us_;
			}
			else {
				if ((now_us_ - last_us_) >= config_.timeout.force_us) {
					present_.status = statuses::released;
					brake_normal_();
				}
			}
			break;

			case statuses::released:
			if (present_.command == commands::set) {
				psc.off();
				present_.status = statuses::fixing;
				last_us_ = now_us_;
			}
			else {
				break;
			}

			case statuses::fixing:
			if ((now_us_ - last_us_) >= config_.timeout.relax_us) {
				present_.status = statuses::fixed;
			}
			break;
			}
			#endif
		}

		brake_time_machine_t(
			const config_s& _config
			, present_s& _present
			, ps::control& _ps
			, const signal_t& _current
		) : config_(_config)
			, present_(_present)
			, psc(_ps)
			, pi_(
				_config.current.pi
				, _present.current.pi
				, _present.current.req
				, _current
				, nullptr
				, nullptr
				, _config.pwm.range.lo
				, _config.pwm.range.hi
				, _present.pwm.req
				, _ps.satstate()
			) {
		}
		#if ROBO_APP_BURST_VARTREE_ENABLED == 1
		virtual void regvar_present(robo::cstr _name) {
			using namespace burst::var;
			push(_name);{
			if (actual_mode >= var::mode::full) {
				push(RT("v"));
				reg(number::var::signal, present_.pwm.req, RT("req"));
				pop();
				push(RT("c"));
				reg(number::var::signal, present_.current.req, RT("req"));
				pop();
			}
			} pop();
		}

		virtual void regvar_conf(robo::cstr _name) {
			using namespace burst::var;
			push(_name);{
				if (actual_mode >= var::mode::tuning) {

					push(RT("v"));
					reg(number::var::signal, config_.pwm.force, RT("force"));
					reg(number::var::signal, config_.pwm.normal, RT("normal"));
					reg(number::var::signal, config_.pwm.range, RT("range"));
					pop();

					push(RT("c"));
					reg(number::var::signal, config_.current.force, RT("force"));
					reg(number::var::signal, config_.current.normal, RT("normal"));
					pi_t::regvar_config(RT("pi"), config_.current.pi);
					pop();

					push(RT("tm"));
					reg(types::time_us, config_.timeout.relax_us, RT("relax_us"));
					reg(types::time_us, config_.timeout.force_us, RT("force_us"));
					pop();

					reg(uint8, config_.mode, RT("mode"));

				}
			} pop();
		}
		#endif

	};
}
#endif