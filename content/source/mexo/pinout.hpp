#ifndef mexo_pinout_hpp
#define mexo_pinout_hpp
#include "core/robosd_common.hpp"
#include "mexo/mexo.hpp"
namespace mexo {	

	template <typename D> class pinout_base : public D{
		enum class state{ set =1,reset = 0} state_ = state::reset;
		protected:
		pinout_base(void):D(){}
		~pinout_base(void){}
		void set(void){				
			if(state_==state::reset){
				D::set();
				state_=state::set;
			}
		}
		void reset(void){
			if(state_==state::set){
				D::reset();
				state_=state::reset;
			}
		}
		void toggle(void){
			if(state_==state::set){
				D::reset();
				state_=state::reset;
			} else {
				D::set();
				state_=state::set;
			}
		}
		bool active(void){
			return state_==state::set;
		}
	};

	template <typename D> class pinout_s : public pinout_base<D>{
		static pinout_s & instance_(){
			static pinout_s instance__;
			return instance__;
		}
	public:
		static void set(void) { instance_().pinout_base<D>::set(); } 
		static void reset(void) { instance_().pinout_base<D>::reset(); } 
		static void toggle(void) { instance_().pinout_base<D>::toggle(); } 
		static bool active(void) { return instance_().pinout_base<D>::active(); } 
	};

	template <typename D> class pinout : public pinout_base<D>{
		static pinout instance_;
	public:
		static void set(void) { instance_.pinout_base<D>::set(); } 
		static void reset(void) { instance_.pinout_base<D>::reset(); } 
		static void toggle(void) { instance_.pinout_base<D>::toggle(); } 
		static bool active(void) { return instance_.pinout_base<D>::active(); } 
		static void perform( uint8_t _command) { 
			switch (_command){
				case 1: set(); break;
				case 2: toggle(); break;
				default : reset();
			}
		} 
	};
	template <typename D>  pinout<D> pinout<D>::instance_;

	template <typename D> class swch:public D  {
	public:
		void on(void) { D::set(); } 
		void off(void) { D::reset(); } 
	};
	
	class  dummy_pinout {
		public:
			static void set(void) { }
			static void reset(void) {  }
	};
	
	template< class P, class T> class blink_driver_t : public ::mexo::pinout_base<P>, public T {
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
	
	
	
	template< class P, class T> class blink_t : public ::mexo::timer_t<blink_driver_t<P,T> > {
		using A = ::mexo::pinout_base<P>;
		using B = ::mexo::timer_t<blink_driver_t<P,T> >;
		static blink_t instance_;
	public:
		static void on(void){
			instance_.A::set();
			instance_.B::stop();
		} 
		static void off(void){
			instance_.A::reset();
			instance_.B::stop();
		}
		static void run(::robo::time_us_t _period ){
			instance_.A::set();
			instance_.B::start(_period);
		}		
		static bool active(void){
			return instance_.A::active();
		}

	};
	template <class P, class T>  blink_t<P,T> blink_t<P,T>::instance_;
	
}

#endif