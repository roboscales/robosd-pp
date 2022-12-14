#define TEMPL_BEGIN
#include "mexo/ps.templ.prepare.hpp"

#include "mexo/dev.front.hpp"
namespace PS_TEMPLATE_NAME {
	namespace front {
		struct mode : public ::mexo::front::dev::mode{ enum { 
			voltage = 1
			#if POWER_SUPPLY_CURRENT_MEASSURY_ENABLED == 1
			, current = 2
			, voltage_cl = 16 
			#endif
		}; };
		template<typename types> struct action_t {
			mexo::front::dev::action_s dev;
			mexo_proto_signal_t voltage;
			#if POWER_SUPPLY_CURRENT_MEASSURY_ENABLED == 1
			mexo_proto_signal_t current;
			#endif
		};
		template<typename types>struct feedback_t {
			mexo::front::dev::feedback_s dev;
			mexo_proto_signal_t voltage;
			#if POWER_SUPPLY_CURRENT_MEASSURY_ENABLED == 1
			mexo_proto_signal_t current;
			#endif
		};
	}
}


#define TEMPL_FINISH
#include "mexo/ps.templ.prepare.hpp"

