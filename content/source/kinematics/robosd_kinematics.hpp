#ifndef rovosd_kinematiks_hpp
#define rovosd_kinematiks_hpp
#include <span>
namespace robo{
	namespace kinematiks {
		template<typename T, size_t N> const std::span<T, N> span(const T(&_arr)[N]) { 			
			return  std::span<T, N>((T *) &(_arr[0]),N);
		}
		/*template <class  T, size_t N> struct span : public std::span<T, N> {
			span() {};
			span(const span & _span) :std::span<T, N>(_span){};
			span& operator = (const span& _span) {
				*(:std::span<T, N> *)this = (:std::span<T, N> &)_span;
			}			
		}*/
		
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
					static void doit(const span& _span, Q& _dst);
				};
			};

			template<class Q> point_t(const typename Q &  _from, const typename Q::span& _span) {				
				Q:: template to<point_t>::doit(_span, *this);
			}
			template<class Q> point_t& operator = (const Q& _src) {
				typedef typename Q::from from;
				from:: template to<point_t>::doit((const from::arr&)_src.memo, *this);
				return *this;
			}
			template<class Q> point_t(const Q& _src) {
				typedef typename Q::from from;
				from:: template to<point_t>::doit (  typename from::span(_src.memo) , *this);
			}
			template<class Q> void applay(const Q& _src, const typename Q::span& _span) {				
				Q:: template to<point_t>::doit(_span, *this);
			}
			void applay(const span & _src) {
				*this = _src;
			}
		};
		namespace point {
			typedef point_t< axis::euclid, axis::avionic > absolute;
			typedef point_t< axis::cilinder, axis::avionic > cilinder;
			typedef point_t< axis::sphere, axis::avionic > sphere;
			typedef point_t< axis::euclid, axis::quaternion > hamilton;
			
			void convert(const absolute::span& _src, hamilton& _dst);
			void convert(const hamilton::span& _src, absolute& _dst);
			void convert(const cilinder::span& _src, hamilton& _dst);
			void convert(const hamilton::span& _src, cilinder& _dst);
			void convert(const sphere::span& _src, hamilton& _dst);
			void convert(const hamilton::span& _src, sphere& _dst);
		}
		template<class P, class O> template <class Q> void point_t<P,O>::from::to<Q>::doit(const span& _span, Q& _dst) {
			point::convert(_span,_dst);
		}


		template< class S, class A > struct target_t : public S, public A {
			typedef A point;
			typedef S specific;						
			operator point & () { return (const point&)*this; };
			//конструктор через конвертацию


			template <class B, size_t N, typename ... Args > target_t(
				const B& from
				, const std::span<float,N>  _span
				, Args ...args
			)
				: S(std::span<float, N - B::count >(_span.data()+B::count, N - B::count), args...)
				, A(from, B::span(_span.data(), B::count)) {}

			//конструктор без конвертации
			template <size_t N, typename ... Args > target_t(
				const std::span<float, N>  _span
				, Args ...args
			)
				: S(std::span<float, N - A::count >(_span.data() + A::count, N - A::count), args...)
				, A(A::span(_span.data(), A::count)) {}

			//по умолчанию
			target_t(): S(), A() {}

			target_t( const target_t & _src) 
				: S((const S &)_src), A((const A&)_src) {
			}

			target_t& operator = (const target_t& _src) {
				*(S*)this = (const S&)_src;
				*(A*)this = (const A&)_src;
			}

			//конструктор копии по умолчанию
			template<class Q> target_t(const Q& _src) 
				: S((const typename  Q::specific&)_src)
				, A((const typename Q::point&)_src) {}

			template<class Q> target_t& operator = (const Q& _src) {
				*(S*)this = (const typename  Q::specific&)_src;
				*(A*)this = (const typename Q::point&)_src;
			}


			template <size_t N, typename ... Args > void applay(
				const std::span<float, N>  _span
				, Args ...args
			) {
				S::applay( std::span<float, N - A::count >(_span.data() + A::count, N - A::count), args...);
				A::applay(A::span(_span.data(), A::count));
			}

			template <class B, size_t N, typename ... Args > void applay(
				const B& from
				, const std::span<float, N>  _span
				, Args ...args
			) {
				S::applay(std::span<float, N - B::count >(_span.data() + B::count, N - B::count), args...);
				A::applay(from, B::span(_span.data(), B::count));
			}

		};


	}
}
#endif