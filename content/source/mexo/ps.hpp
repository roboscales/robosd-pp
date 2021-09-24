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

		template < typename q, typename C > class pwm_b
			: public ps::control
			, private C
			, public to_digit_scale_b< q > {

		protected:
			typedef to_digit_scale_b< q> to_digit_scale_b;
			void execute() {
				present_s& present = iblock::present_cast<present_s>();

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
					to_digit_scale_b::execute();
					C::boot_complete(present.cb.output);
					status_ = status::on;
					return;
				}
				else {
					break;
				}
				case status::on:
				if (command_ == command::on) {
					to_digit_scale_b::execute();
					C::do_run(present.cb.output);
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
				present.cb.satstate = iblock::satstate::both;
			}
			virtual bool do_reconfig(void) {
				ROBO_LBREAKN(to_digit_scale_b::do_reconfig());
				if (status_ == status::configure) {
					status_ = status::off;
				}
				return true;
			}
		public:
			typedef typename to_digit_scale_b::config_s config_s;
			typedef typename to_digit_scale_b::present_s present_s;
			pwm_b(isubsystem& _subsystem, cstr  _name, config_s& _config, present_s& _present)
				: to_digit_scale_b(_subsystem, _name, _config, _present) {}

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
		//typedef ramp_b<signal_t, signal_t>  voltage_regulator_b;
		//typedef filter_b<signal_t, signal_t, parameter_t>  current_filter_b;
		//typedef quazzy_adapt_b<signal_t, signal_t, parameter_t>  current_regulator_b;

		/*
		namespace inverter {
			class voltage {
			public:
				typedef typename signal2ph_s deseired_t;
				typedef typename signal3ph_s actual_t;


				class math {
				public:
					struct dummy {
						deseired_t& dq;
						deseired_t ab;
						actual_t ABC;
						dummy(deseired_t& _dq) : dq(_dq) {}
					} voltage_;
				protected:
					transform transform_;
				public:
					const actual_t& actual() { return voltage_.ABC; };
					virtual void perform(void) {
						transform_.backward(voltage_.dq, voltage_.ab);
						voltage_.ab >> voltage_.ABC;
					}
					void rotate(const signal_t& _angle) {
						transform_.set_angle(_angle);
						perform();
					}
					math(deseired_t& _voltage_dq) : voltage_(_voltage_dq) {}
				};
			protected:
				math math_;
			public:
				const actual_t& actual(void) {
					return math_.actual();
				}

				voltage(const actual_t& _standalone_desirted)
					: range(standalone_range)
				{
					ROBO_UNUSED(_standalone_desirted);
					standalone_range = {};
				}

				bool applay(const config_s& _config) {
					math_.rampStep = _config.rampGain;
					if ((_config.default > _config.range.lo) && (_config.default < _config.range.hi)) {
						standalone_range = _config.range;
						math_.actual = _config.default;
						return true;
					}
					else {
						return false;
					}
				}
			};
		}*/
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