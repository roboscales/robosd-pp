
#define TEMPL_BEGIN
#include "mexo/actuator.templ.prepare.hpp"

#define PS_TEMPLATE_NAME ACTUATOR_PS_TEMPLATE_NAME 
#include "mexo/ps.templ.front.inc.hpp"

namespace ACTUATOR_TEMPLATE_NAME {
	template<typename types> struct action_t {
		ACTUATOR_PS_TEMPLATE_NAME::action_t<types> ps;
		
		#if ACTUATOR_SPEED_OV_CURRENT_MODE_ENABLED == 1  \
		|| ACTUATOR_SPEED_OV_VOLTAGE_CL_MODE_ENABLED == 1 
		typename types::signal_t speed;
		#endif
		#if ACTUATOR_POSITION_OV_CURRENT_MODE_ENABLED == 1  \
		|| ACTUATOR_POSITION_OV_VOLTAGE_CL_MODE_ENABLED == 1 
		typename types::long_signal_t position;
		#endif
	};
	template<typename types>struct feedback_t {
		ACTUATOR_PS_TEMPLATE_NAME::feedback_t<types> ps;
	};
};

#define TEMPL_FINISH
#include "mexo/actuator.templ.prepare.hpp"