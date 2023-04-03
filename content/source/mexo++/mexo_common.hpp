#ifndef mexo_common_h
#define mexo_common_h
#include "core/robosd_common.hpp"

#ifndef ROBO_APP_MEXO_SLOT_COUNT
#define ROBO_APP_MEXO_SLOT_COUNT 4
#endif

#ifndef ROBO_APP_MEXO_REALTIME_SLOT_ENABLE
#define ROBO_APP_MEXO_REALTIME_SLOT_ENABLE 1
#endif


#ifndef ROBO_APP_MEXO_DEBUG_TP1_ENABLED
#define ROBO_APP_MEXO_DEBUG_TP1_ENABLED 0
#endif

#ifndef ROBO_APP_MEXO_EXTERNAL_CONFIGURE_NEED
#define ROBO_APP_MEXO_EXTERNAL_CONFIGURE_NEED 0
#endif
namespace mexo {
	
	struct tp_verb {		enum { frontend = 1, backend = 2, priority = 3, loop = 4 }; 	};
	
	enum class satstate_t { none, both, lo, hi };
	
	template <typename A> struct range_s {
		A lo;
		A hi;
	};

}

#endif