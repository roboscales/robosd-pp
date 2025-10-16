#include "core/robosd_system.hpp"

#include <sys/stat.h>
#include <filesystem>
#include <fstream>
#include <errno.h>
#include <dlfcn.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <linux/limits.h>
#include <unistd.h>
#include <stdio.h>
namespace robo {
	void show_err(cstr _sender) {
		if (errno) {
#if ROBO_UNICODE_ENABLED
			robo_errlog("%s failed with error %d: %S", _sender, errno, strerror(errno));
#else
			robo_errlog("%s failed with error %d: %s", _sender, errno, strerror(errno));
#endif
			errno = 0;
		}
		else {
			robo_errlog("%s failed with no system error", _sender);
		}
	}
	
	bool robo_os_PI_file_exists_(cstr _fn) {
		struct stat sb;
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
	#if ROBO_UNICODE_ENABLED
		ROBO_BREAKN_F(_pinst != nullptr, nullptr, RT("error open lib '%s %S'"), _lib_name, dlerror());
	#else
		ROBO_BREAKN_F(_pinst != nullptr, nullptr, RT("error open lib '%s %s'"), _lib_name, dlerror());
	#endif
	return _pinst;
}
void system::lib::free(void* _instance) {
	dlerror();
	dlclose(_instance);
	const char * e = dlerror();
	ROBO_VBREAKN_F(e ==0, RT("error free lib '%s'"), e);
}

int cp(const char *to, const char *from){
	std::filesystem::copy(from,to);
	return 0;
}
bool system::lib::copy(cstr _src, cstr _dst) {
	ROBO_LBREAKN_F(cp(_dst, _src) == 0, RT("error copy lib from '%s' to '%s'"), _src,_dst);
	return true;
}
bool system::lib::remove(cstr _lib_name) {
	ROBO_LBREAKN_F(std::remove(_lib_name) == 0, RT("error remove lib '%s'"), _lib_name);		
	return true;
}
}
#endif

#if ROBO_APP_SHARED_TYPE == ROBO_APP_TYPE_LINUX
#include <unistd.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "core/robosd_string.hpp"
namespace robo {
	class system::shared::driver {
		friend class shared;
		int handle = -1;
		string name;
		size_t sz = 0;
		void *memo = MAP_FAILED;
		bool open(cstr _name, size_t _sz) {
			handle = -1;
			name = _name;
			sz = _sz;

			// Get shared memory
			if ((handle = shm_open(name.c_str(), O_RDWR | O_CREAT, S_IRWXU | S_IRWXO)) == -1) {
				robo_errlog("shm_open %s", name.c_str());
				show_err("shm_open");
				goto broke;
			}
			if (ftruncate(handle, sz) == -1) {
				show_err("ftruncate");
				goto broke;
			}

			if ((memo = mmap(
				NULL
				, sz
				, PROT_READ | PROT_WRITE
				, MAP_SHARED
				, handle
				,0)
			) == MAP_FAILED) {
				robo_errlog("mmap %s", name.c_str());
				show_err("mmap");
				goto broke;
			}
			return true;
		broke:
			close();
			return false;
		}
		void close(void) {
			if (memo != MAP_FAILED) {
				if (munmap(memo, sz) == -1) {
					robo_errlog("munmap %s", name.c_str());
					show_err("munmap");
				}
				memo = MAP_FAILED;
			}

			if (handle != -1) {
				if (shm_unlink(name.c_str()) != 0) {
					robo_errlog("shm_unlink %s", name.c_str());
					show_err("shm_unlink");
				}
				handle = -1;
			}
		}
		void lock(void) {
			if (mlock(memo, sz) != 0) {
				robo_errlog("mlock %s", name.c_str());
				show_err("mlock");
			}
		}
		void unlock(void) {
			if (munlock(memo, sz) != 0) {
				robo_errlog("mlock %s", name.c_str());
				show_err("mlock");
			}
		}
	};
	system::shared::shared(void) : driver_(new driver), ref_(*this, -1) {
	}
	system::shared::~shared(void) {
		delete driver_;
	}
	bool system::shared::driver_open(cstr _name, size_t _sz) {
		return driver_->open(_name, _sz);
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
	void *system::shared::memo(void) {
		return driver_->memo;
	}
} // namespace robo

#endif

#endif
