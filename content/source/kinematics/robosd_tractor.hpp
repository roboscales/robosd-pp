#ifndef rovosd_tractor_hpp
#define rovosd_tractor_hpp

#include <cmath>
#include <algorithm>
#include "core/robosd_common.hpp"
#include "core/robosd_list.hpp"
#include "core/robosd_span.hpp"
#include "core/robosd_tree.hpp"
#include "core/robosd_ini.hpp"

/*#ifndef ROBO_TRACTOR_ASSERT
#define ROBO_TRACTOR_ASSERT(x,f) 
#endif*/

namespace robo {
	namespace tractor {
		
		template<typename T> constexpr void rangle_sut(T & _values) {
			while (_values > pi<T>) {
				_values -= (2 * pi<T>);
			}
			while (_values < -pi<T>) {
				_values += (2 * pi<T>);
			}			
		}

		template<typename T> constexpr void rangle_add(T& _values, T _add) {
			_values += _add;
			rangle_sut(_values);
		}
		template<typename T> constexpr void rangle_sub(T& _values, T _add) {
			_values -= _add;
			rangle_sut(_values);
		}

		template<typename T,int n> constexpr T epsilon = ::std::numeric_limits<T>::epsilon() * n; //n попугаев

		template<typename T> constexpr T deg2rad = pi<T> / T(180.);
		
		template<typename T> constexpr T rad2deg = T(1.) / deg2rad<T>;
		
		template <typename T> T constexpr  sign(T val) {
			if (::abs(val) < ::std::numeric_limits<T>::epsilon()) return T(0);
			return (T(0) < val) ? -T(1) : T(1);
		}

		template <typename T> T constexpr abs(T val) {
			if (T(0) == val) return 0;
			if (val < T(0)) return -val;
			return val;
		}


		template< typename  T, typename  S> class numbers_t: public S {
		public:
		  typedef T type;

		  constexpr numbers_t(void) : S() {
				::std::fill_n(S::memo, S::size, T(0));
			}

			constexpr numbers_t(const numbers_t & _src) : S() {
				ROBO_APP_ASSERT((int)S::size == (int)_src.size);
				::std::copy_n(_src.memo, S::size, S::memo);
			}

			constexpr numbers_t(const std::initializer_list<T> _src) : S() {
				ROBO_APP_ASSERT(S::size == _src.size());
				::std::copy_n(_src.begin(), S::size, S::memo);
			}
			constexpr numbers_t(const T (&_src)[S::size] ) : S() {
				::std::copy_n(_src, S::size, S::memo);
			}
			template <typename A> constexpr numbers_t(const A& _src) : S() {
				ROBO_APP_ASSERT((int)S::size == (int)_src.size);
				::std::copy_n(_src.memo, S::size, S::memo);
			} 

			template <typename A, typename B> constexpr numbers_t(const A& a,const B& b) : S() {
				*this = a*b;
			}

			constexpr numbers_t& operator = (const numbers_t & _src) {
				ROBO_APP_ASSERT(S::size == _src.size);
				::std::copy_n(_src.memo, S::size, S::memo);
				return *this;
			}

			template <typename A>  constexpr bool operator < (const numbers_t& _ref) const {
				ROBO_APP_ASSERT(S::size == _ref.size);
				const T* ref = _ref.memo;
				T* val = S::memo;
				for (int i = 0; i < S::size; ++i, ++ref, ++val) {
					if (*val >= *ref) {
						return false;
					}
				}
				return true;
			}

			template <typename A>  constexpr bool operator <= (const A& _ref) const {
				ROBO_APP_ASSERT(S::size == _ref.size);
				const T* ref = _ref.memo;
				const T* val = S::memo;
				for (int i = 0; i < S::size; ++i, ++ref, ++val) {
					if (*val > *ref) {
						return false;
					}
				}
				return true;
			}

			template <typename A>  constexpr bool operator > (const A& _ref) const {
				ROBO_APP_ASSERT(S::size == _ref.size);
				const T* ref = _ref.memo;
				const T* val = S::memo;
				for (int i = 0; i < S::size; ++i, ++ref, ++val) {
					if (*val <= *ref) {
						return false;
					}
				}
				return true;
			}

			template <typename A>  constexpr bool operator >= (const A& _ref) const {
				ROBO_APP_ASSERT(S::size == _ref.size);
				const T* ref = _ref.memo;
				const T* val = S::memo;
				for (int i = 0; i < S::size; ++i, ++ref, ++val) {
					if (*val < *ref) {
						return false;
					}
				}
				return true;
			}

			template <typename A> constexpr numbers_t& operator = (const A& _src) {
				ROBO_APP_ASSERT(S::size == _src.size);
				::std::copy_n(_src.memo, S::size, S::memo);
				return *this;
			}

			template <typename A> constexpr numbers_t& operator += (const A& _src) {
				ROBO_APP_ASSERT(S::size == _src.size);
				const T* src = _src.memo;
				T* dst = S::memo;
				for (size_t i = 0; i < S::size; ++i, ++src, ++dst) {
					*dst += *src;
				}
				return *this;
			}

			template <typename A> constexpr numbers_t& operator -= (const A& _src) {
				ROBO_APP_ASSERT(S::size == _src.size);
				const T* src = _src.memo;
				T* dst = S::memo;
				for (size_t i = 0; i < S::size; ++i, ++src, ++dst) {
					*dst -= *src;
				}
				return *this;
			}

			const T  & operator [] (int _index) const {
				return S::memo[_index];
			}
			T &  operator [] (int _index)  {
				return S::memo[_index];
			}

			constexpr numbers_t& operator *= (const T & _t ) {
				T* dst = S::memo;
				for (size_t i = 0; i < S::size; ++i, ++dst) {
					*dst *= _t;
				}
				return *this;
			}

			constexpr numbers_t& operator /= (const T& _t) {
				T* dst = S::memo;
				for (size_t i = 0; i < S::size; ++i, ++dst) {
					*dst /= _t;
				}
				return *this;
			}

			constexpr numbers_t abs (void) {
				numbers_t res;
				T* dst = res.memo;
				T* src = S::memo;
				for (size_t i = 0; i < S::size; ++i) {
					*dst++ = ::abs(*src++);
				}
				return res;
			}

			template <typename A> constexpr T dot(const A& _src) const {
				ROBO_APP_ASSERT(S::size == _src.size);
				T res = T(0);
				const T* src = _src.memo;
				const T* dst = S::memo;
				for (size_t i = 0; i < S::size; ++i, ++src, ++dst) {
					res += *dst * *src;
				}
				return res;
			}
			constexpr T length(void) const {
				return sqrt(dot(*this));
			}
			constexpr T norma(void) const {
				return sqrt(dot(*this)/ T(S::size) );
			}

			constexpr void zeros(void) {
				::std::fill_n(S::memo, S::size, T(0));
			}

			constexpr numbers_t dir(void) const {
				T r = length();
				if (r > ::std::numeric_limits<T>::epsilon()) {
					return *this / r;
				}
				else {
					return numbers_t();
				}
			}

			constexpr void normalize()  {
				T r = length();
				if (r > ::std::numeric_limits<T>::epsilon()) {
					*this /= r;
				}
				else {
					zeros();
				}
			}



			template<typename T1, typename C> numbers_t<T, C> operator *= (const numbers_t<T1, C>& _src1) {
				numbers_t<T1, C> tmp = *this * _src1;
				*this = tmp;
				return *this;
			}
			template<typename T1, typename C> numbers_t<T, C> operator /= (const numbers_t<T1, C>& _src1) {
				numbers_t<T, C> tmp = *this / _src1;
				*this = tmp;
				return *this;
			}

			bool load_raw(cstr _path, cstr key) {
				ROBO_LRET(ini::load_arr(_path, key, S::memo, S::size));
			}

			//operator const span<typename S::element_t, N> & () const { return span<typename  S::element_t, N>(S::memo, N); }
		};
		
