#ifndef mexo_ps_hpp
#define mexo_ps_hpp
#include "mexo/mexo.hpp"
#include "mexo/ramp.hpp"

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
			, public C
		{
		public:
			typedef typename C::deseired_t deseired_t;
			typedef typename C::actual_t actual_t;
			typedef typename C::config_s  config_s;
		private:
			actual_t duty_;
		public:
			void loockup(deseired_t& _actual) {
				C::revert(duty_, _actual);
			}
			actual_t & actual(void) {
				return duty_;
			}

		public:
			pwm(const deseired_t & _default)
			{
				C::dirrect(_default, duty_);
			}
			
			bool applay(const config_s & _config) {
				if (C::applay(_config)) {
					enable();
					return true;
				}
				else {
					return false;
				}
			}

			iblock::satstate  execute(const deseired_t & _deseired) {
				iblock::satstate satstate = iblock::satstate::none;
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
						satstate = C::dirrect(_deseired, duty_);
						C::boot_complete(duty_);
						status_ = status::on;
					}
					else {
						break;
					}
				case status::on:
					if (command_ == command::on) {
						satstate = C::dirrect(_deseired, duty_);
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
					satstate = iblock::satstate::both;
					break;
				}

				return satstate;

			}
		};		
	}
}
#endif