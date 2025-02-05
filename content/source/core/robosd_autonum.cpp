#include "robosd_autonum.hpp"
namespace robo {
	namespace delegat {
		namespace autonum {
			receicledbin::ref::core& receicledbin::ref::frontend_core_(void) {
				static core frontend_core__(RT("frontend"));
				return frontend_core__;
			}

			receicledbin::ref::core& receicledbin::ref::backend_core_(void) {
				static core backend_core__(RT("backend"));
				return backend_core__;
			}
			void receicledbin::ref::release_(void) {
				if (isfrontend_) {
					#if ROBO_SYSTEM_ENABLED
					system::guard g__;
					#endif
					rbref_.attach_to(frontend_core_().rbin);
				}
				else {
					#if ROBO_SYSTEM_ENABLED
					system::guard g__;
					#endif
					rbref_.attach_to(backend_core_().rbin);
				}
			}
			void receicledbin::ref::clean_(rblist& _list) {
				ref* tmp;
				while (true) {
					{
						#if ROBO_SYSTEM_ENABLED
						system::guard g__;
						#endif
						tmp = _list.pop();
					}
					if (tmp) {
						delete tmp;
					}
					else {
						break;
					}
				}
			}
			void receicledbin::frontend_clean(void) { ref::clean_(ref::frontend_core_().rbin); }
			void receicledbin::backend_clean(void) { ref::clean_(ref::backend_core_().rbin); }

		}
	}
}