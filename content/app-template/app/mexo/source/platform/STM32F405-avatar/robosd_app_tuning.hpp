#if (!defined(__robosd_app_tunning_hpp)) && defined(__robosd_common_hpp)
#define __robosd_app_tunning_hpp
#else
#error error of using robosd_app_tunning.hpp
#endif



#define ROBO_APP_DEBUG_LOG_ENABLED 1

#define ROBO_TERMINAL_PRINT_ENABLED 1

#define ROBO_APP_SYSTEM_ENABLED 1

#define ROBO_APP_ALLOC_TYPE ROBO_APP_TYPE_STD

#define ROBO_APP_ENV_TYPE ROBO_APP_TYPE_KEIL

#define ROBO_APP_TYPE_KEIL 101

#include <stdint.h>

//#define APP_MEXO_SIGNAL_T int16_t
//#define APP_MEXO_PARAMETR_T int16_t
//#define APP_MEXO_LONG_SIGNAL_T int32_t

#define MEXO_DEBUG_TP1_ENABLED  1
#define ROBO_APP_FREEMASTER_SERIAL_ENABLED 1
#define ROBO_APP_NET_FLOW_ENABLED 1

#define APP_MEXO_SLOT_COUNT 4
#define FMSTR_REC_TIMEBASE (32768+50)