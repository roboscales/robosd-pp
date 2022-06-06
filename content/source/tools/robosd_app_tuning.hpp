#if (!defined(__robosd_app_tunning_hpp)) && defined(__robosd_common_hpp)
#define __robosd_app_tunning_hpp
#else
#error error of using robosd_app_tunning.hpp
#endif



#define ROBO_APP_DYNAMIC_LIB

#define ROBO_UNICODE_ENABLED 1

#define ROBO_APP_SYSTEM_MULTYTHRAD_ENABLED 1

#define ROBO_APP_DEBUG_LOG_ENABLED 1

#define ROBO_APP_SYSTEM_ENABLED 1

#define ROBO_APP_MODULE_ENABLED 1


#define ROBO_APP_PRINT_TYPE ROBO_APP_TYPE_STD
#define ROBO_APP_FORMATING_TYPE ROBO_APP_TYPE_STD
#define ROBO_LOG_APP_PRINT_TYPE ROBO_APP_TYPE_STD

#define ROBO_APP_ENV_TYPE ROBO_APP_TYPE_WIN
#define ROBO_APP_INI_TYPE ROBO_APP_TYPE_WIN
#define ROBO_APP_LIB_TYPE ROBO_APP_TYPE_NATIVE
#define ROBO_APP_ALLOC_TYPE ROBO_APP_TYPE_STD
#define ROBO_APP_SHARED_TYPE ROBO_APP_TYPE_WIN
#define ROBO_APP_CONSOL_TYPE ROBO_APP_TYPE_WIN

#define  ROBO_APP_MEXO_VAR_ENABLED 1

#include <stdint.h>

