#ifndef robo_prf_discretout_hpp
#define robo_prf_discretout_hpp
#include "core/robosd_common.hpp"
namespace robo {	
	namespace prf {	
		namespace discret {	
			namespace out {	
				class  dummy{
					public:
						static void on(void) { }
						static void off(void) {  }
				};
				//наследник от LED
				namespace masked {	
					enum class states{ on =1,off = 0};
					struct present_s{
						int verb;
						states  state;
					};
					
					template <typename D> class base_t : public D{			
						 present_s & present_;
					public:
						base_t(present_s & _present):D(),present_(_present){ }
						virtual ~base_t(void){}
						void set_verb(unsigned int _verb){
							present_.verb = _verb;
						}
						void on(unsigned int _verb = 0){				
							if(present_.verb == _verb){
								if(present_.state==states::off){
									D::on();
									present_.state=states::on;
								}
							}
						}
						void off(unsigned int _verb = 0){
							if(present_.verb == _verb){
								if(present_.state==states::on){
									D::off();
									present_.state=states::off;
								}
							}
						}
						void toggle(unsigned int _verb = 0){
							if(present_.verb == _verb){
								if(present_.state==states::on){
									D::off();
									present_.state=states::off;
								} else {
									D::on();
									present_.state=states::on;
								}
							}
						}
					};
					
					
					//доступно из любого модуля- инициализация автоматическая
					template < typename D > class common_t: public base_t<D>{
						static common_t & instance_(){
							static common_t instance__;
							return instance__;
						}
						using B =  base_t<D>;
						present_s present;
						common_t(void):B(present){}
					public:
						static void on(unsigned int _verb = 0) { instance_().B::on(_verb); } 
						static void off(unsigned int _verb = 0) { instance_().B::off(_verb); } 
						static void toggle(unsigned int _verb = 0) { instance_().B::toggle(_verb); } 
						static void set_verb(unsigned int _verb) { instance_().B::set_verb(_verb); } 
					};
					
					//работает только после инициализации
					template <typename D> class instance_t : public base_t<D>{
							using B =  base_t<D>;
							static instance_t instance_;
							present_s present;
							instance_t(void):B(present){}
						public:
							static void on(unsigned int _verb = 0) { instance_.B::on(_verb); } 
							static void off(unsigned int _verb = 0) { instance_.B::off(_verb); }
							static void toggle(unsigned int _verb = 0) { instance_.B::toggle(_verb); }
							static void set_verb(unsigned int _verb) { instance_.B::set_verb(_verb); }
					};
					template <typename D>  instance_t<D> instance_t<D>::instance_;
					
					//работает только после инициализации, причем instance объйвляется отдельно- там, где доступна структура present
					template <typename D> class wrapper_t : public base_t<D>{
							static wrapper_t instance;
							using B =  base_t<D>;
							wrapper_t(present_s & present):B(present){}
						public:
							static void on(unsigned int _verb = 0) { instance.B::on(_verb); } 
							static void off(unsigned int _verb = 0) { instance.B::off(_verb); }
							static void toggle(unsigned int _verb = 0) { instance.B::toggle(_verb); }
							static void set_verb(unsigned int _verb) { instance.B::set_verb(_verb); }
					};
				}
				namespace switcher {

					enum class states{ on =1,off = 0};
					enum class commands{ on =1,off = 2, none = 0, toggle = 3};
					struct present_s{
						commands command;
						states  state;
					};
					
					template <typename D> class pin_t : public D{
						public:
						pin_t(void):D(){}
						~pin_t(void){}
						void on(void){				
							D::on();
						}
						void off(void){
							D::off();
						}
						void toggle(void){
							if(D::active()){
								D::off();
							} else {
								D::on();
							}
						}
						bool active(void){
							return D::active();
						}
						template<typename T> pin_t & operator = (const T & _src ){
							if( (bool)_src){
								on();
							} else{
								off();
							}
							return *this;
						}
						operator bool() const{ return D::active(); }

					};
	
					template <typename D> class base_t : public D{			
						 present_s & present_;
						public:
						base_t(present_s & _present):D(),present_(_present){ }
						virtual ~base_t(void){}
						void poll(void){
							switch(present_.command){
								case commands::on:
									on();
									break;
								case commands::off:
									off();
									break;
								case commands::toggle:
									toggle();
									break;
								default:
									break;
							}
						}
						void on(void){				
							if(present_.state==states::off){
								D::on();
								present_.state=states::on;
							}
							present_.command = commands::none;
						}
						void off(void){
							if(present_.state==states::on){
								D::off();
								present_.state=states::off;
							}
							present_.command = commands::none;
						}
						void toggle(void){
							if(present_.state==states::on){
								D::off();
								present_.state=states::off;
							} else {
								D::on();
								present_.state=states::on;
							}
							present_.command = commands::none;
						}
					};
				//доступно из любого модуля- инициализация автоматическая
					template < typename D > class common_t: public base_t<D>{
						static common_t & instance_(){
							static common_t instance__;
							return instance__;
						}
						using B =  base_t<D>;
						present_s present;
						common_t(void):B(present){}
					public:
						static void on(void) { instance_().B::on(); } 
						static void off(void) { instance_().B::off(); } 
						static void toggle(void) { instance_().B::toggle(); } 
					};
					
					//работает только после инициализации
					template <typename D> class instance_t : public base_t<D>{
							using B =  base_t<D>;
							static instance_t instance_;
							present_s present;
							instance_t(void):B(present){}
						public:
							static void on(void) { instance_.B::on(); } 
							static void off(void) { instance_.B::off(); }
							static void toggle(void) { instance_.B::toggle(); }
					};
					template <typename D>  instance_t<D> instance_t<D>::instance_;
					
					//работает только после инициализации, причем instance объйвляется отдельно- там, где доступна структура present
					template <typename D> class wrapper_t : public base_t<D>{
							static wrapper_t instance;
							using B =  base_t<D>;
							wrapper_t(present_s & present):B(present){}
						public:
							static void on(void) { instance.B::on(); } 
							static void off(void) { instance.B::off(); }
							static void toggle(void) { instance.B::toggle(); }
					};
					
				}					
			}
		}
	}
}

#endif