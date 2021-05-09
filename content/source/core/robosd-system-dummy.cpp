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
	bool system::os::is_frontend(void) {
		return bakend_flag_==false;
	}
	bool system::os::is_backend(void) {
		return bakend_flag_==true;
	}
	void* system::os::enter(void) {
		return nullptr;
	}
	void system::os::leave(void* _context) {
		ROBO_UNUSED(_context);
	}
	void system::os::lock(void) {
	}
	void system::os::unlock(void) {
	}
	void system::os::fall(void) {
		bakend_flag_ = true;
	}
	void system::os::comeback(void) {
		bakend_flag_ = false;
	}
	time_us_t system::os::time_us(void) {
		return 0;
	}
	time_us_t system::os::realtime_us(void) {
		return 0;
	}
	time_ms_t system::os::time_ms(void) {
		return 0;
	}
	random_t system::os::rand(random_t _max) {
		return 0;
	}
	void system::os::wakeup(void) {
	}
	time_us_t system::os::period_us(void) {
		return 0;
	}
	void system::os::sleep(void) {
	}
	void system::os::begin(void) {
	}
	void system::os::finish(void) {
	}

	void* system::os::mem_alloc(size_t _size) {
		return malloc(_size);
	}
	void system::os::mem_free(void * _memo) {
		free(_memo);
	}

	bool system::os::sprintf(char_t* _dst, size_t _max_sz, cstr _format, va_list _args) {

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
	bool system::os::ini_init(cstr _ini) {
		g_robo_ini_fn = _ini;
		return true;
	}
	void system::os::ini_finish(void) {
		g_robo_ini_fn = nullptr;
	}
	bool system::os::ini_load_str(char_t* _dst, size_t _max_sz, cstr _section, cstr _key) {
		ROBO_LBREAKN_F(g_robo_ini_fn!=nullptr, "ini is't initialized")

#if ROBO_UNICODE_ENABLED == 1
		return GetPrivateProfileStringW(_section, _key, RT(""), _dst, (DWORD)_max_sz, g_robo_ini_fn) > 0;
#else
		return GetPrivateProfileStringA(_section, _key, _default, _value, (DWORD)_value_max, g_robo_ini_fn) > 0;
#endif
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

