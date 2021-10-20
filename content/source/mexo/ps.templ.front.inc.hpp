#define TEMPL_BEGIN
#include "mexo/ps.templ.prepare.hpp"

#include "mexo/dev.front.hpp"
namespace PS_TEMPLATE_NAME {
	template<typename types> struct action_t {
		::dev::action_s dev;
		bool invers;
		typename types::signal_t voltage;
		#if POWER_SUPPLY_CURRENT_MEASSURY_ENABLED == 1
		typename types::signal_t current;
		#endif
	};
	template<typename types>struct feedback_t {
		::dev::feetback_s dev;
		typename types::signal_t voltage;
		#if POWER_SUPPLY_CURRENT_MEASSURY_ENABLED == 1
		typename types::signal_t current;
		#endif
	};
};


#define TEMPL_FINISH
#include "mexo/ps.templ.prepare.hpp"

