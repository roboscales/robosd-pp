#ifndef rovosd_tractor_hpp
#define rovosd_tractor_hpp
#include <cmath>
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

		template<typename T> constexpr T deg2rad = pi<T> / 180.;
		
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
			

			constexpr numbers_t(void): S() {
				::std::fill_n(S::memo, S::size, T(0));
			}

			constexpr numbers_t(const numbers_t & _src) : S() {
				ROBO_APP_ASSERT(S::size == _src.size);
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
				ROBO_APP_ASSERT(S::size == _src.size);
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
			
			constexpr void zeros(void) {
				::std::fill_n(S::memo, S::size, T(0));
			}

			constexpr numbers_t norma(void) const {
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

			template<typename T, typename C> numbers_t<T, C> operator *= (const numbers_t<T, C>& _src1) {
				numbers_t<T, C> tmp = *this * _src1;
				*this = tmp;
				return *this;
			}
			template<typename T, typename C> numbers_t<T, C> operator /= (const numbers_t<T, C>& _src1) {
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
			return tmp *= _t;
		}
		template<typename T, typename C> constexpr numbers_t<T,C> operator* (const T& _t , const numbers_t<T,C>& _src1 ) {
			numbers_t tmp(_src1);
			return tmp *= _t;
		}
		template<typename T, typename C>constexpr numbers_t<T,C> operator / (const T& _t, const numbers_t<T,C>& _src1) {
			numbers_t tmp(_src1);
			return tmp *= _t;
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
			void mult(const vector3_s& b) {
				x = y * b.z - z * b.y;
				y = -x * b.z + z * b.x;
				z = x * b.y - y * b.x;
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
			constexpr eye(void) : A() {
				for (int i = 0; i < A::size::row; ++i)
					A::memo[i * A::ncols + i] = T(1);
			}
		};
		
		template<typename T> class quaternion_s {
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
			using rotmatrix_t = matrix_t<T, 3, 3>;
			rotmatrix_t A;
			rotmatrix_t IA;
			void rotate(void) {
				T x2 = x * x;
				T y2 = y * y;
				T z2 = z * z;
				T xw = x * w;
				T xy = x * y;
				T xz = x * z;

				T yw = y * w;
				T yz = y * z;

				T zw = z * w;

				T* p = A.memo;
				*p++ = 1 -2 * (y2 + z2);	*p++ = 2 * (xy - zw);		*p++ = 2 * (yw + xz);
				*p++ = 2 * (xy + zw);		*p++ = 1 - 2 * (x2 + z2);	*p++ = 2 * (yz - xw);
				*p++ = 2 * (xz - yw);	    *p++ = 2 * (xw + yz);		*p++ = 1 - 2 * (x2 + y2);
				p = IA.memo;
				*p++ = 1 - 2 * (y2 + z2);	*p++ = 2 * (xy + zw);		*p++ = 2 * ( xz - yw );
				*p++ = 2 * (xy - zw);		*p++ = 1 - 2 * (x2 + z2);	*p++ = 2 * (yz + xw);
				*p++ = 2 * (xz + yw);	    *p++ = 2 * (yz - xw);		*p++ = 1 - 2 * (x2 + y2);
			}
			void mult(const quaternion_s& a, const quaternion_s & b) {
				w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z;
				x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y;
				y = a.w * b.y + a.y * b.w - a.x * b.z + a.z * b.x;
				z = a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w;
			}
			void div(const quaternion_s& a, const quaternion_s& b) {
				w = a.w * b.w + a.x * b.x + a.y * b.y + a.z * b.z;
				x = -a.w * b.x + a.x * b.w - a.y * b.z + a.z * b.y;
				y = -a.w * b.y + a.y * b.w + a.x * b.z - a.z * b.x;
				z = -a.w * b.z - a.x * b.y + a.y * b.x + a.z * b.w;
			}
			bool load(cstr _path, cstr key) {
				T tmp[size];
				ROBO_LBREAKN(ini::load_arr(_path, key, tmp, size));
				T f = tmp[0] * grad2rad<T>;
				w = cos(f / 2);
				vector3_t<T> n{tmp[1],tmp[2] ,tmp[3] };
				n.normalize();
				n *= sin(f / 2);
				x = n.x;
				y = n.y;
				z = n.z;
				rotate();
				return true;
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

		template<typename T> vector3_t<T> operator * (const quaternion_t<T>& a, const vector3_t<T>& b) {
			return a.A*b;
		}
		template<typename T> vector3_t<T> operator / (const vector3_t<T>& b, const quaternion_t<T>& a) {
			return a.IA * b;
		}

		template<typename T> class axis_t {
		public:
			quaternion_t <T> L;
			vector3_t<T> r;
			constexpr axis_t() :L{ 1,0,0,0 }, r{0,0,0} {}
			constexpr axis_t(const axis_t& _src)
				: L(_src.L)
				, r(_src.r) {
				L.rotate();
			}
			constexpr axis_t(const quaternion_t <T>& _L)
				: L(_L){
				L.rotate();
			}
			constexpr axis_t(const vector3_t<T>& _r)
				: r(_r) {
				L.rotate();
			}
			constexpr axis_t(const quaternion_t <T>& _L, const vector3_t<T>& _r)
				: L(_L), r(_r) {
				L.rotate();
			}

			constexpr axis_t(const std::initializer_list<T> _src)
			{
				ROBO_APP_ASSERT(7 == _src.size());
				L = span<T,4>(_src.begin());
				r = span<T,3>(_src.begin()+4);
				L.rotate();
			}
			constexpr axis_t(const T(&_src)[7]){
				L = span<T,4>(_src);
				r = span<T,3>(_src+ 4);
				L.rotate();
			}
			template <typename A> constexpr axis_t(const A& _src) {
				ROBO_APP_ASSERT(7 == _src.size);
				L = span<T,4>(_src.memo);
				r = span<T,3>(_src.memo + 4);
				L.rotate();
			}

			constexpr axis_t& operator = (const axis_t& _src) {
				L = _src.L;
				r = _src.r;
				L.rotate();
				return *this;
			}

			template <typename A> constexpr axis_t& operator = (const A& _src) {
				ROBO_APP_ASSERT(7 == _src.size);
				L = span(_src.memo, 4);
				r = span(_src.memo + 4, 3);
				L.rotate();
				return *this;
			}
			void load(cstr _path) {
				L.load(_path);
				r.load(_path);
			}

		};

		template<typename T> axis_t<T> operator * (const axis_t<T>& a, const axis_t<T>& b) {
			axis_t<T> c;
			c.L = a.L * b.L;
			c.r = a.r + a.L * b.r;
			c.L.rotate();
			return c;
		}

		template<typename T> axis_t<T> operator / (const axis_t<T>& c, const axis_t<T>& b) {
			axis_t<T> a;
			a.L = c.L / b.L;
			a.L.rotate();
			a.r = c.r - a.L * b.r;
			return a;
		}

		template<typename T> axis_t<T> operator - (const axis_t<T>& a, const axis_t<T>& b) {
			axis_t<T> tmp;
			tmp.L = a.L / b.L;
			tmp.r = a.r - b.r / b.L;
			tmp.L.rotate();
			return tmp;
		}

		template<typename T> vector3_t<T> operator * (const axis_t<T>& a, const vector3_t<T>& b) {
			return ((a.L * b)+ a.r);
		}
		template<typename T> vector3_t<T> operator / (const vector3_t<T>& b, const axis_t<T>& a) {
			return  (b - a.r) / a.L;
		}
		template<typename T> struct scene_t {
			using axis = axis_t<T>;
			using vector3 = vector3_t<T>;
			class series;
			class body;
			class actuator;
			class joint;
			class point;
			class robot;

			class point : public tree::item {
				friend class joint;
				friend class body;
				friend class actuator;
			public:
				typedef ::robo::list::unsorted<point> list;
				typedef typename list::ref ref;
			private:
				point * remote_ = nullptr;
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
					if ( name != string(RT("ct")) ) {
						ROBO_LBREAKN(local.L.load(_path, RT("local.L")));
						ROBO_LBREAKN(local.r.load_raw(_path, RT("local.r")));
					}
					return true;
				}
			};

			class joint : public tree::item {
				friend class point;
				friend class body;
			public:
				typedef ::robo::list::unsorted<joint> list;
				typedef typename list::ref ref;
			private:
				point& from_;
				point& to_;
				ref ref_;
				void assign_(joint& _src) {
					deform = _src.deform;
					ddeform = _src.ddeform;
					guk_line = _src.guk_line;
					desep_line = _src.desep_line;
					guk_cicle = _src.guk_cicle;
					desep_cicle = _src.desep_cicle;
				}
			public:
				body& body_ref() { return from_. tree::item:: template owner<body>(); };
				axis deform;
				axis ddeform;
				vector3 guk_line;
				vector3 desep_line;
				vector3 guk_cicle;
				vector3 desep_cicle;
				joint(point& _from, point& _to);
				~joint(void);
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
				friend class actuator;
				friend class point;
				friend class joint;
				friend class link;
				friend class series;
			public:
				typedef ::robo::list::unsorted<body> list;
				typedef typename list::ref ref;
			private:
				point::list points_;
				joint::list joints_;
				ref ref_;
				void arrange_(void) {
					for (typename point::list::ref  * r = points_.first()->next(); r; r = r->next()) {
						point& pt = r->owner();
						if (!pt.manual_arrange) {
							pt.base = ct.base * pt.local;
						}
					}
				}
				void assign_(body & _src) {
					
					{
						typename point::list::ref* d = points_.first();
						typename point::list::ref* s = _src.points_.first();
						for (; d; d = d->next(), s = s->next()) {
							d->owner().assign_(s->owner());
						}
					}
					
					{
						typename joint::list::ref* d = joints_.first();
						typename joint::list::ref* s = _src.joints_.first();
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
				vector3 inertion = {T(1),T(1) ,T(1) };
				body(cstr _name, series& _series);
			};

			class actuator: public point {
				friend class body;
				friend class series;
			public:
				typedef ::robo::list::unsorted<actuator> list;
				typedef typename list::ref ref;
			private:
				ref ref_;
				axis position_;
				void arrange_(void) {
					point::base = point::body_ref().ct.base * point::local;
					axis S = point::base * position_;
					point * rm = point::remote_;
					S = S / rm->local;
					rm->body_ref().ct.base = S;
				}
				void assign_(const actuator& _src) {
					position_ = _src.position_;
				}
			public:
				void rotate_rd(T _angle) {
					position_.L.w = cos(_angle / 2);
					position_.L.z = sin(_angle / 2);
					position_.L.rotate();
				}
				void rotate_gr(T _angle) {
					_angle *= grad2rad<T>;
					position_.L.w = cos(_angle / 2);
					position_.L.z = sin(_angle / 2);
					position_.L.rotate();
				}
				actuator(cstr _name, body & _body);
			};
			
			class series : public tree::item {
				friend class robot;
				friend class body;
				friend class actuator;
			public:
				typedef ::robo::list::unsorted<series> list;
				robot& robot_ref() { return tree::item:: template branch<robot>(); };

			private:
				robot& robot_;
				actuator::list actuators_;
				body::list bodies_;
				list::ref ref_;
				void arrange_(void) {
					for (typename actuator::list::ref* r = actuators_.first(); r; r = r->next()) {
						r->owner().arrange_();
					}
					for (typename body::list::ref* r = bodies_.first(); r; r = r->next()) {
						r->owner().arrange_();
					}
				}
				void assign_( series & _src) {
					{
						typename actuator::list::ref* d = actuators_.first();
						typename actuator::list::ref* s = _src.actuators_.first();
						for (; d; d = d->next(), s = s->next()) {
							d->owner().assign_(s->owner());
						}
					}
					{
						typename body::list::ref* d = bodies_.first();
						typename body::list::ref* s = _src.bodies_.first();
						for (; d; d = d->next(), s = s->next()) {
							d->owner().assign_(s->owner());
						}
					}
				}
			public:
				series(cstr _name, robot& _owner);
			};

			
			class robot: public tree::item {
				friend class series;
			public:
			private:
				series::list series_;
			public:
				robot(cstr _name) : tree::item(_name,nullptr) {}
				void arrange(void) {
					for (typename series::list::ref* r = series_.first() ; r; r = r->next()) {
						r->owner().arrange_();
					}
				}
				robot& assign ( robot& _robot) {
					typename series::list::ref* d = series_.first();
					typename series::list::ref* s = _robot.series_.first();
					for (; d; d = d->next(),s = s->next() ) {
						d->owner().assign_(s->owner());
					}
					arrange();
					return *this;
				}
			};

			class link: public body{
			public:
				point * J;
				actuator A;
				joint* JA;
			public:
				link(cstr _name, link & _prev)
					: body(_name,_prev.series_ref())
					, A(RT("A"),*this)
				{
					J = new point(RT("J"), *this);
					JA = new joint(*J, _prev.A);
				}
				link(cstr _name, series & _series)
					:  body(_name, _series)
					, A(RT("A"), *this) {
					J = nullptr;
					JA = nullptr;
				}
				~link(void) {
					if (J) delete J;
					if (JA) delete JA;
				}
			};
			class payload : public body {
			public:
				point A;
				point B;
				joint AJ;
			public:
				payload(cstr _name, link& _prev)
					: body(_name, _prev.series_ref())
					, A(RT("A"), *this)
					, B(RT("B"), *this) 
					, AJ(A, _prev.A) {
				}
			};

		};

		template<typename T> scene_t<T>::point::point(cstr _name, scene_t<T>::body& _body)
			: tree::item(_name, &_body)
			, ref_(*this)
			{
			ref_.attach_to(body_ref().points_);
		}


		template<typename T> scene_t<T>::joint::joint(point& _from, point& _to)
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
		template<typename T> scene_t<T>::joint::~joint(void) {
			from_.remote_ = nullptr;
			to_.remote_ = nullptr;
		}

		
		template<typename T> scene_t<T>::body::body(cstr _name, series& _series)
			: tree::item(_name, &_series)
			, ref_(*this)
			, ct(RT("ct"), *this) {
			ref_.attach_to(series_ref().bodies_);
			ct.manual_arrange = true;;
		}

		template<typename T> scene_t<T>::actuator::actuator(cstr _name, body& _body)
			: point(_name, _body)
			, ref_(*this)
		{
			point::manual_arrange = true;
			ref_.attach_to(_body.series_ref().actuators_);
		};

		template<typename T> scene_t<T>::series::series(cstr _name, robot& _robot)
			: tree::item(_name, &_robot)
			, robot_(_robot)
			, ref_(*this)
		{
			ref_.attach_to(robot_ref().series_);
		}

		/*		template<typename T, size_t M> quaternion_t<T>  operator * (const quaternion_t<T>& A, const quaternion_t<T>& B) {
			quaternion_t<T> C =A;
			mult<T, M, 3, 1>(A.begin(), B.begin(), C.data());
			return C;
		}*/

/*

		namespace  axis {
			template<typename T, class S> struct orient_span_t : public robo::span<T, S::count> {
				constexpr orient_span_t(const S& _s) : robo::span<T, S::count>((T*)&_s.pmemo[0]) {}
				template< class F > constexpr  orient_span_t(const F & _src) :
					robo::span<T, S::count>( _src.data()+ F::orientofs) {
				}
			};
			template<typename T, class S> struct position_span_t : public ::robo::span<T, S::count> {
				constexpr position_span_t(const S& _s) : ::robo::span<T, S::count>((T*)&_s.pmemo[0]) {}
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
					T pmemo[count];
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
					T pmemo[count];
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
					T pmemo[count];
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
					T pmemo[count];
				};
			};
			template<typename T>struct avionic {
				enum { count = 3 };
				typedef orient_span_t<T, avionic> span;

				union {
					struct {
						T yaw;
						T pith;
						T roll;
					};
					T pmemo[count];
				};
				T yaw_offset = T(0);

				constexpr avionic(void) : yaw(T(0)), pith(T(0)), roll(T(0)) {
				}
				constexpr avionic(const span & _src) {
					::std::copy_n(_src.begin(), count, pmemo);
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
				T pmemo[count];
			};
			
			using avionic = typename  axis::avionic<T>;
			constexpr quaternion(void) {
				::std::fill_n(pmemo,count,0.f);
			}
			//constexpr quaternion(const span& _src) {
				//::std::copy_n(_src.begin(), count, pmemo);
			//}
			constexpr quaternion (const quaternion& _src) {
				::std::copy_n(_src.pmemo, count, pmemo);
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
					::std::copy_n(_src.pmemo, count, pmemo);
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
				::std::copy_n(_src.pmemo, count, pmemo);
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
//				if (abs(tmp.w - 1.0) < 0.000001 ) {
					const T* p1 = tmp.pmemo+1;
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
				const T* p1 = _src.pmemo;
				const T* p2 = pmemo;
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
			typedef P offset_t;
			typedef O orient_t;
			enum { count = P::count + O::count};
			union {
				struct {
					offset_t offset;
					orient_t orient;
				};
				T pmemo[count];
			};
			struct span : public ::robo::span<type, count> {
				typedef P offset_t;
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
				typename offset_t::span position() const {
					return
						typename offset_t::span(::robo::span<type, count>::data() + posofs);
				}
			};
			
			point_t(void) {
				::std::fill_n(pmemo, count, 0.f);
			}
			point_t ( const span& _src ){
				::std::copy_n(_src.data(), count, pmemo);
			}
			point_t(const point_t & _src) {
				::std::copy_n(_src.pmemo, count, pmemo);
			}
			point_t& operator = ( const span& _src ) {
				::std::copy_n(_src.data(), count, pmemo);
				return *this;
			}
			point_t& operator = (const point_t& _src) {
				::std::copy_n(_src.pmemo, count, pmemo);
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
				from:: template to<point_t>::set(typename from::span(_src.pmemo), *this);
				return *this;
			}
			template<class Q> point_t(const Q& _src) {
				typedef typename Q::from from;
				from:: template to<point_t>::set(  typename from::span(_src.pmemo) , *this);
			}
			template<class Q> void applay(const Q& _src, const typename Q::span& _span) {				
				Q:: template to<point_t>::set(_span, *this);
			}
			template<class Q> void applay(const Q& _src) {
				typedef typename Q::from from;
				from:: template to<point_t>::set(typename from::span(_src.pmemo), *this);
			}
			void applay(const span & _src) {
				*this = _src;
			}
			template<class Q> void inc(const Q& _src, const typename Q::span& _span) {
				Q:: template to<point_t>::inc(_span, *this);
			}
			void inc(const span& _src) {
				const type* s = _src.data(), * d = pmemo;
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
					_dst.offset <<  typename P::offset_t::span(_src);
					_dst.orient << typename  P::orient_t::span(_src);
				}
			}
			template <class P> static void convert(const typename hamilton::span& _src, P & _dst, op _op) {
				if (_op == op::set) {
					_dst.offset << _src.position();
					_dst.orient << _src.orient();
				}
			}
		};


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

				struct {
					point local;
					point supply;
					point base;
				} position;
				T mass = 1.0;
				struct{
					vector3<T> local;
					vector3<T> base;
				}ct;

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
					move_dg(_actual_dg);
				}
				void assign( const actuator & _src) {
					min_ = _src.min_;
					max_ = _src.max_;
					offset_ = _src.offset_;					
				}
				actuator(int _index, series& _series, types _type) : ref_(*this, _index) {
					ref_.attach_to(_series.actuators);
					position.local.orient.w = 1;
					position.supply.orient.w = 1;
					position.base.orient.w = 1;
					type_ = _type;
				}
				void update_forvard(actuator* _prev) {
					actuator* next = ref_.next_ptr();
					if (_prev != nullptr) {
						position.base.orient = _prev->position.base.orient;
						position.base.orient *= position.local.orient;
						position.base.offset = _prev->position.base.offset + _prev->position.base.orient * position.local.offset * _prev->position.base.iorient;
					}
					else {
						position.base.offset = position.local.offset;
						position.base.orient = position.local.orient;
					}
					position.base.orient *= position.supply.orient;
					position.base.iorient = position.base.orient.inv();
					if (next) {
						next->update_forvard(this);
					}
				}
				void update_ct(void) {
					actuator* next = ref_.next_ptr();					
					ct.base = map_base(ct.local);
					if (next) {
						next->update_ct();
					}
				}
				quaternion map_base(const quaternion& _q) {
					quaternion tmp = position.base.orient;
					tmp *= _q;
					tmp *= position.base.iorient;
					tmp += position.base.offset;
					return tmp;
				}				
				quaternion map_local(const quaternion& _q) {
					quaternion tmp = position.base.iorient;
					tmp *= (_q- position.base.offset);
					tmp *= position.base.orient;
					return tmp;
				}
				quaternion on_local(const quaternion& _q) {
					quaternion tmp = position.base.iorient;
					tmp *= _q;
					tmp *= position.base.orient;
					return tmp;
				}
				void move_dg(T _position_dg) {
					actual_ = _position_dg  * grad2rad<T> - offset_;
					do_move(actual_);
				}
				void move_rad(T _position_rad) {
					actual_ = _position_rad - offset_;
					do_move(actual_);
				}
				
				virtual T rot_projection(const vector3<T>& R) { 
					return const_cast<vector3<T>&>(R).z();  
				};
				virtual T ro2(const vector3<T>& R) {
					T x = const_cast<vector3<T>&>(R).x();
					T y = const_cast<vector3<T>&>(R).y();
					return x * x + y * y;
				};

				T inertion(const vector3<T>& R, const T& mass) {
					vector3<T> tmp = R - (vector3<T>)position.base.offset;
					quaternion tq0 = on_local(tmp);
					return ro2(tq0)* mass;
				}
				T torque(const vector3<T>& R, const vector3<T>&  F) {
					vector3<T> tmp = R - (vector3<T>)position.base.offset;
					vector3<T> tq = tmp ^ F;
					quaternion tq0 = on_local(tq);
					return rot_projection(tq0);
				}
				T mgtorque(const vector3<T>& R,const T & P) {
					vector3<T> F(0,0,P);
					return torque(R, F);
				}
			};

			class yaw : public actuator {
			protected:
				virtual void do_move(T _data) {
					actuator::position.supply.orient.w = cos(_data / 2);
					actuator::position.supply.orient.z = sin(_data / 2);
				}
			public:
				yaw(int _index, series& _series) : actuator(_index, _series, actuator::types::cicle_signed) {}

			};

			class pitch : public actuator {
			protected:
				virtual void do_move(double _data) {
					actuator::position.supply.orient.w = cos(_data / 2);
					actuator::position.supply.orient.y = sin(_data / 2);
				}
			public:
				virtual T rot_projection(const vector3<T>& R) {
					return const_cast<vector3<T>&>(R).y();
				};
				virtual T ro2(const vector3<T>& R) {
					T x = const_cast<vector3<T>&>(R).x();
					T z = const_cast<vector3<T>&>(R).z();
					return x * x + z * z;
				}
				pitch(int _index, series& _series) : actuator(_index, _series, actuator::types::cicle_signed) {}
			};

			class roll : public actuator {
			protected:
				virtual void do_move(double _data) {
					actuator::position.supply.orient.w = cos(_data / 2);
					actuator::position.supply.orient.x = sin(_data / 2);
				}
			public:
				virtual T rot_projection(const vector3<T>& R) {
					return const_cast<vector3<T>&>(R).x();
				};
				virtual T ro2(const vector3<T>& R) {
					T y = const_cast<vector3<T>&>(R).y();
					T z = const_cast<vector3<T>&>(R).z();
					return z * z + y * y;
				}
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
				void update_ct(void) {
					typename  actuator::ref* r = actuators.first();
					if (r) {
						r->owner().update_ct();
					}
				}

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
		*/
		

	};
}
#endif