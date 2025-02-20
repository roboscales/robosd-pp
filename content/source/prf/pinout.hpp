#ifndef mexo_pinout_hpp
#define mexo_pinout_hpp
#include "core/robosd_common.hpp"
namespace robo {	

	template <typename D> class pinout_base_t : public D{
		enum class state{ on =1,off = 0} state_ = state::off;
		protected:
		pinout_base(void):D(){}
		~pinout_base(void){}
		void on(void){				
			if(state_==state::off){
				D::on();
				state_=state::on;
			}
		}
		void off(void){
			if(state_==state::on){
				D::off();
				state_=state::off;
			}
		}
		void toggle(void){
			if(state_==state::on){
				D::off();
				state_=state::off;
			} else {
				D::on();
				state_=state::on;
			}
		}
		bool active(void){
			return state_==state::on;
		}
	};


	template <typename D> class pinoutcom_s : public pinout_base<D>{
		static pinout_s & instance_(){
			static pinout_s instance__;
			return instance__;
		}
	public:
		static void on(void) { instance_().pinout_base<D>::on(); } 
		static void off(void) { instance_().pinout_base<D>::off(); } 
		static void toggle(void) { instance_().pinout_base<D>::toggle(); } 
		static bool active(void) { return instance_().pinout_base<D>::active(); } 
	};

	template <typename D> class pinout : public pinout_base<D>{
		static pinout instance_;
	public:
		static void on(void) { instance_.pinout_base<D>::on(); } 
		static void off(void) { instance_.pinout_base<D>::off(); } 
		static void toggle(void) { instance_.pinout_base<D>::toggle(); } 
		static bool active(void) { return instance_.pinout_base<D>::active(); } 
		static void perform( uint8_t _command) { 
			switch (_command){
				case 1: on(); break;
				case 2: toggle(); break;
				default : off();
			}
		} 
	};
	template <typename D>  pinout<D> pinout<D>::instance_;

	template <typename D> class swch:public D  {
	public:
		void on(void) { D::on(); } 
		void off(void) { D::off(); } 
	};
	
	class  dummy_pinout {
		public:
			static void on(void) { }
			static void off(void) {  }
	};
	
	template< class P, class T> class blink_driver_t : public ::robo::pinout_base<P>, public T {
		using A = ::mexo::pinout_base<P>;
	protected:
		virtual void operator ()(void){
			A::toggle();
		}

		blink_driver_t(): 
			A()
		, T(){
		}
		
		void start(void){ T::start();}
		void stop(void){ T::stop();}
	};
	
	
	
	template< class P, class T> class blink_t : public ::robo::timer_t<blink_driver_t<P,T> > {
		using A = ::robo::pinout_base<P>;
		using B = ::robo::timer_t<blink_driver_t<P,T> >;
		static blink_t instance_;
	public:
		static void on(void){
			instance_.A::on();
			instance_.B::stop();
		} 
		static void off(void){
			instance_.A::off();
			instance_.B::stop();
		}
		static void run(::robo::time_us_t _period ){
			instance_.A::on();
			instance_.B::start(_period);
		}		
		static void irun(::robo::time_us_t _period ){
			instance_.A::off();
			instance_.B::start(_period);
		}		
		static bool active(void){
			return instance_.A::active();
		}
		static bool blinked(void){
			return instance_.B::started();
		}

	};
	template <class P, class T>  blink_t<P,T> blink_t<P,T>::instance_;
	
}

#endif