#ifndef rovosd_kinematiks_hpp
#define rovosd_kinematiks_hpp
#include <cmath>
#include "core/robosd_common.hpp"
#include "core/robosd_list.hpp"
#include "core/robosd_span.hpp"
namespace robo {
	namespace kinematiks {
		
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

		template<typename T> constexpr T deg2rad = pi<T> / 180.;
		template<typename T> constexpr T rad2deg = T(1.) / deg2rad<T>;
		/*template<typename T, size_t N> constexpr const ::robo::span<T, N> span(const T(&_arr)[N]) {
			return  ::robo::span<T, N>((T *) &(_arr[0]),N);
		}*/
		
		/*template<typename T, size_t N> struct rshp {
			const T(&arr_)[N];
			constexpr rshp(T(&_arr)[N]) : arr_(_arr) {}
			template<size_t O, size_t M> constexpr const ::robo::span<T, M>  at () {
				return  ::robo::span<T, M>((T*)&(arr_[O]), M);
			}			
		};*/

		namespace  axis {
			template<typename T, class S> struct orient_span_t : public robo::span<T, S::count> {
				constexpr orient_span_t(const S& _s) : robo::span<T, S::count>((T*)&_s.memo[0]) {}
				template< class F > constexpr  orient_span_t(const F & _src) :
					robo::span<T, S::count>( _src.data()+ F::orientofs) {
				}
			};
			template<typename T, class S> struct position_span_t : public ::robo::span<T, S::count> {
				constexpr position_span_t(const S& _s) : ::robo::span<T, S::count>((T*)&_s.memo[0]) {}
				template< class F > constexpr  position_span_t(const F& _src) :
					::robo::span<T, S::count>(_src.data() + F::posofs ) {}
			};
			template<typename T> struct euclid {
				enum { count = 3 };
				typedef position_span_t<T,euclid> span;
				union {
					struct {
						T x;
						T y;
						T z;
					};
					T memo[count];
				};
			};
			template<typename T> struct cilinder {
				enum { count = 3 };
				typedef position_span_t<T, cilinder> span;
				union {
					struct {
						T radius;
						T height;
						T yaw;
					};
					T memo[count];
				};
			};

			template<typename T> struct sphere {
				enum { count = 3 };
				typedef position_span_t<T, sphere> span;
				union {
					struct {
						T radius;
						T yaw;
						T pitch;
					};
					T memo[count];
				};
			};

			template<typename T>struct euler {
				enum { count = 3 };
				typedef orient_span_t<T, euler> span;
				union {
					struct {
						T precession;
						T nutation;
						T rotation;
					};
					T memo[count];
				};
			};
			template<typename T>struct avionic {
				enum { count = 3 };
				/*struct span : public ::robo::span<T, count> {
					template<class F> constexpr  span(const F& _src, size_t _offset) :
						::robo::span<T, count>(&_src[_offset], count) {}
				};
				*/
				typedef orient_span_t<T, avionic> span;

				union {
					struct {
						T yaw;
						T pith;
						T roll;
					};
					T memo[count];
				};
				T yaw_offset = T(0);

				constexpr avionic(void) : yaw(T(0)), pith(T(0)), roll(T(0)) {
				}
				constexpr avionic(const span & _src) {
					::std::copy_n(_src.begin(), count, memo);
				}

				constexpr avionic( T _yaw, T _pith, T _roll) : yaw(_yaw), pith(_pith), roll(_roll) {
					rangle_sut(_yaw);
					rangle_sut(_roll);					
				}

				T norm(void) {
					return sqrt( yaw * yaw + pith * pith + roll * roll );
				}

				constexpr avionic& operator -= (const avionic& _src) {
					rangle_sub( yaw , _src.yaw );
					rangle_sub( pith , _src.pith);
					rangle_sub( roll , _src.roll);					
					return *this;
				}
				constexpr avionic& operator += (const avionic& _src) {
					rangle_add(yaw, _src.yaw);
					rangle_add(pith, _src.pith);
					rangle_add(roll, _src.roll);
					return *this;
				}

			};

			/*avionic operator + (const avionic& _src1, const avionic& _src2) {
				avionic tmp(_src1);
				return tmp += _src2;
			}
			avionic operator - (const avionic& _src1, const avionic& _src2) {
				avionic tmp(_src1);
				return tmp -= _src2;
			}*/
		};
		
