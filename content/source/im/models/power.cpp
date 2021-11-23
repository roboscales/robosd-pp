#include "im/models/power.hpp"
#include <math.h>
namespace robo{
	namespace im{
		namespace models{
			namespace power{
				ph3::sdq::sdq():d(0),q(0){
				}
				ph3::sab::sab() : alfa(0), beta(0){
				}
				ph3::sabc::sabc() : A(0), B(0), C(0){
				}
				ph3::ph3(){}
				void ph3::set_dq(float _d, float _q, float _tet){
                    float cs = (float)cos(_tet);
                    float sn = (float)sin(_tet);
                    static const float sqrt3h = (float) sqrt(3.f) / 2.f;
					dq.d = _d;
					dq.q = _q;
					ab.alfa = cs* _d - sn*_q;
					ab.beta = sn* _d + cs*_q;
					phase.A = ab.alfa;
					phase.B = ab.beta * sqrt3h - ab.alfa / 2.f;
					phase.C = -phase.A - phase.B;
				}
				void ph3::set_abc(float _a, float _b, float _c, float _tet){
                    static const float sqrt3f = (float)sqrt(3.f);
                    float cs = (float)cos(_tet);
                    float sn = (float)sin(_tet);
					phase.A = _a;
					phase.B = _b;
					phase.C = _c;
					ab.alfa = _a;
					ab.beta = (_a + 2 * _b) / sqrt3f;
					dq.d = cs* ab.alfa + sn*ab.beta;
					dq.q = -sn* ab.alfa + cs*ab.beta;
				}
			}
		}
	}
}
