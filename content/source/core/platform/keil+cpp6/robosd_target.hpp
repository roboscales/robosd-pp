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
#define ROBO_WEAK __attribute__((weak))

#ifndef ROBO_APP_DMA_BUFFER
#define ROBO_APP_DMA_BUFFER
#endif

#ifndef ROBO_APP_ALIGN_32BYTES
#define ROBO_APP_ALIGN_32BYTES(x)
#endif


