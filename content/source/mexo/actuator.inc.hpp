#ifndef ACTUATOR_TEMPLATE_NAME 
#define ACTUATOR_TEMPLATE_NAME actuator
#endif

#define ACTUATOR_PREFIX(name)  _ACTUATOR_PREFIX(name,ACTUATOR_TEMPLATE_NAME)
#define _ACTUATOR_PREFIX(name,prfx)  __ACTUATOR_PREFIX(name,prfx)
#define __ACTUATOR_PREFIX(name,prfx) prfx##_##name

#include "mexo/actuator.templ.inc.hpp"

#undef ACTUATOR_PREFIX
#undef _ACTUATOR_PREFIX
#undef __ACTUATOR_PREFIX
#undef ACTUATOR_TEMPLATE_NAME