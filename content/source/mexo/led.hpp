#ifndef mexo_led_hpp
#define mexo_led_hpp
#include "core/robosd_common.hpp"
namespace mexo {	

	template <typename D> class led_base : public D{
		enum class state{ on =1,off = 0} state_ = state::off;
		unsigned int verb_ = 0;
		protected:
		led_base(void):D(){}
		~led_base(void){}
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

	template <typename D> class led : public led_base<D>{
		static led & instance_(){
			static led instance__;
			return instance__;
		}
	public:
		static void on(unsigned int _verb = 0) { instance_().on_(_verb); } 
		static void off(unsigned int _verb = 0) { instance_().off_(_verb); } 
		static void toggle(unsigned int _verb = 0) { instance_().toggle_(_verb); } 
		static void set_verb(unsigned int _verb) { instance_().set_verb_(_verb); } 
	};


	template <typename D> class led_s : public led_base<D>{
		static led_s instance_;
	public:
		static void on(unsigned int _verb = 0) { instance_.on_(_verb); } 
		static void off(unsigned int _verb = 0) { instance_.off_(_verb); } 
		static void toggle(unsigned int _verb = 0) { instance_.toggle_(_verb); } 
		static void set_verb(unsigned int _verb) { instance_.set_verb_(_verb); } 
	};
	template <typename D>  led_s<D> led_s<D>::instance_;
	
	class  dummy_led {
		public:
			static void on(unsigned int _verb = 0) { }
			static void off(unsigned int _verb = 0) {  }
			static void toggle(unsigned int _verb = 0) {  }
			static void set_verb(unsigned int _verb) {  }
	};
}

#endif