		template<typename T> struct quaternion {
			enum { count = 4 };
			typedef  ::robo::span<T, count> span  ;
			union {
				struct {
					T w;
					T x;
					T y;
					T z;
				};
				T memo[count];
			};
			
			using avionic = typename  axis::avionic<T>;
			constexpr quaternion(void) {
				::std::fill_n(memo,count,0.f);
			}
			//constexpr quaternion(const span& _src) {
				//::std::copy_n(_src.begin(), count, memo);
			//}
			constexpr quaternion (const quaternion& _src) {
				::std::copy_n(_src.memo, count, memo);
			}
			quaternion(const avionic& _src) {
				from(_src);
			}
			constexpr quaternion(const quaternion& _src, bool _inv) {
				if (_inv) {
					w=  _src.w;
					x= -_src.x;
					y = -_src.y;
					z = -_src.z;
				}
				else {
					::std::copy_n(_src.memo, count, memo);
				}
			}
			constexpr quaternion(
				T _x,
				T _y,
				T _z
			) {
				w = T(0);
				x = _x;
				y = _y;
				z = _z;
			}

			constexpr quaternion(
				T _w,
				T _x,
				T _y,
				T _z
			) {
				w = _w;
				x = _x;
				y = _y;
				z = _z;
			}
			constexpr quaternion& operator = (const quaternion& _src) {
				::std::copy_n(_src.memo, count, memo);
				return *this;
			}


			quaternion& operator = (const avionic& _src) {
				//todo говнокод
				*this << axis::orient_span_t<T, avionic>(_src);
				return *this;
			}
			constexpr quaternion& operator *= (const quaternion& _src) {
				T _w = w;
				T _x = x;
				T _y = y;
				T _z = z;
				w = _w * _src.w - _x * _src.x - _y * _src.y - _z * _src.z;
				x = _x * _src.w + _w * _src.x + _y * _src.z - _z * _src.y;   // x component
				y = _w * _src.y - _x * _src.z + _y * _src.w + _z * _src.x;   // y component
				z = _w * _src.z + _x * _src.y - _y * _src.x + _z * _src.w;   // z component
				return *this;
			}
			constexpr quaternion inv (void) {
				quaternion tmp(*this,true);
				return tmp;
			}
			constexpr quaternion& operator += (const quaternion& _src) {				
				x += _src.x;
				y += _src.y;
				z += _src.z;
				return *this;
			}
			constexpr quaternion& operator -= (const quaternion& _src) {
				x -= _src.x;
				y -= _src.y;
				z -= _src.z;
				return *this;
			}
			static void to(avionic& _avc,T w, T x, T y, T z) {
				/*T w = _src[0];
				T x = _src[1];
				T y = _src[2];
				T z = _src[3];*/
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
			}
			void to(avionic& _avc) {
				to(_avc, w, x, y, z);
			}
			double norm(void) {
				return sqrt( w*w + x*x + y*y +z*z );			
			}
			T compare(const quaternion<T>& _src) {
				quaternion<T> tmp(_src,true);
				tmp *= *this;
				/*/static quaternion<T> zero(0., 1., 0., 0.);
				const T* p1 = tmp.memo;
				const T* p2 = zero.memo;
				T err = T(0);
				for (int i = 0; i < count; ++i, ++p1,++p2) {
					T tmp = *p1-*p2;
					tmp = tmp * tmp;
					err += tmp;
				}
				return sqrt(err / (int)count);*/
//				if (abs(tmp.w - 1.0) < 0.000001 /*epsilon<T, 10>*/) {
					const T* p1 = tmp.memo+1;
					T err = T(0);
					for (int i = 0; i < count-1; ++i, ++p1) {
						T tmp = *p1;
						tmp = tmp * tmp;
						err += tmp;
					}
					return sqrt(err/3)* tmp.w;
	//			}
		//		return tmp.w;
			}
			T diff(const quaternion<T>& _src) {
				const T* p1 = _src.memo;
				const T* p2 = memo;
				T err = T(0);
				for (int i = 0; i < count; ++i, ++p1, ++p2) {
					T tmp = *p1 - *p2;
					tmp = tmp * tmp;
					err += tmp;
				}
				return sqrt(err / (int)count);
			}
		};

		

		template<typename T> constexpr quaternion<T> operator * (const quaternion<T>& _src1, const quaternion<T>& _src2) {
			quaternion tmp(_src1);
			return tmp *= _src2;
		}
		template<typename T> constexpr quaternion<T> operator + (const quaternion<T>& _src1, const quaternion<T>& _src2) {
			quaternion<T>tmp(_src1);
			return tmp += _src2;
		}
		template<typename T>constexpr quaternion<T> operator - (const quaternion<T>& _src1, const quaternion<T>& _src2) {
			quaternion<T> tmp(_src1);
			return tmp -= _src2;
		}


		enum class op { set, inc };
		
