#include "core/robosd_system.hpp"
#include "core/robosd_log.hpp"
#include <stdlib.h>
#include <stdio.h>

namespace robo {
	#if ROBO_APP_ENV_TYPE == ROBO_APP_TYPE_DUMMY

	#if ROBO_APP_MODULE_ENABLED == 1
	bool system::env::begin(void) {
		return true;
	}

	void system::env::finish(void) {}

	bool system::env::start(void) {
		return true;
	}

	void system::env::stop(void) {}

	result system::env::startup(void) {
		return result::complete;
	}

	result system::env::shutdown(void) {
		return result::complete;
	}
	void system::env::frontend_loop(void) {}

	void system::env::backend_loop(void) {}
	#endif
	bool bakend_flag_ = false;

	void system::env::abort(void) {
		//::abort();
	}

	system::guard::op system::env::critical_enter(void) {
		return system::guard::op::skip;
	}

	void system::env::critical_leave(void) {
		ROBO_UNUSED(_op);
	}


	bool system::env::is_frontend(void) {
		return bakend_flag_ == false;
	}

	bool system::env::is_backend(void) {
		return bakend_flag_ == true;
	}

	system::guard::op system::env::enter(void) {
		return system::guard::op::skip;
	}

	void system::env::leave(void) {
		ROBO_UNUSED(_op);
	}

	system::guard::op system::env::lock(void) {
		return system::guard::op::skip;
	}

	void system::env::unlock(void) {
		ROBO_UNUSED(_op);
	}

	void system::env::fall(void) {
		bakend_flag_ = true;
	}

	void system::env::comeback(void) {
		bakend_flag_ = false;
	}

	time_us_t system::env::time_us(void) {
		return 0;
	}

	time_us_t system::env::realtime_us(void) {
		return 0;
	}

	time_ms_t system::env::time_ms(void) {
		return 0;
	}

	random_t system::env::rand(random_t _max) {
		return 0;
	}

	void system::env::wakeup(void) {}

	time_us_t system::env::period_us(void) {
		return 0;
	}

	void system::env::sleep(void) {}

	#if ROBO_APP_FORMATING_TYPE == ROBO_APP_TYPE_DUMMY
	size_t system::env::sprintf(char_t* _dst, size_t _max_sz, cstr _format, va_list _args) {
		return 0;
	}
	#endif

	#if ROBO_APP_PRINT_TYPE == ROBO_APP_TYPE_DUMMY
	void system::env::print(cstr _s) {}

	#if ROBO_APP_DEBUG_LOG_ENABLED == 1
	void system::env::print(robo::log::verb _verb, cstr _format, va_list  _args) {}
	#endif

	#endif

	#if ROBO_APP_ALLOC_TYPE == ROBO_APP_TYPE_DUMMY
	void* system::env::mem_alloc(size_t _size) {
		return malloc(_size);
	}
	void system::env::mem_free(void* _memo) {
		free(_memo);
	}
	#endif

	#endif

	#if ROBO_APP_INI_TYPE == ROBO_APP_TYPE_DUMMY
	bool system::ini::begin(cstr _ini) {
		return true;
	}
	void system::ini::finish(void) {
	}
	void system::ini::load_data(char_t* _dst, size_t _max_sz, cstr _section, cstr _key, size_t& _size)
	{
		
	}
	#endif

	#if ROBO_APP_LIB_TYPE == ROBO_APP_TYPE_DUMMY
	void* system::lib::proc_get(void* _handle, cstr _proc_name) {
		return nullptr;
	}
	bool system::lib::exists(cstr _proc_name) {
		return false;
	}
	void* system::lib::load(cstr _proc_name) {
		return nullptr;
	}
	void system::lib::free(void* _instance) {

	}
	bool system::lib::copy(cstr _src, cstr _dst) {
		return false;
	}

	#endif
	
	#if ROBO_APP_OS_TYPE == ROBO_APP_TYPE_DUMMY
	#if ROBO_APP_ENV_ENABLED ==1
	void system::env::switch_to_normal(void) {
		
	}
	void system::env::switch_to_realtime(void) {
	}
	#endif
	#endif
	
	#if ROBO_APP_CONSOL_ENABLED == 1
	#if ROBO_APP_CONSOL_TYPE ==  ROBO_APP_TYPE_DUMMY
	bool system::consol::driver_begin(void) {
		return true;
	}
	void system::consol::driver_finish(void)
	{
	}
	#endif
	#endif
}



