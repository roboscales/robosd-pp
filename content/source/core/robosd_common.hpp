#ifndef __robosd_common_hpp
#define __robosd_common_hpp

#include "robosd_app_tuning.hpp"
#include "robosd_target.hpp"
#include <limits>
#include <algorithm>

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
//ROBO_APP_CRASH_TYPE лучше определять всегда отдельно
#ifndef ROBO_APP_CRASH_TYPE
#define ROBO_APP_CRASH_TYPE ROBO_APP_TYPE_SPECIFIC
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

void* operator new(size_t size);
void operator delete(void* ptr);

#include <stdint.h>

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
	constexpr T deg2rad = pi<T> / T(180.0) ;

	template<typename T>
	constexpr T rad2deg = T(180.0) / pi<T>;

	double constexpr csqrt_helper(double x, double curr, double prev) {
		return curr == prev
			? curr
			: csqrt_helper(x, (curr + x / curr) / 2., curr);
	}

	template<typename T> bool constexpr parity(T  _n)
	{
			bool res = 0;
			while (_n)
			{
					res = !res;
					_n     = _n & (_n - 1);
			}    
			return res;
	}

	template<typename T>  T  constexpr csqrt(T x) {
		double dx = (double)x;
		double res = (x >= 0 && x < std::numeric_limits<double>::infinity())
			? csqrt_helper(x, x, 0.)
			: std::numeric_limits<double>::quiet_NaN();
		return T(res);
	}

	class ostram {
		uint8_t* memo_;
		size_t size_;
		size_t actual_size_ = 0;
		uint8_t* ptr_;
	public:
		constexpr void reset(void) {
			ptr_ = memo_;
			actual_size_ = 0;
		}
		constexpr size_t actual_size() { return actual_size_;  }
		constexpr ostram(uint8_t* _memo, size_t _sz) : memo_(_memo), size_(_sz), ptr_(_memo) {}
		template <typename T> constexpr bool put(const T& _data) {
			if (sizeof(T) + actual_size_ <= size_) {
				std::copy_n((const uint8_t*)&_data, sizeof(T), ptr_);
				actual_size_ += sizeof(T);
				ptr_ += sizeof(T);
				return true;
			}
			else return false;
		}
		template <typename T> constexpr ostram & operator << (const T& _data) {
			put(_data);
			return *this;
		}
	};

	class istram {
		const uint8_t* memo_;
		size_t size_;
		size_t actual_size_ = 0;
		const uint8_t* ptr_;
	public:
		constexpr void reset(void) {
			ptr_ = memo_;
			actual_size_ = 0;
		}
		constexpr size_t actual_size() { return actual_size_; }
		constexpr istram(const uint8_t* _memo, size_t _sz) : memo_(_memo), size_(_sz), ptr_(_memo) {}
		template <typename T> constexpr bool get( T& _data) {
			if (sizeof(T) + actual_size_ <= size_) {
				std::copy_n(ptr_, sizeof(T), (uint8_t*)&_data);
				actual_size_ += sizeof(T);
				ptr_ += sizeof(T);
				return true;
			}
			else return false;
		}
		template <typename T> constexpr istram& operator >> ( T& _data) {
			get(_data);
			return *this;
		}
	};


	/*
	template<typename  T, int N> void copy(std::initializer_list<T>  _src, T(&_dst)[N]) {
		size_t n = _src.end() - _src.begin();
		if(n==0) {
			std::fill_n(_dst, N, (T)0);
		}
		if (n > N) n = N;
		std::copy_n(_src.begin(), n, _dst);
		std::fill_n(_dst + n, N - n, (T)0);
	}
	template<typename  T, int N> void copyf(const T * _src,size_t n, T(&_dst)[N]) {
		if (n == 0) {
			std::fill_n(_dst, N, (T)0);
		}
		if (n > N) n = N;
		std::copy_n(_src, n, _dst);
		std::fill_n(_dst + n, N - n, (T)0);
	}*/

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

#if ROBO_APP_SYSTEM_ENABLED == 0
#ifdef ROBO_APP_ENV_TYPE
#undef ROBO_APP_ENV_TYPE
#endif
#define ROBO_APP_ENV_TYPE ROBO_APP_ENV_NONE
#endif

#define ROBO_APP_TYPE_NONE 0
#define ROBO_APP_TYPE_NATIVE 1
#define ROBO_APP_TYPE_STD 2
#define ROBO_APP_TYPE_WIN 3
#define ROBO_APP_TYPE_LINUX 4
#define ROBO_APP_TYPE_RASPBERRY 5
#define ROBO_APP_TYPE_UBUNTU 6
#define ROBO_APP_TYPE_ASTRA 7
#define ROBO_APP_TYPE_SPECIFIC 8
#define ROBO_APP_TYPE_DUMMY 9

#endif

