#include "core/robosd_system.hpp"

#if ROBO_APP_SYSTEM_ENABLED == 1
#include "windows.h"
namespace robo{
	
#if ROBO_APP_SYSTEM_MULTYTHRAD_ENABLED == 1	
		static int system_lock_count_ = 0;
		static int system_guest_count_ = 0;
		enum  system_state {system_enabled = 178, system_unknown = -178};
		static system_state system_state_ = system_unknown;
		
		system::guard::guard(void){
			if(system_state_ == system_enabled){
				if( app::is_backend()){
					if(system_lock_count_ == 0){
						os::lock();
					}
					system_lock_count_++;
				}else{
					context_ =os::enter();
					ROBO_APP_ASSERT( system_guest_count_ == 0 )
					system_guest_count_++;
					
				}
			}
		 }

		system::guard::~guard(void){
			if(system_state_ == system_enabled){            
				if(  app::is_backend()){
					ROBO_APP_ASSERT( system_lock_count_ > 0 )
					system_lock_count_--;
					if(system_lock_count_==0){
						os::unlock();
					}
				}else{
					system_guest_count_--;
					os::leave(context_);
				}
			}
		}
		void system::begin(void){
			os::begin();
			app::begin();
			ROBO_APP_ASSERT( system_state_ != system_enabled )
			system_state_ = system_enabled;
		}
		void system::finish(void){
			ROBO_APP_ASSERT( system_state_ != system_unknown )
			app::finish();
			os::finish();
			system_state_ = system_unknown;
		}
		static int system_critical_ = 0;
		system::critical::critical(void){
			if(system_state_ == system_enabled){            
				os::critical_lock();
				ROBO_APP_ASSERT(system_critical_==0);
				system_critical_++;
			}
		}
		system::critical::~critical(void){
			if(system_state_ == system_enabled){            
				ROBO_APP_ASSERT(system_critical_>0);
				system_critical_--;
				os::critical_unlock();
			}
		}
#endif

}
#endif