		template<typename T, typename C> constexpr numbers_t<T,C> operator* (const numbers_t<T,C>& _src1, const T& _t) {
			numbers_t tmp(_src1);
			return tmp *= _t;
		}
		template<typename T, typename C>constexpr numbers_t<T,C> operator / (const numbers_t<T,C>& _src1, const T& _t) {
			numbers_t tmp(_src1);
			return tmp /= _t;
		}
		template<typename T, typename C> constexpr numbers_t<T,C> operator* (const T& _t , const numbers_t<T,C>& _src1 ) {
			numbers_t tmp(_src1);
			return tmp *= _t;
		}
		template<typename T, typename C>constexpr numbers_t<T,C> operator / (const T& _t, const numbers_t<T,C>& _src1) {
			numbers_t tmp(_src1);
			return tmp /= _t;
		}
		
		template<typename T, typename C> constexpr numbers_t<T,C> operator + (const numbers_t<T,C>& _src1, const numbers_t<T,C>& _src2) {
			numbers_t tmp(_src1);
			return tmp += _src2;
		}
		template<typename T, typename C> constexpr numbers_t<T,C> operator - (const numbers_t<T,C>& _src1, const numbers_t<T,C>& _src2) {
			numbers_t tmp(_src1);
			return tmp -= _src2;
		}
		
		/*template<typename T, typename C> constexpr numbers_t<T, C> operator * (const numbers_t<T, C>& _src1, const numbers_t<T, C>& _src2) {
			numbers_t tmp(_src1);
			return tmp *= _src2;
		}
		template<typename T, typename C> constexpr numbers_t<T,C> operator / (const numbers_t<T,C>& _src1, const numbers_t<T,C>& _src2) {
			numbers_t tmp(_src1);
			return tmp /= _src2;
		}*/
		
		template<typename T, typename C> constexpr numbers_t<T,C> operator + (const numbers_t<T,C>& _src1, const span<T, C::size> & _src2) {
			numbers_t tmp(_src1);
			return tmp += _src2;
		}
		template<typename T, typename C>constexpr numbers_t<T,C> operator - (const numbers_t<T,C>& _src1, const span<T, C::size>& _src2) {
			numbers_t tmp(_src1);
			return tmp -= _src2;
		}
		/*
		template<typename T, typename C> constexpr numbers_t<T,C> operator * (const numbers_t<T,C>& _src1, const span<T, C::size>& _src2) {
			numbers_t tmp(_src1);
			return tmp *= _src2;
		}
		template<typename T, typename C>constexpr numbers_t<T,C> operator / (const numbers_t<T,C>& _src1, const span<T, C::size>& _src2) {
			numbers_t tmp(_src1);
			return tmp /= _src2;
		}
		*/

		template<typename T, size_t M, size_t K, size_t N>
		void matrix_mult(const T * A, const T * B, T * C) {
			for (int i = 0; i < M; ++i) {
				T* c = C + i * N;
				for (int j = 0; j < N; ++j)
					c[j] = 0;
				for (int k = 0; k < K; ++k) {
					const T* b = B + k * N;
					T a = A[i * K + k];
					for (int j = 0; j < N; ++j)
						c[j] += a * b[j];
				}
			}
		}
				

		
		template<typename T, size_t M, size_t N> struct matrix_s{
			enum { nrows = M, ncols = N, size=M*N};
			union {
				T memo[size];
				T rows[nrows][ncols];
			};			
		};
		template<class T, size_t M, size_t N> using matrix_t = numbers_t <T, matrix_s<T, M, N>>;

		template<typename T, size_t M, size_t K, size_t N> matrix_t<T, M, N>  operator * (
			const matrix_t<T, M, K>& A
			, const matrix_t<T, K, N>& B
			) {
			matrix_t<T, M, N> C;
			matrix_mult<T,M,K,N>(A.memo, B.memo, C.memo);
			return C;
		}


		template<typename T> struct vector3_s {
		public:
			enum { size = 3 };
			union {
				T memo[size];
				struct {
					T x;
					T y;
					T z;
				};
			};
			vector3_s& mult(const vector3_s& b) {
				auto ax = x;
				auto ay = y;
				auto az = z;
				x = ay*b.z-az*b.y;
				y = az * b.x -ax*b.z;
				z = ax*b.y-ay*b.x;
				return *this;
			}

			template<typename S > void operator << (const S& _src) {
				x = _src.x;
				y = _src.y;
				z = _src.z;
			}

			template<typename S > void operator >> (S& _dst) {
				_dst.x = x;
				_dst.y = y;
				_dst.z = z;
			}


		};
		template<class T>	using vector3_t = numbers_t <T, vector3_s<T> >;

		template<typename T, size_t M> matrix_t<T, M, 1>  operator * (const matrix_t<T, M, 3> & A, const vector3_s<T> & B) {
			matrix_t<T, M, 1> C;
			matrix_mult<T, M, 3, 1>(A.memo, B.memo, C.memo);
			return C;
		}


		template<typename T, size_t N> class eye : public matrix_t<T, N, N> {
		private:
			typedef matrix_s<T, N, N> A;
		public:
			constexpr eye(void) {
				for (int i = 0; i < A::ncols; ++i)
					A::memo[i * A::ncols + i] = T(1);
			}
		};
		
		template<typename T> class quaternion_s {
			using rotmatrix_t = matrix_t<T, 3, 3>;
		public:
			enum {size = 4};
			union {
				T memo[size];
				struct {
					T w;
					T x;
					T y;
					T z;
				};
			};
			void rotate(void) { rotated_ = false;  }
			rotmatrix_t& A(void) { 
				if (!rotated_) rotate_(); 
				return A_; 
			};
			rotmatrix_t& IA(void) {
				if (!rotated_) rotate_();
				return IA_;
			};
			void mult(const quaternion_s& a, const quaternion_s & b) {
				w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z;
				x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y;
				y = a.w * b.y + a.y * b.w - a.x * b.z + a.z * b.x;
				z = a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w;
				rotated_ = false;
			}
			void div(const quaternion_s& a, const quaternion_s& b) {
				w = a.w * b.w + a.x * b.x + a.y * b.y + a.z * b.z;
				x = -a.w * b.x + a.x * b.w - a.y * b.z + a.z * b.y;
				y = -a.w * b.y + a.y * b.w + a.x * b.z - a.z * b.x;
				z = -a.w * b.z - a.x * b.y + a.y * b.x + a.z * b.w;
				rotated_ = false;
			}
			void from_dg(const std::initializer_list<T> _dg) {
				ROBO_APP_ASSERT(size == _dg.size());
				from_dg_(_dg.begin());
			}
			void from_dg(const T (&_dg)[size]) {
				from_dg_(_dg);
			}
		private:
			void from_dg_(const  T * _dg) {
				T f = _dg[0] * deg2rad<T>;
				w = cos(f / 2);
				vector3_t<T> n{ _dg[1],_dg[2] ,_dg[3] };
				n.normalize();
				n *= sin(f / 2);
				x = n.x;
				y = n.y;
				z = n.z;
				rotated_ = false;
			}
		public:
			bool load(cstr _path, cstr key) {
				T tmp[size];
				ROBO_LBREAKN(ini::load_arr(_path, key, tmp, size));
				from_dg_(tmp);
				return true;
			}
		private:
			rotmatrix_t A_;
			rotmatrix_t IA_;
			bool rotated_ = false;
			void rotate_(void) {
				T x2 = x * x * 2;
				T y2 = y * y * 2;
				T z2 = z * z * 2;
				T xw = x * w * 2;
				T xy = x * y * 2;
				T xz = x * z * 2;

				T yw = y * w * 2;
				T yz = y * z * 2;

				T zw = z * w * 2;
				T d1 = 1 - (y2 + z2);
				T d2 = 1 - (x2 + z2);
				T d3 = 1 - (x2 + y2);

				T* p = A_.memo;
				*p++ = d1;				*p++ = (xy - zw);		*p++ = (yw + xz);
				*p++ = (xy + zw);		*p++ = d2;				*p++ = (yz - xw);
				*p++ = (xz - yw);	    *p++ = (xw + yz);		*p++ = d3;

				p = IA_.memo;
				*p++ = d1;				*p++ = (xy + zw);		*p++ = (xz - yw);
				*p++ = (xy - zw);		*p++ = d2;				*p++ = (yz + xw);
				*p++ = (xz + yw);	    *p++ = (yz - xw);		*p++ = d3;

				rotated_ = true;
			}
		};
		template<class T> using quaternion_t = numbers_t <T, quaternion_s<T> >;

