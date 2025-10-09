#ifndef __dcmachine_hpp
#define __dcmachine_hpp

#include "im/edev/joint_link.hpp"
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
		namespace joint {
			namespace dcmachine {
				class ROBO_EXPORT ideal2 : public agent::block {
				private:
#if ARMADILLO_ENABLED
					arma::fmat A;
					arma::fmat IA;
					arma::fmat EA;
					arma::fmat X;
					arma::fmat Xmax;
					arma::fmat U;
#else
				  using matrix2x2 = robo::tractor::matrix2x2_t<double>;
				  using vector2_s = robo::tractor::vector2_t<double>;
				  matrix2x2 A;
				  matrix2x2 IA;
				  matrix2x2 EA;
				  vector2_s X;
				  vector2_s Xmax;
				  vector2_s U;
#endif
					float  A1 = 0.f;
					float IA1 = 0.f;
					float EA1 = 0.f;
					float Xmax1 = 0.f;

					float  A2 = 0.f;
					float IA2 = 0.f;
					float EA2 = 0.f;
					float Xmax2 = 0.f;

				protected:

					float Ls = 0.0001f;
					float Rs = 2.f;
					float Km = 0.02f;
					float Ke = 0.022f;
					float J = 0.000001f;
					float Kv = 0.f;

				protected:
					virtual bool do_load(cstr _specific_sect, cstr _common_sect);
					virtual void do_reconfig(void);
					virtual void do_run(void);
				public:
					iactuator actuator;
					ideal2(agent& _agent, cstr _name);
					float voltage = 0.f;
					float current = 0.f;
					bool powerOn = false;
				};
			}
		}
	}
}
#endif
