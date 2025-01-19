#ifndef robo_prf_led_hpp
#define robo_prf_led_hpp
#include "core/robosd_common.hpp"
namespace robo {	
	namespace prf {	
		template <typename D, typename V> class led_base_t : public D{			
			enum class state{ on =1,off = 0} state_ = state::off;
			V verb_ = 0;
		public:
			led_base_t(V _verb):D(), verb_(_verb){}
			~led_base_t(void){}
			void set_verb(unsigned int _verb){
				verb_ = _verb;
			}
			void on(unsigned int _verb = 0){				
				if(verb_ == _verb){
					if(state_==state::off){
						D::on();
						state_=state::on;
					}
				}
			}
			void off(unsigned int _verb = 0){
				if(verb_ == _verb){
					if(state_==state::on){
						D::off();
						state_=state::off;
					}
				}
			}
			void toggle(unsigned int _verb = 0){
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
		
		//Шаблон для светлячка, который можно использовать из любого модуля проекта
		//инициализируется автоматически
		template <typename D> class ledcom_t
			: public led_base_t<D,unsigned int>{
			using B = led_base_t<D,unsigned int>;
			unsigned int verb = 0;
			static ledcom_t & instance_(){
				static ledcom_t instance__;
				return instance__;
			}
			ledcom_t(void):B(verb){}
		public:
			static void on(unsigned int _verb = 0) { instance_().B::on(_verb); } 
			static void off(unsigned int _verb = 0) { instance_().B::off(_verb); } 
			static void toggle(unsigned int _verb = 0) { instance_().B::toggle(_verb); } 
			static void set_verb(unsigned int _verb) { instance_().B::set_verb(_verb); } 
		};


		//Шаблон для светлячка, который имеет статическую реализацию. Использовать можно
		//только после процедуры инициализации статических объектов
		template <typename D> class led_t : public led_base_t<D,unsigned int>{
			static led_t instance_;
			using B = led_base_t<D,unsigned int>;
		public:
			static void on(unsigned int _verb = 0) { instance_.B::on(_verb); } 
			static void off(unsigned int _verb = 0) { instance_.B::off(_verb); }
			static void toggle(unsigned int _verb = 0) { instance_.B::toggle(_verb); }
			static void set_verb(unsigned int _verb) { instance_.B::set_verb(_verb); }
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