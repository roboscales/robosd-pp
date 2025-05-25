#ifndef burst_modules_brakeR2_hpp
#define burst_modules_brakeR2_hpp
#include "burst++/burst.hpp"
#include "burst++/math.hpp"
#include "burst++/modules/brake.hpp"


namespace burst{
	
template <class number, class D> class brake_time_machineR2_t : public brake
	{
	public:
		using parameter_t = typename number::parameter_t;
		using signal_t = typename number::signal_t;
		using long_signal_t = typename number::long_signal_t;
		

		struct config_s {
			typename D::value_t force;
			typename D::value_t normal;
			::robo::time_us_t relax_us;
			::robo::time_us_t force_us;
		};

		#define BRAKER2_CONFIG(a) BRAKER2_CONFIG_(a)
		#define BRAKER2_CONFIG_(a)\
		{\
				a##_REQ_FORCE\
				,a##_REQ_NORMAL\
				,a##_TIMEOUT_RELAX_US\
				,a##_TIMEOUT_FORCE_US\
		}

		struct present_s {
			commands command;
			statuses status;
		};

		virtual void set(void) { present_.command = commands::set; }
		virtual void release(void) { present_.command = commands::release; }
		virtual bool released(void) { return present_.status == statuses::released; }
		virtual bool fixed(void) { return present_.status == statuses::fixed; }


	private:
		const config_s& config_;
		present_s& present_;
		robo::time_us_t last_us_ = 0;
	
		
	public:
		void poll(void) {
			#if ROBO_APP_ENV_ENABLED ==1
			::robo::time_us_t now_us_ = ::robo::system::time_us();

			switch (present_.status) {
			case statuses::fixed:
			if (present_.command == commands::release) {
				present_.status = statuses::releasing;
				D::on(config_.force);
				last_us_ = now_us_;
			}
			else {
				break;
			}

			case statuses::releasing:
			if (present_.command == commands::set) {
				D::off();
				present_.status = statuses::fixing;
				last_us_ = now_us_;
			}
			else {
				if ((now_us_ - last_us_) >= config_.force_us) {
					present_.status = statuses::released;
					D::on(config_.normal);
				}
			}
			break;

			case statuses::released:
			if (present_.command == commands::set) {
				D::off();
				present_.status = statuses::fixing;
				last_us_ = now_us_;
			}
			else {
				break;
			}

			case statuses::fixing:
			if ((now_us_ - last_us_) >= config_.relax_us) {
				present_.status = statuses::fixed;
			}
			break;
			}
			#endif
		}

		brake_time_machineR2_t(
			const config_s& _config
			, present_s& _present
		) : config_(_config)
			, present_(_present)
		{
		}
		#if ROBO_APP_BURST_VARTREE_ENABLED == 1
		virtual void regvar_present(robo::cstr _name) {
			using namespace burst::var;
			push(_name);{
				if (actual_mode >= var::mode::full) {

					reg(types::uint8, present_.commans, RT("c"));
					reg(types::uint8, present_.status, RT("s"));

				}
			} pop();
		}

		virtual void regvar_conf(robo::cstr _name) {
			using namespace burst::var;
			push(_name);{
				if (actual_mode >= var::mode::tuning) {

					reg(types::time_us, config_.relax_us, RT("relax_us"));
					reg(types::time_us, config_.force_us, RT("force_us"));

				}
			} pop();
		}
		#endif

	};

}
#endif