#include "core/robosd_system.hpp"
#include "core/robosd_log.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>


namespace robo {
#if ROBO_APP_ENV_TYPE == ROBO_APP_TYPE_DUMMY
	bool bakend_flag_ = false;
	void  system::env::abort(void) {
		::abort();
	}

	bool system::env::start(void) {
		return true;
	}

	void system::env::stop(void) {
	}

	result system::env::startup(void) {
		return result::complete;
	}
	
	result system::env::shutdown(void) {
		return result::complete;
	}

	void system::env::frontend_loop(void) {
	}
	
	void system::env::backend_loop(void) {
	}


	bool system::env::is_frontend(void) {
		return bakend_flag_==false;
	}
	bool system::env::is_backend(void) {
		return bakend_flag_==true;
	}
	void* system::env::enter(void) {
		return nullptr;
	}
	void system::env::leave(void* _context) {
		ROBO_UNUSED(_context);
	}
	void system::env::lock(void) {
	}
	void system::env::unlock(void) {
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
	void system::env::wakeup(void) {
	}
	time_us_t system::env::period_us(void) {
		return 0;
	}
	void system::env::sleep(void) {
	}
	bool system::env::begin(void) {
		return true;
	}
	void system::env::finish(void) {
	}
#if ROBO_APP_ALLOC_TYPE == ROBO_APP_TYPE_DUMMY
	void* system::env::mem_alloc(size_t _size) {
		return malloc(_size);
	}
	void system::env::mem_free(void * _memo) {
		free(_memo);
	}
#endif

	size_t system::env::sprintf(char_t* _dst, size_t _max_sz, cstr _format, va_list _args) {

#if ROBO_UNICODE_ENABLED == 1
		size_t sz = vswprintf_s(_dst, _max_sz, _format, _args);
#else
		size_t sz = vsprintf(_dst, _format, _args);
#endif
		if (sz < _max_sz-1) {
			_dst[sz] = 0;
		}
		return sz ;
	}
#endif

	void system::env::print(cstr  _s) {
#if ROBO_UNICODE_ENABLED
		wprintf(_s);
#else
		printf(_s);
#endif
	}

#if ROBO_APP_INI_TYPE == ROBO_APP_TYPE_DUMMY
	cstr g_robo_ini_fn = nullptr;
	bool system::ini::begin(cstr _ini) {
		g_robo_ini_fn = _ini;
		return true;
	}
	void system::ini::finish(void) {
		g_robo_ini_fn = nullptr;
	}
	bool system::ini::load_str(char_t* _dst, size_t _max_sz, cstr _section, cstr _key) {
		ROBO_LBREAKN_F(g_robo_ini_fn!=nullptr, "ini is't initialized")

#if ROBO_UNICODE_ENABLED == 1
		return GetPrivateProfileStringW(_section, _key, RT(""), _dst, (DWORD)_max_sz, g_robo_ini_fn) > 0;
#else
		return GetPrivateProfileStringA(_section, _key, _default, _value, (DWORD)_value_max, g_robo_ini_fn) > 0;
#endif
	}
#endif

#if ROBO_APP_LIB_TYPE == ROBO_APP_TYPE_DUMMY
	void * system::lib::proc_get(void* _handle, cstr _proc_name) {
		return nullptr;
	}
	bool system::lib::exists(cstr _proc_name) {
		return false;
	}
	void*  system::lib::load(cstr _proc_name) {
		return nullptr;
	}
	void system::lib::free(void* _instance) {

	}

#endif

	void * system::env::critical_enter(void) {
		return nullptr;

	}
	void system::env::critical_leave(void * ) {

	}
}



