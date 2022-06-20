#ifndef __dcmachine_hpp
#define __dcmachine_hpp

#include "im/edev/joint_link.hpp"
#include <armadillo>
namespace robo{
	namespace edev{
		namespace joint {
			namespace dcmachine {
				class ROBO_EXPORT ideal2 : public agent::block {
				private:
					arma::fmat A;
					arma::fmat IA;
					arma::fmat EA;
					arma::fmat X;
					arma::fmat Xmax;
					arma::fmat U;

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
