#ifndef __im_edev_joint_pmsm_hpp
#define __im_edev_joint_pmsm_hpp
#include "im/edev/joint_link.hpp"

#include "im/models/power.hpp"

#ifdef ARMADILLO_ENABLED
#define ARMADILLO_ENABLED 0
#endif


#if ARMADILLO_ENABLED
#include <armadillo>
#endif

#if ARMADILLO_ENABLED == 0
#include "kinematics/robosd_tractor.hpp"
#endif
namespace robo{
	namespace edev{
		namespace joint{
			namespace pmsm{
				class ROBO_EXPORT  ideal2 : public agent::block {
				private:
#if ARMADILLO_ENABLED
					arma::fmat A;
					arma::fmat IA;
					arma::fmat EA1;
					arma::fmat EA2;
					arma::fmat X;
					arma::fmat Xmax;
					arma::fmat U;
#else
					using matrix3x3 = robo::tractor::matrix3x3_t<double>;
					using vector3_s = robo::tractor::vector3_t<double>;
					matrix3x3 A;
					matrix3x3 IA;
					matrix3x3 EA1;
					matrix3x3 EA2;
					vector3_s X;
					vector3_s Xmax;
					vector3_s U;
#endif
					float ws = 0;
					float ws2 = 0;
					float ro_ = 0;
					float IA0_0_0 = 0;

					float IA0_1_1 = 0;
					float IA0_1_2 = 0;

					float IA0_2_1 = 0;
					float IA0_2_2 = 0;

				protected:
					float Ls = 0.0001f;
					float Rs = 2.f;
					float Fm = 0.02f;
					float Ke = 0.022f;
					float J = 0.000001f;
					float Kv = 0.f;
					float p = 1;
					virtual bool do_load(cstr _specific_sect, cstr _common_sect);
					virtual void do_reconfig(void);
					virtual void do_run(void);
				public:

					struct{
						float speed = 0.f;
						float phase = 0.f;
						double position = 0.f;
					} electro;

					power::ph3 current;
					power::ph3 voltage;
					joint::iactuator actuator;
					bool powerOn = false;
					ideal2(agent& _agent, cstr _name);
					void set_phase_voltage(float _a_V, float _b_V, float _c_V);
				};
			}
		}
	}
}
#endif