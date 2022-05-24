#ifndef robo_log_hpp
#define robo_log_hpp
#include "core/robosd_common.hpp"
#ifndef __robosd_log_hpp
#define __robosd_log_hpp

#define ROBO_EMPTY_PARAM

#ifndef ROBO_APP_DEBUG_LOG_ENABLED
#define ROBO_APP_DEBUG_LOG_ENABLED 0
#endif

#if ROBO_APP_DEBUG_LOG_ENABLED == 1

#if VA_OPT_SUPPORTED_I(?)
#define ROBO_VA_COMMA(...) __VA_OPT__(,)
#else
#define ROBO_VA_COMMA(...) ,
#endif

#define ROBO_LOG_MASK_DISABLED 0

#ifndef robo_errlog
//#define robo_errlog(format,...) ::robo::log::print(::robo::log::verb::error, ROBO_LOG_MASK_DISABLED,RT(format),__VA_ARGS__) 
#if ROBO_UNICODE_ENABLED == 1
#define robo_errlog(f,...)  ::robo::log::print(robo::log::verb::error, robo::log::mask::disabled,  f RT( "\r\n\t%S\r\n\t%S - %d" ), __VA_ARGS__ ROBO_VA_COMMA(__VA_ARGS__) ROBO_APP_PROC_NAME, ROBO_APP_PROC_FILE, ROBO_APP_PROC_LINE)
#else
#define robo_errlog(f,...)  ::robo::log::print(robo::log::verb::error, robo::log::mask::disabled,  RT( f "\r\n\t%s\r\n\t%s - %d" ), __VA_ARGS__ ROBO_VA_COMMA(__VA_ARGS__) ROBO_APP_PROC_NAME, ROBO_APP_PROC_FILE, ROBO_APP_PROC_LINE)
#endif

#endif
#else
#define robo_errlog(format,...)
#endif

#if ROBO_APP_DEBUG_LOG_ENABLED == 1
#ifndef robo_warninglog
#define robo_warninglog(format,...) ::robo::log::print(::robo::log::verb::warning, robo::log::mask::disabled,RT(format) ROBO_VA_COMMA(__VA_ARGS__) __VA_ARGS__)
#endif
#endif
#else
#define robo_warninglog(format,...)
#endif

#if ROBO_APP_DEBUG_LOG_ENABLED == 1
#ifndef robo_infolog
#define robo_infolog(format,...) ::robo::log::print(::robo::log::verb::info, robo::log::mask::disabled,RT(format) ROBO_VA_COMMA(__VA_ARGS__) __VA_ARGS__)
#endif
#else
#define robo_infolog(format,...)
#endif

#if ROBO_APP_DEBUG_LOG_ENABLED == 1
#ifndef robo_detaillog
#define robo_detaillog(lvl,mask,format,...) robo_detaillog_(lvl,mask,format , __VA_ARGS__)
#define robo_detaillog_(lvl,mask,format,...) ::robo::log::print(::robo::log::verb::detail_##lvl, mask, RT(format) ROBO_VA_COMMA(__VA_ARGS__) __VA_ARGS__)
#endif
#else
#define robo_detaillog(lvl,mask,format,...)
#define robo_detaillog_(lvl,mask,format,...)
#endif


#if ROBO_APP_DEBUG_LOG_ENABLED == 1
#ifndef ROBO_NESTED_ERROR
#define ROBO_NESTED_ERROR "nested error"
#endif
#endif


#if ROBO_APP_DEBUG_LOG_ENABLED == 1
#ifndef ROBO_ALARM
#define ROBO_ALARM()  robo_errlog(ROBO_NESTED_ERROR);
#endif
#else
#define ROBO_ALARM()
#endif


#if ROBO_APP_DEBUG_LOG_ENABLED == 1
#ifndef ROBO_BREAK
#define ROBO_BREAK(fault) {robo_errlog(ROBO_NESTED_ERROR); return fault;}
#endif
#else
#define ROBO_BREAK(fault) return fault;
#endif

#ifndef ROBO_LBREAK
#define ROBO_LBREAK() ROBO_BREAK(false)
#endif

#ifndef ROBO_VBREAK
#define ROBO_VBREAK() ROBO_BREAK(ROBO_EMPTY_PARAM)
#endif


#if ROBO_APP_DEBUG_LOG_ENABLED == 1
#ifndef ROBO_ALARMN
#define ROBO_ALARMN(x) if(!(x)) robo_errlog(ROBO_NESTED_ERROR);
#endif
#else
#define ROBO_ALARMN(x)
#endif

#if ROBO_APP_DEBUG_LOG_ENABLED == 1
#ifndef ROBO_BREAKN
#define ROBO_BREAKN(x,fault) if(!(x)) {robo_errlog(ROBO_NESTED_ERROR); return fault;}
#endif
#else
#define ROBO_BREAKN(x,fault) if( !(x)) return fault;
#endif

#ifndef ROBO_LBREAKN
#define ROBO_LBREAKN(x) ROBO_BREAKN(x,false)
#endif

#ifndef ROBO_VBREAKN
#define ROBO_VBREAKN(x) ROBO_BREAKN(x,ROBO_EMPTY_PARAM)
#endif

#if ROBO_APP_DEBUG_LOG_ENABLED == 1
#ifndef ROBO_RET
#define ROBO_RET(x, success, fault) if(!(x)) {robo_errlog(ROBO_NESTED_ERROR);return fault;} else {return success;}
#endif
#else
#define ROBO_RET(x, success, fault) if( x ) { return success;} else { return fault; }
#endif

#ifndef ROBO_LRET
#define ROBO_LRET(x) ROBO_RET(x,true,false)
#endif

