#include "core/robosd_system.hpp"
namespace robo {
	#if ROBO_APP_ENV_ENABLED ==1
	ROBO_WEAK bool system::env::begin(void) {
		return true;
	}

	ROBO_WEAK void system::env::finish(void) {}

	ROBO_WEAK bool system::env::start(time_us_t& _period_us) {
		return true;
	}

	ROBO_WEAK void system::env::stop(void) {}

	#if ROBO_APP_MODULE_ENABLED==1
	ROBO_WEAK result system::env::startup(void) {
		return result::complete;
	}

	ROBO_WEAK result system::env::shutdown(void) {
		return result::complete;
	}
	#endif

	ROBO_WEAK void system::env::frontend_loop(void) {}

	ROBO_WEAK void system::env::backend_loop(void) {}
	

	bool bakend_flag_ = false;

	ROBO_WEAK void system::env::abort(void) {
		//::abort();
	}

	ROBO_WEAK system::guard::op system::env::critical_enter(void) {
		return system::guard::op::skip;
	}

	ROBO_WEAK void system::env::critical_leave(void) {
	}


	ROBO_WEAK bool system::env::is_frontend(void) {
		return bakend_flag_ == false;
	}

	ROBO_WEAK bool system::env::is_backend(void) {
		return bakend_flag_ == true;
	}

	ROBO_WEAK system::guard::op system::env::enter(void) {
		return system::guard::op::skip;
	}

	ROBO_WEAK void system::env::leave(void) {
	}

	ROBO_WEAK system::guard::op system::env::lock(void) {
		return system::guard::op::skip;
	}

	ROBO_WEAK void system::env::unlock(void) {
	}

	ROBO_WEAK void system::env::fall(void) {
		bakend_flag_ = true;
	}

	ROBO_WEAK void system::env::comeback(void) {
		bakend_flag_ = false;
	}


	ROBO_WEAK time_us_t system::env::realtime_us(void) {
		return system::time_us();
	}

	ROBO_WEAK random_t system::env::rand(random_t _max) {
		ROBO_UNUSED(_max);
		return 0;
	}

	ROBO_WEAK void system::env::wakeup(void) {}

	ROBO_WEAK void system::env::sleep(void) {}

	#if ROBO_APP_FORMATING_TYPE != ROBO_APP_TYPE_NONE
	ROBO_WEAK size_t system::env::sprintf(char_t* _dst, size_t _max_sz, cstr _format, va_list _args) {
		ROBO_UNUSED(_dst);
		ROBO_UNUSED(_max_sz);
		ROBO_UNUSED(_format);
		ROBO_UNUSED(_args);
		return 0;
	}
	#if ROBO_UNICODE_ENABLED == 1
	ROBO_WEAK size_t system::env::sprintf(char* _dst, size_t _max_sz, const char* _format, va_list _args) {
		ROBO_UNUSED(_dst);
		ROBO_UNUSED(_max_sz);
		ROBO_UNUSED(_format);
		ROBO_UNUSED(_args);
		return 0;
	}
	#endif
	#endif

	#if ROBO_APP_PRINT_TYPE != ROBO_APP_TYPE_NONE
	ROBO_WEAK void system::env::print(cstr _s) {
		ROBO_UNUSED(_s);
	}

	#if ROBO_APP_DEBUG_LOG_ENABLED == 1
	ROBO_WEAK void system::env::print(robo::log::verb _verb, cstr _format, va_list  _args) {
		ROBO_UNUSED(_verb);
		ROBO_UNUSED(_format);
		ROBO_UNUSED(_args);
	}
	#endif

	#if ROBO_APP_ALLOC_ENABLED ==1
	ROBO_WEAK void* system::env::mem_alloc(size_t _size) {
		return malloc(_size);
	}
	ROBO_WEAK void system::env::mem_free(void* _memo) {
		free(_memo);
	}
	ROBO_WEAK size_t system::env::mem_size(void* _memo) {
		ROBO_UNUSED(_memo);
		return 0;
	}
	#endif
	#endif
	#endif

	#if ROBO_APP_INI_ENABLED ==1
	ROBO_WEAK bool system::ini::begin(cstr _ini) {
		ROBO_UNUSED(_ini);
		return true;
	}
	ROBO_WEAK void system::ini::finish(void) {
	}
	ROBO_WEAK void system::ini::load_data(char_t* _dst, size_t _max_sz, cstr _section, cstr _key, size_t& _size)
	{
		ROBO_UNUSED(_dst);
		ROBO_UNUSED(_max_sz);
		ROBO_UNUSED(_section);
		ROBO_UNUSED(_key);
		ROBO_UNUSED(_size);
	}
	ROBO_WEAK cstr system::ini::source(void) {
		return nullptr;
	}
	#endif

	#if ROBO_APP_LIB_ENABLED ==1
	ROBO_WEAK void* system::lib::proc_get(void* _handle, cstr _proc_name) {
		ROBO_UNUSED(_handle);
		ROBO_UNUSED(_proc_name);
		return nullptr;
	}
	ROBO_WEAK bool system::lib::exists(cstr _proc_name) {
		ROBO_UNUSED(_proc_name);
		return false;
	}
	ROBO_WEAK void* system::lib::load(cstr _proc_name) {
		ROBO_UNUSED(_proc_name);
		return nullptr;
	}
	ROBO_WEAK void system::lib::free(void* _instance) {
		ROBO_UNUSED(_instance);
	}
	ROBO_WEAK bool system::lib::copy(cstr _src, cstr _dst) {
		return false;
	}
	ROBO_WEAK bool system::lib::remove(cstr _lib_name) {
		ROBO_UNUSED(_lib_name);
		return false;
	}
	#endif
	
	#if ROBO_APP_OS_TYPE != ROBO_APP_TYPE_NONE
	#if ROBO_APP_ENV_ENABLED ==1
	ROBO_WEAK void system::env::switch_to_normal(void) {
		
	}
	ROBO_WEAK void system::env::switch_to_realtime(void) {
	}
	#endif
	#endif

	#if ROBO_APP_CONSOL_ENABLED == 1
	ROBO_WEAK bool system::consol::driver_begin(void) {
		return true;
	}
	ROBO_WEAK void system::consol::driver_finish(void)
	{
	}
	#endif
	
	#if ROBO_APP_SHARED_ENABLED ==1
	bool system::shared::driver_open(cstr _name, size_t _sz) {
		ROBO_UNUSED(_name);
		ROBO_UNUSED(_sz);
		return false;
	}
	void system::shared::driver_close(void) {
	}
	#endif
}

