#ifndef mexo_pinin_hpp
#define mexo_pinin_hpp
#include "core/robosd_common.hpp"
#include "core/robosd_system.hpp"
#include "core/robosd_delegat.hpp"
namespace robo {	
	namespace prf {	
		namespace discret {	
			namespace in {	
				template <typename D> class dry_t : public D{
				public:
					typedef robo::delegat::ref<void> on_push_f;
					typedef robo::delegat::ref<void,robo::time_us_t> on_up_f;
				private:
					on_push_f * on_push_ = nullptr;
					on_up_f * on_up_ = nullptr;
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
					void attach(on_push_f * _on_push ){
						on_push_ = _on_push;
					}
					void attach(on_up_f * _on_up ){
						on_up_ = _on_up;
					}
					void raise(void){
						robo::time_us_t us = D::time_us();
						if( us - last_us_ >= period_us_ ){
							if(on_push_){
								 (*on_push_)();
							}
							last_us_ = us;
						}
					}		
					void poll(void){
						if( D::get() ){
							if(state_ ==  state::reset){
								state_ =  state::set;
								raise();
								pushdown_us_ = D::time_us();
							}else{
								pressed_us_=D::time_us()-pushdown_us_;
							}
						} else {
							if(state_ ==  state::set){
								state_ =  state::reset;
								if(on_up_){
									 (*on_up_)(pressed_us_);
								}
								pressed_us_ = pushdown_us_ = 0;
							}
						}
					}
					bool get(void){
						return D::get();
					}
					robo::time_us_t pressed_us(){ return pressed_us_;}
				};

				template <typename D> class pin_t : public D{
					public:
					pin_t(void):D(){}
					~pin_t(void){}
					bool get(void){
						return D::get();
					}
					operator bool() const{ return D::get(); }

				};		
			}				
		}
	}
}

#endif