//==================
#if ROBO_APP_DEBUG_LOG_ENABLED == 1
#ifndef ROBO_ALARM_F
#define ROBO_ALARM_F(f,...){ \
	robo_errlog(f,__VA_ARGS__);\
}
#endif
#else
#define ROBO_ALARM_F(x,f,...)
#endif


#if ROBO_APP_DEBUG_LOG_ENABLED == 1
#ifndef ROBO_BREAK_F
#define ROBO_BREAK_F(fault,f,...)  \
{\
	robo_errlog(f,__VA_ARGS__); \
	return fault; \
}
#endif
#else
#define ROBO_BREAK_F(x,fault,f,...) if(!(x)) return fault;
#endif

#ifndef ROBO_LBREAK_F
#define ROBO_LBREAK_F(f,...) ROBO_BREAK_F(false,f,__VA_ARGS__)
#endif

#ifndef ROBO_VBREAK_F
#define ROBO_VBREAK_F(x,f,...) ROBO_BREAK_F(x,ROBO_EMPTY_PARAM,f,__VA_ARGS__)
#endif
//=================

#if ROBO_APP_DEBUG_LOG_ENABLED == 1
#ifndef ROBO_ALARMN_F
#define ROBO_ALARMN_F(x,f,...){ \
if(!(x)){\
	robo_errlog(f,__VA_ARGS__);\
}\
}
#endif
#else
#define ROBO_ALARMN_F(x,f,...)
#endif


#if ROBO_APP_DEBUG_LOG_ENABLED == 1
#ifndef ROBO_BREAKN_F
#define ROBO_BREAKN_F(x,fault,f,...)  \
if ( !( x ) ){\
	robo_errlog(f,__VA_ARGS__); \
	return fault; \
}
#endif
#else
#define ROBO_BREAKN_F(x,fault,f,...) if( !(x)) return fault;
#endif

#ifndef ROBO_LBREAKN_F
#define ROBO_LBREAKN_F(x,f,...) ROBO_BREAKN_F(x,false,f,__VA_ARGS__)
#endif

#ifndef ROBO_VBREAKN_F
#define ROBO_VBREAKN_F(x,f,...) ROBO_BREAKN_F(x,ROBO_EMPTY_PARAM,f,__VA_ARGS__)
#endif


#if ROBO_APP_DEBUG_LOG_ENABLED == 1
#ifndef ROBO_RET_F
#define ROBO_RET_F(x, success,fault ,f,...)  \
if (!(x)){\
robo_errlog(f,__VA_ARGS__); \
return fault; \
} else return success;
#endif
#else
#ifndef ROBO_LRET_F
#define ROBO_RET_F(x,success,fault,f,...) return (x)?(success):(fault)
#endif
#endif

#ifndef ROBO_LRET_F
#define ROBO_LRET_F(x,f,...) ROBO_RET_F(x,true,false,f,__VA_ARGS__)
#endif


#if ROBO_APP_DEBUG_LOG_ENABLED == 1
#ifndef ROBO_JAMPN
#define ROBO_JAMPN(x,lbl) if( !(x) ) { robo_errlog(ROBO_NESTED_ERROR); goto lbl;}; 
#endif
#else
#define ROBO_JAMPN(x,lbl) if( !(x) ) goto lbl;
#endif



#if ROBO_APP_DEBUG_LOG_ENABLED == 1
#ifndef ROBO_JAMPN_F
#define ROBO_JAMPN_F(x,lbl,f,...)  \
if ( !( x ) ){\
	robo_errlog(f,__VA_ARGS__); \
	goto lbl; \
}
#endif
#else
#define ROBO_JAMPN_F(x,lbl,f,...) if( !(x)) goto lbl;
#endif


#if ROBO_APP_DEBUG_LOG_ENABLED == 1
#ifndef ROBO_JAMP_F
#define ROBO_JAMP_F(lbl,f,...)  { robo_errlog(f,__VA_ARGS__); goto lbl; }
#endif
#else
#define ROBO_JAMP_F(x,lbl,f,...) goto lbl;
#endif

#if ROBO_APP_DEBUG_LOG_ENABLED == 1
#ifndef ROBO_ASSERT_F
#define ROBO_ASSERT_F(x,f,...)  \
if ( !( x ) ){\
	robo_errlog(f,__VA_ARGS__); \
	ROBO_APP_CRASH();\
}
#endif
#else
#define ROBO_ASSERT_F(x,f,...) if( !(x)) ROBO_APP_CRASH();
#endif

#if ROBO_APP_DEBUG_LOG_ENABLED == 1
namespace robo{
	namespace log{
		enum class verb { skip = -3, error = -2, warning = -1, info = 0, detail_1 = 1, detail_2 = 2, detail_3 = 3, detail_4 = 4, detail_5 = 5, detail_6 = 6, detail_7 = 7 };
		struct mask {
			enum {
				disabled = 0 // выводить все
				, app = 1 // выводить сообщения от модулей приложения - загрузился, стартовал, остановился
				, net = 2 // выводить сообщения от сетевых модулей - получил/отправил
			};
		};

		bool ROBO_EXPORT begin(verb _verb, unsigned int _mask);
		void ROBO_EXPORT finish(void);

		void ROBO_EXPORT print( verb _verb, unsigned int _mask, cstr _format, ...);
	}

}
#endif

#define robo_applog(format,...) robo_detaillog(4,::robo::log::mask::app,format,__VA_ARGS__)
#define robo_netlog(format,...) robo_detaillog(4,::robo::log::mask::net,format,__VA_ARGS__)


#endif
