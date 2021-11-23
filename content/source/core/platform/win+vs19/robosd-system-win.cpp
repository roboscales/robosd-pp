#include "core/robosd_system.hpp"
#include "core/robosd_log.hpp"
#include "core/robosd_string.hpp"
#include "core/robosd_ini.hpp"
#include <iostream>
#include <io.h>
#include <fcntl.h>
#include <thread>
#if ROBO_UNICODE_ENABLED
#ifndef UNICODE
#define UNICODE
#endif
#endif

#if ROBO_LOG_APP_PRINT_TYPE == ROBO_APP_TYPE_WIN
#include <windows.h>
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


	void set_win_consol_color_(consol_color_t text, consol_color_t background) {
		HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hStdOut, (WORD)((background << 4) | text));
	}
	void system::env::print(robo::log::verb _verb, cstr _format, va_list _args) {
		switch (_verb) {
		case robo::log::verb::error:
		set_win_consol_color_(LightRed, Black);
		break;
		case robo::log::verb::warning:
		set_win_consol_color_(Yellow, Black);
		break;
		case robo::log::verb::info:
		set_win_consol_color_(LightGreen, Black);
		break;
		case robo::log::verb::detail_1:
		set_win_consol_color_(LightCyan, Black);
		break;
		case robo::log::verb::detail_2:
		set_win_consol_color_(LightMagenta, Black);
		break;
		case robo::log::verb::detail_3:
		set_win_consol_color_(LightGreen, Blue);
		break;
		case robo::log::verb::detail_4:
		set_win_consol_color_(LightMagenta, Blue);
		break;
		case robo::log::verb::detail_5:
		set_win_consol_color_(LightCyan, Blue);
		break;
		case robo::log::verb::detail_6:
		set_win_consol_color_(White, Blue);
		break;
		case robo::log::verb::detail_7:
		set_win_consol_color_(White, Blue);
		break;
		default:
		set_win_consol_color_((consol_color_t)((int)_verb & 0xF),
							  (consol_color_t)(((int)_verb & 0xF0) >> 4));
		}

		system::printf(_format, _args);
		system::printf(RT("\n\r"));
		set_win_consol_color_(White, Black);
	}
}
#endif


#if ROBO_APP_PRINT_TYPE == ROBO_APP_TYPE_WIN
#ifndef ROBO_LOG_WIN_BUF_SIZE
#define ROBO_LOG_WIN_BUF_SIZE 4096
#endif

namespace robo {
	void system::env::print(cstr _s) {
		#if ROBO_UNICODE_ENABLED == 1
		_setmode(_fileno(stdout), _O_U8TEXT);
		std::wcout << _s;

		#else
		SetConsoleOutputCP(CP_UTF8);
		std::cout << _s;
		#endif
	}
}
#endif

#if ROBO_APP_ENV_TYPE == ROBO_APP_TYPE_WIN

#include <windows.h>
namespace robo {
	#if ROBO_APP_MODULE_ENABLED == 1
	time_us_t current_time_us_;
	time_us_t current_time_ms_;
	time_us_t period_us_;
	LARGE_INTEGER tickCurrent_;
	double us_per_tick_;
	CRITICAL_SECTION critical_;
	CRITICAL_SECTION guard_;
	LARGE_INTEGER ticksPerSecond_;
	LARGE_INTEGER tickNext_;
	DWORD tick_per_period_;
	time_us_t us_acc_ = 0;
	DWORD  step_show_period_;
	std::thread::id backend_thread_id_;
	std::thread::id dummy_thread_id_;
	time_us_t last_time_us_ = 0;
	DWORD step_show_tick_ = 0;
	bool init_ = false;
	bool timer_setup_(void) {
		ROBO_LBREAKN(::robo::ini::try_load(RT("SETTINGS"), RT("TIMER_PERIOD_US"), period_us_));
		QueryPerformanceFrequency(&ticksPerSecond_);
		tick_per_period_ = (DWORD)(1.0 / 1000000.0 * period_us_ * ticksPerSecond_.QuadPart);
		us_per_tick_ = 1000000.0 / ticksPerSecond_.QuadPart;
		QueryPerformanceCounter(&tickCurrent_);
		tickNext_.QuadPart = tickCurrent_.QuadPart + period_us_;
		current_time_us_ = 0;
		current_time_ms_ = 0;
		us_acc_ = 0;
		return true;
	}


	bool system::env::begin(void) {
		InitializeCriticalSection(&critical_);
		InitializeCriticalSection(&guard_);
		return true;
	}

	void system::env::finish(void) {
		DeleteCriticalSection(&critical_);
		DeleteCriticalSection(&guard_);
	}

