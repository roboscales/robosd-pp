#ifndef __robosd_net_master_hpp
#define __robosd_net_master_hpp
#include <stdint.h>

#include "core/robosd_delegat.hpp"

namespace robo{
    namespace net{        
		template< typename P > class imaster_t {
		public:
			typedef  ::robo::delegat::base<void, bool> confirm_delegat;
			virtual void exchange(const P& _outcom_packet, P *  _incom_packet, confirm_delegat* _confirm) = 0;
			virtual void cancel(void) = 0;
			virtual bool ready(void) = 0;
		};

		template< typename phys, typename P > class master_t: public phys, public imaster_t<P> {
		public:
			enum class result { refuse, success, panic };
			private:
			bool wd_enabled = false;
			unsigned int wd_begin_ms_ = 0;
			unsigned int wd_delay_ms_ = 0;
			
			enum class state { idle, send, receive, stopped, disable, panic};
			typedef typename system::guard guard;
			enum {incom_size_bits=5};
			
			state state_ = state::disable;
			
			void panic_(void){
				if( phys::panic() ){
					state_ = state::panic;
				} else {
				state_ = state::idle;
				}
				reset_();
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
					wd_begin_ms_ = system::env::time_ms();
					wd_enabled = true;
					wd_delay_ms_ = phys::wd_us(outcom_packet_);
					phys::send(outcom_packet_);						
				} else {
					panic_();
				}
			}
			
			
			void confirm(void){
				bool cf = false;;
				bool result = false;
				{
					guard g__;
					switch (state_){
					case state::send:
						if(incom_packet_ !=nullptr){
							wd_begin_ms_ = system::env::time_ms();
							state_ = state::receive;
							wd_delay_ms_ = phys::wd_us(incom_packet_);
							phys::receive(incom_packet_);
							break;
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
				case state::idle:
					break;
				case state::send:
					phys::send_cancel();
					break;
				case state::receive:
					phys::receive_cancel();
					break;						
				default:
					panic_();			
				}
				state_ = state::idle;
			}				
			
			void poll(void){
				if(wd_enabled){
					if( phys::time_ms() - wd_begin_ms_ > wd_delay_ms_ ){
						refuse();
					}
				}
			}
			
			virtual bool ready(void){
				guard g__;
				return state_ == state::idle;
			}		
		};
			
    }
}
#endif