		template<typename T> constexpr quaternion_t <T> operator * (const quaternion_t <T> & _src1, const quaternion_t <T> & _src2) {
			quaternion_t <T> tmp;
			tmp.mult(_src1, _src2);
			tmp.normalize();
			return tmp;
		}
		template<typename T> constexpr quaternion_t <T> operator / (const quaternion_t <T>& _src1, const quaternion_t <T>& _src2) {
			quaternion_t <T> tmp;
			tmp.div(_src1, _src2);
			tmp.normalize();
			return tmp;
		}

		template<typename T> vector3_t<T> operator * ( quaternion_t<T>& a, const vector3_t<T>& b) {
			return a.A()*b;
		}
		template<typename T> vector3_t<T> operator / (const vector3_t<T>& b,  quaternion_t<T>& a) {
			return a.IA() * b;
		}

		template<typename T> class avionic_s {
		public:
			enum { size = 3 };
			union {
				T memo[size];
				struct {
					T yaw;
					T pitch;
					T roll;
				};
			};
		};
		template<class T> using avionic_t = numbers_t <T, avionic_s<T> >;

		template<typename T> avionic_t<T> & operator >> (const quaternion_t<T>& _quat, avionic_t<T>& _avc) {
			auto w = _quat.w;
			auto x = _quat.x;
			auto y = _quat.y;
			auto z = _quat.z;

			T d1 = 2. * (x * z - w * y);
			if (d1 > 1.) {
				d1 = 1. - ::std::numeric_limits<T>::epsilon();
			}
			if (d1 < -1.) {
				d1 = -1. + ::std::numeric_limits<T>::epsilon();
			}
			T tmp = -asin(d1);
			_avc.pith = -tmp;
			T d2 = tmp - pi<T> / 2.f;
			if (abs(d1) > 0.999) {
				T YR = atan2(-2 * (x * y - w * z), 1 - 2 * (x * x + z * z));
				//вариант первый
				T roll1, roll2;
				if (tmp < 0) {
					roll1 = YR - _avc.yaw;
				}
				else {
					roll1 = _avc.yaw - YR;
				}
				//вариант второй YAW+PITCH > pi
				if (YR > 0) {
					YR = YR - 2 * pi<T>;
				}
				else {
					YR = YR + 2. * pi<T>;
				}

				if (tmp < 0) {
					roll2 = YR - _avc.yaw;
				}
				else {
					roll2 = _avc.yaw - YR;
				}

				T df1 = roll1 - _avc.roll;

				T df2 = roll2 - _avc.roll;

				if (abs(df2) > abs(df1)) {
					_avc.roll = (T)roll1;
				}
				else {
					_avc.roll = (T)roll2;
				}

			}
			else {
				_avc.yaw = (T)atan2(T(2) * (x * y + w * z), T(1) - T(2) * (y * y + z * z));
				_avc.roll = (T)atan2(T(2) * (w * x + y * z), T(1) - T(2) * (x * x + y * y));
			}
			return _avc;
		}
		template<typename T> quaternion_t<T> &  operator >> (const avionic_t<T>& _avc, quaternion_t<T>& _quat) 
		{
			T Yd2 = _avc.yaw / T(2.);
			T Pd2 = -_avc.pitch / T(2.);
			T Rd2 = _avc.roll / T(2.);

			T  csR = ::std::cos(Rd2);
			T  snR = ::std::sin(Rd2);
			T  csP = ::std::cos(Pd2);
			T  snP = ::std::sin(Pd2);
			T  csY = ::std::cos(Yd2);
			T  snY = ::std::sin(Yd2);
			_quat.w = csR * csP * csY + snR * snP * snY;
			_quat.x = snR * csP * csY - csR * snP * snY;
			_quat.y = csR * snP * csY + snR * csP * snY;
			_quat.z = csR * csP * snY - snR * snP * csY;
			return _quat;
		}
		
		
		
		template<typename T> class quat_axis_t {
		public:
			quaternion_t <T> L;
			vector3_t<T> r;
			constexpr quat_axis_t() :L{ 1,0,0,0 }, r{0,0,0} {}
			constexpr quat_axis_t(const T(&_L)[4], const T(&_r)[3]) {
				L = span<T, 4>(_L);
				r = span<T, 3>(_r);
			}
			constexpr quat_axis_t(const quat_axis_t& _src)
				: L(_src.L)
				, r(_src.r) {
			}
			constexpr quat_axis_t(const quaternion_t <T>& _L)
				: L(_L){
			}
			constexpr quat_axis_t(const vector3_t<T>& _r)
				: r(_r) {
			}
			constexpr quat_axis_t(const quaternion_t <T>& _L, const vector3_t<T>& _r)
				: L(_L), r(_r) {
			}

			constexpr quat_axis_t(const std::initializer_list<T> _src)
			{
				ROBO_APP_ASSERT(7 == _src.size());
				L = span<T,4>(_src.begin());
				r = span<T,3>(_src.begin()+4);
			}
			constexpr quat_axis_t(const std::initializer_list<T> _L, const std::initializer_list<T> _r)
			{
				ROBO_APP_ASSERT(4 == _L.size());
				ROBO_APP_ASSERT(3 == _r.size());
				L = span<T, 4>(_L.begin());
				r = span<T, 3>(_r.begin() + 4);
			}
			constexpr quat_axis_t(const T(&_src)[7]){
				L = span<T,4>(_src);
				r = span<T,3>(_src+ 4);
			}
			template <typename A> constexpr quat_axis_t(const A& _src) {
				ROBO_APP_ASSERT(7 == _src.size);
				L = span<T,4>(_src.memo);
				r = span<T,3>(_src.memo + 4);
			}

			constexpr quat_axis_t& operator = (const quat_axis_t& _src) {
				L = _src.L;
				r = _src.r;
				L.rotate();
				return *this;
			}

			template <typename A> constexpr quat_axis_t& operator = (const A& _src) {
				ROBO_APP_ASSERT(7 == _src.size);
				L = span(_src.memo, 4);
				r = span(_src.memo + 4, 3);
				L.rotate();
				return *this;
			}
			bool load(cstr _path) {
				ROBO_LBREAKN(L.load(_path,RT("L")));
				ROBO_LBREAKN(r.load_raw(_path, RT("r")));
				L.rotate();
				return true;
			}

			template<typename S > void operator << (const S& _src) {
				L.w = _src.L.w;
				L.x = _src.L.x;
				L.y = _src.L.y;
				L.z = _src.L.z;
				r.x = _src.r.x;
				r.y = _src.r.y;
				r.z = _src.r.z;
			}

			template<typename S > void operator >> (S& _dst) {
				_dst.L.w = L.w;
				_dst.L.x = L.x;
				_dst.L.y = L.y;
				_dst.L.z = L.z;
				_dst.r.x = r.x;
				_dst.r.y = r.y;
				_dst.r.z = r.z;
			}

		};

