#include "im/models/dcmachine.hpp"
#include "core/robosd_ini.hpp"
namespace robo{
	namespace edev{
		namespace joint {
			namespace dcmachine {
				ideal2::ideal2(agent& _agent, cstr _name)
					: agent::block(_agent, _name)
					, A(2, 2)
					, IA(2, 2)
					, EA(2, 2)
					, X(2, 1)
					, Xmax(2, 1)
					, U(2, 1)

				{
					X(0) = 0;
					X(1) = 0;
				}
				void ideal2::do_reconfig() {
					A(0, 0) = -Rs / Ls;
					A(0, 1) = -Ke / Ls;

					A(1, 0) = Km / J;
					A(1, 1) = -Kv / J;

					EA = arma::expmat(A * owner.sample_time);
					IA = A.i();

					A1 = -Kv / J;
					EA1 = (float)exp(owner.sample_time* A1);
					IA1 = 1 / A1;

					A2 = -Rs / Ls;
					EA2 = (float)exp(owner.sample_time* A2);
					IA2 = 1 / A2;

				}
				void ideal2::do_run(void) {
					if (powerOn) {
						if (actuator.state != joint::iactuator::istate::blocked) {
							U(0) = voltage / Ls;
							U(1) = -actuator.contr_torque / J;
							Xmax = -IA * U;
							X = Xmax - EA * (Xmax - X);
							actuator.position += ((double)X(1) + actuator.speed) * owner.sample_time / 2;
							actuator.speed = X(1);
						}
						else {
							Xmax2 = -IA2 * voltage / Ls;
							X(0) = Xmax2 - EA2 * (Xmax2 - X(0));
							actuator.speed = X(1) = 0.f;
						}
						current = X(0);
					}
					else {
						voltage = 0.f;
						if (actuator.state != joint::iactuator::istate::blocked) {
							X(0) = current = 0.f;
							Xmax1 = -IA1 * (-actuator.contr_torque / J);
							float speed1 = Xmax1 - EA1 * (Xmax1 - actuator.speed);
							actuator.position += ((double)speed1 + actuator.speed) * owner.sample_time / 2;
							X(1) = actuator.speed = speed1;
						}
						else {
							X(0) = current = 0.f;
							actuator.speed = X(1) = 0.f;
						}
					}
					actuator.driveng_torque = current * Km;
				}

				bool ideal2::do_load(cstr _specific_sect, cstr _common_sect) {
					float Un;
					float nxx;
					ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("Rs"), Rs));
					ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("Ls"), Ls));
					ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("J"), J));
					ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("Kv"), Kv));
					ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("Un"), Un));
					ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("nxx"), nxx));
					Km = Ke =  Un / (nxx * pi<float> / 30.f);
					return true;
				}

			}
		}
	}
}
