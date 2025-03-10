#ifndef tractor_shared_hpp
#define tractor_shared_hpp
namespace robo {
	namespace tractor {
		enum class eside { startup = 0, digitwin = 1, vrep = 2, shutdown = 3 };
		template<typename T>  struct shared_t {
			struct axis_s {
				union {
					struct {
						struct {
							T w;
							T x;
							T y;
							T z;
						} L;
						struct {
							T x;
							T y;
							T z;
						} r;
					};
					T memo[7];
				};
				enum { size = 7 };
			};

			struct vector3_s {
				union {
					struct {
						T x;
						T y;
						T z;
					};
					T memo[3];
				};
				enum { size = 3 };
			};
		};
	}
}
#endif // !tractor_common_hpp
