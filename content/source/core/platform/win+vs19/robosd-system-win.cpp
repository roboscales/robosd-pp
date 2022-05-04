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
#else
#ifdef UNICODE
#undef UNICODE
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
		set_win_consol_color_(LightGreen, Black);
		break;
		case robo::log::verb::detail_4:
		set_win_consol_color_(White, Blue);
		break;
		case robo::log::verb::detail_5:
		set_win_consol_color_(White, Blue);
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
	time_us_t current_time_us_;
	time_us_t current_time_ms_;
	time_us_t period_us_;
	LARGE_INTEGER tickCurrent_;
	double us_per_tick_;
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
		ROBO_LBREAKN(::robo::ini::load(RT("SETTINGS"), RT("TIMER_PERIOD_US"), period_us_));
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

	#if ROBO_APP_MODULE_ENABLED == 1
	CRITICAL_SECTION critical_;
	CRITICAL_SECTION guard_;

	bool system::env::begin(void) {
		InitializeCriticalSection(&critical_);
		InitializeCriticalSection(&guard_);
		init_ = true;
		return true;
	}

	void system::env::finish(void) {
		init_ = false;
		DeleteCriticalSection(&critical_);
		DeleteCriticalSection(&guard_);
	}

	bool system::env::start(void) {
		ROBO_LBREAKN(timer_setup_());
		time_ms_t ms;
		ROBO_LBREAKN(::robo::ini::load(RT("SETTINGS"), RT("TIMER_SHOW_PERIOD_MS"), ms));
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
	system::guard::op system::env::critical_enter(void) {
		if (init_) {
			EnterCriticalSection(&critical_);
			return system::guard::op::enter;
		}
		else {
			return system::guard::op::skip;
		}
	}

	void system::env::critical_leave(void) {
		LeaveCriticalSection(&critical_);
	}

	bool system::env::is_frontend(void) {
		return backend_thread_id_ != std::this_thread::get_id();
	}

	bool system::env::is_backend(void) {
		return backend_thread_id_ == std::this_thread::get_id();
	}

	system::guard::op system::env::enter(void) {
		if (init_) {
			EnterCriticalSection(&guard_);
			return system::guard::op::enter;
		}
		else {
			return system::guard::op::skip;
		}
	}

	void system::env::leave(void) {
		LeaveCriticalSection(&guard_);
	}

	system::guard::op  system::env::lock(void) {
		if (init_) {
			EnterCriticalSection(&guard_);
			return system::guard::op::enter;
		}
		else {
			return system::guard::op::skip;
		}
	}

	void system::env::unlock(void) {
			LeaveCriticalSection(&guard_);
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

	#else

	void* system::env::critical_enter(void) {
		return nullptr;
	}

	void system::env::critical_leave(void* _context) {
		ROBO_UNUSED(_context);
	}
	bool g_fall_ = false;
	bool system::env::is_frontend(void) {
		return g_fall_==false;
	}

	bool system::env::is_backend(void) {
		return g_fall_ == true;
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
		g_fall_ = true;
	}

	void system::env::comeback(void) {
		g_fall_ = false;
	}

#endif

	void system::env::abort(void) {
		::abort();
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
#endif

namespace robo {
	bool file_exists_(cstr _file_name) {
		HANDLE hFile = CreateFile(
			_file_name, // file (or device) name
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
	#if ROBO_APP_LIB_TYPE == ROBO_APP_TYPE_WIN
	bool system::lib::exists(cstr _lib_name) {
		return file_exists_(_lib_name);
	}
	void* system::lib::proc_get(void* _handle, cstr _proc_name) {
		#if ROBO_UNICODE_ENABLED
		size_t sz;
		enum { nsz = 255 };
		char buf[nsz];
		wcstombs_s(&sz, buf, nsz, _proc_name, _TRUNCATE);
		void* proc = GetProcAddress((HMODULE)_handle, buf);
		#else
		void* proc = GetProcAddress((HMODULE)_handle, _proc_name);
		#endif
		ROBO_LBREAKN_F(proc != nullptr, RT("function isn't found '%s'"), _proc_name);
		return proc;
	}
	void* system::lib::load(cstr _lib_name) {
		void * _pinst = (void *)LoadLibrary(_lib_name);
		ROBO_LBREAKN_F(_pinst != nullptr, RT("error open lib '%s'"), _lib_name);
		return _pinst;
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
	#endif

	#if ROBO_APP_INI_TYPE == ROBO_APP_TYPE_WIN
	cstr g_robo_ini_fn = nullptr;
	bool system::ini::begin(cstr _ini) {
		g_robo_ini_fn = nullptr;
		string tmp(RT("%s"),_ini);
		ROBO_LBREAKN_F(file_exists_(_ini), "ini file '%s' is't found", tmp.c_str());
		g_robo_ini_fn = _ini;
		return true;
	}
	cstr system::ini::source(void) {
		return g_robo_ini_fn;
	}
	void system::ini::finish(void) {
		g_robo_ini_fn = nullptr;
	}
	bool system::ini::load_str(char_t* _dst, size_t _max_sz, cstr _section, cstr _key) {
		ROBO_LBREAKN_F(g_robo_ini_fn != nullptr, "ini is't initialized")

			#if ROBO_UNICODE_ENABLED == 1
			return GetPrivateProfileStringW(_section, _key, RT(""), _dst, (DWORD)_max_sz, g_robo_ini_fn) > 0;
		#else
			return GetPrivateProfileStringA(_section, _key, RT(""), _dst, (DWORD)_max_sz, g_robo_ini_fn) > 0;
		#endif
	}
	#endif

	#if ROBO_APP_SHARED_TYPE == ROBO_APP_TYPE_WIN

	class system::shared::driver {
		friend class shared;
		HANDLE hMap = NULL;
		HANDLE hLock = NULL;
		string name;
		string locker_name;
		size_t sz = 0;
		void* memo;
		bool open(cstr _name, size_t _sz) {
			hMap = NULL;
			hLock = NULL;
			name = _name;
			sz = _sz;
			locker_name.format(RT("%s_LOCKER"), _name);
			ROBO_JAMPN( (hLock = CreateMutex(NULL, FALSE, locker_name.c_str() )) != NULL, broke);
			ROBO_JAMPN((hMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, (DWORD)sz, name.c_str() )) != NULL, broke);
			ROBO_JAMPN( (memo = (void *)MapViewOfFile(hMap,  FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0)) != NULL, broke);
			return true;
		broke:
			close();
			return false;
		}
		void close(void) {
			if (memo != NULL) {
				UnmapViewOfFile(memo);
			}

			if (hMap != NULL) {
				CloseHandle(hMap);
				hMap = NULL;
			}

			if (hLock != NULL) {
				CloseHandle(hLock);
				hLock = NULL;
			}
		}
		void lock(void) {
			WaitForSingleObject(hLock, 10000);
		}
		void unlock(void) {
			ReleaseMutex(hLock);
		}
	};
	system::shared::shared(void): driver_(new driver ), ref_(*this,-1) {
	}
	system::shared::~shared(void) {
		delete driver_;
	}
	bool system::shared::driver_open(cstr _name, size_t _sz) {
		return driver_->open(_name,_sz);
	}
	void system::shared::driver_close(void) {
		driver_->close();
	}
	void system::shared::driver_lock(void) {
		driver_->lock();
	}
	void system::shared::driver_unlock(void) {
		driver_->unlock();
	}
	size_t system::shared::size(void) {
		return driver_->sz;
	}
	void * system::shared::memo(void) {
		return driver_->memo;
	}
	#endif

	#if ROBO_APP_CONSOL_TYPE == ROBO_APP_TYPE_WIN
		//namespace consol {
		PHANDLER_ROUTINE g_robo_consol_old_hander;

		BOOL CtrlHandler(DWORD fdwCtrlType) {
			switch (fdwCtrlType) {
				// Handle the CTRL-C signal.
			case CTRL_C_EVENT:
			system::consol::stop(system::consol::event::keypbrd);
			return(TRUE);

			// CTRL-CLOSE: confirm that the user wants to exit.
			case CTRL_CLOSE_EVENT:
			system::consol::stop(system::consol::event::app);
			return(TRUE);

			// Pass other signals to the next handler.
			case CTRL_BREAK_EVENT:
			system::consol::stop(system::consol::event::keypbrd);
			return (TRUE);

			case CTRL_LOGOFF_EVENT:
			system::consol::stop(system::consol::event::app);
			return TRUE;

			case CTRL_SHUTDOWN_EVENT:
			system::consol::stop(system::consol::event::app);
			return TRUE;

			default:
			return FALSE;
			}
		}

		bool system::consol::driver_begin(void){
			ROBO_LRET( (SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE) == TRUE) );
		}
		void system::consol::driver_finish(void) {
			SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, FALSE);
		}

	#endif
}



