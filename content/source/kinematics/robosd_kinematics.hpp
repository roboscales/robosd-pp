#ifndef rovosd_kinematiks_hpp
#define rovosd_kinematiks_hpp
#include <span>
#include <cmath>
#include "core/robosd_common.hpp"
#include "core/robosd_list.hpp"
namespace robo{
	namespace kinematiks {
		
		constexpr void rangle_sut(float & _values) {
			while (_values > pi<float>) {
				_values = -2 * pi<float>;
			}
			while (_values < -pi<float>) {
				_values += 2 * pi<float>;
			}			
		}

		constexpr void rangle_add(float & _values, float _add) {
			_values += _add;
			rangle_sut(_values);
		}
		constexpr void rangle_sub(float& _values, float _add) {
			_values -= _add;
			rangle_sut(_values);
		}

		template<int n> constexpr float epsilon = std::numeric_limits<float>::epsilon() * n; //n попугаев

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
		
		namespace axis {
			struct far {
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

				constexpr avionic(void) : yaw(0.f), pith(0.f), roll(0.f) {
				}

				constexpr avionic( float _yaw, float _pith, float _roll) : yaw(_yaw), pith(_pith), roll(_roll) {
					rangle_sut(_yaw);
					rangle_sut(_roll);
					sutyaw();
				}

				float norm(void) {
					return sqrt( yaw * yaw + pith * pith + roll * roll );
				}
				constexpr void sutyaw(void) {
					 /*if (yaw < -pi<float>*0.99995) {
						yaw = -pi<float>*0.99995;
					}
					if (roll < -pi<float>*0.99995) {
						roll = -pi<float>*0.99995;
					}
					if (yaw > pi<float>*0.99995) {
						yaw = pi<float>*0.99995;
					}
					if (roll > pi<float>*0.99995) {
						roll = pi<float>*0.99995;
					}*/
				}
				constexpr avionic& operator -= (const avionic& _src) {
					rangle_sub( yaw , _src.yaw );
					rangle_sub( pith , _src.pith);
					rangle_sub( roll , _src.roll);
					sutyaw();
					return *this;
				}
				constexpr avionic& operator += (const avionic& _src) {
					rangle_add(yaw, _src.yaw);
					rangle_add(pith, _src.pith);
					rangle_add(roll, _src.roll);
					sutyaw();
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
		}
		
		struct quaternion {
			enum { count = 4 };
			union {
				struct {
					double w;
					double x;
					double y;
					double z;
				};
				double memo[count];
			};
			constexpr quaternion(void) {
				std::fill_n(memo,count,0.f);
			}
			constexpr quaternion (const quaternion& _src) {
				std::copy_n(_src.memo, count, memo);
			}
			quaternion(const axis::avionic& _src) {
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
				double _x,
				double _y,
				double _z
			) {
				w = 0;
				x = _x;
				y = _y;
				z = _z;
			}

