#ifndef __robosd_common_hpp
#define __robosd_common_hpp

#include "robosd_app_tuning.hpp"
#include "robosd_target.hpp"

#ifndef ROBO_TYPE_RANDOM 
#define ROBO_TYPE_RANDOM int
#endif 

#ifndef ROBO_TYPE_TIME_US 
#define ROBO_TYPE_TIME_US uint32_t
#endif 

#ifndef ROBO_TYPE_TIME_MS 
#define ROBO_TYPE_TIME_MS uint32_t
#endif

#ifndef ROBOSD_UNICODE_ENABLED
#define ROBOSD_UNICODE_ENABLED 0
#endif

#include <string>
#if ROBOSD_UNICODE_ENABLED == 1
#define ROBO_CHAR wchar_t
#define ROBO_CONST_STRING wchar_t const *
#define RT(s) L##s
#else
#define ROBO_CHAR char
#define ROBO_CONST_STRING char const *
#define RT(s) s
#endif


namespace robo {
	typedef ROBO_TYPE_RANDOM random_t;
	typedef ROBO_TYPE_TIME_US time_us_t;
	typedef ROBO_TYPE_TIME_MS time_ms_t;

	typedef  ROBO_CHAR char_t;
	typedef  std::wstring str;
	typedef  ROBO_CONST_STRING cstr;

	void ROBO_EXPORT crash(char const * _file, char const * _function, int _line);
}


#ifndef ROBO_APP_CRASH
#define ROBO_APP_CRASH() robo::crash( \
	  ROBO_APP_PROC_NAME\
	, ROBO_APP_PROC_FILE\
	, ROBO_APP_PROC_LINE\
)
#endif

#ifndef ROBO_APP_DEBUG_ENABLED
#define ROBO_APP_DEBUG_ENABLED 1
#endif

#if ROBO_APP_DEBUG_ENABLED == 1
#ifndef ROBO_APP_ASSERT 
#define ROBO_APP_ASSERT(x) if( ((bool)(x)) == false ) ROBO_APP_CRASH();
#endif
#else
#define ROBO_APP_ASSERT(x) 
#endif


#endif