		template<typename T> quat_axis_t<T> operator * ( quat_axis_t<T>& a, const quat_axis_t<T>& b) {
			quat_axis_t<T> c;
			c.L = a.L * b.L;
			c.r = a.r + a.L * b.r;
			return c;
		}

		template<typename T> quat_axis_t<T> operator / (const quat_axis_t<T>& c, const quat_axis_t<T>& b) {
			quat_axis_t<T> a;
			a.L = c.L / b.L;
			a.r = c.r - a.L * b.r;
			return a;
		}
		template<typename T> quat_axis_t<T> operator % (const quat_axis_t<T>& c, const quat_axis_t<T>& b) {
			//c.L = b.L * a.L;
			//c.r = b.r + b.L * a.r;

			//b.L^-1 * c.L =  a.L;
			//b.L^-1 *c.r = b.L^-1 *b.r +  a.r;

			quat_axis_t<T> a;
			quaternion_t<T> bL;
			bL.w = b.L.w;
			bL.x = -b.L.x;
			bL.y = -b.L.y;
			bL.z = -b.L.z;

			a.L = bL*c.L;
			a.r = bL*c.r - bL * b.r;

			return a;
		}
		template<typename T> quat_axis_t<T> operator - ( quat_axis_t<T>& a,  quat_axis_t<T>& b) {
			quat_axis_t<T> tmp;
			tmp.L = a.L / b.L;
			tmp.r = a.r - b.r / b.L;
			return tmp;
		}

