#ifndef burst_common_hpp
#define burst_common_hpp
#include "core/robosd_common.hpp"

#ifndef BURST_SIGNAL_T 
#define BURST_SIGNAL_T int16_t
#endif
#ifndef BURST_LONG_SIGNAL_T 
#define BURST_LONG_SIGNAL_T int32_t
#endif

namespace burst {
	typedef robo::time_us_t time_us_t;
	typedef BURST_SIGNAL_T signal_t;
	typedef BURST_LONG_SIGNAL_T long_signal_t;
	enum class satstate_t { none, both, low, up };
	template <typename A> struct range_s {
		A low;
		A hi;
	};
	struct  hyst_s {
		signal_t overhi;
		signal_t hi;
		signal_t lo;
		signal_t ultralo;
	};
}

#ifndef BURST_PROTECTION_ENABLED
#define BURST_PROTECTION_ENABLED 1
#endif

#ifndef BURST_PANICS_MASTER_LOST_ENABLED
#define BURST_PANICS_MASTER_LOST_ENABLED 1
#endif

#ifndef BURST_PANICS_BOARD_TEMPER_ENABLED
#define BURST_PANICS_BOARD_TEMPER_ENABLED 1
#endif

#ifndef BURST_PANICS_BOARD_VOLTAGE_ENABLED
#define BURST_PANICS_BOARD_VOLTAGE_ENABLED 1
#endif

#ifndef BURST_PANICS_BOARD_CURRENT_ENABLED
#define BURST_PANICS_BOARD_CURRENT_ENABLED 1
#endif

#ifndef BURST_QUEUE_ENABLED
#define BURST_QUEUE_ENABLED 1
#endif

#ifndef BURST_VERCION
#define BURST_VERCION 0
#endif

#ifndef BURST_DEBUG_TP_ENABLED
#define BURST_DEBUG_TP_ENABLED 1
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

#ifndef ROBO_APP_BURST_VARTABLE_ENABLE
#define ROBO_APP_BURST_VARTABLE_ENABLE 0
#endif

#ifndef ROBO_APP_BURST_PARANOIC_ENABLE
#define ROBO_APP_BURST_PARANOIC_ENABLE 0
#endif

#endif