	bool system::env::start(void) {
		ROBO_LBREAKN(timer_setup_());
		time_ms_t ms;
		ROBO_LBREAKN(::robo::ini::try_load(RT("SETTINGS"), RT("TIMER_SHOW_PERIOD_MS"), ms));
		step_show_period_ = 1000 * ms / period_us_;
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
	void system::env::frontend_loop(void) {}

	void system::env::backend_loop(void) {
		uint32_t tm = realtime_us();
		us_acc_ += period_us_;
		while (us_acc_ > 1000) {
			us_acc_ -= 1000;
			current_time_ms_++;
		}
		current_time_us_ += period_us_;

		if (++step_show_tick_ == step_show_period_) {
			robo_infolog("tick  %3.3f", 0.000001 * realtime_us());
			step_show_tick_ = 0;
		}

	}

	#endif

	void system::env::abort(void) {
		::abort();
	}

	void* system::env::critical_enter(void) {
		if (init_) {
			EnterCriticalSection(&critical_);
		}
		return nullptr;
	}

	void system::env::critical_leave(void* _context) {
		ROBO_UNUSED(_context);
		if (init_) {
			LeaveCriticalSection(&critical_);
		}

	}

	bool system::env::is_frontend(void) {
		return backend_thread_id_ != std::this_thread::get_id();
	}

	bool system::env::is_backend(void) {
		return backend_thread_id_ == std::this_thread::get_id();
	}

	void* system::env::enter(void) {
		if (init_) {
			EnterCriticalSection(&critical_);
			EnterCriticalSection(&guard_);
		}
		return nullptr;
	}

	void system::env::leave(void* _context) {
		ROBO_UNUSED(_context);
		if (init_) {
			LeaveCriticalSection(&critical_);
			LeaveCriticalSection(&guard_);
		}
	}

	void system::env::lock(void) {
		if (init_) {
			EnterCriticalSection(&guard_);
		}
	}

	void system::env::unlock(void) {
		if (init_) {
			LeaveCriticalSection(&guard_);
		}
	}

	void system::env::fall(void) {
		time_us_t tm = realtime_us();
		while (tm - last_time_us_ < period_us_) {
			Sleep(0); //?
			tm = realtime_us();
		}
		last_time_us_ = tm;
		backend_thread_id_ = std::this_thread::get_id();
	}

	void system::env::comeback(void) {
		backend_thread_id_ = dummy_thread_id_;
	}

	time_us_t system::env::time_us(void) {
		return current_time_us_;
	}

	time_us_t system::env::realtime_us(void) {
		QueryPerformanceCounter(&tickCurrent_);
		double us = us_per_tick_ * tickCurrent_.LowPart;
		return (time_us_t)(us);
	}

	time_ms_t system::env::time_ms(void) {
		return current_time_ms_;
	}

	random_t system::env::rand(random_t _max) {
		return (random_t)std::rand() % _max;
	}

	void system::env::wakeup(void) {}

	time_us_t system::env::period_us(void) {
		return 0;
	}

	void system::env::sleep(void) {}
}
#endif

#if ROBO_APP_LIB_TYPE == ROBO_APP_TYPE_WIN
#include <windows.h>
#include <cstdlib>
namespace robo {
	bool system::lib::exists(cstr _lib_name) {
		HANDLE hFile = CreateFile(
			_lib_name, // file (or device) name
			0, // query access only
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, // share mode
			NULL, // security attributes
			OPEN_EXISTING, // disposition
			FILE_FLAG_NO_BUFFERING | FILE_FLAG_SEQUENTIAL_SCAN, // flags & attributes
			NULL // template file
		);
		if (INVALID_HANDLE_VALUE != hFile) {
			CloseHandle(hFile);
			return true;
		}
		return false;
	}
	void* system::lib::proc_get(void* _handle, cstr _proc_name) {
		enum {nsz=255};
		char buf[nsz];
		size_t sz;
		wcstombs_s(&sz, buf, nsz, _proc_name, _TRUNCATE);
		return GetProcAddress((HMODULE)_handle, buf);
	}
	void* system::lib::load(cstr _lib_name) {
		void * _pinst = (void *)LoadLibrary(_lib_name);
		ROBO_LBREAKN_F(_pinst != nullptr, RT("error open lib '%s'"), _lib_name);
		return nullptr;
	}
	void system::lib::free(void* _instance) {
		ROBO_VBREAKN(FreeLibrary((HMODULE)_instance));
	}
	bool system::lib::copy(cstr _src, cstr _dst) {
		ROBO_LBREAKN_F(CopyFile(_src,_dst,FALSE), RT("error copy lib from '%s' to '%s'"), _src,_dst);
		return true;
	}
	bool system::lib::remove(cstr _lib_name) {
		ROBO_LBREAKN_F(DeleteFile(_lib_name), RT("error remove lib '%s'"), _lib_name);		
		return true;
	}
}
#endif


