#ifndef __power_hpp
#define __power_hpp

namespace robo{
	namespace im{
		namespace models{
			namespace power{
				struct ph3{
					struct sdq{
						float d;
						float q;
						sdq();
					};
					struct sab{
						float alfa;
						float beta;
						sab();
					};
					struct sabc{
						float A;
						float B;
						float C;
						sabc();
					};
					sdq dq;
					sab ab;
					sabc phase;
					ph3();
					void set_dq(float _d, float _q, float _tet);
					void set_abc(float _a, float _b, float _c, float _tet);
				};
			}
		}
	}
}

#endif