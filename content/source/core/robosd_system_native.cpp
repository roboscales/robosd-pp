#include "core/robosd_system_native.hpp"
#include "core/robosd_system.hpp"
#if ROBO_APP_LIB_TYPE == ROBO_APP_TYPE_NATIVE
namespace robo {
	namespace native {

		bool lib::reg(proc _procs[], int _proc_count) {
			while (_proc_count--) {
				if (!reg_proc(_procs++)) {
					return false;
				}
			}
			return true;
		}

		void lib::unreg(void) {
			while (procs_.count()) {
				functor::ref* _ref = procs_.first();
				functor* f = &_ref->owner();
				_ref = nullptr;
				delete f;
			}

		}

		lib* lib::find_by_name_(cstr _name) {			
			return libs().find(hash(_name));
		}

		lib* lib::find_by_instance_(const void* _instance) {
			int id = (int)(_instance);
			return libs().find(id);
		}

		bool lib::reg_proc(proc* _proc) {
			functor* f = new functor(*this, _proc);
			if (f) {
				if (f->ref_.attached()) {
					return true;
				}
				else {
					delete f;
				}
			}
			return false;
		}

		lib::functor* lib::find_functor_by_name_(cstr _name) {
			return procs_.find(hash(_name));
		}

		bool lib::exists(cstr _name) {
			return find_by_name_(_name) != nullptr;
		}

		void * lib::proc_get(void* _instance, cstr _proc_name) {
			lib* _lib = find_by_instance_(_instance);
			if (_lib) {
				functor* f = _lib->find_functor_by_name_(_proc_name);
				if (f) {
					return f->proc_->instance;
				}
			}
			return nullptr;
		}

		void* lib::load(cstr _lib_name) {
			lib* _lib = find_by_name_(_lib_name);
			if (_lib != nullptr) {
				_lib->used_++;
				return (void*)_lib->ref_.key();
			}
			else {
				return  nullptr;
			}
		}

		void lib::free(void* _instance) {
			lib* _lib = find_by_instance_(_instance);
			if (_lib != nullptr) {
				_lib->used_--;
			}
		}

		lib::map& lib::libs(void) {
			static map instance_;
			return instance_;
		}

	}

	bool system::lib::exists(cstr _lib_name) {
		return native::lib::exists(_lib_name);
	}
	void * system::lib::proc_get(void* _handle, cstr  _lib_name) {
		return native::lib::proc_get(_handle, _lib_name);
	}
	void *  system::lib::load( cstr _lib_name) {
		return native::lib::load(_lib_name);
	}
	void system::lib::free(void* _instance) {
		native::lib::free(_instance);
	}
	bool system::lib::copy(cstr _src, cstr _dst) {
		return false;
	}
	bool system::lib::remove(cstr _lib_name) {
		return false;
	}

}
#endif

#if ROBO_APP_INI_TYPE == ROBO_APP_TYPE_NATIVE	
#include "core/robosd_ini_parser.h"
#include <fstream>

namespace robo {
	cstr g_robo_ini_fn = nullptr;
	bool system::ini::begin(cstr _ini) {
		g_robo_ini_fn = _ini;
		std::FILE* iniFile = std::fopen(_ini, "r");
		ROBO_LBREAKN_F(iniFile !=nullptr , "error load file %s", _ini)
		size_t bytesread;
		char * buf = new char[16000];
		bytesread = std::fread(&buf[0], sizeof buf[0], 16000, iniFile);
		robo_ipa_init(ROBO_IPA_NORMAL);
		robo_ipa_applay(buf, bytesread);
		delete[] buf;
		return true;
	}
	
	void system::ini::finish(void) {
		g_robo_ini_fn = nullptr;
	}
	
	void system::ini::load_data(char_t* _dst, size_t _max_sz, cstr _section, cstr _key, size_t& _size)
	{
		ROBO_VBREAKN_F(g_robo_ini_fn != nullptr, "ini is't initialized")
		_size = robo::robo_ipa_string_get(_section, _key, "", _dst, _max_sz);
	}
}
#endif