		template< typename T, class P, class  O > struct point_t {
			typedef T type;
			typedef P position_t;
			typedef O orient_t;
			enum { count = P::count + O::count};
			union {
				struct {
					position_t position;
					orient_t orient;
				};
				T memo[count];
			};
			struct span : public ::robo::span<type, count> {
				typedef P position_t;
				typedef O orient_t;
				enum { orientofs = P::count, posofs = 0 };
				//using ::robo::span<type, count>::span;
				span(const type(&_arr)[count]) : ::robo::span<type, count>((type*)(&_arr[0])) {}
				span(const ::robo::span<type, count> & _src ) : ::robo::span<type, count>(_src) {}
				span(const span& _src) : ::robo::span<type, count>(_src.data()) {}
				typename orient_t::span orient() const {
					return
						typename orient_t::span( ::robo::span<type, count>::data() + orientofs);
				}
				typename position_t::span position() const {
					return
						typename position_t::span(::robo::span<type, count>::data() + posofs);
				}
			};
			
			point_t(void) {
				::std::fill_n(memo, count, 0.f);
			}
			point_t ( const span& _src ){
				::std::copy_n(_src.data(), count, memo);
			}
			point_t(const point_t & _src) {
				::std::copy_n(_src.memo, count, memo);
			}
			point_t& operator = ( const span& _src ) {
				::std::copy_n(_src.data(), count, memo);
				return *this;
			}
			point_t& operator = (const point_t& _src) {
				::std::copy_n(_src.memo, count, memo);
				return *this;
			}

			struct from {
				typedef point_t owner; 
				typedef point_t::span span;
				enum {
					count = point_t::count
				};
				template<class Q>  struct to {
					static void doit(const span& _span, Q& _dst, op _op);
					static void set(const span& _span, Q& _dst) { doit(_span, _dst, op::set);  };
					static void inc(const span& _span, Q& _dst) { doit(_span, _dst, op::inc); };
				};
			};

			template<typename Q> point_t(const Q &  _from, const typename Q::span& _span) {				
				Q:: template to<point_t>::set(_span, *this);
			}
			template<class Q> point_t& operator = (const Q& _src) {
				typedef typename Q::from from;
				from:: template to<point_t>::set(typename from::span(_src.memo), *this);
				return *this;
			}
			template<class Q> point_t(const Q& _src) {
				typedef typename Q::from from;
				from:: template to<point_t>::set(  typename from::span(_src.memo) , *this);
			}
			template<class Q> void applay(const Q& _src, const typename Q::span& _span) {				
				Q:: template to<point_t>::set(_span, *this);
			}
			template<class Q> void applay(const Q& _src) {
				typedef typename Q::from from;
				from:: template to<point_t>::set(typename from::span(_src.memo), *this);
			}
			void applay(const span & _src) {
				*this = _src;
			}
			template<class Q> void inc(const Q& _src, const typename Q::span& _span) {
				Q:: template to<point_t>::inc(_span, *this);
			}
			void inc(const span& _src) {
				const type* s = _src.data(), * d = memo;
				for (int i = 0; i < count; ++s, ++d) *d += *s;
			}
		};

		template<typename T> quaternion<T>& operator  << (quaternion<T>& _dst, const typename axis::avionic<T>::span& _src) {
			T Yd2 = _src[0] / 2.;
			T Pd2 = -_src[1] / 2.;
			T Rd2 = _src[2] / 2.;

			T  csR = ::std::cos(Rd2);
			T  snR = ::std::sin(Rd2);
			T  csP = ::std::cos(Pd2);
			T  snP = ::std::sin(Pd2);
			T  csY = ::std::cos(Yd2);
			T  snY = ::std::sin(Yd2);
			_dst.w = csR * csP * csY + snR * snP * snY;
			_dst.x = snR * csP * csY - csR * snP * snY;
			_dst.y = csR * snP * csY + snR * csP * snY;
			_dst.z = csR * csP * snY - snR * snP * csY;
			return _dst;
		}
		template<typename T> quaternion<T>& operator  << (quaternion<T>& _dst, const typename axis::euclid<T>::span& _src) {
			_dst.w = 0;
			_dst.x = _src[0];
			_dst.y = _src[1];
			_dst.z = _src[2];
			return _dst;
		}
		
		template<typename T> typename axis::euclid<T>& operator  << (typename axis::euclid<T>& _dst, const  typename quaternion<T>::span& _src) {
			_dst.x = _src[1];
			_dst.y = _src[2];
			_dst.z = _src[3];
			return _dst;
		}

