#include "core/robosd_system.hpp"
#include <cstdlib>
#include <cstdio>
#if ROBO_LOG_APP_PRINT_TYPE == ROBO_APP_TYPE_STD
#if ROBO_APP_DEBUG_LOG_ENABLED == 1
namespace robo {
	typedef enum {
		Black = 0,
		Blue = 1,
		Green = 2,
		Cyan = 3,
		Red = 4,
		Magenta = 5,
		Brown = 6,
		LightGray = 7,
		DarkGray = 8,
		LightBlue = 9,
		LightGreen = 10,
		LightCyan = 11,
		LightRed = 12,
		LightMagenta = 13,
		Yellow = 14,
		White = 15
	} consol_color_t;

	static cstr consol_colors_[16] = {
			RT("\x1B[30m"),
			RT("\x1B[34m"),
			RT("\x1B[32m"),
			RT("\x1B[36m"),
			RT("\x1B[31m"),
			RT("\x1B[35m"),
			RT("\x1B[30m"),
			RT("\x1B[37m"),
			RT("\x1B[37m"),
			RT("\x1B[34m"),
			RT("\x1B[32m"),
			RT("\x1B[36m"),
			RT("\x1B[31m"),
			RT("\x1B[35m"),
			RT("\x1B[33m"),
			RT("\x1B[37m")
	};

	static cstr background_colors_[16] = {
			RT("\x1B[40m"), //Black = 0,
			RT("\x1B[44m"), //Blue = 1,
			RT("\x1B[42m"), //Green = 2,
			RT("\x1B[46m"), //Cyan = 3,
			RT("\x1B[41m"), //Red = 4,
			RT("\x1B[45m"), //Magenta = 5,
			RT("\x1B[40m"), //Brown = 6,
			RT("\x1B[47m"), //LightGray = 7,
			RT("\x1B[47m"), //DarkGray = 8,
			RT("\x1B[44m"), //LightBlue = 9,
			RT("\x1B[42m"), //LightGreen = 10,
			RT("\x1B[46m"), //LightCyan = 11,
			RT("\x1B[41m"), //LightRed = 12,
			RT("\x1B[45m"), //LightMagenta = 13,
			RT("\x1B[43m"), //Yellow = 14,
			RT("\x1B[47m") //White = 15
	};

	void set_consol_color_(consol_color_t text, consol_color_t background) {
		#if ROBO_APP_PRINT_TYPE != ROBO_APP_TYPE_NONE	
		system::printf(consol_colors_[text]);
		system::printf((cstr)background_colors_[background]);
		#endif
	}
	#if ROBO_APP_PRINT_TYPE != ROBO_APP_TYPE_NONE
	#if ROBO_APP_DEBUG_LOG_ENABLED == 1
	void system::env::print(robo::log::verb _verb, cstr _format, va_list _args) {
		switch (_verb) {
		case robo::log::verb::error:
		set_consol_color_(LightRed, Black);
		break;
		case robo::log::verb::warning:
		set_consol_color_(Yellow, Black);
		break;
		case robo::log::verb::info:
		set_consol_color_(LightGreen, Black);
		break;
		case robo::log::verb::detail_1:
		set_consol_color_(White, DarkGray);
		break;
		case robo::log::verb::detail_2:
		set_consol_color_(White, Blue);
		break;
		case robo::log::verb::detail_3:
		set_consol_color_(White, Green);
		break;
		case robo::log::verb::detail_4:
		set_consol_color_(White, Magenta);
		break;
		case robo::log::verb::detail_5:
		set_consol_color_(White, Magenta);
		break;
		case robo::log::verb::detail_6:
		set_consol_color_(White, Magenta);
		break;
		case robo::log::verb::detail_7:
		set_consol_color_(White, Magenta);
		break;
		default:
		set_consol_color_((consol_color_t)((int)_verb & 0xF),
						  (consol_color_t)(((int)_verb & 0xF0) >> 4));
		}

		system::printf(_format, _args);
		system::printf(RT("\n\r"));
		set_consol_color_(White, Black);
	}
	#endif
	#endif
}
#endif
#endif

#if ROBO_APP_FORMATING_TYPE == ROBO_APP_TYPE_STD
namespace robo {
	size_t system::env::sprintf(char_t* _dst, size_t _max_sz, cstr _format, va_list _args) {
		#if ROBO_UNICODE_ENABLED == 1
		//todo косячная функция  - если вся информация не помещается в буфер, то она не выведеться вовсе
		int sz = vswprintf(_dst, _max_sz, _format, _args);
		#else
		int sz = vsnprintf(_dst, _max_sz, _format, _args);
		#endif
		if (sz > 0) {
			if (sz < (int) _max_sz - 1) {
				_dst[sz] = 0;
			}
			else {
				_dst[_max_sz-1] = 0;
			}
			return (size_t) sz;
		}
		else return 0;
	}
	#if ROBO_UNICODE_ENABLED == 1
	size_t system::env::sprintf(char* _dst, size_t _max_sz, const char* _format, va_list _args) {
		int sz = vsnprintf(_dst, _max_sz, _format, _args);
		if (sz > 0) {
			if (sz < (int)_max_sz - 1) {
				_dst[sz] = 0;
			}
			else {
				_dst[_max_sz - 1] = 0;
			}
		}
		return (size_t)sz;
	}
	#endif
}
#endif

