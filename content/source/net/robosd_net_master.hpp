#ifndef __robosd_net_master_hpp
#define __robosd_net_master_hpp
#include <stdint.h>
namespace robo{
    namespace net{        
		template< typename phys > class master: private phys {
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
				phys::panic();
				reset_();
				state_ = state::panic;
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
				case state::receive:
					phys::send_cancel();
				break;
				default:;
				}
				reset_();
				state_ = state::stopped;
			}

			void exchange(const uint8_t * _outcom_buf,  unsigned char _outcom_size, uint8_t * _incom_buf,  unsigned char _incom_size){
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
				guard g__;
				switch (state_){
				case state::send:
					wd_begin_ms_ = phys::time_ms();
					state_ = state::receive;
					wd_delay_ms_ = incom_size_*10;
					phys::receive(incom_buf_, incom_size_);
					return;						
				case state::receive:
					reset_();
					state_ = state::idle;
					return;
				default:;
				}
				panic_();
			}

			void refuse(void){
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
			
			bool ready(void){
				guard g__;
				return state_ == state::idle;
			}		
		};
			
    }
}
#endif