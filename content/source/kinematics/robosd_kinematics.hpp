#ifndef rovosd_kinematiks_hpp
#define rovosd_kinematiks_hpp
#include <span>
#include <cmath>
#include "core/robosd_common.hpp"
#include "core/robosd_list.hpp"
namespace robo{
	namespace kinematiks {
		
		template<typename T> constexpr void rangle_sut(T & _values) {
			while (_values > pi<T>) {
				_values = -2 * pi<T>;
			}
			while (_values < -pi<T>) {
				_values += 2 * pi<T>;
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

		template<typename T,int n> constexpr T epsilon = std::numeric_limits<T>::epsilon() * n; //n попугаев

		/*template<typename T, size_t N> constexpr const std::span<T, N> span(const T(&_arr)[N]) {
			return  std::span<T, N>((T *) &(_arr[0]),N);
		}*/
		
		/*template<typename T, size_t N> struct rshp {
			const T(&arr_)[N];
			constexpr rshp(T(&_arr)[N]) : arr_(_arr) {}
			template<size_t O, size_t M> constexpr const std::span<T, M>  at () {
				return  std::span<T, M>((T*)&(arr_[O]), M);
			}			
		};*/

		namespace  axis {
			template<typename T, class S> struct orient_span_t : public std::span<T, S::count> {
				constexpr orient_span_t(const S& _s) : std::span<T, S::count>((T*)&_s.memo[0], S::count) {}
				template< class F > constexpr  orient_span_t(const F & _src) :
					std::span<T, S::count>(_src.subspan < F::orientofs, S::count > ()) {
				}
			};
			template<typename T, class S> struct position_span_t : public std::span<T, S::count> {
				constexpr position_span_t(const S& _s) : std::span<T, S::count>((T*)&_s.memo[0], S::count) {}
				template< class F > constexpr  position_span_t(const F& _src) :
					std::span<T, S::count>(_src.subspan < F::posofs, S::count >()) {}
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
				/*struct span : public std::span<T, count> {
					template<class F> constexpr  span(const F& _src, size_t _offset) :
						std::span<T, count>(&_src[_offset], count) {}
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

				constexpr avionic(void) : yaw(T(0)), pith(T(0)), roll(T(0)) {
				}
				constexpr avionic(const span & _src) {
					std::copy_n(_src.begin(), count, memo);
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
			typedef  std::span<T, count> span  ;
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
				std::fill_n(memo,count,0.f);
			}
			//constexpr quaternion(const span& _src) {
				//std::copy_n(_src.begin(), count, memo);
			//}
			constexpr quaternion (const quaternion& _src) {
				std::copy_n(_src.memo, count, memo);
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
					std::copy_n(_src.memo, count, memo);
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
				std::copy_n(_src.memo, count, memo);
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
			constexpr quaternion operator ! () {
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
			
			void to(avionic& _avc) {
			}
			double norm(void) {
				return sqrt( w*w + x*x + y*y +z*z );			
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
			struct span : public std::span<type, count> {
				typedef P position_t;
				typedef O orient_t;
				enum { orientofs = P::count, posofs = 0 };
				using std::span<type, count>::span;
				span(const type(&_arr)[count]) : std::span<type, count>((type*)(&_arr[0]), count) {}
				orient_t::span orient() const {
					return
						std::span(*this).subspan <orientofs, O::count>();
				}
				position_t::span position() const {
					return 
						std::span(*this).subspan<posofs, P::count>();
				}
			};
			
			point_t(void) {
				std::fill_n(memo, count, 0.f);
			}
			point_t ( const span& _src ){
				std::copy_n(_src.data(), count, memo);
			}
			point_t(const point_t & _src) {
				std::copy_n(_src.memo, count, memo);
			}
			point_t& operator = ( const span& _src ) {
				std::copy_n(_src.data(), count, memo);
				return *this;
			}
			point_t& operator = (const point_t& _src) {
				std::copy_n(_src.memo, count, memo);
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

			template<class Q> point_t(const typename Q &  _from, const typename Q::span& _span) {				
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

			T  csR = std::cos(Rd2);
			T  snR = std::sin(Rd2);
			T  csP = std::cos(Pd2);
			T  snP = std::sin(Pd2);
			T  csY = std::cos(Yd2);
			T  snY = std::sin(Yd2);
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
			T d1 = 2. * (x * z - w * y);
			if (d1 > 1.) {
				d1 = 1. - std::numeric_limits<T>::epsilon();
			}
			if (d1 < -1.) {
				d1 = -1. + std::numeric_limits<T>::epsilon();
			}
			T tmp = -asin(d1);
			_avc.pith = -tmp;
			T d2 = tmp - pi<T> / 2.f;
			if (abs(d2) < pi<T>*0.00000003f) {
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
				_avc.yaw = (T)atan2(2 * (x * y + w * z), 1 - 2 * (y * y + z * z));
				_avc.roll = (T)atan2(2 * (w * x + y * z), 1 - 2 * (x * x + y * y));
			}
			return _avc;
		}


		template<typename T>  struct point {
			typedef point_t< T, typename axis::euclid<T>, typename axis::avionic<T> > absolute;
			typedef point_t< T, typename axis::cilinder<T>, typename axis::avionic<T> > cilinder;
			typedef point_t< T, typename axis::sphere<T>, typename axis::avionic<T> > sphere;
			typedef point_t< T, typename quaternion<T>, quaternion<T> > hamilton;

			template <class P> static void convert(const P & _src, hamilton& _dst, op _op) {
				if (_op == op::set) {
					_dst.position <<  P::position_t::span(_src);
					_dst.orient <<  P::orient_t::span(_src);
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
			}

			// тогда, когда нагрузка не имеет векторных жанных
			//конструктор через конвертацию
			template <class B, typename ... Args > target_t(
				const B& from
				, const std::span<T, B::count>  _span
				, Args ...args
			)
				: P(args...)
				, A(from, B::span(_span.data(), B::count)) {}

			//конструктор без конвертации
			template <typename ... Args > target_t(
				const std::span<T, A::count>  _span
				, Args ...args
			)
				: P( args...)
				, A(A::span(_span.data(), A::count)) {}

			//установить новые данные без коныертации
			template <typename ... Args > void applay(
				const std::span<T,A::count>  _span
				, Args ...args
			) {
				P::applay(args...);
				A::applay(A::span(_span.data(), A::count));
			}

			//установить новые данные с конвертацией
			template <class B, typename ... Args > void applay(
				const B& from
				, const std::span<T, B::count>  _span
				, Args ...args
			) {
				P::applay(args...);
				A::applay(from, B::span(_span.data(), B::count));
			}

			//установить новые данные с конвертацией из чуждого буфера
			template <typename T2, class B, typename ... Args > void applay(
				const B& from
				, const std::span<T2, B::count>  _span
				, Args ...args
			) {
				P::applay(args...);
				T buf[B::count];
				std::copy_n(_span.data(), B::count, buf);
				A::applay(from, B::span(buf, B::count));
			}

			//установить новые данные без конвертации из чуждого буфера
			template <typename T2, typename ... Args > void applay(
				const std::span<T, A::count>  _span
				, Args ...args
			) {
				P::applay(args...);
				T buf[A::count];
				std::copy_n(_span.data(), A::count, buf);
				A::applay(A::span(buf, A::count));
			}

			//Инкрментировать данные с конвертацией
			template <class B, typename ... Args > void inc(
				const B& from
				, const std::span<T, B::count>  _span
				, Args ...args
			) {
				P::applay(args...);
				A::inc(from, B::span(_span.data(), B::count));
			}

			//Инкрментировать данные без конвертации
			template <typename ... Args > void inc(
				const std::span<T, A::count>  _span
				, Args ...args
			) {
				P::applay(args...);
				A::inc(A::span(_span.data(), A::count));
			}

			//Инкрментировать новые данные с конвертацией из чуждого буфера
			template <typename T2, class B, typename ... Args > void inc(
				const B& from
				, const std::span<T2, B::count>  _span
				, Args ...args
			) {
				P::applay(args...);
				T buf[B::count];
				std::copy_n(_span.data(), B::count, buf);
				A::inc(from, B::span(buf, B::count));
			}

			//Инкрментировать новые данные без конвертации из чуждого буфера
			template <typename T2, typename ... Args > void inc(
				const std::span<T, A::count>  _span
				, Args ...args
			) {
				P::applay(args...);
				T buf[A::count];
				std::copy_n(_span.data(), A::count, buf);
				A::inc(A::span(buf, A::count));
			}

		};

		template <typename T> T signt(T val) {
			if ( abs(val) < std::numeric_limits<T>::epsilon()  ) return T(0);
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

			constexpr matrix(bool _init=false) {
				if (_init) {
					std::fill_n(memo, size::total, T(0));
				}
			}
			constexpr matrix(const T(&_array)[size::total]) {
				std::copy_n(_array, size::total, memo);
			}
			constexpr matrix & operator = (const T(&_array)[size::total]) {
				std::copy_n(_array, size::total, memo);
				return *this;
			}
			
			/*constexpr matrix(const std::initializer_list<T>& _l) {
				std::copy_n(_array, size::total, memo);
			}
			constexpr matrix& operator = (const std::initializer_list<T>& _l) {
				std::copy_n(_l, size::total, memo);
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
		
		template<typename T> struct joint {
			using quaternion = ::robo::kinematiks::quaternion<T>;			
			class series;

			class actuator {
			protected:
				virtual void do_move(double _data) = 0;
				//virtual void do_inc(float _data) = 0;
			public:
				typedef ::robo::list::unique<actuator, int> list;
				typedef list::ref ref;
			private:
				ref ref_;
				//void update_back_(actuator* _prev) {}

			public:

				friend class series;

				struct point: public ::robo::kinematiks::point<T>::hamilton {
					quaternion iorient;
				};

				point local;
				point supply;
				point base;

				actuator(int _index, series& _series) : ref_(*this, _index) {
					ref_.attach_to(_series.actuators);
					local.orient.w = 1;
					supply.orient.w = 1;
					base.orient.w = 1;
				}
				void update_forvard(actuator* _prev) {
					actuator* next = ref_.next_ptr();
					if (_prev != nullptr) {
						base.orient = _prev->base.orient;
						base.position = _prev->base.position;
						base.position = _prev->base.position + _prev->base.orient * local.position * _prev->base.iorient;
					}
					else {
						base.position = local.position;
					}
					base.orient *= local.orient;
					base.orient *= supply.orient;
					base.iorient = !base.orient;
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
				virtual void move(double _data) {
					do_move(_data);
				}
			};

			class yaw : public actuator {
			protected:
				virtual void do_move(double _data) {
					actuator::supply.orient.w = cos(_data / 2);
					actuator::supply.orient.z = sin(_data / 2);
				}
			public:
				yaw(int _index, series& _series) : actuator(_index, _series) {}
			};

			class pitch : public actuator {
			protected:
				virtual void do_move(double _data) {
					actuator::supply.orient.w = cos(_data / 2);
					actuator::supply.orient.y = sin(_data / 2);
				}
			public:
				pitch(int _index, series& _series) : actuator(_index, _series) {}
			};

			class roll : public actuator {
			protected:
				virtual void do_move(double _data) {
					actuator::supply.orient.w = cos(_data / 2);
					actuator::supply.orient.x = sin(_data / 2);
				}
			public:
				roll(int _index, series& _series) : actuator(_index, _series) {}
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
				template<unsigned N> void move(const std::span<double, N>& _src) {
					ROBO_APP_ASSERT(N == actuators.count);
					typename actuator::ref* r = actuators.first();
					const double* src = &_src.first();
					for (;r; r = r->next(), ++src) {
						r->owner().move(src);
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