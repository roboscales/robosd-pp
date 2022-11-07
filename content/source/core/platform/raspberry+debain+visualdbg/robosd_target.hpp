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

#ifndef ROBO_EXPORT_RUNTIME_DECL
#define ROBO_EXPORT_RUNTIME_DECL __cdecl
#endif

#ifndef ROBO_EXPORT_RUNTIME
#define ROBO_EXPORT_RUNTIME _declspec(dllexport)
#endif


#define ROBO_APP_PROC_NAME __func__
#define ROBO_APP_PROC_FILE __FILE__
#define ROBO_APP_PROC_LINE __LINE__
#define ROBO_UNUSED(x) ((void)(x))

#ifndef ROBO_ZEROS_STRUCT
#define ROBO_ZEROS_STRUCT
#endif


//некоторые сообщениия могут не печататься 
#define ROBO_STRING_BUFFER_SIZE 4096

#if ROBO_APP_DEBUG_LOG_ENABLED == 1

/* expands to the first argument */
#define ROBO_FIRST(...) FIRST_HELPER(__VA_ARGS__, throwaway)
#define FIRST_HELPER(first, ...) first
/*
 * if there's only one argument, expands to nothing.  if there is more
 * than one argument, expands to a comma followed by everything but
 * the first argument.  only supports up to 9 arguments but can be
 * trivially expanded.
 */
#define ROBO_REST(...) REST_HELPER(NUM(__VA_ARGS__), __VA_ARGS__)
#define REST_HELPER(qty, ...) REST_HELPER2(qty, __VA_ARGS__)
#define REST_HELPER2(qty, ...) REST_HELPER_##qty(__VA_ARGS__)
#define REST_HELPER_ONE(first)
#define REST_HELPER_TWOORMORE(first, ...) , __VA_ARGS__
#define NUM(...) \
    SELECT_10TH(__VA_ARGS__, TWOORMORE, TWOORMORE, TWOORMORE, TWOORMORE,\
    TWOORMORE, TWOORMORE, TWOORMORE, TWOORMORE, ONE, throwaway)
#define SELECT_10TH(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, ...) a10

#define robo_errlog(...)  ::robo::log::print(robo::log::verb::error, robo::log::mask::disabled,  RT( ROBO_FIRST(__VA_ARGS__) "\r\n\t%s\r\n\t%s - %d" ) ROBO_REST(__VA_ARGS__),  __func__, __FILE__, __LINE__)	
#define robo_infolog(...) ::robo::log::print(::robo::log::verb::info, robo::log::mask::disabled,RT(ROBO_FIRST(__VA_ARGS__)) ROBO_REST(__VA_ARGS__) )
#define ROBO_BREAKN_F(x,fault,...)  \
if ( !( x ) ){\
	robo_errlog(__VA_ARGS__); \
	return fault; \
}
#define ROBO_LBREAKN_F(x,...) ROBO_BREAKN_F(x,false,__VA_ARGS__)
#define ROBO_VBREAKN_F(x,...) ROBO_BREAKN_F(x,ROBO_EMPTY_PARAM,__VA_ARGS__)
#define ROBO_JAMPN_F(x,lbl,...)  \
if ( !( x ) ){\
	robo_errlog(__VA_ARGS__); \
	goto lbl; \
}

#define ROBO_BREAK_F(fault,...)  \
{\
	robo_errlog(__VA_ARGS__); \
	return fault; \
}
#define ROBO_LBREAK_F(...) ROBO_BREAK_F(false,__VA_ARGS__)

#endif



