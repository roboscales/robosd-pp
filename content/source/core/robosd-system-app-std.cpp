#include "core/robosd_system.hpp"
#include <cstdlib>
#if ROBO_APP_SYSTEM_RANDOM_ENABLED == 1
#include <ctime>
#endif
#if ROBO_APP_SYSTEM_MULTYTHRAD_ENABLED == 1
#include <thread>
#endif
namespace robo{
	void system::app::begin(void){
#if ROBO_APP_SYSTEM_RANDOM_ENABLED == 1
		std::srand((unsigned int)std::time(nullptr));
#endif
	}
	void system::app::finish(void){
	}
	time_us_t system::app::time_us(void){
		return 0;
	}
	time_ms_t system::app::time_ms(void){
		return 0;
	}
	time_us_t system::app::period_us(void){
		return 0;
	}
#if ROBO_APP_SYSTEM_MULTYTHRAD_ENABLED == 1
	bool system::app::is_backend(void){
		return false;
	}
	void system::app::yeld (void){
		std::this_thread::yield();
	}
	void system::app::wakeup(void){
	}
#endif
#if ROBO_APP_SYSTEM_RANDOM_ENABLED == 1
	random_t system::app::rand(random_t _max){
		return std::rand() % _max;
	}
#endif
	void system::app::crash(char const * /*_file*/, char const * /*_function*/, int /*_line*/){
		abort();
	}
}
