#ifndef mexo_ps_hpp
#define mexo_ps_hpp
#include "mexo/mexo.hpp"
#include "mexo/math.hpp"
#include "mexo/adc.hpp"
#include "mexo/regulator.hpp"

namespace mexo {
	namespace ps {

		class  control {
		public:
			enum class command { on = 1, off = 0 };
			enum class status { configure = 0, off = 1, boot = 2, on = 3, shutdown = 4 };
		protected:
			command command_ = command::off;
			status status_ = status::configure;
		public:
			void enable(void) { if (status_ == status::configure) status_ = status::off; }
			void on(void) { command_ = command::on; }
			void set(command _command) { command_ = _command; }
			void off(void) { command_ = command::off; }
			bool active(void) { return status_ == status::on; }
			control(void) {}
			virtual ~control(void) {}
		};

		class dev :public ::mexo::dev {
			control& control_;
		public:
			dev(cstr  _name, action_s& _action, present_s& _present, control& _control)
				: ::mexo::dev(_name, _action, _present)
				, control_(_control) {}
			void enable(void) { control_.enable(); }
			void on(void) { control_.on(); }
			void set(control::command _command) { control_.set(_command); }
			void off(void) { control_.off(); }
			bool active(void) { return control_.active(); }
			class mode : public ::mexo::dev::mode {
			public:
				dev& owner(void) { return owner_cast<dev>(); }
				mode(int _index, cstr  _name, dev& _dev)
					: ::mexo::dev::mode(_index, _name, _dev) {}
				void power_enable(void) { owner().enable(); }
				void power_on(void) { owner().on(); }
				void power_set(control::command _command) { owner().set(_command); }
				void power_off(void) { owner().off(); }
				bool power_active(void) { return owner().active(); }
			};

		};

		template < typename C > class pwm
			: public ps::control
			, public finall_controller_handler< typename C::inverter::signal_t> {
			typedef finall_controller_handler<  typename C::inverter::signal_t> A;
		public:
			typedef typename C::inverter inverter_t;
			typedef typename inverter_t::types types;
			struct config_s {
				typename A::config_s cb;				
				range_s < typename inverter_t::discret_t > duty;
				range_s < typename inverter_t::signal_t > voltage;
			};
			struct present_s {
				typename A::present_s cb;
				typename inverter_t::present_s inverter;
			};
			typename inverter_t inverter;
		protected:
			void execute(void) {
				present_s& present = A::present_cast<present_s>();

				switch (status_) {
				case status::off:
				if (command_ == command::on) {
					C::boot_begin();
					status_ = status::boot;
				}
				else {
					break;
				}

				case status::boot:
				if (C::do_boot()) {
					present.cb.actual_satstate = satstate_t::none;
					inverter.run(*A::deseired);
					C::boot_complete(present.inverter.duty);
					status_ = status::on;
					return;
				}
				else {
					break;
				}
				case status::on:
				if (command_ == command::on) {
					present.cb.actual_satstate = satstate_t::none;
					inverter.run(*A::deseired);
					C::do_run(present.inverter.duty);
					return;
				}
				else {
					status_ = status::shutdown;
					C::shutdown_begin();
				}
				case status::shutdown:
				if (C::do_shutdown()) {
					C::shutdown_complete();
					status_ = status::off;
				}
				else {
					break;
				}
				case status::configure:
				break;
				}
				present.cb.actual_satstate = satstate_t::both;
			}
			virtual bool do_handler_reconfig(void) {
				ROBO_LBREAKN(A::do_handler_reconfig());
				const config_s& config = A::config_cast<config_s>();
				inverter.reconfig(config.voltage.low, config.voltage.hi, config.duty.low, config.duty.hi);
				if (status_ == status::configure) {
					status_ = status::off;
				}
				return true;
			}
			#if ROBO_APP_MEXO_VAR_ENABLED == 1
			virtual void do_handler_create_vars(var::record::list& _vars, int _master_key) {
				A::do_handler_create_vars(_vars, _master_key);
				const config_s& config =  A::config_cast<config_s>();
				present_s& present = A::present_cast<present_s>();
				if (var::machine::actual_mode() >= var::machine::mode::full) {
					::mexo::var::record::create(::mexo::var::const_uint8, command_, RT("cmd"));
					::mexo::var::record::create(::mexo::var::const_uint8, status_, RT("status"));

					var::record::create(types::var::const_discret, config.duty.low, RT("duty.low"), _master_key, _vars);
					var::record::create(types::var::const_discret, config.duty.hi, RT("duty.hi"), _master_key, _vars);
					var::record::create(types::var::const_signal, config.voltage.low, RT("v.low"), _master_key, _vars);
					var::record::create(types::var::const_signal, config.voltage.hi, RT("v.hi"), _master_key, _vars);
					inverter.create_var(_master_key, _vars );
				}
			}
			#endif

		public:

			pwm(const config_s& _config, present_s& _present)
				: A(_config.cb, _present.cb), inverter(_present.inverter) {}

			const range_s < typename C::inverter::signal_t >& pwm_voltage_limits(void) { return handler::config_cast<config_s>().voltage; }
			//void set_voltage_req(const C::inverter::signal_t* _voltage_req) { set_input(_voltage_req); }
		};

