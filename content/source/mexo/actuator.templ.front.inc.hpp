
#define TEMPL_BEGIN
#include "mexo/actuator.templ.prepare.hpp"
#include "mexo/ps.templ.front.inc.hpp"

namespace ACTUATOR_TEMPLATE_NAME {
	template<typename types> struct action_t {
		ACTUATOR_PS_TEMPLATE_NAME::action_t<types> dev;
		bool invers;
		typename types::signal_t voltage;
		#if POWER_SUPPLY_CURRENT_MEASSURY_ENABLED == 1
		typename types::signal_t current;
		#endif
	};
	template<typename types>struct feedback_t {
		ACTUATOR_PS_TEMPLATE_NAME::feedback_t<types> dev;
		typename types::signal_t voltage;
		#if POWER_SUPPLY_CURRENT_MEASSURY_ENABLED == 1
		typename types::signal_t current;
		#endif
	};
};



#define TEMPL_FINISH
#include "mexo/ps.templ.prepare.hpp"