		template<typename T> typename axis::avionic<T>& operator  << (typename axis::avionic<T>& _avc, const  typename quaternion<T>::span& _src) {
			T w = _src[0];
			T x = _src[1];
			T y = _src[2];
			T z = _src[3];
			quaternion<T>::to(_avc, w, x, y, z);
			return _avc;
		}


		template<typename T>  struct point {
			typedef point_t< T, typename axis::euclid<T>, typename axis::avionic<T> > absolute;
			typedef point_t< T, typename axis::cilinder<T>, typename axis::avionic<T> > cilinder;
			typedef point_t< T, typename axis::sphere<T>, typename axis::avionic<T> > sphere;
			typedef point_t< T, quaternion<T>, quaternion<T> > hamilton;

			template <class P> static void convert(const P & _src, hamilton& _dst, op _op) {
				if (_op == op::set) {
					_dst.position <<  typename P::position_t::span(_src);
					_dst.orient << typename  P::orient_t::span(_src);
				}
			}
			template <class P> static void convert(const typename hamilton::span& _src, P & _dst, op _op) {
				if (_op == op::set) {
					_dst.position << _src.position();
					_dst.orient << _src.orient();
				}
			}
		};
		/*template <class P> hamilton& operator  <<(hamilton& _dstconst, P& _src) {
			_dst.position << P::position_t::span(_src);
			_dst.orient << P::orient_t::span(_src);
		}
		template <class P>  P& operator << (P& _dst, const typename hamilton::span& _src, ) {
			_dst.position << _src.position();
			_dst.orient << _src.orient();
		}*/

		template<typename T, class P, class O> template <class Q> void point_t<T,P,O>::from::to<Q>::doit(const span& _span, Q& _dst, op _op) {			
			point<T>::convert(_span, _dst, _op);
		}
		
		template< class P, class A > struct target_t : public P, public A {
			typedef A point;
			typedef P payload;
			using T = typename A::type;
			//operator point& () { return (const point&)*this; };
			
			//по умолчанию
			target_t() : P(), A() {}

			target_t(const target_t& _src)
				: P((const P&)_src), A((const A&)_src) {}

			target_t& operator = (const target_t& _src) {
				*(P*)this = (const P&)_src;
				*(A*)this = (const A&)_src;
				return *this;
			}

			// тогда, когда нагрузка не имеет векторных данных
			//конструктор через конвертацию
			template <class B, typename ... Args > target_t(
				const B& from
				, const ::robo::span<T, B::count>  _span
				, Args ...args
			)
				: P(args...)
				, A(from, B::span(_span.data(), B::count)) {}

			//конструктор без конвертации
			template <typename ... Args > target_t(
				const ::robo::span<T, A::count>  _span
				, Args ...args
			)
				: P( args...)
				, A(A::span(_span.data(), A::count)) {}

			//установить новые данные без коныертации
			template <typename ... Args > void applay(
				const ::robo::span<T,A::count>  _span
				, Args ...args
			) {
				P::applay(args...);
				A::applay(A::span(_span.data(), A::count));
			}

			//установить новые данные с конвертацией
			template <class B, typename ... Args > void applay(
				const B& from
				, const ::robo::span<T, B::count>  _span
				, Args ...args
			) {
				P::applay(args...);
				A::applay(from, _span);
			}

			//установить новые данные с конвертацией из чуждого буфера
			template <typename T2, class B, typename ... Args > void applay(
				const B& from
				, const ::robo::span<T2, B::count>  _span
				, Args ...args
			) {
				P::applay(args...);
				T buf[B::count];
				::std::copy_n(_span.data(), B::count, buf);
				A::applay(from, B::span(buf, B::count));
			}

			//установить новые данные без конвертации из чуждого буфера
			template <typename T2, typename ... Args > void applay(
				const ::robo::span<T, A::count>  _span
				, Args ...args
			) {
				P::applay(args...);
				T buf[A::count];
				::std::copy_n(_span.data(), A::count, buf);
				A::applay(A::span(buf, A::count));
			}

			//Инкрментировать данные с конвертацией
			template <class B, typename ... Args > void inc(
				const B& from
				, const ::robo::span<T, B::count>  _span
				, Args ...args
			) {
				P::applay(args...);
				A::inc(from, B::span(_span.data(), B::count));
			}

			//Инкрментировать данные без конвертации
			template <typename ... Args > void inc(
				const ::robo::span<T, A::count>  _span
				, Args ...args
			) {
				P::applay(args...);
				A::inc(A::span(_span.data(), A::count));
			}

			//Инкрментировать новые данные с конвертацией из чуждого буфера
			template <typename T2, class B, typename ... Args > void inc(
				const B& from
				, const ::robo::span<T2, B::count>  _span
				, Args ...args
			) {
				P::applay(args...);
				T buf[B::count];
				::std::copy_n(_span.data(), B::count, buf);
				A::inc(from, B::span(buf, B::count));
			}

			//Инкрментировать новые данные без конвертации из чуждого буфера
			template <typename T2, typename ... Args > void inc(
				const ::robo::span<T, A::count>  _span
				, Args ...args
			) {
				P::applay(args...);
				T buf[A::count];
				::std::copy_n(_span.data(), A::count, buf);
				A::inc(A::span(buf, A::count));
			}

		};

