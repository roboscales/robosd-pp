#include "core/robosd_system.hpp"

#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
#include <errno.h>
#include <dlfcn.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
namespace robo {
	bool robo_os_PI_file_exists_(cstr _fn) {	
		struct stat sb;
		//int result = stat(_fn, &sb);
		FILE *in = NULL;
		if ((in = fopen(_fn, "rb")) == NULL) {
			return false;
		}
		else {
			fclose(in);
			return true;				
		}
	}
}
#if ROBO_APP_SYSTEM_ENABLED  == 1

#if ROBO_APP_ENV_TYPE == ROBO_APP_TYPE_LINUX

#include <pthread.h>
namespace robo {
	double us_per_tick_;
	DWORD tick_per_period_;
	std::thread::id backend_thread_id_;
	std::thread::id dummy_thread_id_;
	DWORD  step_show_period_;
	DWORD step_show_tick_ = 0;
	time_us_t last_time_us_ = 0;


	bool init_ = false;
	bool system::env::begin(void) {
		init_ = true;
		#if ROBO_APP_CRITICAL_TYPE == ROBO_APP_TYPE_LINUX
		//todo
		#endif 
		switch_to_realtime_();
		return true;
	}
	void system::env::finish(void) {
		switch_to_normal_();
		#if ROBO_APP_CRITICAL_TYPE == ROBO_APP_TYPE_LINUX
		#endif
		init_ = false;
	}
	//todo костыль
	bool system_realtime_disabled_ = false;
	bool system::env::start(time_us_t & _period_us) {
		if (_period_us == 0) {
			system_realtime_disabled_ = true;
			ROBO_LBREAKN(::robo::ini::load(RT("SETTINGS"), RT("TIMER_PERIOD_US"), _period_us));
		}
		#if 0
		#if ROBO_APP_REALTIME_TYPE == ROBO_APP_TYPE_LINUX

		QueryPerformanceFrequency(&ticksPerSecond_);
		tick_per_period_ = (DWORD)(1.0 / 1000000.0 * _period_us * ticksPerSecond_.QuadPart);
		us_per_tick_ = 1000000.0 / ticksPerSecond_.QuadPart;
		QueryPerformanceCounter(&tickCurrent_);
		tickNext_.QuadPart = tickCurrent_.QuadPart + _period_us;
		#endif
		#endif
		
		ROBO_LBREAKN_F(_period_us > 0, "clock period is zero!");
		time_ms_t ms;
		::robo::ini::try_load(RT("SETTINGS"), RT("TIMER_SHOW_PERIOD_MS"), ms);
		step_show_period_ = 1000 * ms / _period_us;
		last_time_us_ = realtime_us();
		return true;
	}

	void system::env::stop(void) {}

	#if ROBO_APP_MODULE_ENABLED == 1
	result system::env::startup(void) {
		return result::complete;
	}
	result system::env::shutdown(void) {
		return result::complete;
	}
	#endif

	void system::env::backend_loop(void) {
		if (++step_show_tick_ == step_show_period_) {
			robo_infolog("tick  %3.3f", 0.000001 * realtime_us());
			step_show_tick_ = 0;
		}

	}
	

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

#if ROBO_APP_REALTIME_TYPE == ROBO_APP_TYPE_LINUX
	time_us_t system::env::realtime_us(void) {
		QueryPerformanceCounter(&tickCurrent_);
		double us = us_per_tick_ * tickCurrent_.LowPart;
		return (time_us_t)(us);
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
		Sleep(0);
	}
	#if ROBO_APP_OS_TYPE == ROBO_APP_TYPE_LINUX
	void system::env::switch_to_realtime(void) {
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
	}

	void system::env::switch_to_normal(void) {
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
	}	
	#endif
	
}
#endif
	
#if ROBO_APP_CONSOL_ENABLED == 1
#if ROBO_APP_CONSOL_TYPE ==  ROBO_APP_TYPE_LINUX
namespace robo {	
	extern "C" void robo_consol_break__(int s) {
		system::consol::stop(system::consol::event::keypbrd);	
	}	

	bool system::consol::driver_begin(void) {
		
		struct sigaction act = { };
		act.sa_handler = robo_consol_break__;
		sigemptyset(&act.sa_mask);                                                             
		sigaddset(&act.sa_mask, SIGINT); 
		ROBO_LBREAKN_F(sigaction(SIGINT, &act, 0) == 0, "sigaction error %d: %s", errno, strerror(errno));
		
		//ROBO_LBREAKN_F(signal(SIGINT, robo_consol_break__) == 0, "sigaction error %d: %s", errno, strerror(errno));
		return true;
	}
	
	void system::consol::driver_finish(void) {
	}
}
#endif
#endif


#if ROBO_APP_LIB_TYPE == ROBO_APP_TYPE_LINUX
namespace robo{
bool system::lib::exists(cstr _lib_name) {
	return robo_os_PI_file_exists_(_lib_name);
}
void* system::lib::proc_get(void* _handle, cstr _proc_name) {
	void * proc = dlsym(_handle, _proc_name);
	dlerror();
	ROBO_BREAKN_F(proc != nullptr, nullptr, RT("function isn't found '%s'"), _proc_name);
	return proc;
}
void* system::lib::load(cstr _lib_name) {
	dlerror();    
    void *_pinst = dlopen(_lib_name, RTLD_NOW);
	ROBO_BREAKN_F(_pinst != nullptr, nullptr, RT("error open lib '%s %S'"), _lib_name,dlerror());
	return _pinst;
}
void system::lib::free(void* _instance) {
	dlerror();
	dlclose(_instance);
	const char * e = dlerror();
	ROBO_VBREAKN_F(e ==0, RT("error free lib '%s'"), e);
}

int cp(const char *to, const char *from){
	int fd_to, fd_from;
	char buf[4096];
	ssize_t nread;
	int saved_errno;

	fd_from = open(from, O_RDONLY);
	if (fd_from < 0)
		return -1;

	fd_to = open(to, O_WRONLY | O_CREAT | O_EXCL, 0666);
	if (fd_to < 0)
		goto out_error;

	while (nread = read(fd_from, buf, sizeof buf), nread > 0)
	{
		char *out_ptr = buf;
		ssize_t nwritten;

		do {
			nwritten = write(fd_to, out_ptr, nread);

			if (nwritten >= 0)
			{
				nread -= nwritten;
				out_ptr += nwritten;
			}
			else if (errno != EINTR)
			{
				goto out_error;
			}
		} while (nread > 0);
	}

	if (nread == 0)
	{
		if (close(fd_to) < 0)
		{
			fd_to = -1;
			goto out_error;
		}
		close(fd_from);

		/* Success! */
		return 0;
	}

out_error:
	saved_errno = errno;

	close(fd_from);
	if (fd_to >= 0)
		close(fd_to);

	errno = saved_errno;
	return -1;
}
bool system::lib::copy(cstr _src, cstr _dst) {
	ROBO_LBREAKN_F(cp(_dst, _src) == 0, RT("error copy lib from '%s' to '%s'"), _src,_dst);
	return true;
}
bool system::lib::remove(cstr _lib_name) {
	ROBO_LBREAKN_F(remove(_lib_name) != 0, RT("error remove lib '%s'"), _lib_name);		
	return true;
}
}
#endif

#endif
