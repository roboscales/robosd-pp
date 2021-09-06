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
			return libs().find(fast_hash(_name));
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
			return procs_.find(fast_hash(_name));
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

	bool system::lib::exists(cstr _proc_name) {
		return native::lib::exists(_proc_name);
	}
	void * system::lib::proc_get(void* _handle, cstr _proc_name) {
		return native::lib::proc_get(_handle, _proc_name);
	}
	void *  system::lib::load( cstr _proc_name ) {
		return native::lib::load( _proc_name );
	}
	void system::lib::free(void* _instance) {

	}
}
#endif