		template <typename T> T signt(T val) {
			if ( abs(val) < ::std::numeric_limits<T>::epsilon()  ) return T(0);
			return (T(0) < val) ? -T(1) : T(1);
		}

		template <typename T> T abst(T val) {
			if (T(0) == val) return 0;
			if (val < T(0)) return -val;
			return val;
		}
		
		template<typename T, size_t M, size_t N> struct matrix  {
			struct size {
				enum { row = M, col = N, total=N*M};
			} ;
			T memo[size::total];

			constexpr matrix() {
				::std::fill_n(memo, size::total, T(0));
			}
			constexpr matrix(const T(&_array)[size::total]) {
				::std::copy_n(_array, size::total, memo);
			}
			constexpr matrix & operator = (const T(&_array)[size::total]) {
				::std::copy_n(_array, size::total, memo);
				return *this;
			}
			constexpr matrix& operator += (const T(&_array)[size::total]) {
				const T* src = _array;
				T* dst = memo;
				for (int i = 0; i < size::total; ++i, ++src, ++dst) {
					*dst += *src;
				}
				return *this;
			}
			constexpr matrix& operator -= (const T(&_array)[size::total]) {
				const T* src = _array;
				T* dst = memo;
				for (int i = 0; i < size::total; ++i, ++src, ++dst) {
					*dst -= *src;
				}
				return *this;
			}
		

			constexpr matrix& operator += (const matrix & _mx) {
				const T* src = _mx.memo;
				T* dst = memo;
				for (int i = 0; i < size::total; ++i, ++src, ++dst) {
					*dst += *src;
				}
				return *this;
			}

			constexpr matrix& operator /= (const T& _r) {
				T* dst = memo;
				for (int i = 0; i < size::total; ++i, ++dst) {
					*dst /= _r;
				}
				return *this;
			}
			constexpr matrix& operator *= (const T& _r) {
				T* dst = memo;
				for (int i = 0; i < size::total; ++i, ++dst) {
					*dst *= _r;
				}
				return *this;
			}

			constexpr matrix& operator -= (const matrix & _mx) {
				const T* src = _mx.memo;
				T* dst = memo;
				for (int i = 0; i < size::total; ++i, ++src, ++dst) {
					*dst -= *src;
				}
				return *this;
			}


			constexpr matrix& operator = (const matrix & _mx) {
				::std::copy_n(_mx.memo, size::total, memo);
				return *this;
			}

			/*constexpr matrix(const ::std::initializer_list<T>& _l) {
				::std::copy_n(_array, size::total, memo);
			}
			constexpr matrix& operator = (const ::std::initializer_list<T>& _l) {
				::std::copy_n(_l, size::total, memo);
				return *this;
			}*/
		};
		
		template<typename T, size_t M, size_t N> class eye: public matrix<T,M,N> {
		private:
			typedef matrix<T, M, N> A;
		public:
			constexpr eye(void): A() {
				for (int i = 0; i < A::size::row; ++i)
					for (int j = 0; j < A::size::col; ++j)
						if (i == j) {
							A::memo[i * A::size::col + j] = T(1);
						}
						else {
							A::memo[i * A::size::col + j] = T(0);
						}				
			}
		};

		template<typename T, size_t M, size_t K, size_t N> matrix< T, M, N>  operator * (const matrix<T, M, K>& _m1, const matrix<T, K, N>& _m2) {
			matrix< T, M, N> tmp;
			const T* A =&_m1.memo[0];
			const T* B =& _m2.memo[0];
			T* C = &tmp.memo[0];
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
			return tmp;
		}

