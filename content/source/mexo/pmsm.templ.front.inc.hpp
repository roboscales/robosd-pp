#define TEMPL_BEGIN
#include "mexo/pmsm.templ.prepare.hpp"

#define ACTUATOR_TEMPLATE_NAME PMSM_ACTUATOR_TEMPLATE_SUB_NAME
#define ACTUATOR_PS_TEMPLATE_NAME PMSM_PS_CROSS_TEMPLATE_NAME

#include "mexo/actuator.templ.front.inc.hpp"

namespace PMSM_TEMPLATE_NAME {

	template<typename types> struct action_t {
		::PMSM_ACTUATOR_TEMPLATE_SUB_NAME::action_t<types> actuator;
		typename types::long_signal_t freq;
		typename types::long_signal_t angle;
		typename types::signal_t voltage_lateral;
	};
	template<typename types>struct feedback_t {
		::PMSM_ACTUATOR_TEMPLATE_SUB_NAME::action_t<types> actuator;
	};
};

#define TEMPL_FINISH
#include "mexo/pmsm.templ.prepare.hpp"