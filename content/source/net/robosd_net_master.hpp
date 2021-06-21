#ifndef __robosd_net_master_hpp
#define __robosd_net_master_hpp
#include <stdint.h>

#include "core/robosd_delegat.hpp"

namespace robo{
    namespace net{        
		class master{
			public:
				virtual void exchange(const uint8_t * _outcom_buf,  unsigned char _outcom_size, uint8_t * _incom_buf,  unsigned char _incom_size, ::robo::delegat::base<void, bool> * _confirm ) = 0;
				virtual void cancel(void) = 0;
				virtual bool ready(void) = 0;
		};
		template< typename phys > class master_t: private phys, public master {
		public:
			enum class result { refuse, success, panic };
			private:
			bool wd_enabled = false;
			unsigned int wd_begin_ms_ = 0;
			unsigned int wd_delay_ms_ = 0;
			
			enum class state { idle, send, receive, stopped, disable, panic};
			typedef typename phys::guard guard;
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
			
			const uint8_t * outcom_buf_ = nullptr;
			unsigned int outcom_size_ = 0;
			uint8_t * incom_buf_ = nullptr;
			unsigned int  incom_size_ = 0;
			void reset_(void){
				outcom_buf_ = nullptr;
				outcom_size_ = 0;
				incom_buf_ = nullptr;
				incom_size_ = 0;
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
			::robo::delegat::base<void, bool> * confirm_ = nullptr;
				
			virtual void exchange(const uint8_t * _outcom_buf,  unsigned char _outcom_size, uint8_t * _incom_buf,  unsigned char _incom_size, ::robo::delegat::base<void, bool> * _confirm = nullptr){
				confirm_ = _confirm;
				guard g__;
				outcom_buf_ = _outcom_buf;
				outcom_size_ = _outcom_size;
				incom_buf_ = _incom_buf;
				incom_size_ = _incom_size;
				if (state_ == state::idle){
					state_ = state::send;
					wd_begin_ms_ = phys::time_ms();
					wd_enabled = true;
					//todo
					wd_delay_ms_ = incom_size_*10;
					phys::send(outcom_buf_, outcom_size_);						
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
						wd_begin_ms_ = phys::time_ms();
						state_ = state::receive;
						wd_delay_ms_ = incom_size_*10;
						phys::receive(incom_buf_, incom_size_);
						break;
					case state::receive:
						if(incom_size_>0){
							wd_begin_ms_ = phys::time_ms();
							state_ = state::receive;
							wd_delay_ms_ = incom_size_*10;
							phys::receive(incom_buf_, incom_size_);
						} else{
							reset_();
							state_ = state::idle;
						}
						return;	
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