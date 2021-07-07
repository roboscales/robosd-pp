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

		template < typename C > class pwm 
			: public ps::control
			, public C
			, public  controller_t < typename C::config_s, typename C::required_t >
		{
		public:
			typedef typename C::required_t R;
			typedef typename C::duty_t A;
			typedef typename C::config_s  config_s;
			//???
			//typedef typename block_t < typename C::config_s >::config_s config_s;
		private:
			A duty_;
		public:
			void actual(R& _actual) {
				C::revert(duty_, _actual);
			}

		public:
			pwm(isubsystem& _subsystem, cstr  _name, config_s & _config, const R& _default )
				: block_t < config_s >(_subsystem,_name, _config)
			{
				C::dirrect(def_req, duty_);
			}
			
			virtual bool applay(const config_s & _config) {
				if (C::applay(_config)) {
					enable();
					return true;
				}
				else {
					return false;
				}
			}

			virtual void execute(void) {
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
						satstate_ = C::dirrect(required.value(), duty_);
						C::boot_complete(duty_);
						status_ = status::on;
					}
					else {
						break;
					}
				case status::on:
					if (command_ == command::on) {
						satstate_ = C::dirrect(required.value(), duty_);
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
					satstate_ = iblock::satstate::both;
					break;
				}

			}
		};

		class  ramp {
		protected:
			signal_t desired = (signal_t)0;
			parametr_t rampGain = (signal_t)0;
			mexo::range_s def_range;
			signal_t def_req = (signal_t)0;
			iblock::satstate satstate_val;
		public:
			struct config_s {
				iblock::config_s block;
				parametr_t rampGain;
				mexo::range_s range;
			};

			iblock::input_t<signal_t> required;
			iblock::input_t<mexo::range_s> range;
			iblock::input_t<iblock::satstate> master_satstate;
			iblock::output_t<iblock::satstate> satstate;
		protected:
			void run(void) {
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

			ramp(void)
				: required(def_req)
				, range(def_range)
				, satstate(satstate_val) {

			}
		};


		class voltage
			: public ramp, public  block_t < ramp::config_s >{
		public:

			typedef  mexo::ps::voltage::config_s config_s;
			typedef  block_t < ramp::config_s > block;
		public:
			voltage(subsystem& _subsystem, cstr  _name, config_s& _config)
				: block(_subsystem, _name, _config)
			{

			}

			virtual bool applay(const config_s& _config) {
				voltage::rampGain = _config.rampGain;
				voltage::def_range = _config.range;
				voltage::desired = _config.def;
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
		*/
	}
}
#endif