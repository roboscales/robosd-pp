
#define TEMPL_BEGIN
#include "mexo/brake.templ.prepare.hpp"

#define PS_TEMPLATE_NAME BRAKE_PS_TEMPLATE_NAME 
#include "mexo/ps.templ.front.inc.hpp"
namespace BRAKE_TEMPLATE_NAME {
	namespace front {
		struct mode : public mexo::front::dev::mode{
		};
		struct power_mode {
			enum {
				dummy = 0
				#if BRAKE_VOLTAGE_MODE_ENABLED == 1
				, voltage = 1
				#endif
				#if BRAKE_CURRENT_MODE_ENABLED == 1  
				,current = 2
				#endif
			};
		};
		template<typename types> struct action_t {
			BRAKE_PS_TEMPLATE_NAME::front::action_t<types> ps;
			#if BRAKE_VOLTAGE_MODE_ENABLED == 1 \
			|| BRAKE_CURRENT_MODE_ENABLED == 1 
			#endif
		};
		template<typename types>struct feedback_t {
			BRAKE_PS_TEMPLATE_NAME::front::feedback_t<types> ps;
		};
	}
}

#define TEMPL_FINISH
#include "mexo/brake.templ.prepare.hpp"