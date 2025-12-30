#ifndef burst_common_hpp
#define burst_common_hpp
#include "core/robosd_common.hpp"


namespace burst {
	typedef robo::time_us_t time_us_t;

	enum class satstates { none, both, low, up };
	template <typename A> struct range_s {
		A lo;
		A hi;
	};
	#define BURST_RANGE_CONFIG(a) BURST_RANGE_CONFIG_(a)
	#define BURST_RANGE_CONFIG_(a) \
	{\
		a##_LO\
		, a##_HI\
	}
	template<typename R> struct  hyst_t {
		R overhi;
		R hi;
		R lo;
		R ultralo;
	};
}

#ifndef BURST_PROTECTION_ENABLED
#define BURST_PROTECTION_ENABLED 0
#endif

#ifndef BURST_PANICS_BOARD_RESET_TIMEOUT_PP
#define BURST_PANICS_BOARD_RESET_TIMEOUT_PP 5000000
#endif

#ifndef BURST_PANICS_MASTER_LOST_ENABLED
#define BURST_PANICS_MASTER_LOST_ENABLED 1
#endif

#ifndef BURST_PANICS_BOARD_TEMPER_ENABLED
#define BURST_PANICS_BOARD_TEMPER_ENABLED 0
#endif

#ifndef BURST_PANICS_BOARD_VOLTAGE_ENABLED
#define BURST_PANICS_BOARD_VOLTAGE_ENABLED 0
#endif

#ifndef BURST_PANICS_BOARD_CURRENT_ENABLED
#define BURST_PANICS_BOARD_CURRENT_ENABLED 0
#endif

#ifndef BURST_QUEUE_ENABLED
#define BURST_QUEUE_ENABLED 1
#endif

#ifndef BURST_VERCION
#define BURST_VERCION 0
#endif

#ifndef BURST_DEBUG_TP_ENABLED
#define BURST_DEBUG_TP_ENABLED 0
#endif

#ifndef BURST_CORE_DEBUG
#define BURST_CORE_DEBUG 1
#endif

#if BURST_QUEUE_ENABLED == 1
#ifndef BURST_FRONT_QUEUE_SIZE_BITS
#define BURST_FRONT_QUEUE_SIZE_BITS 2
#endif
#ifndef BURST_BACKEND_QUEUE_SIZE_BITS
#define BURST_BACKEND_QUEUE_SIZE_BITS 2
#endif
#endif

#ifndef ROBO_APP_BURST_REALTIME_SLOT_ENABLE
#define ROBO_APP_BURST_REALTIME_SLOT_ENABLE 1
#endif


#ifndef ROBO_APP_BURST_PARANOIC_ENABLE
#define ROBO_APP_BURST_PARANOIC_ENABLE 0
#endif

#ifndef ROBO_APP_BURST_VARTREE_ENABLED
#define ROBO_APP_BURST_VARTREE_ENABLED 0
#endif


#endif