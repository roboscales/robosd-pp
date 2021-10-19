#ifndef __as5048a_hpp
#define __as5048a_hpp
#include "core/robosd_common.hpp"

namespace robo{
	namespace  prf{
		template <typename D> class AS5048A: private D{
		private:
			union{
				uint8_t memo_[2];
				uint16_t answer_;
			};
			uint16_t native_ = 0;
			uint16_t command_ = 0xFFFF;
			bool error_ = false;
			enum class state { none,put, pause, get } state_ = state::none;

			void put_(void){
				//if(state_ == state::none){
					state_ = state::put;	
					D::cs_low();
					D::put(command_);
				//}
			}
			
			void get_(void){
				if(state_ == state::pause){
					state_ = state::get;	
					D::cs_low();
					D::get(answer_);
				}
			}
			
			void confirm_put_(void){
				if  (state_ == state::put){
					D::cs_hi();
					state_ = state::pause;
				}
			}
			
			void confirm_get_(void){
				if  (state_ == state::get){
					D::cs_hi();
					state_ = state::none;
					uint16_t res = ((uint16_t)(memo_[0]))<<8;
					res += memo_[1];
					res	&= ~0xC000;
					error_ = ((res & 0x4000) == 0x4000);
					native_ = (res+1)>>2; //12 бит
				}
			}
			
			static AS5048A instance_;
			AS5048A(void){
					memo_[0]=0;
					memo_[1]=0;
			}
		public:
			typedef uint16_t  unative_t;
			typedef int16_t  native_t;

			static void put(void){
				instance_.put_();
			}
			static void get(void){
				instance_.get_();
			}
			static void confirm_put(void){
				instance_.confirm_put_();
			}
			static void confirm_get(void){
				instance_.confirm_get_();
			}		
			bool static error(void){ 
				return instance_.error_;
			}
			static unative_t native(void){
				return instance_.native_;
			}		
		};
		template <typename D> AS5048A<D> AS5048A<D>::instance_;
	}
}
#endif