		template<typename T, size_t M, size_t N> matrix< T, M, N>  operator + (const matrix<T, M, N>& _m1, const matrix<T, M, N>& _m2) {
			matrix< T, M, N> tmp (_m1);
			tmp += _m2;
			return tmp;
		}
		template<typename T, size_t M, size_t N> matrix< T, M, N>  operator - (const matrix<T, M, N>& _m1, const matrix<T, M, N>& _m2) {
			matrix< T, M, N> tmp (_m1);
			tmp -= _m2;
			return tmp;
		}
		template<typename T, size_t M, size_t N> matrix< T, M, N>  operator / (const matrix<T, M, N>& _m1, const T & _r) {
			matrix< T, M, N> tmp(_m1);
			tmp /= _r;
			return tmp;
		}
		template<typename T, size_t M, size_t N> matrix< T, M, N>  operator * (const matrix<T, M, N>& _m1, const T& _r) {
			matrix< T, M, N> tmp(_m1);
			tmp *= _r;
			return tmp;
		}

		template<typename T, size_t M> class vector : public matrix<T, M, 1> {
			using A =  matrix<T, M, 1>;
		public:
			constexpr vector(): A(){
			}
			constexpr vector(const T(&_array)[A::size::total]) : A(_array) {
			}
			constexpr vector(const A & _a) : A(_a) {}
			constexpr vector& operator = (const vector& _src) {
				return (vector&) A::operator = (_src);
			}
			constexpr vector& operator = (const A & _src) {
				return (vector&)A::operator = (_src);
			}
			constexpr T dot(const vector& _mx) const {
				T res = T(0);
				const T* src = _mx.memo;
				const T* dst = A::memo;
				for (int i = 0; i < A:: size::total; ++i, ++src, ++dst) {
					res += *dst * *src;
				}
				return res;
			}
			constexpr T length(void) const {
				return sqrt(dot(*this));
			}
			constexpr vector direction(void) const {
				T r = length();
				if (r > ::std::numeric_limits<T>::epsilon()) {
					return *this / r;
				}
				else {
					return vector({ 0,0,0 });
				}
			}

		};
		template<typename T, size_t M> vector<T, M>  operator + (const vector<T, M>& _m1, const vector<T, M>& _m2) {
			vector<T, M> tmp (_m1);
			_m1 += _m2;
			return tmp;
		}
		template<typename T, size_t M> vector<T, M>  operator - (const vector<T, M>& _m1, const vector<T, M>& _m2) {
			vector<T, M> tmp (_m1);
			_m1 -= _m2;
			return tmp;
		}

		template<typename T> class vector3 : public vector<T, 3> {
			using A = vector<T, 3>;
		public:
			T& x = A::memo[0];
			T& y = A::memo[1];
			T& z = A::memo[2];

			constexpr vector3& operator = (const quaternion<T>& _src) {
				::std::copy_n(_src.memo + 1, A::size::total, A::memo);
				return *this;
			}

			constexpr vector3(void) : A() {}
			constexpr vector3(const T(&_array)[A::size::total]) : A(_array) {}
			constexpr vector3(const quaternion<T>& _src) : A() {
				::std::copy_n(_src.memo + 1, A::size::total, A::memo);
			}

			constexpr vector3& operator = (const vector3& _src) {
				return (vector3&) A::operator = (_src);
			}
			constexpr vector3& operator = (const vector<T,3>& _src) {
				return (vector3&)A::operator = (_src);
			}
			constexpr vector3(T _x, T _y, T _z) : A() {
				x = _x;
				y = _y;
				z = _z;
			}

			constexpr  operator quaternion<T>() {
				quaternion<T> tmp;
				::std::copy_n(A::memo, A::size::total, tmp.memo + 1);
				return tmp;
			}

			constexpr vector3 back_transform(const quaternion<T>& _q, const quaternion<T>& _iq, const quaternion<T>& _offs) {
				quaternion<T> tmp = *this;
				return _offs + _q*tmp*_iq;
			}

			constexpr vector3 forward_transform(const quaternion<T>& _q, const quaternion<T>& _iq, const quaternion<T>& _offs) {
				quaternion<T> tmp = (*this-_offs);
				return _iq * tmp * _q;
			}

		};

		template<typename T> vector3<T> operator + (const vector3<T>& _m1, const vector3<T>& _m2) {
			vector3<T> tmp (_m1);
			tmp += _m2;
			return tmp;
		}

		template<typename T> vector3<T> operator - (const vector3<T>& _m1, const vector3<T>& _m2) {
			vector3<T> tmp(_m1);
			tmp -= _m2;
			return tmp;
		}

		template<typename T> vector3<T> operator ^ (const vector3<T>& a, const vector3<T>& b) {
			//i(ay bz - azby) - j(axbz - azbx) + k(axby - aybx)
			return  vector3({a.y*b.z -a.z*b.y, -a.x*b.z+a.z*b.x, a.x*b.y-a.y*b.x});
		}

