#include "core/robosd_system.hpp"

#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
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

#if ROBO_APP_OS_TYPE == ROBO_APP_TYPE_LINUX
#if ROBO_APP_ENV_ENABLED ==1
#include <pthread.h>
namespace robo {
	/*
	static volatile struct linux_begin
	{
		linux_begin(void)
		{
			pthread_attr_t tattr;
			pthread_attr_init(&tattr);
			ROBO_APP_ASSERT(pthread_attr_setschedpolicy(&tattr, SCHED_RR) == 0);		
		}
	} linux_begin_;
	int linux_set_priority_(int tPriority) {		
		int  policy;
		int old;
		struct sched_param param;
		pthread_attr_t tattr;
		pthread_attr_init(&tattr);
		ROBO_APP_ASSERT(pthread_attr_setschedpolicy(&tattr, SCHED_RR) == 0);
		
		pthread_getschedparam(pthread_self(), &policy, &param);
		old = param.sched_priority;
		param.sched_priority = tPriority;
		ROBO_APP_ASSERT( pthread_setschedparam(pthread_self(), SCHED_RR, &param) == 0 );
		return old;
	}
	*/
	//int old_priority_ = -1;
	
	void system::env::switch_to_normal(void) {
		//linux_set_priority_(old_priority_);
	}
	
	void system::env::switch_to_realtime(void) {
		//old_priority_ = linux_set_priority_(1/*sched_get_priority_max(SCHED_RR)*/);
	}
	
}
#endif
#endif
	
#if ROBO_APP_CONSOL_ENABLED == 1
#if ROBO_APP_CONSOL_TYPE ==  ROBO_APP_TYPE_LINUX
#include <signal.h>
#include <errno.h>
#include <string.h>
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

#endif


#if ROBO_APP_INI_TYPE == ROBO_APP_TYPE_LINUX	
#include "minIni.h"
namespace robo {
	cstr g_robo_ini_fn = nullptr;
	bool system::ini::begin(cstr _ini) {
		enum { PATH_MAX = 4000 };
		static char ini_fn_s[PATH_MAX];
		realpath(_ini, ini_fn_s);

		g_robo_ini_fn = ini_fn_s;
		ROBO_LBREAKN_F(robo_os_PI_file_exists_(ini_fn_s), "ini file ""%s"" isn't found", ini_fn_s);
		robo_infolog("accept ini file %s ", ini_fn_s);
		return true;
	}
	
	void system::ini::finish(void) {
		g_robo_ini_fn = nullptr;
	}

	void system::ini::load_data(char_t* _dst, size_t _max_sz, cstr _section, cstr _key, size_t& _size) {
		ROBO_VBREAKN_F(g_robo_ini_fn != nullptr, "ini is't initialized")
		if(_key == nullptr)
		{
			int ix = 0;
			while (_max_sz>1)
			{
				int cnt = ini_getkey(_section, ix, _dst, _max_sz, g_robo_ini_fn);
				if (cnt)
				{
					_dst[cnt] = 0;
					ix++;
					cnt++;
					_dst += cnt;
					_size += cnt;
					_max_sz -= cnt;					
				}
				else
				{
					break;
				}
			}

		} else
		{
			_size = ini_gets(_section, _key, "", _dst, _max_sz, g_robo_ini_fn);		
		}
	}
}
#endif