			constexpr quaternion& operator = (const quaternion& _src) {
				std::copy_n(_src.memo, count, memo);
				return *this;
			}
			void from(const axis::avionic& _src) {
				double Rd2 = _src.roll/2.;
				double Pd2 = -_src.pith / 2.;
				double Yd2 = _src.yaw / 2.;

				double  csR = std::cos(Rd2);
				double  snR = std::sin(Rd2);
				double  csP = std::cos(Pd2);
				double  snP = std::sin(Pd2);
				double  csY = std::cos(Yd2);
				double  snY = std::sin(Yd2);
				w = csR * csP * csY + snR * snP * snY;
				x = snR * csP * csY - csR * snP * snY;
				y = csR * snP * csY + snR * csP * snY;
				z = csR * csP * snY - snR * snP * csY;
			}
			quaternion& operator = (const axis::avionic& _src) {
				from(_src);
				return *this;
			}
			constexpr quaternion& operator *= (const quaternion& _src) {
				double _w = w;
				double _x = x;
				double _y = y;
				double _z = z;
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
			
			void to(axis::avionic & _avc) {
				double d1 = 2. * (x * z - w * y);
				if (d1 > 1.) {
					d1= 1. - std::numeric_limits<double>::epsilon();
				}
				if (d1< -1.) {
					d1= -1. + std::numeric_limits<double>::epsilon();
				}
				double tmp  = -asin(d1);
				_avc.pith = (float)-tmp;
				double d2= tmp - pi<double> / 2.f;
				if ( abs(d2) < pi<double>*0.00000003f) {
					double YR = atan2(-2 * (x * y - w * z), 1 - 2 * (x * x + z * z));
					//вариант первый
					double roll1,roll2;
					if (tmp < 0) {
						roll1 = YR - _avc.yaw;
					}
					else {
						roll1 = _avc.yaw - YR;
					}
					//вариант второй YAW+PITCH > pi
					if (YR > 0) {
						YR = YR - 2 * pi<double>;
					}
					else {
						YR = YR + 2. * pi<double>;
					}
					
					if (tmp < 0) {
						roll2 = YR - _avc.yaw;
					}
					else {
						roll2 = _avc.yaw - YR;
					}

					double df1 = roll1 - _avc.roll;
					
					double df2 = roll2 - _avc.roll;
					
					if (abs(df2 > df1)) {
						_avc.roll = (float)roll1;
					}
					else {
						_avc.roll = (float)roll2;
					}

				}
				else {
					_avc.yaw = (float)atan2( 2 * (x * y + w * z) , 1 - 2 * ( y*y + z*z ) );
					_avc.roll = (float)atan2( 2 * ( w * x + y * z), 1 - 2 * ( x*x + y*y ));
				}
			}
			double norm(void) {
				return sqrt( w*w + x*x + y*y +z*z );			
			}
		};

		constexpr quaternion operator * (const quaternion& _src1, const quaternion& _src2) {
			quaternion tmp(_src1);
			return tmp *= _src2;
		}
		constexpr quaternion operator + (const quaternion& _src1, const quaternion& _src2) {
			quaternion tmp(_src1);
			return tmp += _src2;
		}
		constexpr quaternion operator - (const quaternion& _src1, const quaternion& _src2) {
			quaternion tmp(_src1);
			return tmp -= _src2;
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
			typedef point_t< axis::euclid, quaternion > hamilton;
			
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
		namespace joint {
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
				struct point {
					quaternion offset;
					quaternion orient;
					quaternion iorient;
				};
				point local;
				point supply;
				point base;

				actuator(int _index, series& _series);
				void update_forvard(actuator* _prev) {
					actuator* next = ref_.next_ptr();
					if (_prev != nullptr) {
						base.orient = _prev->base.orient;
						base.offset = _prev->base.offset;
						base.offset = _prev->base.offset + _prev->base.orient * local.offset * _prev->base.iorient;
					}
					else {
						base.offset = local.offset;
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
					tmp += base.offset;
					return tmp;
				}				
				quaternion map_target(const quaternion& _q) {
					quaternion tmp = base.iorient;
					tmp *= (_q- base.offset);
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
					supply.orient.w = cos(_data / 2);
					supply.orient.z = sin(_data / 2);
				}
			public:
				yaw(int _index, series& _series) : actuator(_index, _series) {}
			};

			class pitch : public actuator {
			protected:
				virtual void do_move(double _data) {
					supply.orient.w = cos(_data / 2);
					supply.orient.y = sin(_data / 2);
				}
			public:
				pitch(int _index, series& _series) : actuator(_index, _series) {}
			};

			class roll : public actuator {
			protected:
				virtual void do_move(double _data) {
					supply.orient.w = cos(_data / 2);
					supply.orient.x = sin(_data / 2);
				}
			public:
				roll(int _index, series& _series) : actuator(_index, _series) {}
			};

			class series {
				actuator::list actuators_;
				friend class actuator;
			public:
				series() {}
				void update_forvard(void) {
					actuator::ref* r = actuators_.first();
					if (r) {
						r->owner().update_forvard(nullptr);
					}
				}
			};
		}

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
	}
}
#endif