		template<typename T> struct joint {
			using quaternion = ::robo::kinematiks::quaternion<T>;			
			class series;

			class actuator {
				T delta_ = 0.0;
				T actual_ =0.0;
				T min_ = 0.0;
				T max_ = 0.0;
				T offset_ = 0.0;
			protected:
				virtual void do_move(T _data) = 0;
			public:
				typedef ::robo::list::unique<actuator, int> list;
				typedef typename list::ref ref;
				enum class types {
					none
					, line
					, cicle_infinite
					, multirotated
					, cicle_signed
					, cicle_unsigned
				};
			private:
				ref ref_;
				types type_ = types::none;
				//void update_back_(actuator* _prev) {}

			public:

				friend class series;

				struct point: public ::robo::kinematiks::point<T>::hamilton {
					quaternion iorient;
				};

				point local;
				point supply;
				point base;
				
				T get(void) const {
					return actual_;
				}

				T getby(T _src) const {
					T dummy;
					getby(_src, dummy);
				}
				T getby(T _src, T & _delta) const {
					_src += offset_;
					_delta = _src - actual_;
					switch (type_) {
					case types::none:
					case types::line:
					return  _src;
					case types::cicle_infinite:
					case types::multirotated:
					{
						while (_delta > pi<T>) {
							_delta = _delta - T(2) *pi<T>;
						}
						while (_delta < - pi<T>) {
							_delta = _delta + T(2) * pi<T>;
						}
						T ret = actual_ + _delta;
						if (ret > max_) {
							ret -= (T(2) *pi<T>);
						}
						if (ret < min_) {
							ret += (T(2) * pi<T>);
						}
						return ret;
					}

					case types::cicle_signed:
					{	
						while (_src > pi<T>) {
							_src -= T(2) * pi<T>;
						}
						while (_src < -pi<T>) {
							_src += T(2) * pi<T>;
						}
						return _src;
					}
					case types::cicle_unsigned:
					{
						while (_src >= T(2) *pi<T>) {
							_src -= T(2) * pi<T>;
						}
						while (_src < T(0)) {
							_src += T(2) * pi<T>;
						}
						return _src;
					}
					}
					return T(0);

				}
				bool check(T _val, T & _delta) const {
					T tmp = getby(_val, _delta);
					switch (type_) {
					case types::none:
					case types::cicle_infinite:
					return true;
					default:
					return ((tmp >= min_ - 0.00000000000008) && (tmp <= max_+0.00000000000008));
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
						return T(1) - T(1) / (T(1) + exp( T(40) * s * (T(1) - s) - T(5)));
					}
					else {
						return T(0);
					}
					}
				}
				
				void begin( T _actual_dg, T _min_dg, T _max_dg, T _offset_dg) {
					min_ = _min_dg *deg2rad<T>;
					max_ = _max_dg * deg2rad<T>;
					offset_ = _offset_dg * deg2rad<T>;
					move(_actual_dg * deg2rad<T>);
				}
				void assign( const actuator & _src) {
					min_ = _src.min_;
					max_ = _src.max_;
					offset_ = _src.offset_;					
				}
				actuator(int _index, series& _series, types _type) : ref_(*this, _index) {
					ref_.attach_to(_series.actuators);
					local.orient.w = 1;
					supply.orient.w = 1;
					base.orient.w = 1;
					type_ = _type;
				}
				void update_forvard(actuator* _prev) {
					actuator* next = ref_.next_ptr();
					if (_prev != nullptr) {
						base.orient = _prev->base.orient;						
						base.orient *= local.orient;
						base.position = _prev->base.position + _prev->base.orient * local.position * _prev->base.iorient;
					}
					else {
						base.position = local.position;
						base.orient = local.orient;
					}
					base.orient *= supply.orient;
					base.iorient = base.orient.inv();
					if (next) {
						next->update_forvard(this);
					}
				}
				quaternion map_base(const quaternion& _q) {
					quaternion tmp = base.orient;
					tmp *= _q;
					tmp *= base.iorient;
					tmp += base.position;
					return tmp;
				}				
				quaternion map_target(const quaternion& _q) {
					quaternion tmp = base.iorient;
					tmp *= (_q- base.position);
					tmp *= base.orient;
					return tmp;
				}
				void move(T _position_dg) {
					actual_ = _position_dg  * grad2rad<T> - offset_;
					do_move(actual_);
				}
			};

			class yaw : public actuator {
			protected:
				virtual void do_move(T _data) {
					actuator::supply.orient.w = cos(_data / 2);
					actuator::supply.orient.z = sin(_data / 2);
				}
			public:
				yaw(int _index, series& _series) : actuator(_index, _series, actuator::types::cicle_signed) {}
			};

