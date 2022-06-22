#ifndef rovosd_kinematiks_hpp
#define rovosd_kinematiks_hpp
namespace robo{
	namespace kinematiks {
		struct far {
			enum { count = 3 };
			union {
				struct {
					double x;
					double y;
					double z;
				};
				float v[count];
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
				float v[count];
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
				float v[count];
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
				float v[count];
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
				float v[count];
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
				float v[count];
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
				float v[count];
			};
		};

		namespace point {

			struct absolut {
				enum { count = euclid::count + avionic::count };
				union {
					struct {
						euclid point;
						avionic orient;
					};
					float v[count] = {};
				};
			};
			struct cilindrical {
				enum { count = cilinder::count + avionic::count };
				union {
					struct {
						cilinder point;
						avionic orient;
					};
					float v[count] = {};
				};
			};
			struct spherical {
				enum { count = sphere::count + avionic::count };
				union {
					struct {
						sphere point;
						avionic orient;
					};
					float v[count] = {};
				};
			};
			struct slider {
				enum { count = euclid::count + avionic::count };
				union {
					struct {
						euclid point;
						avionic orient;
					};
					float v[count] = {};
				};
			};

			struct hamilton {
				enum { count = euclid::count + quaternion::count };
				union {
					euclid		pos;
					quaternion	orient;
				};
				float v[count] = {};
				hamilton(const absolut& _coord) {}
				hamilton(const cilindrical& _coord) {}
				hamilton(const spherical& _coord) {}
				hamilton(const hamilton& _base, const slider& _coord) {}
			};
		}


		template <typename S, typename C> struct point_t: public S, public C {
			point_t() : S(), C() {}
			template<  typename ... Args > point_t(float(&_v)[C::count], Args... args ): S(args...),C() {
				std::copy_n(_v, C::count, C::v);
			}
			template<  typename ... Args > point_t(std::initializer_list<float> _src, Args... args) : S(args...), C() {
				::robo::copy(_src, C::v);
			}
			template<  size_t N, typename ... Args > point_t(float(&_v)[C::count], float(&_a)[N], Args... args) : S(_a, args...), C() {
				std::copy_n(_v, C::count, C::v);
			}
			template<  size_t N, typename ... Args > point_t(std::initializer_list<float> _src, float(&_a)[N], Args... args) : S(_a, args...), C() {
				::robo::copy(_src, C::v);
			}
			template< typename ... Args > point_t(std::initializer_list<float> _src, std::initializer_list<float> _a, Args... args) : S(_a, args...), C() {
				::robo::copy(_src,C::v);
			}

			
			template<  typename ... Args > point_t(float(&_v)[C::count+S::count], Args... args) : S(_v+ C::count, args...), C() {
				std::copy_n(_v, C::count, C::v);
			}
		};

		template <typename S > struct hamilton_t : public S, public point::hamilton {
			template <typename T> hamilton_t()
				: S()
				, point::hamilton() {}

			template <typename T> hamilton_t(const point_t<S,T>& _coord)
				: S((const S&)_coord)
				, point::hamilton((const T&)_coord) {}

			hamilton_t(const point::hamilton& _base, const point_t<S, point::slider >& _coord)
				: S((const S&)_coord)
				, point::hamilton(_base, _coord) {}

		};
	}
}
#endif