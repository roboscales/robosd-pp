#ifndef robo_prf_led_hpp
#define robo_prf_led_hpp
#include "core/robosd_common.hpp"
namespace robo {	
	namespace prf {	
		template <typename D> class led_base_t : public D{
			enum class state{ on =1,off = 0} state_ = state::off;
			unsigned int verb_ = 0;
			protected:
			led_base_t(void):D(){}
			~led_base_t(void){}
			void set_verb_(unsigned int _verb){
				verb_ = _verb;
			}
			void on_(unsigned int _verb = 0){				
				if(verb_ == _verb){
					if(state_==state::off){
						D::on();
						state_=state::on;
					}
				}
			}
			void off_(unsigned int _verb = 0){
				if(verb_ == _verb){
					if(state_==state::on){
						D::off();
						state_=state::off;
					}
				}
			}
			void toggle_(unsigned int _verb = 0){
				if(verb_ == _verb){
					if(state_==state::on){
						D::off();
						state_=state::off;
					} else {
						D::on();
						state_=state::on;
					}
				}
			}
		};

		template <typename D> class ledcom_t : public led_base_t<D>{
			static ledcom_t & instance_(){
				static ledcom_t instance__;
				return instance__;
			}
		public:
			static void on(unsigned int _verb = 0) { instance_().on_(_verb); } 
			static void off(unsigned int _verb = 0) { instance_().off_(_verb); } 
			static void toggle(unsigned int _verb = 0) { instance_().toggle_(_verb); } 
			static void set_verb(unsigned int _verb) { instance_().set_verb_(_verb); } 
		};


		template <typename D> class led_t : public led_base_t<D>{
			static led_t instance_;
		public:
			static void on(unsigned int _verb = 0) { instance_.on_(_verb); } 
			static void off(unsigned int _verb = 0) { instance_.off_(_verb); }
			static void toggle(unsigned int _verb = 0) { instance_.toggle_(_verb); }
			static void set_verb(unsigned int _verb) { instance_.set_verb_(_verb); }
			static void perform( uint8_t _command, unsigned int _verb  = 0) {
				switch (_command){
					case 1: on(_verb); break;
					case 2: toggle(_verb); break;
					default : off(_verb);
				}
			} 
		};
		template <typename D>  led_t<D> led_t<D>::instance_;
		

		
		class  dummy_led {
			public:
				static void on(unsigned int _verb = 0) { }
				static void off(unsigned int _verb = 0) {  }
		};
	}
}

#endif