			class pitch : public actuator {
			protected:
				virtual void do_move(double _data) {
					actuator::supply.orient.w = cos(_data / 2);
					actuator::supply.orient.y = sin(_data / 2);
				}
			public:
				pitch(int _index, series& _series) : actuator(_index, _series, actuator::types::cicle_signed) {}
			};

			class roll : public actuator {
			protected:
				virtual void do_move(double _data) {
					actuator::supply.orient.w = cos(_data / 2);
					actuator::supply.orient.x = sin(_data / 2);
				}
			public:
				roll(int _index, series& _series) : actuator(_index, _series, actuator::types::cicle_signed) {}
			};

			class series {
				
				friend class actuator;
			public:
				typename actuator::list actuators;
				series() {}
				void update_forvard(void) {
					typename  actuator::ref* r = actuators.first();
					if (r) {
						r->owner().update_forvard(nullptr);
					}
				}



				/*template<typename C, unsigned N> void move(const C& _src) {
					ROBO_APP_ASSERT(N == actuators_.count);
					for (actuator::ref* r = actuators_.first(), const typename C::feetback fole * src = &_src.first(); r = r->next(), ++src) {
						r->owner().move(src->position);
					}
				}*/
				template<unsigned N> void move(const ::robo::span<double, N>& _src) {
					ROBO_APP_ASSERT(N == actuators.count);
					typename actuator::ref* r = actuators.first();
					const double* src = &_src.first();
					for (;r; r = r->next(), ++src) {
						r->owner().move(src);
					}
				}
				void assign(const series& _src_robot) {
					typename actuator::ref* d = actuators.first();
					typename actuator::ref* s = _src_robot.actuators.first();
					for (; d; d = d->next(), s = s->next()) {
						d->owner().assign( s->owner() );
					}
				}
			};
		};

		/*
		namespace actuator {
			class joint {
			protected:
				virtual void do_move(float _data) = 0;
				virtual void do_inc(float _data) = 0;
				virtual void do_calc_forward(void) = 0;
				virtual void do_calc_back(void) = 0;
			public:
				constexpr static inline const eye<float, 4, 4> I;
				constexpr static inline const matrix<float, 4, 1> Ro = matrix<float, 4, 1>({ 0,0,0,1 });
				constexpr static inline const matrix<float, 1, 4> Qo = matrix<float, 1, 4>({ 0,0,0,1 });
				typedef matrix<float, 4, 4> A;
				typedef matrix<float, 4, 1> R;
				using Q = quaternion;
				struct {
					Q q;
					R  r = Ro;
				} offset;

				quartenion deform;
				quartenion deform;

				//to do подумать
				//point::hamilton local;
				//quartenion local = Qo;
				
				A ilocal = I;
				A local=I;
				A back = I;
				A iback = I;
				A forward = I;
				A iforward = I;
				void move(float _data) { do_move(_data); }
				void inc(float _data) { do_inc(_data); };
			};
			class yaw :public joint
			{
			protected:
				virtual void do_move(float _data) {

				}
				virtual void do_inc(float _data) {}
				virtual void do_calc_forward(void) {}
				virtual void do_calc_back(void) {}
			};
		}*/
		/*
		template<typename T, size_t M, size_t N> constexpr static matrix<T,M,N>  sign(const matrix<T, M, N> & _m) {
			matrix< T, M, N> tmp;
			return tmp;
		}
		template<typename T, size_t M, size_t N> constexpr static matrix<T, M, N>  abs(const matrix<T, M, N>& _m) {
			matrix< T, M, N> tmp;
			return tmp;
		}


		arma::fvec& abs(const arma::fvec& _src, arma::fvec& _dst);

		arma::fvec& add(arma::fvec& _dst, const float* _src, const float _scale);

		arma::fvec& sum(arma::fvec& _dst, const arma::fvec& _p1, const float* _p2, float _scale);

		float scale_score(const arma::fvec& _src, const arma::fvec& _max);

		float normalize(arma::fvec& _dst, const arma::fvec& _src, const arma::fvec& _max);

		bool check_lim(const arma::fvec& _src, const arma::fvec& _lim);

		float angle_satt(float _angle);

		void angle_sat(arma::fvec& _angles);

		float angle_sattg(float _angle);

		void angle_satg(arma::fvec& _angles);

		float angle_deltat(float _angle1, float _angle2);

		void angle_delta(const arma::fvec& _p1, const  arma::fvec& _p2, arma::fvec& _dst);

		float angle_deltatg(float _angle1, float _angle2);

		void angle_deltag(const arma::fvec& _p1, const  arma::fvec& _p2, arma::fvec& _dst);
		*/
	};
}
#endif