#if (!defined(__robosd_target_hpp)) && defined(__robosd_common_hpp)
#define __robosd_target_hpp
#else
#error error of using robosd_target.hpp
#endif

#ifdef ROBO_APP_DYNAMIC_LIB
#define ROBO_EXPORT  _declspec(dllexport)
#else
	#ifdef ROBO_APP_DYNAMIC_APP
	#define ROBO_EXPORT  _declspec(dllexport)
	#else
	#define ROBO_EXPORT 
	#endif
#endif

#define ROBO_APP_PROC_NAME __FUNCTION__
#define ROBO_APP_PROC_FILE __FILE__
#define ROBO_APP_PROC_LINE __LINE__
#define ROBO_UNUSED(x) ((void)(x))

#if ROBO_UNICODE_ENABLED == 1
#define robo_errlog(f,...)  ::robo::log::print(robo::log::verb::error, robo::log::mask::disabled,  RT( f "\r\n\t%S\r\n\t%S - %d" ), __VA_ARGS__ , ROBO_APP_PROC_NAME, ROBO_APP_PROC_FILE, ROBO_APP_PROC_LINE)
#else
#define robo_errlog(f,...)  ::robo::log::print(robo::log::verb::error, robo::log::mask::disabled,  RT( f "\r\n\t%s\r\n\t%s - %d" ), __VA_ARGS__ , ROBO_APP_PROC_NAME, ROBO_APP_PROC_FILE, ROBO_APP_PROC_LINE)
#endif
