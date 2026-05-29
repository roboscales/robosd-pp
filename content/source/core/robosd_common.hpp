#ifndef __robosd_common_hpp
#define __robosd_common_hpp

#include "robosd_app_tuning.hpp"
#include "robosd_target.hpp"
#include <limits>
#include <algorithm>
#include <type_traits>
#include <cmath>

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

#ifndef ROBO_APP_MATH_SHIFT_ENABLE
#define ROBO_APP_MATH_SHIFT_ENABLE 0
#endif

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
	void ROBO_EXPORT crash(char const* _function, char const* _file, int _line);
	enum class result { complete, resume, panic };

	template<typename T>
	constexpr T pi = T(3.1415926535897932385);

	template<typename T>
	constexpr T deg2rad = pi<T> / T(180.0) ;

	template<typename T>
	constexpr T rad2deg = T(180.0) / pi<T>;

	template<typename T>
	constexpr T g = T(9.8066);
		
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

	template<typename T > constexpr T abs(const T & x){
		return (x>0) ? x:-x;
	}
	#ifdef min 
	#undef min
	#endif
	template<typename T > constexpr T min(const T & x,const T & y){
		return (x>y) ? y:x;
	}
	#ifdef max 
	#undef max
	#endif

	template<typename T > constexpr T max(const T & x,const T & y){
		return (x>y) ? x:y;
	}

	template<typename T>	 constexpr  T fma(T a, T b, T c)
	{
			return a * b + c;
	}
	
	template <typename T, typename H> constexpr T saturate(T _x, H  _lo, H _hi) {
		if (_x < _lo) {
			_x = _lo;
		}
		else if (_x > _hi) {
			_x = _hi;
		}
		return _x;
	}
	
	template <typename T, typename R> constexpr T saturate(T _x, const R & _r) {
		if (_x < _r.lo) {
			_x = _r.lo;
		}
		else if (_x > _r.hi) {
			_x = _r.hi;
		}
		return _x;
	}
	
	//смещение вправо с округлением
	namespace digit{
		template <typename T> constexpr T rsh(T _x, int8_t _sh) {
			#if ROBO_APP_MATH_SHIFT_ENABLE
			return _x >> _sh;
			#else
			return _x > 0 ? (_x >> _sh) : (-((-_x) >> _sh));
			#endif
		}
		
		// перегрузки для float и double
    inline float rsh(float _x, int8_t _sh) {
        return _sh >= 0 ? _x * (1.0f / (1 << _sh)) : _x * (1 << -_sh);
    }
    inline double rsh(double _x, int8_t _sh) {
        return _sh >= 0 ? _x * (1.0 / (1 << _sh)) : _x * (1 << -_sh);
    }
		
		template <typename T> constexpr T lsh(T _x, int8_t _sh) {
			#if ROBO_APP_MATH_SHIFT_ENABLE
			return _x << _sh;
			#else
			if constexpr ( std::is_signed<T>::value ) {
				return _x > 0 ? (_x << _sh) : (-((-_x) << _sh));
			}
			else {
				return  (_x << _sh);
			}
			#endif
		}


		template <typename T> constexpr T signed_round(const T _src, uint8_t _shift) {
			auto max = std::numeric_limits<T>::max();
			auto min = -max;
			auto r = ((T)1 << (_shift - 1))-1;

			if (_src > 0) {
				if (_src >= max - r) {
					return (_src >> _shift) + 1;
				}
				else {
					return ((_src + r) >> _shift);
				}
			}
			else {
				if (_src <= min + r) {
				#if ROBO_APP_MATH_SHIFT_ENABLE
					return (_src >> _shift) - 1;
				#else
					return -((-_src) >> _shift) - 1;
				#endif
				}
				else {
				#if ROBO_APP_MATH_SHIFT_ENABLE
					return ((_src - r) >> _shift);
				#else
					return -((-(_src - r)) >> _shift);
				#endif
				}
			}
		}

		template <typename T> constexpr T unsigned_round(const T _src, uint8_t _shift) {
			auto max = std::numeric_limits<T>::max();
			auto r = ((T)1 << (_shift - 1)) - 1;
			if (_src >= max - r) {
				return (_src >> _shift) + 1;
			}
			else {
				return ((_src + r) >> _shift);
			}
		}

		template <typename T> constexpr T round(const T _src, uint8_t _shift) {
			if (_src == 0) {
				return (T)0;
			}
			else {
				if (std::is_signed<T>() == true) {
					return  signed_round(_src, _shift);
				} else {
					return  unsigned_round(_src, _shift);
				}
			}
		}
		template <> constexpr uint64_t round(const uint64_t _src, uint8_t _shift) {
			if (_src == 0) {
				return 0;
			}
			else {
				return  unsigned_round(_src, _shift);
			}
		}
		template <typename D, typename S > D constexpr pack(const S& _x, uint8_t _shift) {
			if (_x == S(0)) {
				return (D)0;
			}
			else {
				S tmp = _x;
				if (_shift > 0) {
					int  r = (1 << (_shift - 1)) - 1;
					if (tmp > S(0)) {
						if ((int)(std::numeric_limits<S>::max() - _x) < r) {
							tmp = std::numeric_limits<S>::max() >> _shift;
						}
						else {
							tmp = (tmp + r) >> _shift;
						}
					}
					else {
						if ((int)(tmp + std::numeric_limits<S>::max()) < r) {
							tmp = -(std::numeric_limits<S>::max() >> _shift);
						}
						else {
							tmp = -((r - tmp) >> _shift);
						}
					}
				}
				if (std::numeric_limits<D>::digits < std::numeric_limits<S>::digits) {
					if (tmp < -std::numeric_limits<D>::max()) {
						return -std::numeric_limits<D>::max();
					}
					else if (_x > std::numeric_limits<D>::max()) {
						return std::numeric_limits<D>::max();
					}
				}
				return D(tmp);
			}

		}
		template<typename T>
			T select(const T & mask, const T & source0, const T & source1) {
			return (source0 & ~mask) | (source1 & mask);
		}
	}
	template<typename T, typename R>
	T rand_t(random_t _reso, const T& _min, const T& _max) {
		return ((T)R::rand(_reso)) / _reso * (_max - _min) + _min;
	}

	constexpr   double catan2(double y, double x){
		if( y ==0. && x==0.) return 0.;
		const double atan_tbl[] = {
		-3.333333333333333333333333333303396520128e-1,
		 1.999999117496509842004185053319506031014e-1,
		-1.428514132711481940637283859690014415584e-1,
		 1.110012236849539584126568416131750076191e-1,
		-8.993611617787817334566922323958104463948e-2,
		 7.212338962134411520637759523226823838487e-2,
		-5.205055255952184339031830383744136009889e-2,
		 2.938542391751121307313459297120064977888e-2,
		-1.079891788348568421355096111489189625479e-2,
		 1.858552116405489677124095112269935093498e-3
		};
		/* argument reduction: 
		   arctan (-x) = -arctan(x); 
		   arctan (1/x) = 1/2 * pi - arctan (x), when x > 0
		*/

		double ax = abs(x);
		double ay = abs(y);
		double t0 = max(ax, ay);
		double t1 = min(ax, ay);
    
		double a = 1 / t0;
		a *= t1;

		double s = a * a;
		double p = atan_tbl[9];

		p = fma( fma( fma( fma( fma( fma( fma( fma( fma( fma(p, s,
			atan_tbl[8]), s,
			atan_tbl[7]), s, 
			atan_tbl[6]), s,
			atan_tbl[5]), s,
			atan_tbl[4]), s,
			atan_tbl[3]), s,
			atan_tbl[2]), s,
			atan_tbl[1]), s,
			atan_tbl[0]), s*a, a);

		double r = ay > ax ? (pi<double>/2 - p) : p;

		r = x < 0 ?  pi<double> - r : r;
		r = y < 0 ? -r : r;

		return r;
		
	}
	template<typename T, int N > struct atan2_table_t{
		enum { size = N };
		T table [N];
		constexpr atan2_table_t(void){
			//N=3
			//min = 0
			//max = pi/4
			//d = pi/4/2=pi/8
			//table[0]=0
			//table[1]=pi/8
			//table[2]=pi/8+pi/8=pi/4
			double max = pi<double> /4.;
			double d = max/ static_cast<double>(N-1);
			for (size_t i = 0; i < N; ++i) {						
				double y = d * static_cast<double>(i) ;
				// Вычисляем значение синуса и масштабируем амплитудой
				// Округляем до ближайшего целого
				table[i] = (T)catan2(y,max);
			}
		}
	};
	
	template<typename T> constexpr T csin(T x) {
		T result = 0;
		int sign = 1;
		T xx = x * x;
		T pw = x;
		T fti = 1.0;
		for(int i = 1; i < 25; i += 2) {
			fti /= i;
			result += sign * pw * fti;
			fti /= ( i + 1 );
			sign = -sign;
			pw  *= xx;
		}
		return result;
	}
	constexpr double clog(double x) {
			if (x <= 0.0) return -INFINITY;  // или выбросить ошибку
			double y = (x - 1.0) / (x + 1.0);
			double y2 = y * y;
			double term = y;
			double sum = term;
			for (int k = 1; k < 20; ++k) {
					term *= y2;
					sum += term / (2.0 * k + 1.0);
			}
			return 2.0 * sum;
	}
	template <typename M>
	M expm(const M & _A, typename M::type _eps){
		M R = {};
		using T = typename M::type;
		for (int i = 0; i < M::n; ++i) {
			R(i, i) = (T)1.;
		}
		auto ATN = _A;
		R += ATN;
		double nf = 1;
		for (int i = 2; i <= 28; ++i) {
			ATN = ATN * _A;
			nf = nf * i;
			auto D = ATN / nf;
			R += D;
			if (D.norma() < _eps) {
				return R;
			}
		}
		return R;
	}

	constexpr double taylor_sin(double x) {
		// Приводим к диапазону [-?, ?]

		double term = x;
		double sum = term;
		double x2 = x * x;

		for (int i = 1; i < 10; ++i) {
			term *= -x2 / ((2 * i) * (2 * i + 1));
			sum += term;
		}

		return sum;
	}
	template<typename C > struct generator_t {
		using T =typename C::type;
		constexpr static inline const T table_zero = C::zero;
		constexpr static inline const T amplitude = C::mag;
		enum{ size = C::count };
		T table[size] = {};
		constexpr generator_t() {
			constexpr double two_pi = 2.0 * pi<double>;
			for (size_t i = 0; i < size; ++i) {
				// Вычисляем угол от 0 до 2?
				double angle = double(C::offset) + two_pi * static_cast<double>(i) / static_cast<double>(size);

				// Вычисляем значение синуса и масштабируем амплитудой
				// Округляем до ближайшего целого
				double sin_value = (double)table_zero + (double)amplitude * taylor_sin(angle);
				table[i] = static_cast<T>(sin_value + 0.5);
			}
		}
	};

	//===================================================
	template<size_t N, typename M, typename S> class debouncer_t {
	private:
		robo::time_us_t begins_us_[N] = {};
		M stable_states_ = 0;  // Битовое поле стабильных состояний
	//    uint16_t threshold;
			
	public:
		debouncer_t( void){
		}
			
		// Обновление с массивом текущих значений
		template<typename T> void update( robo::time_us_t ( &threshold_us)[N], T & current_values ) {
			robo::time_us_t now = S::time_us();
			for (size_t i = 0; i < N; ++i) {
				bool current = current_values[i];

				if (current != ((stable_states_ >> i) & 1)) {
						// Состояние отличается от стабильного
					if (begins_us_[i] >= threshold_us[i]) {
							// Изменяем стабильное состояние
							if (current) {
									stable_states_ |= (1 << i);
							} else {
									stable_states_ &= ~(1 << i);
							}
					}
				} else {
					// Состояние совпадает со стабильным - сбрасываем счетчик
					begins_us_[i] = now;
				}
			}
		}
		template<typename T> void begin(  T & current_values ) {
			for (size_t i = 0; i < N; ++i) {
			bool current = current_values[i];
				robo::time_us_t now = S::time_us();			
				if (current) {
						stable_states_ |= (1 << i);
				} else {
						stable_states_ &= ~(1 << i);
				}
				begins_us_[i] = now;
			}
		}		
		M stable() const {
				return stable_states_;
		}
	};
	template<typename T> struct ROBO_EXPORT math_t {
		static T cos(const T& _t) { return (T)::cos((double)_t);  }
		static T sin(const T& _t) { return (T)::sin((double)_t); }
		static T sqrt(const T& _t) { return (T)::sqrt((double)_t); }
	};
	template<> struct ROBO_EXPORT math_t<float> {
		static float cos(const float & _t) { return ::cosf(_t); }
		static float sin(const float& _t) { return ::sinf(_t); }
		static float sqrt(const float& _t) { return ::sqrtf(_t); }
	};

}