		template<typename T> vector3_t<T> operator * ( quat_axis_t<T>& a, const vector3_t<T>& b) {
			return ((a.L * b)+ a.r);
		}
		template<typename T> vector3_t<T> operator / (const vector3_t<T>& b,  quat_axis_t<T>& a) {
			return  (b - a.r) / a.L;
		}
		template<typename T> struct scene_t {
			using axis = quat_axis_t<T>;
			using vector3 = vector3_t<T>;
			using avionic = avionic_t<T>;
			class series;
			class body;
			class actuator_s;
			class joint_s;
			class point;
			class robot;

			class point : public tree::item {
				friend class joint_s;
				friend class body;
				friend class actuator_s;
			public:
				typedef ::robo::list::unsorted<point> list;
				typedef typename list::ref ref;
			private:
				point* remote_ = nullptr;
				ref ref_;
				void assign_(point& _src) {
					local = _src.local;
				}
			public:
				body& body_ref() { return tree::item:: template branch<body>(); };
				axis local;
				axis base;
				vector3 torque;
				vector3 force;
				point(cstr _name, body& _body);
			protected:
				bool manual_arrange = false;
				virtual bool do_load(cstr _path) {
					if (name != string(RT("ct"))) {
						ROBO_LBREAKN(local.L.load(_path, RT("local.L")));
						ROBO_LBREAKN(local.r.load_raw(_path, RT("local.r")));
					}
					return true;
				}
			};

			class joint_s : public tree::item {
				friend class point;
				friend class body;
			public:
				typedef ::robo::list::unsorted<joint_s> list;
				typedef typename list::ref ref;
			private:
				point& from_;
				point& to_;
				ref ref_;
				void assign_(joint_s& _src) {
					deform = _src.deform;
					ddeform = _src.ddeform;
					guk_line = _src.guk_line;
					desep_line = _src.desep_line;
					guk_cicle = _src.guk_cicle;
					desep_cicle = _src.desep_cicle;
				}
			public:
				body& body_ref() { return from_.tree::item:: template branch<body>(); };
				axis deform;
				axis ddeform;
				vector3 guk_line;
				vector3 desep_line;
				vector3 guk_cicle;
				vector3 desep_cicle;
				joint_s(point& _from, point& _to);
				~joint_s(void);
			protected:
				virtual bool do_load(cstr _path) {
					ROBO_LBREAKN(guk_line.load_raw(_path, RT("guk_line")));
					ROBO_LBREAKN(desep_line.load_raw(_path, RT("desep_line")));
					ROBO_LBREAKN(guk_cicle.load_raw(_path, RT("guk_cicle")));
					ROBO_LBREAKN(desep_cicle.load_raw(_path, RT("desep_cicle")));
					return true;
				}
			};

			class body : public tree::item {
				friend class actuator_s;
				friend class point;
				friend class joint_s;
				friend class link_s;
				friend class series;
			public:
				typedef ::robo::list::unsorted<body> list;
				typedef typename list::ref ref;
			private:
				typename point::list points_;
				typename joint_s::list joints_;
				ref ref_;
				void arrange_(void) {
					for (typename point::list::ref* r = points_.first()->next(); r; r = r->next()) {
						point& pt = r->owner();
						if (!pt.manual_arrange) {
							pt.base = ct.base * pt.local;
						}
					}
				}
				void assign_(body& _src) {

					{
						typename point::list::ref* d = points_.first();
						typename point::list::ref* s = _src.points_.first();
						for (; d; d = d->next(), s = s->next()) {
							d->owner().assign_(s->owner());
						}
					}

					{
						typename joint_s::list::ref* d = joints_.first();
						typename joint_s::list::ref* s = _src.joints_.first();
						for (; d; d = d->next(), s = s->next()) {
							d->owner().assign_(s->owner());
						}
					}
					mass = _src.mass;
					inertion = _src.inertion;
				}
			public:
				series& series_ref() { return tree::item:: template branch<series>(); };
				point ct;
				T mass = T(1);
				vector3 inertion = { T(1),T(1) ,T(1) };
				body(cstr _name, series& _series);
			};

			class actuator_s : public point {
				friend class body;
				friend class series;
				T dg_ = T(0.);
				T rd_ = T(0.);
			protected:
				virtual void rotate(const T& _angle) = 0;
			public:
				typedef ::robo::list::unsorted<actuator_s> list;
				typedef typename list::ref ref;
				const T & dg(void) { return dg_;  };
				const T& rd(void) { return rd_; };

				void rotate_dg(const T& _angle) {
					dg_ = _angle;
					rd_ = _angle * deg2rad<T>;
					rotate(rd_);
				};
				void rotate_rd(const T& _angle) {
					dg_ = _angle * rad2deg<T>;
					rd_ = _angle;
					rotate(_angle);
				};

				actuator_s(cstr _name, body& _body);

				

				virtual T rot_projection(const vector3 & R) = 0;/*
				{
					return const_cast<vector3<T>&>(R).z();
				};*/

				virtual T ro2(const vector3 & R)=0;/*
				{
					T x = const_cast<vector3<T>&>(R).x();
					T y = const_cast<vector3<T>&>(R).y();
					return x * x + y * y;
				};*/

				T inertion(const vector3& R, const T& mass) {
					vector3 tmp = R/point::base ;
					return ro2(tmp)/1000000. * mass;
				}
				T torque(const vector3 & R, const vector3 & F) {
					vector3 tmp = R- point::base.r;
					tmp.mult(F);
					vector3 tq = tmp / point::base.L;
					return rot_projection(tq)/1000.;
				}
				T mgtorque(const vector3 & R, const T& P) {
					vector3 F({ T(0.), T(0.), P });
					return torque(R, F);
				}

			protected:
				axis position;
			private:
				ref ref_;

				void arrange_(void) {
					point::base = point::body_ref().ct.base * point::local;
					axis S = point::base * position;
					point* rm = point::remote_;
					S = S / rm->local;
					rm->body_ref().ct.base = S;
				}
				void assign_(const actuator_s& _src) {
					position = _src.position_;
				}
			};

			class yaw : public actuator_s {
			protected:
				virtual void rotate(const T& _angle) {
					actuator_s::position.L.w = cos(_angle / 2);
					actuator_s::position.L.z = sin(_angle / 2);
					actuator_s::position.L.rotate();
				}

			public:
				virtual T rot_projection(const vector3& R)
				{
					return R.z;
				}

				virtual T ro2(const vector3& R)
				{
					T x = R.x;
					T y = R.y;
					return x * x + y * y;
				}

				yaw(body& _body) :actuator_s(RT("A"), _body) {}
			};

			class pitch : public actuator_s {
			protected:
				virtual void rotate(const T& _angle) {
					actuator_s::position.L.w = cos(_angle / 2);
					actuator_s::position.L.y = sin(_angle / 2);
					actuator_s::position.L.rotate();
				}
			public:

				virtual T rot_projection(const vector3& R) {
					return R.y;
				}

				virtual T ro2(const vector3& R) {
					T x = R.x;
					T z = R.z;
					return x * x + z * z;
				}

				pitch(body& _body) :actuator_s(RT("A"), _body) {}
			};

			class roll : public actuator_s {
			protected:
				virtual void rotate(const T& _angle) {
					actuator_s::position.L.w = cos(_angle / 2);
					actuator_s::position.L.x = sin(_angle / 2);
					actuator_s::position.L.rotate();
				}
			public:
				virtual T rot_projection(const vector3& R) {
					return R.x;
				}

				virtual T ro2(const vector3& R) {
					T y = R.y;
					T z = R.z;
					return y * y + z * z;
				}
				roll(body& _body) :actuator_s(RT("A"), _body) {}
			};


			class series : public tree::item {
				friend class robot;
				friend class body;
				friend class actuator_s;
			public:
				typedef ::robo::list::unsorted<series> list;
				robot& robot_ref() { return tree::item:: template branch<robot>(); };
				typename actuator_s::list actuators;
				typename body::list bodies;

			private:
				robot& robot_;
				typename list::ref ref_;

				void assign_(series& _src) {
					{
						typename actuator_s::list::ref* d = actuators.first();
						typename actuator_s::list::ref* s = _src.actuators.first();
						for (; d; d = d->next(), s = s->next()) {
							d->owner().assign_(s->owner());
						}
					}
					{
						typename body::list::ref* d = bodies.first();
						typename body::list::ref* s = _src.bodies.first();
						for (; d; d = d->next(), s = s->next()) {
							d->owner().assign_(s->owner());
						}
					}
				}
			public:
				series(cstr _name, robot& _owner);				
				void position_nat_set(const T * _arr) {
					for (auto* a = actuators.first(); a; a = a->next()) {
						a->owner().rotate_rd(*_arr++);
					}
				}
				void position_deg_set(const T* _arr) {
					for (auto* a = actuators.first(); a; a = a->next()) {
						a->owner().rotate_dg(*_arr++);
					}
				}
				void position_nat_get(const T* _arr) {
					for (auto* a = actuators.first(); a; a = a->next(), ++_arr) {
						*_arr = a->owner().rd();
					}
				}
				void position_deg_get(const T* _arr) {
					for (auto* a = actuators.first(); a; a = a->next(), ++_arr) {
						*_arr = a->owner().dg();
					}
				}
				void position_move_to_zero(void) {
					for (auto* a = actuators.first(); a; a = a->next()) {
						a->owner().rotate_rd(0);
					}
				}
				void position_move_to_zero( T* _arr) {
					for (auto* a = actuators.first(); a; a = a->next(), ++_arr) {
						*_arr = a->owner().rd();
						a->owner().rotate_rd(0);
					}
				}
				void arrange(void) {
					for (typename actuator_s::list::ref* r = actuators.first(); r; r = r->next()) {
						r->owner().arrange_();
					}
					for (typename body::list::ref* r = bodies.first(); r; r = r->next()) {
						r->owner().arrange_();
					}
				}
			};


			class robot : public tree::item {
				friend class series;
			public:
			private:
				typename series::list series_;
			public:
				//const typename series::list & serieses(void) { return series_; }
				robot(cstr _name) : tree::item(_name, nullptr) {}
				void arrange(void) {
					for (typename series::list::ref* r = series_.first(); r; r = r->next()) {
						r->owner().arrange();
					}
				}
				robot& assign(robot& _robot) {
					typename series::list::ref* d = series_.first();
					typename series::list::ref* s = _robot.series_.first();
					for (; d; d = d->next(), s = s->next()) {
						d->owner().assign_(s->owner());
					}
					arrange();
					return *this;
				}
			};

			class link_s : public body {
			public:
				actuator_s* actuator = nullptr;
				point* clamp;
				joint_s* joint;
			public:
				link_s(cstr _name,  link_s & _prev)
					: body(_name, _prev.series_ref())
				{
					clamp = new point(RT("C"), *this);
					joint = new joint_s(*clamp, *_prev.actuator);
				}
				link_s(cstr _name, series& _series)
					: body(_name, _series)
				{
					clamp = nullptr;
					joint = nullptr;
				}
				~link_s(void) {
					if (actuator) delete actuator;
					if (clamp) delete clamp;
					if (joint) delete joint;
				}
				void rotate_rd(const T& _angle) {
					actuator -> rotate_rd(_angle);
				}
				void rotate_dg(const T& _angle) {
					actuator->rotate_dg(_angle);
				}
			};

			template<class TA> class link_t : public link_s {
			public:
				link_t(cstr _name, link_s& _prev) :link_s(_name, _prev) {
					link_s::actuator = new TA(*this);
				}
				link_t(cstr _name, series& _series) : link_s(_name, _series){
					link_s::actuator = new TA(*this);
				}
			};

			class payload : public body {
			public:
				point clamp;
				point target;
				joint_s joint;
			public:
				payload(cstr _name, link_s& _prev)
					: body(_name, _prev.series_ref())
					, clamp(RT("C"), *this)
					, target(RT("T"), *this)
					, joint(clamp,*_prev.actuator) {
				}
			};

		};

		template<typename T> scene_t<T>::point::point(cstr _name, scene_t<T>::body& _body)
			: tree::item(_name, &_body)
			, ref_(*this)
			{
			ref_.attach_to(body_ref().points_);
		}


		template<typename T> scene_t<T>::joint_s::joint_s(point& _from, point& _to)
			: tree::item( string(RT("%s-%s.%s")
								, _from.name.c_str()
								, _to.body_ref().name.c_str()
								, _to.name.c_str()
			), & _from.body_ref())
			, ref_(*this )
			, from_(_from)
			, to_(_to) {
			from_.remote_ = &to_;
			to_.remote_ = &from_;
			ref_.attach_to(_from.body_ref().joints_);
		}
		template<typename T> scene_t<T>::joint_s::~joint_s(void) {
			from_.remote_ = nullptr;
			to_.remote_ = nullptr;
		}

		
		template<typename T> scene_t<T>::body::body(cstr _name, series& _series)
			: tree::item(_name, &_series)
			, ref_(*this)
			, ct(RT("ct"), *this) {
			ref_.attach_to(series_ref().bodies);
			ct.manual_arrange = true;;
		}

		template<typename T> scene_t<T>::actuator_s::actuator_s(cstr _name, body& _body)
			: point(_name, _body)
			, ref_(*this)
		{
			point::manual_arrange = true;
			ref_.attach_to(_body.series_ref().actuators);
		};

		template<typename T> scene_t<T>::series::series(cstr _name, robot& _robot)
			: tree::item(_name, &_robot)
			, robot_(_robot)
			, ref_(*this)
		{
			ref_.attach_to(robot_ref().series_);
		}


