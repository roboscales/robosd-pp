#include "mexo/mexo.hpp"
#ifndef mexo_ps_hpp
#define mexo_ps_hpp
namespace mexo {
	/*class regulator {
	public:
		enum class saturate { none, low, hi };
		block::output_t< saturate > status;
	};*/
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

		class  voltage: public control {
		protected:
			signal_t desired = (signal_t)0;
			parametr_t rampGain = (signal_t)0;
			mexo::range_s def_range;
			signal_t def_req = (signal_t)0;
			iblock::satstate satstate_val;
		private:
		public:
			struct config_s {
				iblock::config_s  block;
				parametr_t rampGain;
				mexo::range_s range;
				signal_t def;
			};

			iblock::input_t<signal_t> required;
			iblock::input_t<mexo::range_s> range;
			iblock::output_t<iblock::satstate> satstate;
		protected:
			void ramp(void) {
				signal_t r = required.value();
				if (r >= range.value().hi) {
					r = range.value().hi;
					satstate_val = iblock::satstate::up;
				}
				else if (r <= range.value().lo) {
					r = range.value().lo;
					satstate_val = iblock::satstate::low;
				}
				else {
					satstate_val = iblock::satstate::none;
				}
				signal_t delta = r - desired;
				if (delta > 0) {
					if (delta < rampGain) {
						desired = r;
					}
					else {
						desired += rampGain;
					}
				}
				else {
					if (delta < 0) {
						if ((-delta) < rampGain) {
							desired = r;
						}
						else {
							desired -= rampGain;
						}
					}
				}
			}

			voltage(void)
				: required(def_req)
				, range(def_range)
				, satstate(satstate_val) {

			}
		};



		template < typename C,  typename D, typename R > class machine : public D, public C {
		private:
			const R& required_;
		protected:
			machine(const R& _required) :required_(_required) {}
			void execute(void) {
				switch (C::status_) {
				case status::off:
					if (C::command_ == command::on) {
						D::boot_begin();
						C::status_ = status::boot;
					}
					else {
						break;
					}

				case status::boot:
					if (D::do_boot()) {
						D::boot_complete(required_);
						C::status_ = status::on;
					}
					else {
						break;
					}
				case status::on:
					if (C::command_ == command::on) {
						D::do_run(required_);
						break;
					}
					else {
						C::status_ = status::shutdown;
						D::shutdown_begin();
					}
				case status::shutdown:
					if (D::do_shutdown()) {
						D::shutdown_complete();
						C::status_ = status::off;
					}
					else {
						break;
					}
				case status::configure:
					break;
				}
			}
		};


	

		template < typename D > class vdc
			: public  block_t < mexo::ps::voltage::config_s >, public machine<mexo::ps::voltage, D,  signal_t>{
		public:
			typedef  mexo::ps::voltage::config_s config_s;
			typedef  machine<mexo::ps::voltage, D, signal_t> machine;
			typedef  block_t < config_s> block;
		public:
			vdc(subsystem& _subsystem, cstr  _name, config_s& _config)
				: block(_subsystem, _name, _config)
				, machine(voltage::desired)
			{

			}

			virtual bool applay(const config_s& _config) {
				voltage::rampGain = _config.rampGain;
				voltage::def_range = _config.range;
				voltage::desired = _config.def;
				machine::enable();
				return true;
			}

			virtual void execute(void) {
				if (voltage::active()) {
					voltage::ramp();
				}
				else {
					voltage::satstate_val = iblock::satstate::both;
				}
				machine::execute();
			}
		};





	}
}
#endif