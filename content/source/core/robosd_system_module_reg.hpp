#ifndef MODULE_NAME
#define MODULE_NAME unknown
#endif
#ifndef MODULE_NAME_STR
#define MODULE_NAME_STR "unknown"
#endif
#define MODULE_NAME_PREFIX(name)  _MODULE_NAME_PREFIX(name,MODULE_NAME)
#define _MODULE_NAME_PREFIX(name,prfx)  __MODULE_NAME_PREFIX(name,prfx)
#define __MODULE_NAME_PREFIX(name,prfx) prfx##_##name


#include "core/robosd_app.hpp"

#if ROBO_APP_LIB_TYPE == ROBO_APP_TYPE_NATIVE

#include "core/robosd_system_native.hpp"

#if ROBO_APP_MODULE_ENABLED  == 1
namespace MODULE_NAME {
	robo::app::module* MODULE_NAME_PREFIX(query)(void) {
		return &module::instance();
	}
	void MODULE_NAME_PREFIX(release)(robo::app::module*) {
	}
	
	robo::native::lib::proc procs[2] = {
		{ RT("robo_module_query"), (void*) & MODULE_NAME_PREFIX(query) }
		, { RT("robo_module_release"), (void*) & MODULE_NAME_PREFIX(release) }
	};
	robo::native::lib  lib_instance( MODULE_NAME_STR, procs, 2);
}
#endif
#else

#ifndef ROBO_EXPORT_RUNTIME_DECL
#define ROBO_EXPORT_RUNTIME_DECL
#endif

#ifndef ROBO_EXPORT
#define ROBO_EXPORT
#endif

extern "C" {
	ROBO_EXPORT robo::app::module* ROBO_EXPORT_RUNTIME_DECL robo_module_query(void) {
		return	 & MODULE_NAME::module::instance();
	}
	ROBO_EXPORT void ROBO_EXPORT_RUNTIME_DECL robo_module_release(robo::app::module*) {
	}
}
#endif
#undef MODULE_NAME
#undef MODULE_NAME_STR
#undef MODULE_NAME_PREFIX
#undef _MODULE_NAME_PREFIX
#undef __MODULE_NAME_PREFIX
