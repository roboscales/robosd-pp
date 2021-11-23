#ifndef __dcmachine_hpp
#define __dcmachine_hpp

#include "im/edev/joint_link.hpp"
#include <armadillo>
namespace robo{
	namespace im{
		namespace dcmachine{
			class ideal2 : public robo::models::block{
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

			public:
				struct isettings {
					parametr<float> Ls;
					parametr<float> Rs;
					parametr<float> Km;
					parametr<float> Ke;
					parametr<float> J;
					parametr<float> Kv;
					isettings(ideal2 & _owner);
				};

			protected:
				virtual void reconfig(void);
				virtual bool setup(void);
			public:
				isettings settings;
				joint::iactuator actuator;
				ideal2(void);
				float voltage = 0.f;
				float current = 0.f;
				bool powerOn = false;
				virtual void run(void);
			};
		}
	}
}
#endif
