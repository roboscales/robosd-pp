#include "core/robosd_system.hpp"
#include <mutex>
#include <chrono>
namespace robo{
#if ROBO_APP_SYSTEM_MULTYTHRAD_ENABLED == 1
	static std::mutex system_guard_mutex;
	static std::mutex system_critical_mutex;

	void *  system::os::enter(void){
		system_guard_mutex.lock();
	}
	void system::os::leave(void * /*_context*/){
		system_guard_mutex.unlock();
	}
	void system::os::lock(void){
		system_guard_mutex.lock();
	}
	void system::os::unlock(void){
		system_guard_mutex.unlock();
	}
	void system::os::critical_lock(void){
		system_critical_mutex.lock();
	}
	void system::os::critical_unlock(void){
		system_critical_mutex.unlock();
	}
#endif

	std::chrono::high_resolution_clock::time_point  system_begin_time_;
	
	time_us_t system::os::realtime_us(void){
		std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> time_span = std::chrono::duration<double> (now - system_begin_time_);
		return (time_us_t)( 1000000. * time_span.count() );
	}
	
	void system::os::begin(void){
		system_begin_time_ = std::chrono::high_resolution_clock::now();
	}
	void system::os::finish(void){
	}
}
