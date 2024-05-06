#ifndef power3ph_hpp
#define power3ph_hpp

#include "burst++/modules/actor.hpp"
#include "burst++/math.hpp"

namespace burst {
	template<typename number> struct powe3ph {
		using signal_t = typename number::signal_t;
		using long_signal_t = typename number::long_signal_t;
		constexpr static signal_t sqrt3_div_2 = number::frac( robo::csqrt<double>(3.0) / 2);// number::round(robo::csqrt<double>(3.0) / 2 * number::max);
		constexpr static signal_t scale = number::frac( robo::csqrt<double>(2.0) -1 );
		constexpr static signal_t sqrt2_div_2 = number::frac( robo::csqrt<double>(2.0) / 2 );
		constexpr static signal_t one_div_3 = number::frac(1./ 3);
		constexpr static signal_t one_div_sqrt3 = number::frac( 1. / robo::csqrt<double>(3.0) );
		static inline signal_t& standby = burst::standby<signal_t>();
		static inline long_signal_t  mult_(long_signal_t x1, long_signal_t x2) {
			return  fast::rsh(x1 * x2 ,15);
		}
		//static inline long_signal_t fast::rsh
		static inline long_signal_t dot_(long_signal_t _x1, long_signal_t _y1, long_signal_t _x2, long_signal_t _y2) {
			return fast::rsh(_x1 * _y1 + _x2 * _y2,15);
		}

		static inline long_signal_t sum_x_ya_(long_signal_t x, long_signal_t y, long_signal_t a) {
			long_signal_t tmp = ((long_signal_t)y) * a;
			tmp = fast::rsh(tmp , 15);
			tmp += x;
			return tmp;
		}
		static inline long_signal_t l_sat_s(const long_signal_t& _x) {
			return saturate(_x, number::min, number::max);
		}
		static inline long_signal_t l_sat_s(const long_signal_t& _x, signal_t _lo, signal_t _hi) {
			return saturate(_x, _lo, _hi);
		}


		struct abc_s {
			signal_t A;
			signal_t B;
			signal_t C;
		};

		struct ab_s {
			long_signal_t alfa;
			long_signal_t beta;
		};

		struct dq_s {
			signal_t lateral;
			signal_t cross;
		};

		struct rot_s {
			signal_t sn;
			signal_t cs;
		};

		class rotator_t : public actor {
		public:
			typedef actor::config_s config_s;
			#define POWER3PH_ROTATOR_CONFIG(a) ACTOR_CONFIG(a)

		struct present_s {
				actor::present_s tag;
				rot_s rot;
				signal_t angle;
			};
			rotator_t(const config_s& _config, present_s& _present)
				: actor(_config, _present.tag) {
			};
			rotator_t(const config_s& _config, present_s& _present, subsystem& _subsystem)
				: actor(_config, _present.tag, _subsystem) {
			};
		};

		class enco32_adapter_t : public rotator_t {
		protected:
			uint32_t * angle;
		public:
			struct config_s {
				typename rotator_t::config_s tag;
				bool inverce;
				signal_t offset;
				uint8_t pole_count;
			};
			#define POWER3PH_ENCO32_ADAPTER_CONFIG(a) POWER3PH_ENCO32_ADAPTER_CONFIG_(a)
			#define POWER3PH_ENCO32_ADAPTER_CONFIG_(a)\
			{\
				ACTOR_CONFIG(a)\
				,a##_INVERCE\
				,a##_OFFSET\
				,a##_POLE_COUNT\
			}
			typedef typename rotator_t::present_s present_s;

			enco32_adapter_t(const config_s& _config, present_s& _present)
				: rotator_t(_config.tag, _present) {
				connect(nullptr);
			};
			enco32_adapter_t(const config_s& _config, present_s& _present, subsystem& _subsystem)
				: rotator_t(_config.tag, _present, _subsystem) {
				connect(nullptr);
			};
			virtual void connect(uint32_t* _angle) {
				connectto(angle, _angle);
			}
			virtual void run(void) {
				ACTOR_PRESENT_S(p);
				ACTOR_CONFIG_S(c);
				long_signal_t tmp = (long_signal_t)*angle;
				if (c.inverce) tmp = -tmp;
				tmp *= c.pole_count;
				tmp -= c.offset;
				p.angle = tmp;
				p.rot.sn = number::sin(tmp);
				p.rot.cs = number::cos(tmp);
			}
		};

		class inverter : public actor {
		protected:
			rotator_t & rotator;
			signal_t* cross;
			signal_t* lateral;			
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
				range_s<long_signal_t> native;
				long_signal_t pwm_force;
				deform_s deform;
			};


			#define POWER3PH_INVERTER_CONFIG(a) POWER3PH_INVERTER_CONFIG_(a)
			#define POWER3PH_INVERTER_CONFIG_(a)\
			{\
				ACTOR_CONFIG(a)\
				, BURST_RANGE_CONFIG(a##_NATIVE_RANGE)\
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
				abc_s duty;
				abc_s pwm;
				ab_s ab;
				dq_s dq;
				uint8_t swm;
			};

			long_signal_t scale_gain;
			signal_t discret_lo;
			signal_t discret_hi;
			signal_t discret_delta_lo;
			signal_t discret_delta_hi;
			long_signal_t pwm_force;
			const deform_s* deform;

			signal_t scale_(signal_t _signal) {
				long_signal_t tmp = scale_gain * ((long_signal_t)_signal - number::min);
				tmp += (1 << 15);
				tmp = fast::rsh(tmp, 16);

				if (tmp < discret_delta_lo) {
					return discret_lo;
				}
				else if (tmp > discret_delta_hi) {
					return discret_hi;
				}
				else {
					return discret_lo + tmp;
				}
			}

			long_signal_t deform_pwm_(signal_t _src) {
				if (_src > deform->level) {
					return (fast::rsh((deform->lo_gain_16 * _src + deform->lo_bevel_16), 16));
				}
				else if (_src < -deform->level) {
					return  -(fast::rsh((deform->lo_gain_16 * (-_src) + deform->lo_bevel_16), 16));
				}
				else {
					return  (fast::rsh(deform->hi_gain_16 * _src, 16));
				}
			}
			virtual void run(void) {
				ACTOR_PRESENT_S(p);
				p.dq.lateral = *lateral;
				p.dq.cross = *cross;
				long_signal_t lateral = mult_(p.dq.lateral, sqrt2_div_2);
				long_signal_t cross = mult_(p.dq.cross, sqrt2_div_2);
				ACTOR_ALIEN_PRESENT_S(rotator_t, rotator, rot);
				p.ab.alfa = dot_(rot.rot.cs, lateral, -rot.rot.sn, cross);
				p.ab.beta = dot_(rot.rot.sn, lateral, rot.rot.cs, cross);

				long_signal_t pwmA;
				long_signal_t pwmB;
				long_signal_t pwmC;

				p.swm = 1;

				long_signal_t x, y, z;
				long_signal_t v2 = p.ab.beta >> 1;
				x = p.ab.beta;
				y = sum_x_ya_(v2, p.ab.alfa, sqrt3_div_2);
				z = sum_x_ya_(v2, p.ab.alfa, -sqrt3_div_2);
				if (y < 0) {
					if (z < 0) {
						pwmA = y - z;
						pwmB = pwmA + 2 * z;
						pwmC = pwmA - 2 * y;
						p.swm = 5;
					}
					else {
						if (x > 0) {
							pwmA = -x + y;
							pwmC = pwmA - 2 * y;
							pwmB = pwmC + 2 * x;
							p.swm = 3;
						}
						else {
							pwmA = x - z;
							pwmB = pwmA + 2 * z;
							pwmC = pwmB - 2 * x;
							p.swm = 4;
						}
					}
				}
				else {
					if (z < 0) {
						if (x > 0) {
							pwmA = x - z;
							pwmB = pwmA + 2 * z;
							pwmC = pwmB - 2 * x;
							p.swm = 1;
						}
						else {
							pwmA = -x + y;
							pwmC = pwmA - 2 * y;
							pwmB = pwmC + 2 * x;
							p.swm = 6;
						}
					}
					else {
						pwmA = y - z;
						pwmB = pwmA + 2 * z;
						pwmC = pwmA - 2 * y;
						p.swm = 2;
					}
				}
				//* sqrt(2) 
				pwmA += mult_(pwmA, scale);
				pwmB += mult_(pwmB, scale);
				pwmC += mult_(pwmC, scale);

				if (deform->enabled) {
					pwmA = l_sat_s(pwmA);
					pwmB = l_sat_s(pwmB);
					pwmC = l_sat_s(pwmC);
					p.pwm.A = (signal_t)pwmA;
					p.pwm.B = (signal_t)pwmB;
					p.pwm.C = (signal_t)pwmC;
					pwmA = deform_pwm_(pwmA);
					pwmB = deform_pwm_(pwmB);
					pwmC = deform_pwm_(pwmC);
					pwmA = l_sat_s(pwmA);
					pwmB = l_sat_s(pwmB);
					pwmC = l_sat_s(pwmC);
				}
				else {
					//long_signal_t pwm_force = cfg.pwm_force;
					if (pwm_force > 0) {
						long_signal_t lo = number::min + pwm_force;
						long_signal_t hi = number::max - pwm_force;
						pwmA = l_sat_s(pwmA,lo,hi);
						pwmB = l_sat_s(pwmB,lo, hi);
						pwmC = l_sat_s(pwmC,lo, hi);

						p.pwm.A = (signal_t)pwmA;
						p.pwm.B = (signal_t)pwmB;
						p.pwm.C = (signal_t)pwmC;

						if (pwmA > 0) {
							pwmA += pwm_force;
						}
						if (pwmB > 0) {
							pwmB += pwm_force;
						}
						if (pwmC > 0) {
							pwmC += pwm_force;
						}

						if (pwmA < 0) {
							pwmA -= pwm_force;
						}
						if (pwmB < 0) {
							pwmB -= pwm_force;
						}
						if (pwmC < 0) {
							pwmC -= pwm_force;
						}

					}
					else {

						pwmA = l_sat_s(pwmA);
						pwmB = l_sat_s(pwmB);
						pwmC = l_sat_s(pwmC);

						p.pwm.A = (signal_t)pwmA;
						p.pwm.B = (signal_t)pwmB;
						p.pwm.C = (signal_t)pwmC;
					}
				}

				p.duty.A = scale_(pwmA);
				p.duty.B = scale_(pwmB);
				p.duty.C = scale_(pwmC);
			}

			virtual void begin(void) {
				ACTOR_CONFIG_S(cfg);
				discret_hi = cfg.native.hi;
				discret_lo = cfg.native.lo;
				pwm_force = cfg.pwm_force;
				long_signal_t delta = cfg.native.hi - cfg.native.lo;
				long_signal_t gain = (long_signal_t)(cfg.native.hi - cfg.native.lo);
				gain = fast::lsh(gain, 16);
				gain += ((long_signal_t)number::max - number::min) / 2; //округление
				gain /= ((long_signal_t)number::max - number::min);
				scale_gain = gain;
				discret_delta_lo = 0;//-_config->native.lo;
				discret_delta_hi = delta;
				deform = &cfg.deform;
			}
			virtual void finish(void) {}

			inverter(const config_s& _config, present_s& _present, rotator_t& _rotator)
				: actor(_config.tag, _present.tag), rotator(_rotator){
				connect(nullptr, nullptr);
			};
			inverter(const config_s& _config, present_s& _present, rotator_t& _rotator, subsystem& _subsystem)
				: actor(_config.tag, _present.tag, _subsystem), rotator(_rotator) {
				connect(nullptr, nullptr);
			};
			virtual void connect(signal_t* _cross, signal_t* _lateral) {
				connectto(cross, _cross);
				connectto(lateral, _lateral);
			}
		};

		struct cursencor :public actor {
		protected:
			rotator_t & rotator;
		public:
			struct config_s {
				actor::config_s tag;
				int adc_index[3];
				struct {
					long_signal_t matrix[9];
					bool enable;
				} deform;
			};

			#define POWER3PH_CURSENSOR_CONFIG(a) POWER3PH_CURSENSOR_CONFIG_(a)
			#define POWER3PH_CURSENSOR_CONFIG_(a)\
			{\
				ACTOR_CONFIG(a)\
				,a##_ADC_INDEX\
				,{\
					a##_DEFORM\
					,a##_DEFORM_ENABLE\
				}\
			}

			struct present_s {
				actor::present_s tag;
				abc_s abc;
				ab_s ab;
				dq_s dq;
			};
			struct {
				signal_t* A = &standby;
				signal_t* B = &standby;
				signal_t* C = &standby;
			} raw;
			const long_signal_t* deform = nullptr;

			virtual void begin(void) {
				ACTOR_CONFIG_S(cfg);
				if (cfg.deform.enable) {
					deform = cfg.deform.matrix;
				}
				else {
					deform = nullptr;
				}
			}
			cursencor(const config_s& _config, present_s& _present, rotator_t& _rotator, inverter & _inverter)
				: actor(_config.tag, _present.tag), rotator(_rotator) {};
			cursencor(const config_s& _config, present_s& _present, rotator_t& _rotator, subsystem& _subsystem)
				: actor(_config.tag, _present.tag, _subsystem), rotator(_rotator) {};
			void connect(signal_t * _adc) {
				ACTOR_CONFIG_S(cfg);
				connectto( raw.A , _adc + cfg.adc_index[0] );
				connectto( raw.B , _adc + cfg.adc_index[1] );
				connectto(raw.C , _adc + cfg.adc_index[2]);
			}
			virtual void run(void) {
				long_signal_t a;
				long_signal_t b;
				long_signal_t c;
				if (deform) {
					signal_t A = *raw.A;
					signal_t B = *raw.B;
					signal_t C = *raw.C;
					a = fast::rsh ( deform[0] * A + deform[1] * B + deform[2] * C, 15);
					b = fast::rsh ( deform[3] * A + deform[4] * B + deform[5] * C, 15);
					c = fast::rsh ( deform[6] * A + deform[7] * B + deform[8] * C, 15);
					long_signal_t ofs = fast::rsh( (a + b + c) * one_div_3, 15 );
					a -= ofs;
					b -= ofs;
					c -= ofs;
				}
				else {
					a = *raw.A;
					b = *raw.B;
					c = *raw.C;
				}
				long_signal_t beta =  fast::rsh ((b * 2 + a) * one_div_sqrt3 , 15 );
				ACTOR_ALIEN_PRESENT_S(rotator_t,rotator, rot);
				//typename inverter::present_s& ip = inverter_.actor::template present<typename inverter::present_s>();
				signal_t sn = rot.rot.sn;
				signal_t cs = rot.rot.cs;
				ACTOR_PRESENT_S(p);
				p.dq.lateral = dot_(cs, a, sn, beta);
				p.dq.cross = dot_(-sn, a, cs, beta);
				p.ab.alfa = a;
				p.ab.beta = beta;
				p.abc.A = a;
				p.abc.B = b;
				p.abc.C = c;

			}
		};

	};
}
#endif