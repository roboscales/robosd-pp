#ifndef powerdc_hpp
#define powerdc_hpp

#include "burst++/modules/actor.hpp"
#include "burst++/math.hpp"

namespace burst {
	template<typename number> struct powerdc {
		using signal_t = typename number::signal_t;
		using usignal_t = typename number::usignal_t;
		using long_signal_t = typename number::long_signal_t;
		using ulong_signal_t = typename number::ulong_signal_t;
		static inline signal_t& standby = burst::standby<signal_t>();

		class inverter : public actor {
		protected:
			signal_t* voltage = &standby;
		public:
			struct deform_s {
				bool enabled;
				signal_t level;
				long_signal_t hi_gain_16;
				long_signal_t lo_gain_16;
				long_signal_t lo_bevel_16;
			};

			struct config_s {
				actor::config_s tag;
				signal_t duty_max;
				signal_t pwm_force;
				deform_s deform;
			};


			#define POWERDC_INVERTER_CONFIG(a) POWERDC_INVERTER_CONFIG_(a)
			#define POWERDC_INVERTER_CONFIG_(a)\
			{\
				ACTOR_CONFIG(a)\
				,a##_DUTY_MAX\
				,a##_PWM_FORCE\
				,{\
					a##_DEFORM_ENABLED\
					,a##_DEFORM_LEVEL\
					,a##_DEFORM_HI_GAIN_16\
					,a##_DEFORM_LO_GAIN_16\
					,a##_DEFORM_LO_BEVEL_16\
				}\
			}

			struct present_s {
				actor::present_s tag;
				signal_t duty;
				signal_t pwm;
			};

			long_signal_t scale_gain;
			signal_t discret_lo;
			signal_t discret_hi;
			long_signal_t pwm_force;
			#if ROBO_APP_BURST_VARTREE_ENABLED
			virtual void do_regvar_present(void) {
			}
			virtual void do_regvar_conf(void) {
			}
			#endif
			
			signal_t scale_(long_signal_t _signal) {
				long_signal_t tmp = scale_gain * ( (long_signal_t)_signal - number::min + pwm_force );
				tmp += (1 << 15);
				tmp = robo::digit::rsh(tmp, 16);
				tmp += discret_lo;
				return (signal_t)saturate( tmp, discret_lo, discret_hi );
			}

			long_signal_t deform_pwm_(signal_t _src) {
				ACTOR_CONFIG_S(c);
				if (_src > c.deform.level) {
					return (robo::digit::rsh((c.deform.lo_gain_16 * _src + c.deform.lo_bevel_16), 16));
				}
				else if (_src < -c.deform.level) {
					return  -(robo::digit::rsh((c.deform.lo_gain_16 * (-_src) + c.deform.lo_bevel_16), 16));
				}
				else {
					return  (robo::digit::rsh(c.deform.hi_gain_16 * _src, 16));
				}
			}
			
			virtual void run(void) {
				ACTOR_CONFIG_S(c);
				ACTOR_PRESENT_S(p);
				long_signal_t pwm = *voltage;
				if (c.deform.enabled) {
					pwm = deform_pwm_(pwm);
				} else {
					if (pwm_force > 0) {
						if (pwm > 0) {
							pwm += pwm_force;
						} else if (pwm < 0) {
							pwm -= pwm_force;
						}
					}
				}
				p.pwm = number::s_sat(pwm);
				p.duty = scale_(pwm);
			}

			virtual void begin(void) {
				ACTOR_CONFIG_S(cfg);
				discret_hi = cfg.duty_max;
				discret_lo = -cfg.duty_max;
				pwm_force = cfg.pwm_force;
				long_signal_t gain = (long_signal_t)( 2 * cfg.duty_max );
				gain = robo::digit::lsh(gain, 16);
				gain += ((long_signal_t)number::max - number::min + 2* pwm_force) / 2; //округление
				gain /= ((long_signal_t)number::max - number::min + 2* pwm_force);
				scale_gain = gain;
			}
			virtual void finish(void) {}

			inverter(const config_s& _config, present_s& _present)
				: actor(_config.tag, _present.tag){
				connect(nullptr);
			};
			inverter(const config_s& _config, present_s& _present, subsystem& _subsystem)
				: actor(_config.tag, _present.tag, _subsystem) {
				connect(nullptr);
			};
			virtual void connect(signal_t* _voltage) {
				connectto(voltage, _voltage);
			}
		};
	};
}
#endif