		template<typename T> class matrix3x3_s {
		public:
		  enum { n = 3,	 size = n * n };
			union {
				T memo[size];
				T rows[n][n];
				struct {
					T xx;
					T xy;
					T xz;
					T yx;
					T yy;
					T yz;
					T zx;
					T zy;
					T zz;
				};
			};

			void mult(const matrix3x3_s& a, const matrix3x3_s& b) {
				xx = a.xx * b.xx + a.xy * b.yx + a.xz * b.zx;
				xy = a.xx * b.xy + a.xy * b.yy + a.xz * b.zy;
				xz = a.xx * b.xz + a.xy * b.yz + a.xz * b.zz;
				yx = a.yx * b.xx + a.yy * b.yx + a.yz * b.zx;
				yy = a.yx * b.xy + a.yy * b.yy + a.yz * b.zy;
				yz = a.yx * b.xz + a.yy * b.yz + a.yz * b.zz;
				zx = a.zx * b.xx + a.zy * b.yx + a.zz * b.zx;
				zy = a.zx * b.xy + a.zy * b.yy + a.zz * b.zy;
				zz = a.zx * b.xz + a.zy * b.yz + a.zz * b.zz;
			}
			void onediv(const matrix3x3_s& b) {
				xx = b.yy * b.zz - b.yz * b.zy;
				xy = b.xz * b.zy - b.xy * b.zz;
				xz = b.xy * b.yz - b.xz * b.yy;
				yx = b.yz * b.zx - b.yx * b.zz;
				yy = b.xx * b.zz - b.xz * b.zx;
				yz = b.xz * b.yx - b.xx * b.yz;
				zx = b.yx * b.zy - b.yy * b.zx;
				zy = b.xy * b.zx - b.xx * b.zy;
				zz = (b.xx * b.yy - b.xy * b.yx);
			}
			void det_normalize(void) {
				T det = (xx * yy * zz
						 - xx * yz * zy
						 - xy * yx * zz
						 + xy * yz * zx
						 + xz * yx * zy
						 - xz * yy * zx);
				det = std::cbrt(det);
				xx /= det;
				xy /= det;
				xz /= det;
				yx /= det;
				yy /= det;
				yz /= det;
				zx /= det;
				zy /= det;
				zz /= det;
			}
			bool load(cstr _path, cstr key) {
				ROBO_LBREAKN(ini::load_arr(_path, key, memo, size));
				return true;
			}
			T  & operator () (int _raw,int _cal) {
				return memo[_raw *3+ _cal];
			}
		};
		template<class T> using matrix3x3_t = numbers_t <T, matrix3x3_s<T> >;

		template<typename T> constexpr matrix3x3_t <T> operator * (const matrix3x3_t <T>& _src1, const matrix3x3_t <T>& _src2) {
			matrix3x3_t <T> tmp;
			tmp.mult(_src1, _src2);
			return tmp;
		}

		template<typename T> vector3_t<T> operator * (const matrix3x3_t<T>& a, const vector3_t<T>& c) {
			vector3_t<T> tmp;
			tmp.x = a.xx* c.x + a.xy * c.y + a.xz * c.z;
			tmp.y = a.yx* c.x + a.yy * c.y + a.yz * c.z;
			tmp.z = a.zx* c.x + a.zy * c.y + a.zz * c.z;
			return tmp;
		}
		/*template<typename T> vector3_t<T> operator / (const vector3_t<T>& b, const matrix3x3_t<T>& a) {
			vector3_t<T> tmp;
			return tmp;
		}*/
		template<typename T>  avionic_t<T>& operator >> (const matrix3x3_t<T>& _transform, avionic_t<T>& _avc) {
			//[cos(P) * cos(Y), -cos(R) * sin(Y) - cos(Y) * sin(P) * sin(R), sin(R) * sin(Y) - cos(R) * cos(Y) * sin(P)]
			//[cos(P) * sin(Y), cos(R) * cos(Y) - sin(P) * sin(R) * sin(Y), -cos(Y) * sin(R) - cos(R) * sin(P) * sin(Y)]
			//[sin(P), cos(P) * sin(R), cos(P) * cos(R)]
			 T d1 = _transform.zx;
			if (d1 > T(1.) ) {
				d1 = T(1.) - ::std::numeric_limits<T>::epsilon();
			}
			if (d1 < -T(1.)) {
				d1 = -T(1.) + ::std::numeric_limits<T>::epsilon();
			}
			T tmp = asin(d1);
			_avc.pitch = tmp;

			if (abs(d1) > 0.9999) {
				T YR;
				YR = atan2(-_transform.yz, _transform.yy);
				//вариант первый
				T roll1, roll2;
				if (tmp > 0) {
					//YR = R + Y  R = YR - Y;
					roll1 = YR - _avc.yaw;
				}
				else {
					//YR = R - Y  R = YR + Y;
					roll1 = YR+ _avc.yaw;
				}
				//вариант второй YAW+PITCH > pi
				if (YR > 0) {
					YR = YR - T(2) * pi<T>;
				}
				else {
					YR = YR + T(2) * pi<T>;
				}

				if (tmp > 0) {
					roll2 = YR - _avc.yaw;
				}
				else {
					roll2 = YR + _avc.yaw;
				}

				T df1 = roll1 - _avc.roll;

				T df2 = roll2 - _avc.roll;

				if (abs(df2) > abs(df1)) {
					_avc.roll = roll1;
				}
				else {
					_avc.roll = roll2;
				}

			}
			else {
			_avc.yaw = atan2(_transform.yx, _transform.xx);
			_avc.roll = atan2(_transform.zy, _transform.zz);
			}
			return _avc;
		};

		template<typename T> matrix3x3_t<T>& operator >> (const avionic_t<T>& _avc, matrix3x3_t<T>& _transform) {
			auto Y = _avc.yaw;
			auto P = _avc.pitch;
			auto R = _avc.roll;
			_transform.xx = cos(P) * cos(Y);
			_transform.xy = -cos(R) * sin(Y) - cos(Y) * sin(P) * sin(R);
			_transform.xz = sin(R) * sin(Y) - cos(R) * cos(Y) * sin(P);
			_transform.yx = cos(P) * sin(Y);
			_transform.yy = cos(R) * cos(Y) - sin(P) * sin(R) * sin(Y);
			_transform.yz = -cos(Y) * sin(R) - cos(R) * sin(P) * sin(Y);
			_transform.zx = sin(P);
			_transform.zy = cos(P) * sin(R);
			_transform.zz = cos(P) * cos(R);
			return _transform;
		}


