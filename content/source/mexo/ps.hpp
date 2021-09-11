#ifndef mexo_ps_hpp
#define mexo_ps_hpp
#include "mexo/mexo.hpp"
#include "mexo/math.hpp"
#include "mexo/adc.hpp"

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

		template < typename C > class pwm
			: public ps::control
			, public C {
		public:
			typedef typename C::input_t input_t;
			typedef typename C::output_t output_t;
			typedef typename C::config_s  config_s;
		private:
			output_t duty_;
		public:
			void loockup(input_t& _output) {
				C::revert(duty_, _output);
			}
			output_t& output_value(void) {
				return duty_;
			}

		public:
			pwm(void) {}

			pwm(const input_t& _default) {
				C::dirrect(_default, duty_);
			}

			bool applay(const config_s& _config) {
				if (C::applay(_config)) {
					enable();
					return true;
				}
				else {
					return false;
				}
			}

			iblock::satstate  execute(const input_t& _input, const iblock::satstate _master_sat, const range_s<output_t>& _range) {
				if (_master_sat == iblock::satstate::both) return iblock::satstate::both;
				iblock::satstate satstate = iblock::satstate::both;
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
					C::dirrect(_input, _range, duty_);
					C::boot_complete(duty_);
					status_ = status::on;
				}
				else {
					break;
				}
				case status::on:
				if (command_ == command::on) {
					satstate = C::dirrect(_input, _range, duty_);
					C::do_run(duty_);
					break;
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

				return satstate;

			}
		};

		typedef controller_block_t< ramp< signal_t > > voltage;
		typedef atom_block_t< filter< signal_t, signal_t > > filter;
		typedef controller_block_t<  quazzy_adapt<signal_t, signal_t, signal_t> > current;

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