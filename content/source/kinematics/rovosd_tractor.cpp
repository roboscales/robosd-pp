#include "kinematics/robosd_tractor.hpp"

namespace robo {
	namespace tractor {



		class test {
			
		public:
			test(void) {

				using V = vector3_t<float>;
				using S = span<float,3>;
				using S1 = span<float, 4>;
				using M = matrix_t<float,3,1> ;
				S s1{ 1.,2.,3. };
				V v = s1;
				V v2 = v;
				v+= S{ 1.,2.,3. };
				v += v2;
				v = v * 0.1f;
				V v3 = v + (2.f * v2) -  (v + V{1,2,3});
				M m = v3;
				M m1 = m;
				M m2 = m + m1*0.5f;
				M m3 = s1;
				m3 = v2;
				m3 = m2;
				float rr[3] = { 1,2,3 };
				M R(rr);
				R += S(rr);
				R += S({10,20,30});
				//std::span
				
				matrix_t<float, 3, 2> M1 = {1,2,3,4,5,6};
				matrix_t<float, 2, 3> M2 = { 1,2,3,4,5,6 };
				matrix_t<float, 2, 3> M21 = M2;
				matrix_t<float, 2, 2> M3 (  M2 * M1 );
				matrix_t<float, 3, 3> M4 = M1 * M2;
				matrix_t<float, 3, 1> a{1,2,3};
				matrix_t<float, 3, 1> M5 = M4 * a;
				
				matrix_t<float, 2, 1>  v4 ( M2 , M{ 1,2,3 } );
				matrix_t<float, 3, 1> M6 = M4 * a;
				V  v5 = M4 * V{ -1,-2,-3 };
				matrix_t<float, 2, 2> M7(M2,M1);
				V V8(M4, M5);

				quaternion_t<double>  q1{1,0,0,0};
				quaternion_t<double>  q2{ cos(0.5*pi<double>/2),0,0,sin(0.5 * pi<double> / 2) };
				quaternion_t<double>  q3(q1, q2);
				quaternion_t<double>  q4(q3, quaternion_t<double>  { cos(-0.5 * pi<double> / 2),0,0,sin(-0.5 * pi<double> / 2) });

			}
		} test_;
	}
}