#define PP_THIRD_ARG(a,b,c,...) c
#define VA_OPT_SUPPORTED_I(...) PP_THIRD_ARG(__VA_OPT__(,),true,false,)
#define VA_OPT_SUPPORTED VA_OPT_SUPPORTED_I(?)

#ifndef ROBO_APP_DEBUG_ENABLED
#define ROBO_APP_DEBUG_ENABLED 1
#endif

#if ROBO_APP_DEBUG_ENABLED

#ifndef ROBO_APP_CRASH
#define ROBO_APP_CRASH() robo::crash( \
	  ROBO_APP_PROC_NAME\
	, ROBO_APP_PROC_FILE\
	, ROBO_APP_PROC_LINE\
)
#endif

#else

#ifndef ROBO_APP_CRASH
#define ROBO_APP_CRASH() robo::crash( \
	  ""\
	, ""\
	,0\
)
#endif


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


#ifndef ROBO_APP_RTTI_ENABLED
#define ROBO_APP_RTTI_ENABLED 1
#endif

#ifndef ROBO_APP_REALTIME_TYPE
#define ROBO_APP_REALTIME_TYPE ROBO_APP_TYPE_SPECIFIC
#endif

#ifndef ROBO_APP_ULTRACOMPACT
#define ROBO_APP_ULTRACOMPACT 0
#endif



#endif

