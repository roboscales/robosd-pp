#if (!defined(__robosd_app_tunning_hpp)) && defined(__robosd_common_hpp)
#define __robosd_app_tunning_hpp
#else
#error error of using robosd_app_tunning.hpp
#endif

#define ROBO_APP_DYNAMIC_LIB 
#define ROBO_APP_SYSTEM_ENABLED 1

#define ROBO_APP_SYSTEM_MULTYTHRAD_ENABLED 1
#define ROBO_APP_SYSTEM_TIME_ENABLED 1
#define ROBO_APP_SYSTEM_RANDOM_ENABLED 1
#define ROBO_APP_DEBUG_LOG_ENABLED 1

#define ROBOSD_UNICODE_ENABLED 1

#include <stdint.h>