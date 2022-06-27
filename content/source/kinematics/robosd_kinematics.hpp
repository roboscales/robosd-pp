#ifndef rovosd_kinematiks_hpp
#define rovosd_kinematiks_hpp
#include <span>
namespace robo{
	namespace kinematiks {
		template<typename T, size_t N> constexpr const std::span<T, N> span(const T(&_arr)[N]) {
			return  std::span<T, N>((T *) &(_arr[0]),N);
		}
		
		template<typename T, size_t N> struct rshp{
			const T(&arr_)[N];
			constexpr rshp(T(&_arr)[N]) : arr_(_arr) {}
			template<size_t O, size_t M> constexpr const std::span<T, M>  at () {
				return  std::span<T, M>((T*)&(arr_[O]), M);
			}			
		};
		
		namespace axis {
			struct far {
				enum { count = 3 };
				union {
					struct {
						double x;
						double y;
						double z;
					};
					float memo[count];
				};
			};
			struct euclid {
				enum { count = 3 };
				union {
					struct {
						float x;
						float y;
						float z;
					};
					float memo[count];
				};
			};
			struct cilinder {
				enum { count = 3 };
				union {
					struct {
						float radius;
						float height;
						float yaw;
					};
					float memo[count];
				};
			};

			struct sphere {
				enum { count = 3 };
				union {
					struct {
						float radius;
						float yaw;
						float pitch;
					};
					float memo[count];
				};
			};

			struct euler {
				enum { count = 3 };
				union {
					struct {
						float precession;
						float nutation;
						float rotation;
					};
					float memo[count];
				};
			};
			struct avionic {
				enum { count = 3 };
				union {
					struct {
						float yaw;
						float pith;
						float roll;
					};
					float memo[count];
				};
			};
			struct quaternion {
				enum { count = 4 };
				union {
					struct {
						float x;
						float y;
						float z;
						float angle;
					};
					float memo[count];
				};
			};

		}
		
		enum class op { set, inc };

		
		template<class P, class O > struct point_t {
			enum { count = P::count + O::count };
			union {
				struct {
					P position;
					O orient;
				};
				float memo[count];
			};
			struct span : public std::span<float, count> {
				using bz =  std::span<float, count>;
				span(const std::span<float, count>& _src) : bz(_src) {}
				span(float * _src,size_t _count) : bz(_src, _count) {}
				span& operator = (const span& _src) {
					*((bz*)this) = (const bz&)_src;
				}
				span(const float(&_arr)[count]): bz((float *)(&_arr[0]),count) {					
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
				from:: template to<point_t>::set((const from::arr&)_src.memo, *this);
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
				const float* s = _src.data(), * d = memo;
				for (int i = 0; i < count; ++s, ++d) *d += *s;
			}
		};
		namespace point {
			typedef point_t< axis::euclid, axis::avionic > absolute;
			typedef point_t< axis::cilinder, axis::avionic > cilinder;
			typedef point_t< axis::sphere, axis::avionic > sphere;
			typedef point_t< axis::euclid, axis::quaternion > hamilton;
			
			void convert(const absolute::span& _src, hamilton& _dst, op _op);
			void convert(const hamilton::span& _src, absolute& _dst, op _op);
			void convert(const cilinder::span& _src, hamilton& _dst, op _op);
			void convert(const hamilton::span& _src, cilinder& _dst, op _op);
			void convert(const sphere::span& _src, hamilton& _dst, op _op);
			void convert(const hamilton::span& _src, sphere& _dst, op _op);
		}
		template<class P, class O> template <class Q> void point_t<P,O>::from::to<Q>::doit(const span& _span, Q& _dst, op _op) {
			point::convert(_span,_dst, _op);
		}
		
		template< class P, class A > struct target_t : public P, public A {
			typedef A point;
			typedef P payload;
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
				, const std::span<float, B::count>  _span
				, Args ...args
			)
				: P(args...)
				, A(from, B::span(_span.data(), B::count)) {}

			//конструктор без конвертации
			template <typename ... Args > target_t(
				const std::span<float, A::count>  _span
				, Args ...args
			)
				: P( args...)
				, A(A::span(_span.data(), A::count)) {}

			//установить новые данные без коныертации
			template <typename ... Args > void applay(
				const std::span<float,A::count>  _span
				, Args ...args
			) {
				P::applay(args...);
				A::applay(A::span(_span.data(), A::count));
			}

			//установить новые данные с конвертацией
			template <class B, typename ... Args > void applay(
				const B& from
				, const std::span<float, B::count>  _span
				, Args ...args
			) {
				P::applay(args...);
				A::applay(from, B::span(_span.data(), B::count));
			}

			//Инкрментировать данные с конвертацией
			template <class B, typename ... Args > void inc(
				const B& from
				, const std::span<float, B::count>  _span
				, Args ...args
			) {
				P::applay(args...);
				A::inc(from, B::span(_span.data(), B::count));
			}

			//Инкрментировать данные без конвертации
			template <typename ... Args > void inc(
				const std::span<float, A::count>  _span
				, Args ...args
			) {
				P::applay(args...);
				A::inc(A::span(_span.data(), A::count));
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
			constexpr matrix(const float(&_array)[size::total]) {
				std::copy_n(_array, size::total, memo);
			}
			constexpr matrix & operator = (const float(&_array)[size::total]) {
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
				float* c = C + i * N;
				for (int j = 0; j < N; ++j)
					c[j] = 0;
				for (int k = 0; k < K; ++k) {
					const float* b = B + k * N;
					float a = A[i * K + k];
					for (int j = 0; j < N; ++j)
						c[j] += a * b[j];
				}
			}
			return tmp;
		}
		namespace actuator {
			class joint {				
			protected:
				virtual void do_move(float _data) = 0;
				virtual void do_inc(float _data) = 0;
				virtual void do_calc_forward(void) = 0;
				virtual void do_calc_back(void) = 0;
			public:
				constexpr static inline const eye<float, 4, 4> I;
				typedef matrix<float, 4, 4> A;
				point::hamilton local;
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
				virtual void do_move(float _data) {}
				virtual void do_inc(float _data) {}
				virtual void do_calc_forward(void) {}
				virtual void do_calc_back(void) {}
			};
		}
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
	}
}
#endif