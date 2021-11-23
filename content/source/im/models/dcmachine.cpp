#include "im/models/dcmachine.hpp"
#include "core/robosd_ini.hpp"
namespace robo{
	namespace im{
		namespace dcmachine{
			ideal2::isettings::isettings(ideal2 & _owner)
                : Ls(_owner, _owner.Rs)
                , Rs(_owner, _owner.Ls)
				, Km(_owner, _owner.Km)
				, Ke(_owner, _owner.Ke)
				, J(_owner, _owner.J)
				, Kv(_owner, _owner.Kv)
			{

			}

			ideal2::ideal2(void)
				: robo::models::block()
                , A(2, 2)
                , IA(2, 2)
				, EA(2, 2)
                , X(2, 1)
                , Xmax(2, 1)
				, U(2, 1)
                , settings(*this)

			{
					X(0) = 0;
					X(1) = 0;
            }
			void ideal2::reconfig(){
				A(0, 0) = -Rs / Ls;
				A(0, 1) = -Ke / Ls;

				A(1, 0) = Km / J;
				A(1, 1) = -Kv / J;

				EA = arma::expmat(A*model_period_sec);
				IA = A.i();

				A1 = -Kv / J;
				EA1 = exp(A1*model_period_sec);
				IA1 = 1 / A1;

				A2 = -Rs / Ls;
				EA2 = exp(A2*model_period_sec);
				IA2 = 1 / A2;

			}
			void ideal2::run(void){
				if (powerOn){
					if (actuator.state != joint::iactuator::istate::blocked){
						U(0) = voltage / Ls;
						U(1) = - actuator.contr_torque / J;
						Xmax = -IA * U;
						X = Xmax - EA * (Xmax - X);
						actuator.position += (X(1) + actuator.speed) * model_period_sec / 2;
						actuator.speed = X(1);
					}
					else{
						Xmax2 = -IA2 * voltage / Ls;
						X(0) = Xmax2 - EA2 * (Xmax2 - X(0));
						actuator.speed = X(1)= 0.f;
					}
					current = X(0);
				}
				else{
					voltage = 0.f;
					if (actuator.state !=joint::iactuator::istate::blocked){
						X(0) = current = 0.f;
						Xmax1 = -IA1*(-actuator.contr_torque / J);
						float speed1 = Xmax1 - EA1 * (Xmax1 - actuator.speed);
						actuator.position += (speed1 + actuator.speed) * model_period_sec / 2;
						X(1) = actuator.speed = speed1;
					}
					else{
						X(0) = current = 0.f;
						actuator.speed = X(1) = 0.f;
					}
				}
				actuator.driveng_torque = current* Km;
			}

			bool ideal2::setup(void){
				ROBO_BREAKEN_LOAD_FLOAT(ini_section, RS("Rs"), &Rs);
				ROBO_BREAKEN_LOAD_FLOAT(ini_section, RS("Ls"), &Ls);
				ROBO_BREAKEN_LOAD_FLOAT(ini_section, RS("Km"), &Km);
				ROBO_BREAKEN_LOAD_FLOAT(ini_section, RS("Ke"), &Ke);
				ROBO_BREAKEN_LOAD_FLOAT(ini_section, RS("J"), &J);
				ROBO_BREAKEN_LOAD_FLOAT(ini_section, RS("Kv"), &Kv);
				return true;
			}
		}
	}
}
