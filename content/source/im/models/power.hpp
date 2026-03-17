#ifndef __power_hpp
#define __power_hpp
#include "core/robosd_common.hpp"
namespace robo{
	namespace edev{
		namespace power {
			template<typename T> struct ROBO_EXPORT ph3_t {
				constexpr  static inline  const T sqrt3h = (T)(csqrt(3.0) );
				struct ROBO_EXPORT sdq {
					T d = (T)0.;
					T q = (T)0.;
				};
				struct ROBO_EXPORT sab {
					T alfa = (T)0.;
					T beta = (T)0.;
				};
				struct ROBO_EXPORT sabc {
					T A = (T)0.;
					T B = (T)0.;
					T C = (T)0.;
				};
				sdq dq;
				sab ab;
				sabc phase;
				void set_dq(T _d, T _q, T _tet) {
					T cs = math_t<T>::cos(_tet);
					T sn = math_t<T>::sin(_tet);
					dq.d = _d;
					dq.q = _q;
					ab.alfa = cs * _d - sn * _q;
					ab.beta = sn * _d + cs * _q;
					phase.A = ab.alfa;
					phase.B = (ab.beta * sqrt3h - ab.alfa) / 2.f;
					phase.C = -phase.A - phase.B;
				}
				void set_abc(T _a, T _b, T _c, T _tet) {
					T cs = math_t<T>::cos(_tet);
					T sn = math_t<T>::sin(_tet);
					phase.A = _a;
					phase.B = _b;
					phase.C = _c;
					ab.alfa = _a;
					ab.beta = (_a + 2 * _b) / sqrt3h;
					dq.d = cs * ab.alfa + sn * ab.beta;
					dq.q = -sn * ab.alfa + cs * ab.beta;
				}
			};
			using ph3 = ph3_t<float>;
		}
	}
}

#endif
