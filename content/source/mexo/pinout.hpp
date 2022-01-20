#ifndef mexo_pinout_hpp
#define mexo_pinout_hpp
#include "core/robosd_common.hpp"
namespace mexo {	

	template <typename D> class pinout_base : public D{
		enum class state{ set =1,reset = 0} state_ = state::reset;
		unsigned int verb_ = 0;
		protected:
		pinout_base(void):D(){}
		~pinout_base(void){}
		void set_(void){				
			if(state_==state::reset){
				D::set();
				state_=state::set;
			}
		}
		void reset_(unsigned int _verb = 0){
			if(state_==state::set){
				D::reset();
				state_=state::reset;
			}
		}
		void toggle_(unsigned int _verb = 0){
			if(state_==state::set){
				D::reset();
				state_=state::reset;
			} else {
				D::set();
				state_=state::set;
			}
		}
		bool active_(void){
			return state_==state::set;
		}
	};

	template <typename D> class pinout_s : public pinout_base<D>{
		static pinout_s & instance_(){
			static pinout_s instance__;
			return instance__;
		}
	public:
		static void set(void) { instance_().set_(); } 
		static void reset(void) { instance_().reset_(); } 
		static void toggle(void) { instance_().toggle_(); } 
		static bool active(void) { return instance_().active_(); } 
	};

	template <typename D> class pinout : public pinout_base<D>{
		static pinout instance_;
	public:
		void set(void) { instance_.set_(); } 
		void reset(void) { instance_.reset_(); } 
		void toggle(void) { instance_.toggle_(); } 
		bool active(void) { return instance_.active_(); } 
		void perform( uint8_t _command) { 
			switch (_command){
				case 1: set(); break;
				case 2: toggle(); break;
				default : reset();
			}
		} 

	};
	template <typename D>  pinout<D> pinout<D>::instance_;
	
	class  dummy_pinout {
		public:
			static void set(void) { }
			static void reset(void) {  }
	};
	
}

#endif