		template<typename T> class matrix_axis_t {
		public:
			using matrix = matrix3x3_t<T>;
			matrix& transform() {
				if (!dirrect_exists_) {
					normalize_();
				}
				return transform_;
			};
			vector3_t<T> offset;
			matrix& itransform(void) {
				if (!inverted_exists_) {
					invert_();
				}
				return itransform_;
			}
			constexpr matrix_axis_t() :transform_(eye<T, 3>()), offset{ 0,0,0 } {
			}
			constexpr matrix_axis_t(const matrix_axis_t& _src)
				: transform_(_src.transform_)
				, offset(_src.offset) {}
			constexpr matrix_axis_t(const matrix& _L)
				: transform_(_L) {}
			constexpr matrix_axis_t(const vector3_t<T>& _r)
				: transform_(eye<T, 3>()), offset(_r) {}
			constexpr matrix_axis_t(const matrix& _L, const vector3_t<T>& _r)
				: transform_(_L), offset(_r) {}

			constexpr matrix_axis_t(const quaternion_t<T>& _q)
				: transform_(quaternion_t<T>(_q).A()) {}

			constexpr matrix_axis_t(const quaternion_t<T>& _q, const vector3_t<T>& _r)
				: transform_(quaternion_t<T>(_q).A()), offset(_r) {}

			constexpr matrix_axis_t(const std::initializer_list<T> _src) {
				ROBO_APP_ASSERT(7 == _src.size());
				transform_ = span<T, 9>(_src.begin());
				offset = span<T, 3>(_src.begin() + 9);
			}
			constexpr matrix_axis_t(const T(&_src)[7]) {
				transform_ = span<T, 9>(_src);
				offset = span<T, 3>(_src + 9);
			}
			template <typename A> constexpr matrix_axis_t(const A& _src) {
				ROBO_APP_ASSERT(12 == _src.size);
				transform_ = span<T, 9>(_src.memo);
				offset = span<T, 3>(_src.memo + 9);
			}

			constexpr matrix_axis_t& operator = (const matrix_axis_t& _src) {
				transform_ = _src.transform_;
				offset = _src.offset;
				begin_update();
				return *this;
			}

			template <typename A> constexpr matrix_axis_t& operator = (const A& _src) {
				ROBO_APP_ASSERT(12 == _src.size);
				transform_ = span(_src.memo, 9);
				offset = span(_src.memo + 9, 3);
				begin_update();
				return *this;
			}
			enum class load_format { matrix = 0, quaternion =1, avionic = 2};
			bool load(cstr _path) {
				load_format format = load_format::matrix;
				ini::try_load(_path, RT("format"), (int&)format);
				switch (format) {
				case load_format::matrix:
				{
					ROBO_LBREAKN(transform_.load_raw(_path, RT("T")));
					break;
				}
				case load_format::quaternion:
				{
					quaternion_t<T> q;
					ROBO_LBREAKN(q.load(_path, RT("L")));
					transform_ = q.A();
					break;
				}
				case load_format::avionic:
				{
					avionic_t<T> a;
					ROBO_LBREAKN(a.load_raw(_path, RT("A")));
					a >> transform_;
					break;
				}
				}
				offset.load_raw(_path, RT("r"));
				begin_update();
				return true;
			}

			void begin_update(void) {
				inverted_exists_ = false; 
				dirrect_exists_ = false;
			}

		private:
			bool inverted_exists_ = false;
			bool dirrect_exists_ = false;

			void normalize_(void) {
				transform_.det_normalize();

				dirrect_exists_ = true;
			}

			void invert_(void) {
				if (!dirrect_exists_) normalize_();
				
				itransform_.onediv(transform_);
				
				inverted_exists_ = true;
			}
			public:
			matrix itransform_;
			matrix transform_;
		};

		template<typename T> matrix_axis_t<T> operator * ( matrix_axis_t<T>& a,  matrix_axis_t<T>& b) {
			matrix_axis_t<T> c;
			c.transform_ = a.transform() * b.transform();
			c.offset = a.offset + a.transform() * b.offset;
			return c;
		}

		template<typename T> matrix_axis_t<T> operator / (  matrix_axis_t<T>& c,  matrix_axis_t<T>& b) {
			matrix_axis_t<T> a;
			a.transform_ = c.transform() * b.itransform();
			a.offset = c.offset - a.transform() * b.offset;
			return a;
		}

		
		template<typename T> vector3_t<T> operator * ( matrix_axis_t<T>& a, const vector3_t<T>& b) {
			return ((a.transform() * b) + a.offset);
		}

		template<typename T> vector3_t<T> operator / ( vector3_t<T>& b, matrix_axis_t<T>& a) {
			return a.itransform() *(b - a.offset);
		}
		//===========================================================
		template <typename T>
		class matrix2x2_s {
		  public:
			enum {  n = 2, size = n*n};
			union {
				T memo[size];
				T rows[2][2];
				struct {
					T xx;
					T xy;
					T yx;
					T yy;
				};
			};

			void mult(const matrix2x2_s &a, const matrix2x2_s &b) {
				xx = a.xx * b.xx + a.xy * b.yx;
				xy = a.xx * b.xy + a.xy * b.yy;
				yx = a.yx * b.xx + a.yy * b.yx;
				yy = a.yx * b.xy + a.yy * b.yy;
			}
			
			bool load(cstr _path, cstr key) {
				ROBO_LBREAKN(ini::load_arr(_path, key, memo, size));
				return true;
			}
			T &operator()(int _index1, int _index2) {
				return memo[_index2 * 3 + _index1];
			}
		};
		template <class T>
		using matrix2x2_t = numbers_t<T, matrix2x2_s<T>>;

		template <typename T>
		constexpr matrix2x2_t<T> operator*(const matrix2x2_t<T> &_src1, const matrix2x2_t<T> &_src2) {
			matrix2x2_t<T> tmp;
			tmp.mult(_src1, _src2);
			return tmp;
		}
		template <typename T>
		struct vector2_s {
		  public:
			enum { size = 2 };
			union {
				T memo[size];
				struct {
					T x;
					T y;
				};
			};
			T &mult(const vector2_s &b) {
				return  x * b.y - y * b.x;
			}

			template <typename S>
			void operator<<(const S &_src) {
				x = _src.x;
				y = _src.y;
			}

