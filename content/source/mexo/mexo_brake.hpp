#ifndef __mexo_brake_hpp
#define __mexo_brake_hpp
#include "mexo/mexo.hpp"
#include "mexo/ps.hpp"
#include "core/robosd_system.hpp"
namespace mexo {

    namespace  brake {
        class itf {
        public:
            enum class status_t {
                configure = 0,
                fixed = 1,
                releasing = 2,
                released = 3,
                fixing = 4
            };

            enum class command_t {
                set = 0,
                release = 1
            };

        private:
            command_t & command_;
            status_t & status_;
        public:
            void enable(void) { if (status_ == status_t::configure) status_ = status_t::fixed; }
            void set(void) { command_ = command_t::set; }
            void release(void) { command_ = command_t::release; }
            bool released(void) { return status_ == status_t::released; }
            bool fixed(void) { return status_ == status_t::fixed; }
            itf(command_t& _command, status_t& _status ): command_(_command), status_(_status){}
            virtual ~itf(void) {}
        };
        /*
        //управляет потреблением тормоза. При снятии с тормоза повышенное потребление, потом некоторая пауза
		template< typename D>  class  softmachine
			: public handler, public itf{
			typedef handler;
            ::robo::time_us_t last_us_;
        public:
			struct config_s {
				typename handler::config_s cb;
                struct {
                    ::robo::time_us_t relax_us;
                    ::robo::time_us_t force_us;
                } timeout;
            };
			struct present_s {
				typename handler::present_s cb;
                command_t command;
                status_t status;
            };
		protected:
			#if ROBO_APP_MEXO_VAR_ENABLED == 1					
			virtual void do_handler_create_vars(var::record::list& _vars, int _master_key) {
				handler::do_handler_create_vars(_vars, _master_key);
				const config_s& config = handler::config_cast<config_s>();
				present_s& present = handler::present_cast<present_s>();
                if (var::machine::actual_mode() >= var::machine::mode::tuning) {
                    var::record::create(::mexo::var::uint32, config.timeout.relax_us, RT("tm.relax_us"), _master_key, _vars);
                    var::record::create(::mexo::var::uint32, config.timeout.force_us, RT("tm.force_us"), _master_key, _vars);
                }
                if (var::machine::actual_mode() >= var::machine::mode::full) {
                    var::record::create(::mexo::var::uint8, present.command, RT("cmd"), _master_key, _vars);
                    var::record::create(::mexo::var::const_uint8, present.status, RT("stat"), _master_key, _vars);
                }
			};
			#endif

			void do_handler_adjust(void) {
				present_s& present = handler::present_cast<present_s>();
				const config_s& config = handler::config_cast<config_s>();
			}

			void execute(void) {
                ::robo::time_us_t now_us_ = ::robo:system::env::time_us();
                present_s& present = handler::present_cast<present_s>();
                const config_s& config = handler::config_cast<config_s>();

                switch (present.status) {
                case status_t::fixed:
                if (present.command == command_t::release) {
                    present.status = status_t::releasing;
                    D::brake_force();
                    last_us_ = now_us_;
                }
                else {
                    break;
                }

                case status_t::releasing:
                if (present.command == command_t::set) {
                    present.status = status_t::fixing;
                    D::brake_set();
                    last_us_ = now_us_;
                }
                else {
                    if ((now_us_ - last_us_) >= config.timeout.force_us) {
                        present.status = status_t::released;
                        D::brake_normal();
                    }
                }
                break;

                case status_t::released:
                if (present.command == command_t::set) {
                    present.status = status_t::fixing;
                    D::brake_set();
                    last_us_ = now_us_;
                }
                else {
                    break;
                }

                case status_t::fixing:                
                    if ((now_us_ - last_us_) >= config.timeout.fix_us) {
                        present.status = status_t::fixed;
                    }
                    break;
                }
            }

		public:
			itf(const config_s& _config
			   , present_s& _present
			)
				: handler(_config.cb, _present.cb)	{}
		};*/

    }    
}
#endif
