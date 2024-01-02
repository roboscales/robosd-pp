#if (!defined(__robosd_app_tunning_hpp)) && defined(__robosd_common_hpp)
#define __robosd_app_tunning_hpp
#else
#error error of using robosd_app_tunning.hpp
#endif
#include "mexo++drive.common.hpp"
#define ROBO_APP_DEBUG_LOG_ENABLED 0

#define ROBO_APP_SYSTEM_ENABLED 1

#define ROBO_APP_ALLOC_TYPE ROBO_APP_TYPE_STD

#define ROBO_APP_ENV_TYPE ROBO_APP_TYPE_SPECIFIC

#define ROBO_LOG_APP_PRINT_TYPE ROBO_APP_TYPE_STD

#define ROBO_APP_FORMATING_TYPE ROBO_APP_TYPE_STD

#define ROBO_APP_FORMATING_TYPE ROBO_APP_TYPE_STD

#define ROBO_APP_PRINT_TYPE ROBO_APP_TYPE_SPECIFIC

#include <stdint.h>

#if MPPD_MEXO_TP1_ENABLED == 1
#define ROBO_APP_MEXO_DEBUG_TP1_ENABLED  1
#endif

#define APP_MEXO_SLOT_COUNT MPPD_MEXO_SLOT_COUNT

#define FMSTR_REC_TIMEBASE (32768+MPPD_TIMER_TICK_US)

#define ROBO_APP_MEXO_SIDE 

#define ROBO_APP_MEXO_SAMPLE_US MPPD_TIMER_TICK_US