			template <typename S>
			void operator>>(S &_dst) {
				_dst.x = x;
				_dst.y = y;
			}
		};
		template <class T>
		using vector2_t = numbers_t<T, vector3_s<T>>;
		template <typename T>
		vector2_t<T> operator*(const matrix2x2_t<T> &a, const vector2_t<T> &c) {
			vector2_t<T> tmp;
			tmp.x = a.xx * c.x + a.xy * c.y;
			tmp.y = a.yx * c.x + a.yy * c.y;
			return tmp;
		}
		//===========================================================
		template<typename T> class kinematic_t {
		public:
			static inline const T pi = ::robo::pi<T>;
			static inline const T deg2rad = ::robo::deg2rad<T>;
			static inline const T rad2deg = ::robo::rad2deg<T>;
			static inline const T epsilon = ::std::numeric_limits<T>::epsilon()*10;


			class series_s;
			class link_s {
			public:
				enum class types {
					none
					, line
					, cicle_infinite
					, multirotated
					, cicle_signed
					, cicle_unsigned
				};

				friend class series_s;
			private:
				series_s & series_;
				link_s* prev_;
				link_s* next_;
				T actual_ = 0;
				T min_ = 0;
				T max_ = 0;
				types type_;
				void attach_(void) {
					if (series_.last_ != nullptr) {
						prev_ = series_.last_;
						prev_->next_ = this;
					}
					else {
						series_.first_ = this;
					}
					series_.last_ = this;
					series_.count_++;

				}			
			public:
				void update_forvard(void) {
					if (prev_ != nullptr) {
						base = prev_-> base * local;
					}
					else {
						base = local;
					}
				}
				matrix_axis_t<T> native;
				matrix_axis_t<T> local;
				matrix_axis_t<T> base;

				virtual void do_move(const T& _dest) { };

				
				link_s(const matrix_axis_t<T>& _s, series_s & _series, types _type)
				: series_(_series)
				, next_(nullptr) 
				, type_(_type)
				{
					local = native = _s;
					do_move(actual_);
					base = local;
					attach_();
				}
				const link_s* cnext(void) const {
					return next_;
				}
				const link_s* cprev(void) const {
					return prev_;
				}
				link_s* next(void) {
					return next_;
				}
				link_s* prev(void) {
					return prev_;
				}
				virtual void assign(const link_s & _src) {
					native = _src.native;
					local = _src.local;
					base = _src.base;
					min_ = _src.min_;
					max_ = _src.max_;
					actual_ = _src.actual_;
				}

				T get(void) const {
					return actual_;
				}

				T getby(T _src) const {
					T dummy;
					getby(_src, dummy);
				}

				T getby(T _src, T& _delta) const {
					//_src += offset_;
					_delta = _src - actual_;
					switch (type_) {
					case types::none:
					case types::line:
					return  _src;
					case types::cicle_infinite:
					case types::multirotated:
					{
						while (_delta > pi) {
							_delta = _delta - T(2) * pi;
						}
						while (_delta < -pi) {
							_delta = _delta + T(2) * pi;
						}
						T ret = actual_ + _delta;
						if (ret > max_) {
							ret -= (T(2) * pi);
						}
						if (ret < min_) {
							ret += (T(2) * pi);
						}
						return ret;
					}

					case types::cicle_signed:
					{
						while (_src > pi) {
							_src -= T(2) * pi;
						}
						while (_src < -pi) {
							_src += T(2) * pi;
						}
						return _src;
					}
					case types::cicle_unsigned:
					{
						while (_src >= T(2) * pi) {
							_src -= T(2) * pi;
						}
						while (_src < T(0)) {
							_src += T(2) * pi;
						}
						return _src;
					}
					}
					return T(0);

				}
				bool check(T _val, T& _delta) const {
					T tmp = getby(_val, _delta);
					switch (type_) {
					case types::none:
					case types::cicle_infinite:
					return true;
					default:
					return ((tmp >= min_ - 0.00000000000008) && (tmp <= max_ + 0.00000000000008));
					}
				}

				T score(double _val) const {
					T tmp = get(_val);
					switch (type_) {
					case types::none:
					case types::cicle_infinite:
					return T(1);
					default:
					if ((tmp >= min_) && (tmp <= max_)) {
						T s = (tmp - min_) / (max_ - min_);
						return T(1) - T(1) / (T(1) + exp(T(40) * s * (T(1) - s) - T(5)));
					}
					else {
						return T(0);
					}
					}
				}

				void begin(T _actual_dg, T _min_dg, T _max_dg) {
					min_ = _min_dg * deg2rad;
					max_ = _max_dg * deg2rad;
					move_deg(_actual_dg);
				}

				void move_deg(T _dg) {
					actual_ = _dg * deg2rad;
					do_move(actual_);
				}
				void move_nat(T _nat) {
					actual_ = _nat ;
					do_move(actual_);
				}
				
			};

			class yaw_s : public link_s {
			public:
				virtual void do_move(const T& _dest) {
					T cs = cos(_dest);
					T sn = sin(_dest);
					auto& m = link_s::local.transform();
					const auto& n = link_s::native.transform();
					m.xx = n.xx * cs + n.xy * sn;
					m.xy = n.xy * cs - n.xx * sn;

					m.yx = n.yx * cs + n.yy * sn;
					m.yy = n.yy * cs - n.yx * sn;

					m.zx = n.zx * cs + n.zy * sn;
					m.zy = n.zy * cs - n.zx * sn;
				}
				yaw_s(const matrix_axis_t<T>& _s, series_s& _series, typename link_s::types _type):link_s(_s, _series, _type){}
			};

			class pitch_s : public link_s {
			public:
				virtual void do_move(const T& _dest) {
					T cs = cos(_dest);
					T sn = sin(_dest);
					auto& m = link_s::local.transform();
					const auto& n = link_s::native.transform();
					m.xx = n.xx * cs - n.xz * sn;
					m.xz = n.xz * cs + n.xx * sn;

					m.yx = n.yx * cs - n.yz * sn;
					m.yz = n.yz * cs + n.yx * sn;

					m.zx = n.zx * cs - n.zz * sn;
					m.zz = n.zz * cs + n.zx * sn;
				}
				pitch_s(const matrix_axis_t<T>& _s, series_s& _series, typename link_s::types _type) :link_s(_s, _series, _type) {}
			};

			class roll_s : public link_s {
			public:
				virtual void do_move(const T& _dest) {
					T cs = cos(_dest);
					T sn = sin(_dest);
					auto& m = link_s::local.transform();
					const auto& n = link_s::native.transform();
					m.xy = n.xy * cs + n.xz * sn;
					m.xz = n.xz * cs - n.xy * sn;

					m.yy = n.yy * cs + n.yz * sn;
					m.yz = n.yz * cs - n.yy * sn;

					m.zy = n.zy * cs + n.zz * sn;
					m.zz = n.zz * cs - n.zy * sn;
				}
				roll_s(const matrix_axis_t<T>& _s, series_s& _series, typename link_s::types _type) :link_s(_s, _series, _type) {}
			};
			
			class series_s {
				friend class link_s;
				link_s* first_ = nullptr;
				link_s* last_ = nullptr;
				int count_ = 0;
			public:
				const link_s* cfirst(void)  const {
					return first_;
				} 
				const link_s* clast(void) const  {
					return last_;
				}

				link_s* first(void) {
					return first_;
				}
				link_s* last(void) {
					return last_;
				}

				void update_forvard(void) {
					for (link_s* l = first_   ; l; l = l->next_ ) {
						l->update_forvard();
					}
				}
				void assign(const series_s & _series) {
					const link_s* _s = _series.first_;
					link_s* _d = first_;
					while ( (_s!=nullptr)  &&  (_d != nullptr) ) {
						_d->assign(*_s);
						_d = _d->next_;
						_s = _s->next_;
					}
				}
				//
			void assign(typename scene_t<T>::series & _src) {
					ROBO_APP_ASSERT(_src.actuators.count() >= count_);
					//base
					/*				const axis& ofsetof(const joint& _pt) {
					point::base = point::body_ref().ct.local * point::local;
					axis S = point::base * position;
					point* rm = point::remote_;
				}*/

					T * store = new T[count_];
					_src.position_move_to_zero(store);
					_src.arrange();
					auto* it = _src.actuators.first();
					auto axis0 = it->owner().base;
					link_s * link = first_;
					link->local = link->native = matrix_axis_t<T>(axis0.L, axis0.r);
					
					//link->move_nat(it->owner().rd());
					it = it->next();
					link = link->next_;
					for (; it; it = it->next(), link = link->next_) {
						//it->owner().base = axis0 * axis;
						
						const auto & base = it->owner().base;
						auto axis = base  % axis0;
						auto b1 = axis0 * axis;
						auto z = base / b1;
						link->local = link->native = matrix_axis_t<T>(axis.L, axis.r);
						//link->move_nat(it->owner().rd());
						axis0 = base;
					}


					_src.position_nat_set(store);
					_src.arrange();
					move_nat(store, count_);
					update_forvard();
					delete[] store;

				}
				void move(typename scene_t<T>::series& _src) {
					ROBO_APP_ASSERT(_src.actuators.count() >= count_);
					link_s* link = first_;
					for (auto* it = _src.actuators.first(); it; it = it->next(), link = link->next_) {
						link->move_nat( it->owner().rd());
					}					
				}
			protected:
				void move_nat(const T * _arr, int _count) {
					for (link_s* l = first_; l; l = l->next_) {
						if (_count) {
							l->move_nat(*_arr++);
							_count--;
						}
						l->update_forvard();
					}
				}
				void move_deg(const T* _arr, int _count) {
					for (link_s* l = first_; l; l = l->next_) {
						if (_count) {
							l->move_deg(*_arr++);
							_count--;
						}
						l->update_forvard();
					}
				}

			};

			template<class S, typename ... Args > class payload_t : public link_s, public S {
			public:
				virtual void do_move(const T& _dest) {}
				virtual void on_set_tool(void) {
					link_s::native = link_s::local = S::tool_position();
				}
				payload_t(const matrix_axis_t<T>& _s, series_s & _series, Args... args) :link_s(_s, _series, link_s::types::none), S(args...) {}
			};
		};
		
	};
}
#endif