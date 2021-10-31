#ifndef robosd_system_std_hpp
#define robosd_system_std_hpp

#include "core/robosd_app.hpp"
#include "core/robosd_log.hpp"


#if ROBO_APP_LIB_TYPE == ROBO_APP_TYPE_NATIVE
namespace robo {
	namespace native {
		class lib {
		public:
			typedef  ::robo::list::unique<lib, int> map;
			typedef  map::ref ref;
			struct proc {
				cstr name;
				void* instance;
			};
		private:
			ref ref_;
			string name_;

			static map& libs(void);

			friend class functor;
			class functor {
				friend class lib;
				typedef  ::robo::list::unique<functor, int> map;
				typedef  map::ref ref;
				ref ref_;
				proc* proc_;
				functor(lib& _owner, proc* _proc) : ref_(*this, hash(_proc->name) ), proc_(_proc) {
					ref_.attach_to(_owner.procs_);
				}
			};

			functor::map procs_;


			static lib* find_by_instance_(const void* _instance);

			bool reg_proc(proc* _proc);

			functor* find_functor_by_name_(cstr _name);

			bool reg(proc _procs[], int _proc_count);
			void unreg();

			static lib* find_by_name_(cstr _name);
			int used_ = 0;
		public:
			static bool exists(cstr _name);
			static void *  proc_get(void* _instance, cstr _proc_name);
			static void* load(cstr _lib_name);
			static void free(void* _instance);
			lib(cstr _name,  proc _procs[], int _proc_count) : ref_(*this, hash(_name)) , name_(_name) {
				if (ref_.attach_to(libs())) {
					reg(_procs, _proc_count);
				}
			}
			~lib(void) {
				unreg();
			}
		};
	}
}
#endif
#endif
