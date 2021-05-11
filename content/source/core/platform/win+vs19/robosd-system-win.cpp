#include "core/robosd_system.hpp"
#include "core/robosd_log.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>

#ifndef ROBO_APP_SYSTEM_DUMMY_ENABLED 
#define ROBO_APP_SYSTEM_DUMMY_ENABLED  0
#endif

#ifndef ROBO_APP_INI_DUMMY_ENABLED 
#define ROBO_APP_INI_DUMMY_ENABLED  0
#endif


#if ROBO_APP_SYSTEM_DUMMY_ENABLED == 1
namespace robo {
	bool bakend_flag_ = false;

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

	void* system::env::mem_alloc(size_t _size) {
		return malloc(_size);
	}
	void system::env::mem_free(void * _memo) {
		free(_memo);
	}

	bool system::env::sprintf(char_t* _dst, size_t _max_sz, cstr _format, va_list _args) {

#if ROBO_UNICODE_ENABLED == 1
		size_t sz = vswprintf_s(_dst, _max_sz, _format, _args);
#else
		size_t sz = vsprintf(_dst, _format, _args);
#endif
		if (sz < _max_sz-1) {
			_dst[sz] = 0;
		}
		return sz < _max_sz-1;

	}
#if ROBO_APP_INI_ENABLED ==1
	#if ROBO_APP_INI_DUMMY_ENABLED ==1 
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
#endif

#if ROBO_APP_LIB_ENABLED ==1
#if ROBO_APP_LIB_DUMMY_ENABLED ==1
	bool system::lib::proc_get(void* _handle, cstr _proc_name) {
		return false;
	}
	bool system::lib::exists(cstr _proc_name) {
		return false;
	}
	bool system::lib::load(cstr _proc_name, void*& _instance) {
		return false;
	}
	void system::lib::free(void* _instance) {

	}

#endif
#endif


	char const* fault_file_ = nullptr;
	char const* fault_function_ = nullptr;
	int fault_line = 0;

	void crash(char const* _file, char const* _function, int _line) {
		fault_file_ = _file;
		fault_function_ = _function;
		fault_line = _line;
		abort();
	}



}
#endif

