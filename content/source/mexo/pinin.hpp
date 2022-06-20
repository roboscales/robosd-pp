#ifndef mexo_pinin_hpp
#define mexo_pinin_hpp
#include "core/robosd_common.hpp"
#include "core/robosd_system.hpp"
#include "core/robosd_delegat.hpp"
namespace mexo {	
	template <typename D> class pinin : public D{
		public:
		typedef robo::delegat::base<void> delegat;
		private:
		delegat * on_raise_ = nullptr;
		enum class state{ set =1,reset = 0} state_ = state::reset;
		robo::time_us_t last_us_ = 0;
		robo::time_us_t period_us_ = 0;
		robo::time_us_t pressed_us_ = 0;
		robo::time_us_t pushdown_us_ = 0;
		public:
		void begin(robo::time_us_t _period_us){
			period_us_ = _period_us;
			if( D::get() ){
				state_ =  state::set;
			} else {
				state_ =  state::reset;
			}
		}
		void attach(delegat * _on_raise ){
			on_raise_ = _on_raise;
		}
		void raise(void){
			robo::time_us_t us = robo::system::env::time_us();
			if( us - last_us_ >= period_us_ ){
				if(on_raise_){
					 (*on_raise_)();
				}
				last_us_ = us;
			}
		}		
		void poll(void){
			if( D::get() ){
				if(state_ ==  state::reset){
					state_ =  state::set;
					raise();
					pushdown_us_ = ::robo::system::env::time_us();
				}else{
					pressed_us_=::robo::system::env::time_us()-pushdown_us_;
				}
			} else {
				if(state_ ==  state::set){
					state_ =  state::reset;
					pressed_us_ = pushdown_us_ = 0;
				}
			}
		}
		bool get(void){
			return D::get();
		}
		robo::time_us_t pressed_us(){ return pressed_us_;}
	};
}
#endif