#include "core/robosd_common.hpp"
#ifndef __robosd_log_hpp
#define __robosd_log_hpp

#ifndef ROBO_APP_DEBUG_LOG_ENABLED
#define ROBO_APP_DEBUG_LOG_ENABLED 0
#endif

#if ROBO_APP_DEBUG_LOG_ENABLED == 1

#ifdef ROBO_STD_ARGS
#include ROBO_STD_ARGS
#else
#include <stdarg.h>
#endif

#define ROBO_LOG_MASK_DISABLED 0

#ifndef robo_errlog
#define robo_errlog(format,...) ::robo::log(::robo::verb::error, ROBO_LOG_MASK_DISABLED,RT(format),__VA_ARGS__) 
#endif

#ifndef robo_warninglog
#define robo_warninglog(format,...) ::robo::log(::robo::verb::warning, ROBO_LOG_MASK_DISABLED,RT(format),__VA_ARGS__) 
#endif

#ifndef robo_infolog
#define robo_infolog(format,...) ::robo::log(::robo::verb::info, ROBO_LOG_MASK_DISABLED,RT(format) , __VA_ARGS__)
#endif

#ifndef robo_detaillog
#define robo_detaillog(lvl,mask,format,...) robo_detaillog_(lvl,mask,RT(format),__VA_ARGS__)
#define robo_detaillog_(lvl,mask,format,...) ::robo::log(::robo::verb::detail##lvl, mask, RT(format) ,__VA_ARGS__)
#endif

#define robo_log_init(verb,mask, logger) ::robo::log::begin(verb, mask, logger)
#define robo_log_deinit() ::robo::log::finish()


#ifndef ROBO_NESTED_ERROR
#define ROBO_NESTED_ERROR "nested error"
#endif

#define ROBO_ALARM()  robo_errlog(ROBO_NESTED_ERROR);
#define ROBO_BREAK() {robo_errlog(ROBO_NESTED_ERROR); return false;}
#define ROBO_VBREAK() {robo_errlog(ROBO_NESTED_ERROR); return;}


#define ROBO_ALARMN(x) if(!(x)) robo_errlog(ROBO_NESTED_ERROR);
#define ROBO_BREAKN(x) if(!(x)) {robo_errlog(ROBO_NESTED_ERROR); return false;}
#define ROBO_VBREAKN(x) if(!(x)) {robo_errlog(ROBO_NESTED_ERROR); return;}
#define ROBO_RET(x) if(!(x)) {robo_errlog(ROBO_NESTED_ERROR);return false;} else {return true;}
#define ROBO_VRET(x) {if(!(x)) {robo_errlog(ROBO_NESTED_ERROR);} return;}


namespace robo{
	namespace log{
		enum class verb { skip = -3, error = -2, warning = -1, info = 0, detail_1 = 1, detail_2 = 2, detailL_3 = 3, detail_4 = 4, detail_5 = 5, detail_6 = 6, detail_7 = 7 } robo_log_verb_t;
		typedef  void ( * print_f)(verb _verb, cstr, va_list  _args);

		bool ROBO_EXPORT begin(verb _verb, unsigned int _mask, print_f _print);
		void ROBO_EXPORT fiish(void);

		void ROBO_EXPORT log( verb _verb, unsigned int _mask, cstr _format, ...);
	}
}
#endif
#else
#define robo_disp(format,...) 

#ifndef robo_errlog
#define robo_errlog(format,...) 
#endif

#define robo_warninglog(format,...) 

#ifndef robo_infolog
#define robo_infolog(format,...) 
#endif

#ifndef robo_detaillog
#define robo_detaillog(lvl,mask,format,...) 
#define robo_detaillog_(lvl,mask,format,...)
#endif


#define robo_log_init(verb,mask, logger)
#define robo_log_deinit() 
#endif



#ifndef ROBO_ALARM_F
#define ROBO_ALARM_F(f,...){ \
	robo_errlog(f,__VA_ARGS__);\
}
#endif

#ifndef ROBO_BREAK_F
#define ROBO_BREAK_F(f,...) \
	robo_errlog( f,__VA_ARGS__); \
    return false;
#endif

#ifndef ROBO_VBREAK_F
#define ROBO_VBREAK_F(f,...) \
	robo_errlog( f,__VA_ARGS__); \
    return ;
#endif


#ifndef ROBO_ALARMN_F
#define ROBO_ALARMN_F(x,f,...){ \
if(!(x)){\
	robo_errlog(f,__VA_ARGS__);\
}\
}
#endif


#ifndef ROBO_BREAKN_F
#define ROBO_BREAKN_F(x,f,...)  \
if ( !( x ) ){\
	robo_errlog(f,__VA_ARGS__); \
	return false; \
} 
#endif

#ifndef ROBO_VBREAKN_F
#define ROBO_VBREAKN_F(x,f,...)  \
if ( !( x ) ){\
	robo_errlog(f,__VA_ARGS__); \
	return; \
} 
#endif

#ifndef ROBO_RET_F
#define ROBO_RET_F(x,f,...)  \
if (!(x)){\
	robo_errlog(f,__VA_ARGS__); \
	return false; \
} else return true;
#endif

#ifndef ROBO_VRET_F
#define ROBO_VRET_F(x,f,...)  \
{\
	if (!(x)){\
		robo_errlog(f,__VA_ARGS__); \
	}\
	return; \
}
#endif


