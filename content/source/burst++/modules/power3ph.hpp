#ifndef power3ph_hpp
#define power3ph_hpp

#include "burst++/modules/actor.hpp"
#include "burst++/math.hpp"
#ifndef BURST_PANICS_ACWC_OVERCURRENT_REALTIME_ENABLED
#define BURST_PANICS_ACWC_OVERCURRENT_REALTIME_ENABLED 1
#endif
namespace burst {
	template<typename number> struct powe3ph {
		using signal_t = typename number::signal_t;
		using discret_t = typename number::discret_t;
		//using usignal_t = typename number::usignal_t;
		using long_signal_t = typename number::long_signal_t;
		using ulong_signal_t = typename number::ulong_signal_t;
		using long_discret_t = typename number::long_discret_t;
		#if ROBO_APP_ULTRACOMPACT == 0
		static inline signal_t& standby = burst::standby<signal_t>();		
		#endif

		struct abc_s {
			signal_t A;
			signal_t B;
			signal_t C;
		};
		struct discret_abc_s {
			discret_t A;
			discret_t B;
			discret_t C;
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
		protected:
		#if ROBO_APP_ULTRACOMPACT == 0
			signal_t* synchro_anglee = nullptr;
		#else
			signal_t & synchro_anglee;
		#endif
		public:
			typedef actor::config_s config_s;
			#define POWER3PH_ROTATOR_CONFIG(a) ACTOR_CONFIG(a)

			struct present_s {
				actor::present_s tag;
				rot_s rot;
				struct {
					signal_t electro;
					signal_t actual;
				} angle;
				bool synchro;
			};
			void switch_to_synchro(void) {
				ACTOR_PRESENT_S(p);
				p.synchro = true;
			}
			void switch_to_enco(void) {
				ACTOR_PRESENT_S(p);
				p.synchro = false;
			}
		#if ROBO_APP_ULTRACOMPACT == 0
			rotator_t(const config_s& _config, present_s& _present)
				: actor(_config, _present.tag) {
			};
			rotator_t(const config_s& _config, present_s& _present, subsystem& _subsystem)
				: actor(_config, _present.tag, _subsystem) {
			};
			void connect(signal_t* _synchro) {
				connectto(synchro_anglee, _synchro);
			}
			#else
			rotator_t(const config_s& _config, present_s& _present, signal_t & _synchro_anglee )
				: actor(_config, _present.tag),synchro_anglee(_synchro_anglee) {
			}
			#endif

			#if ROBO_APP_BURST_VARTREE_ENABLED
			virtual void do_regvar_present(void) {
				using namespace burst::var;
				if (actual_mode >= mode::full) {
					ACTOR_PRESENT_S(p);
					push(RT("rot"));
					{
						reg(var::types::const_uint8, p.synchro, RT("synchro"));
						reg(number::var::const_signal, p.rot.sn, RT("sn"));
						reg(number::var::const_signal, p.rot.cs, RT("cs"));
					} pop();
					push(RT("angle"));
					{
						reg(number::var::const_signal, p.angle.electro, RT("electro"));
						reg(number::var::const_signal, p.angle.actual, RT("actual"));
					} pop();
				}
			}
			virtual void do_regvar_conf(void) {}
			#endif	
		};

		class enco32_adapter_t : public rotator_t {
		protected:
		#if ROBO_APP_ULTRACOMPACT == 0
			uint32_t * angle;
		#else
			uint32_t & angle;
		#endif
		public:
			struct config_s {
				typename rotator_t::config_s tag;
				bool inverce;
				discret_t offset;
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
			#if ROBO_APP_ULTRACOMPACT == 0
			enco32_adapter_t(const config_s& _config, present_s& _present)
				: rotator_t(_config.tag, _present) {
				connect(nullptr);
			};
			enco32_adapter_t(const config_s& _config, present_s& _present, subsystem& _subsystem)
				: rotator_t(_config.tag, _present, _subsystem) {
				connect(nullptr, nullptr);
			};
			void connect(uint32_t* _angle, signal_t * _synchro) {
				connectto(angle, _angle);
				rotator_t::connect(_synchro);
			}
			#else
			enco32_adapter_t(const config_s& _config, present_s& _present, signal_t & _synchro_anglee , uint32_t & _mech_anglee )
				: rotator_t(_config.tag, _present,_synchro_anglee) , angle(_mech_anglee){
			};
			#endif
			virtual void run(void) {
				ACTOR_PRESENT_S(p);
				ACTOR_CONFIG_S(c);
				#if ROBO_APP_ULTRACOMPACT == 0
				discret_t tmp = number::ul2discret(*angle);
				#else
				discret_t tmp = number::ul2discret(angle);
				#endif
				
				if (c.inverce) tmp = -tmp;

				tmp *= c.pole_count;
				tmp -= c.offset;
				p.angle.actual = number::discret2rad(tmp);

				if (p.synchro) {
					#if ROBO_APP_ULTRACOMPACT == 0
					p.angle.electro = *rotator_t::synchro_anglee;
					#else
					p.angle.electro = rotator_t::synchro_anglee;
					#endif
				} else {
					p.angle.electro = p.angle.actual;
				}
				//todo можно сократить на одно умножение
				p.rot.sn = number::sin(p.angle.electro);
				p.rot.cs = number::cos(p.angle.electro);
			}
			#if ROBO_APP_BURST_VARTREE_ENABLED
			virtual void do_regvar_present(void) {
				rotator_t::do_regvar_present();
			}

			

			virtual void do_regvar_conf(void) {
				using namespace burst::var;
				rotator_t::do_regvar_conf();
				if (actual_mode >= mode::tuning) {
					ACTOR_CONFIG_S(p);
					reg( number::var::discret, p.offset, RT("offset"));
					if (actual_mode >= mode::config) {
						reg(types::uint8, p.inverce, RT("inv"));
						reg(types::uint8, p.pole_count, RT("pole"));
					}
				}
			}
			#endif	
		};
		

		class hall_adapter_t : public rotator_t {
		protected:
			const signal_t & angle_;
		public:
			typedef typename rotator_t::config_s config_s;
			typedef typename rotator_t::present_s present_s;

			#define POWER3PH_HALL_ADAPTR_CONFIG(a) POWER3PH_ROTATOR_CONFIG(a)
			#if ROBO_APP_ULTRACOMPACT == 0
			hall_adapter_t(const config_s& _config, present_s& _present, const signal_t & _angle)
				: rotator_t(_config, _present), angle_(_angle) {
					rotator_t::connect(nullptr);
			};
			hall_adapter_t(const config_s& _config, present_s& _present, const signal_t & _angle, subsystem& _subsystem)
				: rotator_t(_config, _present, _subsystem) , angle_(_angle) {
				rotator_t::connect(nullptr);
			};
			#else
			hall_adapter_t(const config_s& _config, present_s& _present, signal_t & _synchro_anglee, const signal_t & _angle )
				: rotator_t(_config, _present, _synchro_anglee), angle_(_angle) {
			};
			#endif
			virtual void run(void) {
				ACTOR_PRESENT_S(p);
				p.angle.actual = angle_;

				if (p.synchro) {
					#if ROBO_APP_ULTRACOMPACT == 0
					p.angle.electro = *rotator_t::synchro_anglee;
					#else
					p.angle.electro = rotator_t::synchro_anglee;
					#endif
				} else {
					p.angle.electro = p.angle.actual;
				}
				p.rot.sn = number::sin(p.angle.electro);
				p.rot.cs = number::cos(p.angle.electro);
			}
			#if ROBO_APP_BURST_VARTREE_ENABLED
			virtual void do_regvar_present(void) {
				rotator_t::do_regvar_present();
			}

			virtual void do_regvar_conf(void) {
				rotator_t::do_regvar_conf();
			}
			#endif	
		};
		class inverter : public actor {
		protected:
			rotator_t & rotator;
			#if ROBO_APP_ULTRACOMPACT == 0
			signal_t* cross;
			signal_t* lateral;			
			#else
			signal_t & cross;
			signal_t & lateral;			
			#endif
		public:
		#if ROBO_APP_ULTRACOMPACT == 0
			struct deform_s {
				bool enabled;
				signal_t level;
				long_signal_t hi_gain_16;
				long_signal_t lo_gain_16;
				long_signal_t lo_bevel_16;
			};
			#endif
			struct config_s {
				actor::config_s tag;
				range_s<discret_t> native;
				#if ROBO_APP_ULTRACOMPACT == 0
				signal_t pwm_force;
				deform_s deform;
				#endif
			};


			#define POWER3PH_INVERTER_CONFIG(a) POWER3PH_INVERTER_CONFIG_(a)
			#if ROBO_APP_ULTRACOMPACT == 0
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
			#else
			#define POWER3PH_INVERTER_CONFIG_(a)\
			{\
				ACTOR_CONFIG(a)\
				, BURST_RANGE_CONFIG(a##_NATIVE_RANGE)\
			}
			#endif
			struct present_s {
				actor::present_s tag;
				discret_abc_s duty;
				abc_s pwm;
				ab_s ab;
				dq_s dq;
				uint8_t swm;
			};

			long_discret_t scale_gain;
			discret_t discret_lo;
			discret_t discret_hi;
			discret_t discret_delta_lo;
			discret_t discret_delta_hi;
			//long_signal_t pwm_force;
			#if ROBO_APP_BURST_VARTREE_ENABLED
			virtual void do_regvar_present(void) {
				using namespace burst::var;
				ACTOR_PRESENT_S(p);
				if (actual_mode >= mode::full) {
					//abc_s duty;
					push(RT("duty"));
					reg(number::var::const_discret, p.duty.A, RT("A"));
					reg(number::var::const_discret, p.duty.B, RT("B"));
					reg(number::var::const_discret, p.duty.C, RT("C"));
					pop();
					//abc_s pwm;
					push(RT("pwm"));
					reg(number::var::const_signal, p.pwm.A, RT("A"));
					reg(number::var::const_signal, p.pwm.B, RT("B"));
					reg(number::var::const_signal, p.pwm.C, RT("C"));
					pop();
					//ab_s ab;
					push(RT("ab"));
					reg(number::var::const_signal, p.ab.alfa, RT("alfa"));
					reg(number::var::const_signal, p.ab.beta, RT("beta"));
					pop();
					//dq_s dq;
					push(RT("dq"));
					reg(number::var::const_signal, p.dq.lateral, RT("lat"));
					reg(number::var::const_signal, p.dq.cross, RT("cross"));
					pop();
					//uint8_t swm;
					reg(types::const_uint8, p.swm, RT("swm"));
					//reg(number::var::long_signal, pwm_force, RT("force"));

				}
			}
			virtual void do_regvar_conf(void) {
				using namespace burst::var;
				if (actual_mode >= mode::tuning) {
					ACTOR_CONFIG_S(c);
					if (actual_mode >= mode::config) {
					#if ROBO_APP_ULTRACOMPACT == 0
						reg(number::var::signal, c.pwm_force, RT("force"));
						push(RT("deform"));
						reg(types::uint8, c.deform.enabled, RT("en"));
						reg(number::var::signal, c.deform.level, RT("lvl"));
						reg(number::var::long_signal, c.deform.hi_gain_16, RT("hi_g"));
						reg(number::var::long_signal, c.deform.lo_gain_16, RT("lo_g"));
						reg(number::var::long_signal, c.deform.lo_bevel_16, RT("lo_b"));
						pop();
						#endif
						push(RT("native"));
						reg(number::var::discret, c.native.lo, RT("lo"));
						reg(number::var::discret, c.native.hi, RT("hi"));
						pop();
					}
				}
			}
			#endif
			

			#if ROBO_APP_ULTRACOMPACT == 0
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
			#endif
			virtual void run(void) {
				ACTOR_CONFIG_S(c);
				ACTOR_PRESENT_S(p);
				#if ROBO_APP_ULTRACOMPACT == 0
				p.dq.lateral = *lateral;
				p.dq.cross = *cross;
				#else
				p.dq.lateral = lateral;
				p.dq.cross = cross;
				#endif
				//todo для защиты целочисленных вычислений - получакется, что амплитуда dq != амплитуде ab
				long_signal_t lateral = number::lsf::mult(p.dq.lateral, number::sqrt2_div_2);
				long_signal_t cross = number::lsf::mult(p.dq.cross, number::sqrt2_div_2);
				//auto lateral = p.dq.lateral;
				//auto cross = p.dq.cross;
				ACTOR_ALIEN_PRESENT_S(rotator_t, rotator, rot);
				p.ab.alfa = number::lsf::dot(rot.rot.cs, lateral, -rot.rot.sn, cross);
				p.ab.beta = number::lsf::dot(rot.rot.sn, lateral, rot.rot.cs, cross);

				long_signal_t pwmA;
				long_signal_t pwmB;
				long_signal_t pwmC;

				p.swm = 1;

				long_signal_t x, y, z;
				long_signal_t v2 = number::lsf::div2(p.ab.beta);
				x = p.ab.beta;
				y = number::lsf::sum_x_ya(v2, p.ab.alfa, number::sqrt3_div_2);
				z = number::lsf::sum_x_ya(v2, p.ab.alfa, -number::sqrt3_div_2);
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
				pwmA += number::lsf::mult(pwmA, number::scale);
				pwmB += number::lsf::mult(pwmB, number::scale);
				pwmC += number::lsf::mult(pwmC, number::scale);
				#if ROBO_APP_ULTRACOMPACT == 0
				if (c.deform.enabled) {
					pwmA = number::lsf::sat_s(pwmA);
					pwmB = number::lsf::sat_s(pwmB);
					pwmC = number::lsf::sat_s(pwmC);
					p.pwm.A = (signal_t)pwmA;
					p.pwm.B = (signal_t)pwmB;
					p.pwm.C = (signal_t)pwmC;
					pwmA = deform_pwm_(pwmA);
					pwmB = deform_pwm_(pwmB);
					pwmC = deform_pwm_(pwmC);
					pwmA = number::lsf::sat_s(pwmA);
					pwmB = number::lsf::sat_s(pwmB);
					pwmC = number::lsf::sat_s(pwmC);
				}
				else 
				#endif
					{
				#if ROBO_APP_ULTRACOMPACT == 0
					auto pwm_force = c.pwm_force;
					if (pwm_force > 0) {
						long_signal_t lo = number::min + pwm_force;
						long_signal_t hi = number::max - pwm_force;
						pwmA = number::lsf::sat(pwmA,lo,hi);
						pwmB = number::lsf::sat(pwmB,lo, hi);
						pwmC = number::lsf::sat(pwmC,lo, hi);

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
					else 
					#endif
					{

						pwmA = number::lsf::sat_s(pwmA);
						pwmB = number::lsf::sat_s(pwmB);
						pwmC = number::lsf::sat_s(pwmC);

						p.pwm.A = (signal_t)pwmA;
						p.pwm.B = (signal_t)pwmB;
						p.pwm.C = (signal_t)pwmC;
					}
				}

				p.duty.A = scale_(number::lsf::todiscret(pwmA));
				p.duty.B = scale_(number::lsf::todiscret(pwmB));
				p.duty.C = scale_(number::lsf::todiscret(pwmC));
			}

			virtual void begin(void) {
				ACTOR_CONFIG_S(cfg);
				discret_hi = cfg.native.hi;
				discret_lo = cfg.native.lo;
				discret_t delta = cfg.native.hi - cfg.native.lo;
				long_discret_t gain = delta;
				gain = robo::digit::lsh(gain, number::discret_bits+1);
				auto tmp = ((long_discret_t)number::discret_max - number::discret_min);
				gain += tmp/2; //округление
				gain /= tmp;
				scale_gain = gain;
				discret_delta_lo = 0;
				discret_delta_hi = delta;
			}
			discret_t scale_(long_discret_t _signal) {
				long_discret_t tmp = scale_gain * (_signal - number::discret_min);
				tmp += (1 << 15);
				tmp = robo::digit::rsh(tmp, 16);

				if (tmp < discret_delta_lo) {
					return discret_lo;
				}
				else if (tmp > discret_delta_hi) {
					return discret_hi;
				}
				else {
					return discret_lo + (discret_t)tmp;
				}
			}
			virtual void finish(void) {}
			#if ROBO_APP_ULTRACOMPACT == 0

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
			#else
			inverter(const config_s& _config, present_s& _present, rotator_t& _rotator,	signal_t & _cross,	signal_t & _lateral)
				: actor(_config.tag, _present.tag), rotator(_rotator), cross(_cross),	lateral(_lateral){
			};
			#endif
		};

		struct cursencor :public actor {
		protected:
			rotator_t & rotator;
			#if ROBO_APP_ULTRACOMPACT != 0
			inverter & inv;
			#endif
		public:
			struct config_s {
				actor::config_s tag;
				#if ROBO_APP_ULTRACOMPACT == 0
				int adc_index[3];
				struct {
					long_signal_t matrix[9];
					bool enable;
				} deform;
				#endif
			};

			#if ROBO_APP_ULTRACOMPACT == 0
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
			#else
			#define POWER3PH_CURSENSOR_CONFIG(a) POWER3PH_CURSENSOR_CONFIG_(a)
			#define POWER3PH_CURSENSOR_CONFIG_(a)\
			{\
				ACTOR_CONFIG(a)\
			}
			#endif

			struct present_s {
				actor::present_s tag;
				abc_s abc;
				ab_s ab;
				dq_s dq;
				#if BURST_PROTECTION_ENABLED == 1
				#if BURST_PANICS_ACWC_OVERCURRENT_ENABLED ==1
				signal_t magnitude;
				#endif
				#endif
			};
			#if ROBO_APP_ULTRACOMPACT == 0
			struct {
				signal_t* A = &standby;
				signal_t* B = &standby;
				signal_t* C = &standby;
			} raw;
			#else
			struct raw_s{
				signal_t & A;
				signal_t & B;
				signal_t & C;
				raw_s(signal_t & _A, signal_t & _B, signal_t & _C):A(_A), B(_B),C(_C){}
			} raw;
			#endif
			#if ROBO_APP_BURST_VARTREE_ENABLED
			virtual void do_regvar_present(void) {
				using namespace burst::var;
				ACTOR_PRESENT_S(p);
				if (actual_mode >= mode::full) {
					push(RT("abc"));
					reg(number::var::const_signal, p.abc.A, RT("A"));
					reg(number::var::const_signal, p.abc.B, RT("B"));
					reg(number::var::const_signal, p.abc.C, RT("C"));
					pop();
					//ab_s ab;
					push(RT("ab"));
					reg(number::var::const_signal, p.ab.alfa, RT("alfa"));
					reg(number::var::const_signal, p.ab.beta, RT("beta"));
					pop();
					//dq_s dq;
					push(RT("dq"));
					reg(number::var::const_signal, p.dq.lateral, RT("lat"));
					reg(number::var::const_signal, p.dq.cross, RT("cross"));
					pop();
				}
			}
			virtual void do_regvar_conf(void) {
				using namespace burst::var;
				ACTOR_CONFIG_S(c);
				if (actual_mode >= mode::config) {
					#if ROBO_APP_ULTRACOMPACT == 0
					push(RT("adc_ix")); {
						reg(types::uint8, c.adc_index[0], RT("0"));
						reg(types::uint8, c.adc_index[1], RT("1"));
						reg(types::uint8, c.adc_index[2], RT("2"));
						pop(); {
							push(RT("deform"));
							reg(types::uint8, c.deform.enable, RT("en"));
							push(RT("matrix"));
							reg(number::var::long_signal, c.deform.matrix[0], RT("0"));
							reg(number::var::long_signal, c.deform.matrix[1], RT("1"));
							reg(number::var::long_signal, c.deform.matrix[2], RT("2"));
							reg(number::var::long_signal, c.deform.matrix[3], RT("3"));
							reg(number::var::long_signal, c.deform.matrix[4], RT("4"));
							reg(number::var::long_signal, c.deform.matrix[5], RT("5"));
							reg(number::var::long_signal, c.deform.matrix[6], RT("6"));
							reg(number::var::long_signal, c.deform.matrix[7], RT("7"));
							reg(number::var::long_signal, c.deform.matrix[8], RT("8"));
						}pop();
					} pop();
					#endif
				}
			}
			#endif
			const typename number::long_signal_t* deform = nullptr;
			virtual void begin(void) {
				ACTOR_CONFIG_S(cfg);
				#if ROBO_APP_ULTRACOMPACT == 0
				if (cfg.deform.enable) {
					deform = cfg.deform.matrix;
				}
				else {
					deform = nullptr;
				}
				#endif
			}
			#if ROBO_APP_ULTRACOMPACT == 0
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
			#else
			cursencor(const config_s& _config, present_s& _present, rotator_t& _rotator, inverter & _inverter,signal_t & _adc0, signal_t & _adc1, signal_t & _adc2)
				: actor(_config.tag, _present.tag),inv(_inverter), rotator(_rotator) , raw( _adc0, _adc1, _adc2){};
			#endif
			#if BURST_PROTECTION_ENABLED == 1
			#if BURST_PANICS_ACWC_OVERCURRENT_REALTIME_ENABLED ==0
			void update_magnituse(void){
				ACTOR_PRESENT_S(p);
				p.magnitude = number::sqrt( ( ulong_signal_t) ( (long_signal_t)p.dq.lateral * p.dq.lateral + (long_signal_t)p.dq.cross * p.dq.cross));
			}
			#endif
			#endif
			virtual void run(void) {
				ACTOR_CONFIG_S(cfg);
				long_signal_t a;
				long_signal_t b;
				long_signal_t c;
				#if ROBO_APP_ULTRACOMPACT == 0
				if (deform) {
					signal_t A = *raw.A;
					signal_t B = *raw.B;
					signal_t C = *raw.C;
					a = robo::digit::rsh ( deform[0] * A + deform[1] * B + deform[2] * C, 15);
					b = robo::digit::rsh ( deform[3] * A + deform[4] * B + deform[5] * C, 15);
					c = robo::digit::rsh ( deform[6] * A + deform[7] * B + deform[8] * C, 15);
					long_signal_t ofs = robo::digit::rsh( (a + b + c) * number::one_div_3, 15 );
					a -= ofs;
					b -= ofs;
					c -= ofs;
				}
				else 
				#endif
				{
					#if ROBO_APP_ULTRACOMPACT == 0
					a = *raw.A;
					b = *raw.B;
					c = *raw.C;
					#else
					ACTOR_ALIEN_PRESENT_S(inverter,inv, pinv);
					switch(pinv.swm){
					case 2:
					case 3: 
						a = raw.A;
						b = - raw.A - raw.C;
						c = raw.C; 
						break;
					case 4: 
					case 5: 
						a = raw.A;
						b = raw.B;
						c = - raw.A - raw.B;
						break;        
					case 1:
					case 6:
					default:
						a = - raw.B - raw.C;
						b = raw.B;
						c = raw.C; 
						break;
					}
					#endif
				}
				long_signal_t beta =  number::lsf::mult ((b * 2 + a) , number::one_div_sqrt3  );
				ACTOR_ALIEN_PRESENT_S(rotator_t,rotator, rot);
				//typename inverter::present_s& ip = inverter_.actor::template present<typename inverter::present_s>();
				signal_t sn = rot.rot.sn;
				signal_t cs = rot.rot.cs;
				ACTOR_PRESENT_S(p);
				signal_t dql = number::lsf::dot(cs, a, sn, beta);
				signal_t dqc = number::lsf::dot(-sn, a, cs, beta);
				p.dq.lateral = dql;
				p.dq.cross = dqc;
				p.ab.alfa = a;
				p.ab.beta = beta;
				p.abc.A = a;
				p.abc.B = b;
				p.abc.C = c;
				#if BURST_PROTECTION_ENABLED == 1
				#if BURST_PANICS_ACWC_OVERCURRENT_ENABLED ==1

				#if BURST_PANICS_ACWC_OVERCURRENT_REALTIME_ENABLED ==1
				auto tmp =  (long_signal_t)dql * dql;
				auto tmp2 =	(long_signal_t)dqc * dqc;
				p.magnitude = number::s_sqrt( ( ulong_signal_t) (tmp+tmp2 ));
				#endif
				#endif
				#endif
			}
		};

	};
}
#endif