		template< typename T, typename D, typename O> class pwm_t
			: public ::mexo::finall_controller_t<
			T
			, pwm<D>
			, O
			> {
			typedef ::mexo::finall_controller_t <
				T
				, pwm< D>
				, O
			> A;
		public:
			pwm_t(cstr _name, O* _owner, const typename  pwm<D>::config_s& _config, typename  pwm<D>::present_s& _present)
				: A(_name, _owner, _config, _present) {}
		};


		template< typename D, typename O> class pwm_block_t : public pwm_t <
			::mexo::subsystem_handler
			, D
			, O
		> {
		public:
			typedef pwm_t <
				::mexo::subsystem_handler
				, D
				, O
			> A;
			pwm_block_t(cstr _name, O* _owner, const typename A::config_s& _config, typename  A::present_s& _present)
				: A(_name, _owner, _config, _present) {}
		};

		template< typename D, typename O> class pwm_task_t : public pwm_t <
			O
			, D
			, ::mexo::node
		> {
		public:
			typedef pwm_t <
				O
				, D
				, ::mexo::node
			> A;
			pwm_task_t(cstr _name, ::mexo::node* _owner, const typename A::config_s& _config, typename  A::present_s& _present)
				: A(_name, _owner, _config, _present) {}
		};

		template<typename q> struct dc_inverter : public q::scaler {
			typedef typename q::signal_t signal_t;
			typedef typename q::discret_t discret_t;
			typedef discret_t duty_t;
			struct present_s {
				duty_t duty;
			};
			present_s& present;
			void run(signal_t _voltage) {
				q::scaler::run(_voltage, present.duty)
			}
			static void create_var(int  _master_key, var::record::list& _list) {
				var::record::create(types::var::const_discret, _present.duty, RT("duty"), _master_key, _list);
			}
			dc_inverter(present_s & _present) : present(_present) {}
		};

