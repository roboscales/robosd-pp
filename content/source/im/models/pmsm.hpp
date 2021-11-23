#include "im/edev/joint_link.hpp"
#include "im/models/power.hpp"
#include <armadillo>
namespace robo{
	namespace im{
		namespace models{
			namespace pmsm{
				class ideal2 : public robo::models::block{
				private:
					arma::fmat A;
					arma::fmat IA;
					arma::fmat EA1;
					arma::fmat EA2;
					arma::fmat X;
					arma::fmat Xmax;
					arma::fmat U;
					float ws;
					float ws2;
					float ro_;
					float IA0_0_0;

					float IA0_1_1;
					float IA0_1_2;

					float IA0_2_1;
					float IA0_2_2;

				protected:
					float Ls;
					float Rs;
					float Fm;
					float J;
					float Kv;
					float p;
				protected:
					virtual void reconfig(void);
					virtual bool setup(void);
				public:
					struct isettings {
						parametr<float> Ls;
						parametr<float> Rs;
						parametr<float> Fm;
						parametr<float> J;
						parametr<float> Kv;
						parametr<float> p;
						isettings(ideal2 & _owner);
					};

					struct{
						float speed;
						float phase;
						float position;
					} electro;

					power::ph3 current;
					power::ph3 voltage;
					isettings settings;
					joint::iactuator actuator;
					bool powerOn = false;
					virtual void run(void);
					ideal2();
					void set_phase_voltage(float _a_V, float _b_V, float _c_V);
				};
			}
		}
	}
}