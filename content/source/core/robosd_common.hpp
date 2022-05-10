#ifndef __robosd_common_hpp
#define __robosd_common_hpp

#include "robosd_app_tuning.hpp"
#include "robosd_target.hpp"
#include <limits>

#ifndef ROBO_TYPE_RANDOM
#define ROBO_TYPE_RANDOM int
#endif 

#ifndef ROBO_TYPE_TIME_US 
#define ROBO_TYPE_TIME_US uint32_t
#endif 

#ifndef ROBO_TYPE_TIME_MS 
#define ROBO_TYPE_TIME_MS uint32_t
#endif

#ifndef ROBO_UNICODE_ENABLED
#define ROBO_UNICODE_ENABLED 0
#endif


#ifndef ROBO_APP_MODULE_ENABLED
#define ROBO_APP_MODULE_ENABLED 0
#endif

#if ROBO_UNICODE_ENABLED == 1
#define ROBO_CHAR wchar_t
#define ROBO_CONST_STRING wchar_t const *

#ifndef RT
#define RT(s) RT_(s)
#define RT_(s) L##s
#endif

#ifndef RT8
#define RT8(s) s
#endif

#else

#define ROBO_CHAR char
#define ROBO_CONST_STRING char const *

#ifndef RT
#define RT(s) s
#endif

#ifndef RT8
#define RT8(s) s
#endif

#endif

#ifdef ROBO_STD_ARGS
#include ROBO_STD_ARGS
#else
#include <stdarg.h>
#endif

#include <stdint.h>


void* operator new(size_t size);
void operator delete(void* ptr);

namespace robo {
	typedef ROBO_TYPE_RANDOM random_t;
	typedef ROBO_TYPE_TIME_US time_us_t;
	typedef ROBO_TYPE_TIME_MS time_ms_t;

	typedef  ROBO_CHAR char_t;
	typedef  ROBO_CONST_STRING cstr;

	/*bool ROBO_EXPORT begin(cstr _ini);
	bool ROBO_EXPORT start(void);
	void ROBO_EXPORT stop(void);
	bool ROBO_EXPORT active(void);
	void ROBO_EXPORT finish(void);*/

	int32_t ROBO_EXPORT hash(cstr _src, int32_t _begin = 0);
	int32_t ROBO_EXPORT hash(cstr _beg, cstr _end, int32_t _begin = 0);
	//unsigned short fast_hash(cstr _src, unsigned short _begin);
	//unsigned short ROBO_EXPORT fast_hash(cstr _src, unsigned short _begin = 0);
	void ROBO_EXPORT crash(char const* _file, char const* _function, int _line);
	enum class result { complete, resume, panic };

	template<typename T>
	constexpr T pi = T(3.1415926535897932385);

	template<typename T>
	constexpr T grad2rad = pi<T> / T(180.0) ;

	template<typename T>
	constexpr T rad2grad = T(180.0) / pi<T>;

	double constexpr csqrt_helper(double x, double curr, double prev) {
		return curr == prev
			? curr
			: csqrt_helper(x, (curr + x / curr) / 2., curr);
	}



	template<typename T>  T  constexpr csqrt(T x) {
		double dx = (double)x;
		double res = (x >= 0 && x < std::numeric_limits<double>::infinity())
			? csqrt_helper(x, x, 0.)
			: std::numeric_limits<double>::quiet_NaN();
		return T(res);
	}



	/*template<typename T>
	constexpr T one_div_sqrt3 = T(1) / csqrt(T(3));

	template<typename T>
	constexpr T one_div_sqrt2 = T(1) / csqrt(T(2));

	template<typename T>
	constexpr T sqrt3_div_2 = csqrt(T(3)) / T(2);

	template<typename T>
	constexpr T sqrt2_div_2 = csqrt(T(2)) / T(2);
	*/


	template<typename T, unsigned C> struct array {
		T values[C] = {};
	};
	//todo put get
	template<typename T, typename S, S C> struct stack_t: public array<T, C> {
		S len = 0;
	};

}

#define PP_THIRD_ARG(a,b,c,...) c
#define VA_OPT_SUPPORTED_I(...) PP_THIRD_ARG(__VA_OPT__(,),true,false,)
#define VA_OPT_SUPPORTED VA_OPT_SUPPORTED_I(?)


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


#ifndef ROBO_EXPORT_FUNCTION_PREFIX
#define ROBO_EXPORT_FUNCTION_PREFIX RT("")
#endif

#ifndef ROBO_APP_SYSTEM_ENABLED
#define ROBO_APP_SYSTEM_ENABLED 0
#endif

#ifndef ROBO_APP_NET_FLOW_ENABLED
#define ROBO_APP_NET_FLOW_ENABLED 0
#endif

#ifndef ROBO_MEMORY_HEAP_ENABLED
#define ROBO_MEMORY_HEAP_ENABLED 0
#endif 

#define ROBO_APP_TYPE_NONE 0
#define ROBO_APP_TYPE_DUMMY 1
#define ROBO_APP_TYPE_NATIVE 2
#define ROBO_APP_TYPE_STD 3
#define ROBO_APP_TYPE_WIN 4
#define ROBO_APP_TYPE_LINUX 5
#define ROBO_APP_TYPE_RASPBERRY 6
#define ROBO_APP_TYPE_UBUNTU 7
#define ROBO_APP_TYPE_ASTRA 8
#define ROBO_APP_TYPE_SPECIFIC 9

#endif