		template<typename q> struct abc_inverter : public q::scaler {
			typedef typename q::signal_t signal_t;
			typedef typename q::discret_t discret_t;
			typedef typename q::long_signal_t long_signal_t;
			typedef abc_t<q> abc_t;
			typedef ab_t<q> ab_t;
			typedef dq_t<q> dq_t;
			typedef cs_t<q> cs_t;


			struct duty_t {
				discret_t A;
				discret_t B;
				discret_t C;
			};

			struct present_s {
				dq_t dq_required;
				ab_t ab;
				abc_t pwm;
				uint8_t swm;
				duty_t duty;
				cs_t cs;
			};
			present_s& present;
			long_signal_t sum_x_ya(signal_t x, signal_t y, signal_t a) {
				long_signal_t tmp = ((long_signal_t)y) * a;
				tmp = q::s_rshift<long_signal_t>(tmp, 15);
				tmp += x;
				return tmp;
			}
			void run(const signal_t _voltage) {
				constexpr static signal_t one_div_2 = q::round(0.5 * q::max);
				constexpr static signal_t sqrt3_div_2 = q::round(robo::csqrt<double>(3.0) / 2 * q::max);
				constexpr static signal_t scale = q::round((2 / csqrt<double>(2.0) - 1.0) * q::max);

				present.dq_required.cross = _voltage;
				present.ab.scale_inverce(present.dq_required, present.cs);

				long_signal_t pwmA;
				long_signal_t pwmB;
				long_signal_t pwmC;
				present.swm = 1;
				signal_t x, y, z;
				signal_t v2 = q::s_rshift<signal_t>(present.ab.beta, 1);
				x = present.ab.beta;
				y = sum_x_ya(v2, present.ab.alfa, sqrt3_div_2);
				z = sum_x_ya(v2, present.ab.alfa, -sqrt3_div_2);
				if (y < 0) {
					if (z < 0) {
						pwmA = y - z;
						pwmB = pwmA + 2 * z;
						pwmC = pwmA - 2 * y;
						present.swm = 5;
					}
					else {
						if (x > 0) {
							pwmA = -x + y;
							pwmC = pwmA - 2 * y;
							pwmB = pwmC + 2 * x;
							present.swm = 3;
						}
						else {
							pwmA = x - z;
							pwmB = pwmA + 2 * z;
							pwmC = pwmB - 2 * x;
							present.swm = 4;
						}
					}
				}
				else {
					if (z < 0) {
						if (x > 0) {
							pwmA = x - z;
							pwmB = pwmA + 2 * z;
							pwmC = pwmB - 2 * x;
							present.swm = 1;
						}
						else {
							pwmA = -x + y;
							pwmC = pwmA - 2 * y;
							pwmB = pwmC + 2 * x;
							present.swm = 6;
						}
					}
					else {
						pwmA = y - z;
						pwmB = pwmA + 2 * z;
						pwmC = pwmA - 2 * y;
						present.swm = 2;
					}
				}

				pwmA += q::s_mult(pwmA, scale);
				pwmB += q::s_mult(pwmB, scale);
				pwmC += q::s_mult(pwmC, scale);

				present.pwm.A = (signal_t)saturate<long_signal_t>(pwmA, q::min, q::max);
				present.pwm.B = (signal_t)saturate<long_signal_t>(pwmB, q::min, q::max);
				present.pwm.C = (signal_t)saturate<long_signal_t>(pwmC, q::min, q::max);

				q::scaler::run(present.pwm.A, present.duty.A);
				q::scaler::run(present.pwm.B, present.duty.B);
				q::scaler::run(present.pwm.C, present.duty.C);
			}

			void create_var(int  _master_key	, var::record::list& _vars) {
				if (var::machine::actual_mode() >= var::machine::mode::full) {
					var::record::create(types::var::const_discret, present.duty.A, RT("duty.A"), _master_key, _vars);
					var::record::create(types::var::const_discret, present.duty.B, RT("duty.B"), _master_key, _vars);
					var::record::create(types::var::const_discret, present.duty.C, RT("duty.C"), _master_key, _vars);
					var::record::create(q::var::const_signal, present.pwm.A, RT("pwm.A"), _master_key, _vars);
					var::record::create(q::var::const_signal, present.pwm.B, RT("pwm.B"), _master_key, _vars);
					var::record::create(q::var::const_signal, present.pwm.C, RT("pwm.C"), _master_key, _vars);
					var::record::create(q::var::const_signal, present.cs.si, RT("ab.sin"), _master_key, _vars);
					var::record::create(q::var::const_signal, present.cs.co, RT("ab.cos"), _master_key, _vars);
					var::record::create(q::var::const_signal, present.ab.alfa, RT("ab.alfa"), _master_key, _vars);
					var::record::create(q::var::const_signal, present.ab.beta, RT("ab.beta"), _master_key, _vars);
					var::record::create(q::var::const_signal, present.dq_required.cross, RT("dq.cross"), _master_key, _vars);
					var::record::create(q::var::const_signal, present.dq_required.lateral, RT("dq.lat"), _master_key, _vars);
					var::record::create(q::var::const_signal, present.cs.angle, RT("dq.angle"), _master_key, _vars);
					var::record::create(::mexo::var::uint8, present.swm, RT("swm"), _master_key, _vars);
				}
			}

			abc_inverter(present_s & _present) : present(_present) {}

			void lat_voltage_set(signal_t _voltage) {
				present.dq_required.lateral = _voltage;
			}

			void angle_set(signal_t _angle) {
				present.cs.rotate(_angle);
			}

			signal_t  & lat_voltage_ref(void) {
				return present.dq_required.lateral;
			}
		};


		/*
			class current {
			public:
				class math : public voltage::math {
					struct dummy {
						signal2ph_s dq;
						signal2ph_s ab;
						signal3ph_s& ABC;
						dummy(signal3ph_s & _ABC) : ABC(_ABC) {}
					} current_;

				public:
					const signal2ph_s& current() { return current_.dq; };

					virtual void perform(void) {
						voltage::math::perform();
						current_.ABC >> current_.ab;
						transform_.forward(current_.ab, current_.dq);
					}

					math(signal2ph_s& _voltage_dq, signal3ph_s& _current_ABC)
						: voltage::math(_voltage_dq), current_(_current_ABC) {}
				};

			};
		}
		*/

		/*
		struct dq_to_ABC {
			signal2ph_s& dq;
			transform& transform_;
			signal2ph_s ab;
			signal3ph_s ABC;
			dq_to_ABC(signal2ph_s& _dq, transform _transform) : dq(_dq) {}
		};
		struct current  {
			signal2ph_s dq;
			signal2ph_s ab;
			signal3ph_s & ABC;
			current(signal3ph_s& _ABC) : ABC(_ABC) {}
		};
		struct no_current {};

		template <typename current_t = void> class  inverter {
			transform transform_;
			//��� ��� �������
		public:
			const signal3ph_s& voltage() { return voltage_.ABC_ };
			virtual void update(void) {
				transform_.backward(voltage_.dq_, voltage_.ab);
				voltage_.ab >> voltage_.ABC;
			}
			void rotate(const signal_t& _angle) {
				transform_.set_angle(_angle);
				update();
			}
			inverter(signal2ph_s& _voltage_dq) : voltage_(_voltage_dq) {}
		};*/

	}
}
#endif