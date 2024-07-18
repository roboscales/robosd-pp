#ifndef __robosd_net_master_hpp
#define __robosd_net_master_hpp
#include <stdint.h>

#include "core/robosd_delegat.hpp"
#include "core/robosd_system.hpp"

namespace robo{
    namespace net{        
		template< typename P > class imaster_t {
		public:
			typedef  ::robo::delegat::ref<void, bool> confirm_delegat;
			virtual void exchange(const P& _outcom_packet, P *  _incom_packet, confirm_delegat* _confirm) = 0;
			virtual void cancel(void) = 0;
			virtual bool ready(void) = 0;
		};
		#if ROBO_APP_MODULE_ENABLED ==1
		template< typename phys, typename P > class master_t: public phys, public imaster_t<P>, public app::node {
		#else
		template< typename phys, typename P > class master_t : public phys, public imaster_t<P> {
		#endif
		public:
			enum class result { refuse, success, panic };
			private:
			bool wd_enabled = false;
			unsigned int wd_begin_us_ = 0;
			unsigned int wd_delay_us_ = 0;
			
			enum class state { idle, send, receive, stopped, disable, panic};
			typedef typename system::guard guard;
			enum {incom_size_bits=5};
			
			state state_ = state::disable;
			
			void panic__(void){
				if( phys::panic() ){
					state_ = state::panic;
				} else {
					if(state_!=state::disable && state_ != state::panic ){
						state_ = state::idle;
					}
				}
				reset_();
			}
			void panic_(void){
				refuse();
				panic__();
			}
			
			const P * outcom_packet_ = nullptr;
			P * incom_packet_ = nullptr;
			void reset_(void){
				outcom_packet_ = nullptr;
				incom_packet_ = nullptr;
				wd_enabled = false;
			}
		
		public:
			
			void begin(void){
				guard g__;
				state_ = state::stopped;
			}
			
			void start(void){
				guard g__;
				state_ = state::idle;
			}
			
			void stop(void){
				guard g__;
				switch (state_){
				case state::send:
					phys::send_cancel();
				break;
				case state::receive:
					phys::receive_cancel();
				break;
				default:;
				}
				reset_();
				state_ = state::stopped;
			}
			
			virtual void cancel(void){
				guard g__;
				switch (state_){
				case state::send:
					phys::send_cancel();
				break;
				case state::receive:
					phys::receive_cancel();
				break;
				default:;
				}
				reset_();
				state_ = state::idle;
			}
			typename imaster_t<P>::confirm_delegat * confirm_ = nullptr;
				
			virtual void exchange(const P& _outcom_packet, P* _incom_packet, typename  imaster_t<P>::confirm_delegat * _confirm){
				confirm_ = _confirm;
				guard g__;
				outcom_packet_ = &_outcom_packet;
				incom_packet_ = _incom_packet;
				if (state_ == state::idle){
					state_ = state::send;
					wd_begin_us_ = system::time_us();
					wd_enabled = true;
					wd_delay_us_ = phys::wd_us(outcom_packet_);
					if(incom_packet_){
						wd_delay_us_ += phys::wd_us(incom_packet_);
					}
					phys::send(outcom_packet_);						
				} else {
					panic_();
				}
			}
			
			
			virtual void confirm(void){
				bool cf = false;;
				bool result = false;
				{
					guard g__;
					switch (state_){
					case state::send:
						if(incom_packet_ !=nullptr){
							wd_begin_us_ = system::time_us();
							state_ = state::receive;
							wd_delay_us_ = phys::wd_us(incom_packet_);
							phys::receive(incom_packet_);
							return;
						}
					case state::receive:
						reset_();
						state_ = state::idle;
						cf = true;
						result = true;
						break;
					default:;
						cf = true;
						result = false;
						panic_();					
					}
				}
				if(cf) if(confirm_) (*confirm_)(result);
			}

			void refuse(void){
				if(confirm_) (*confirm_)(false);
				guard g__;
				reset_();
				switch (state_){
				case state::send:
					phys::send_cancel();
					state_ = state::idle;
					break;
				case state::receive:
					phys::receive_cancel();
					state_ = state::idle;
					break;						
					default:
					break;						
				}
				//if (confirm_) (*confirm_)(false);
			}				
			
			void poll(void){
				if(wd_enabled){
					if( system::time_us() - wd_begin_us_ > wd_delay_us_ ){
						refuse();
					}
				}
			}
			
			virtual bool ready(void){
				guard g__;
				return (state_ == state::idle) && (phys::ready());
			}		
			#if ROBO_APP_MODULE_ENABLED ==1
			master_t(cstr _name, app::node* _owner) : app::node(_name, _owner) {
}
			virtual bool do_load(void) {
				ROBO_LBREAKN(app::node::do_load());
				ROBO_LBREAKN(phys::do_load(current_path(), defaults_path()));
				return true;
			}
			virtual void do_clean(void) {
				app::node::do_load();
				phys::do_clean();
			}

			#endif
		};
			
    }
}
#endif