#if ROBO_APP_PRINT_TYPE == ROBO_APP_TYPE_STD
namespace robo {
	void system::env::print(cstr _s) {
		#if ROBO_UNICODE_ENABLED
		wprintf(_s);
		#else
		::printf(_s);
		#endif
	}
}
#endif

#if ROBO_APP_ALLOC_TYPE == ROBO_APP_TYPE_STD
namespace robo {
	void* system::env::mem_alloc(size_t _size) {
		size_t* ptr = (size_t*)malloc(_size + sizeof(size_t));
		ROBO_APP_ASSERT(ptr!=nullptr);
		return ptr;
	}
	void system::env::mem_free(void* _memo) {
		free(_memo);
	}
	size_t system::env::mem_size(void* _memo) {
		//size_t* ptr = (size_t*)(_memo);
		//ptr--;
		//todo ?
		return 0;
	}
}
#endif


#if ROBO_APP_ENV_TYPE == ROBO_APP_TYPE_STD

#include <thread>
#include <chrono>
#include "core/robosd_ini.hpp"

#if ROBO_APP_MODULE_ENABLED == 1
#include "core/robosd_app.hpp"
#endif 

namespace robo {
	double us_per_tick_;
	time_us_t us_acc_ = 0;
	int  step_show_period_;
	std::thread::id backend_thread_id_;
	std::thread::id dummy_thread_id_;
	time_us_t last_time_us_ = 0;
	int step_show_tick_ = 0;
	
	std::chrono::steady_clock::time_point begin_ = std::chrono::steady_clock::now();
	

	bool system::env::begin(void) {
		//ROBO_LBREAKN_F( SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL) , "Failed to enter runtime mode (%d)", GetLastError())
		return true;
	}

	void system::env::finish(void) {
	}
	void system::env::frontend_loop(void) {}
//	bool system::env::start(unsigned int & /*_tmp*/ ) { return true; }
	//void system::env::stop(void) {  }
	void system::env::backend_loop(void) {
#if ROBO_APP_MODULE_ENABLED == 1
		if (++step_show_tick_ == step_show_period_) {
			robo_infolog("tick  %3.3f", 0.000001 * realtime_us());
			step_show_tick_ = 0;
		}
#endif
	}
	
#if ROBO_APP_MODULE_ENABLED == 1
	//todo костыль
	bool system::env::start(time_us_t & _period_us) {
		if (_period_us == 0) {
			ROBO_LBREAKN(::robo::ini::load(RT("SETTINGS"), RT("TIMER_PERIOD_US"), _period_us));
		}

		time_ms_t ms;
		ROBO_LBREAKN(::robo::ini::load(RT("SETTINGS"), RT("TIMER_SHOW_PERIOD_MS"), ms));
		step_show_period_ = 1000 * ms / _period_us;
		last_time_us_ = realtime_us();
		return true;
	}

	void system::env::stop(void) {}
	result system::env::startup(void) {
		return result::complete;
	}

	result system::env::shutdown(void) {
		return result::complete;
	}

	#endif


	bool system::env::is_frontend(void) {
		return backend_thread_id_ != std::this_thread::get_id();
	}

	bool system::env::is_backend(void) {
		return backend_thread_id_ == std::this_thread::get_id();
	}


	void system::env::fall(void) {
		backend_thread_id_ = std::this_thread::get_id();
	}

	void system::env::comeback(void) {
		backend_thread_id_ = dummy_thread_id_;
	}

#if ROBO_APP_REALTIME_TYPE == ROBO_APP_TYPE_STD
	time_us_t system::env::realtime_us(void) {
		std::chrono::steady_clock::time_point now_ = std::chrono::steady_clock::now();
		return std::chrono::duration_cast<std::chrono::microseconds>(now_ - begin_).count();
	}
#endif

	random_t system::env::rand(random_t _max) {
		return (random_t)std::rand() % _max;
	}

	random_t system::env::rand_maxd(void) {
		return RAND_MAX;
	}

	void system::env::wakeup(void) {}

	void system::env::sleep(void) {
		std::this_thread::yield();		
	}
}
#endif


#if ROBO_APP_ABORT_TYPE == ROBO_APP_TYPE_STD
void robo::system::env::abort(void) {
	::abort();
}
#endif

#if ROBO_APP_CRITICAL_TYPE == ROBO_APP_TYPE_STD
#include <mutex>
namespace robo {
	class ec {
	public:
		std::recursive_mutex critical;
		std::recursive_mutex guard;
		static ec & instance() {
			static ec  ec_;
			return ec_;
		}
	};
	system::guard::op system::env::critical_enter(void) {
		ec::instance().critical.lock();
		return system::guard::op::enter;
	}

	void system::env::critical_leave(void) {
		ec::instance().critical.unlock();
	}

	system::guard::op system::env::enter(void) {
		switch_to_realtime_();
		ec::instance().guard.lock();
		return system::guard::op::enter;
	}

	void system::env::leave(void) {
		ec::instance().guard.unlock();
		switch_to_normal_();
	}

	system::guard::op  system::env::lock(void) {
		ec::instance().guard.lock();
		return system::guard::op::enter;
	}

	void system::env::unlock(void) {
		ec::instance().guard.unlock();
	}
}
#endif

