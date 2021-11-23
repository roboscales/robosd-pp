#include "im/models/pmsm.hpp"
#include "core/robosd_ini.hpp"
#include <iostream>
namespace robo{
	namespace im{
		namespace models{
			namespace pmsm{				

				ideal2::isettings::isettings(ideal2 & _owner)
                    :
                     Ls(_owner, _owner.Ls)
                    , Rs(_owner, _owner.Rs)
					, Fm(_owner, _owner.Fm)
					, J(_owner, _owner.J)
					, Kv(_owner, _owner.Kv)
					, p(_owner, _owner.p)
				{

				}


				ideal2::ideal2() 
					: block()
					, A(3,3)
                    , IA(3,3)
                    , EA1(3, 3)
                    , EA2(3, 3)
                    , X(3, 1)
                    , Xmax(3, 1)
                    , U(3, 1)
                    , settings(*this)

				{
					X(0) = 0;
					X(1) = 0;
					X(2) = 0;
				}
				void ideal2::reconfig(void){
					ws = Rs / Ls;
					ws2 = ws*ws;

					ro_ = (Fm*Fm * ws + Kv *Ls*ws2);

					IA0_0_0 = (-Fm*Fm - Kv*ws*Ls);

					IA0_1_1 = -Kv*ws*Ls;
					IA0_1_2 = Fm * J *ws;

					IA0_2_1 = -Fm*ws*Ls;

					/*
					IA = [...
					[  -Fm^2-Kv*ws*Ls, -Kv*w*Ls,           Fm*J*w];...
					[         Kv*w*Ls, -Kv*ws*Ls,          Fm*J*ws];...
					[         Fm*w*Ls, -Fm*ws*Ls,     -J*Ls*(w^2+ws^2)];...
					] /( Fm^2*ws + Kv*Ls*w^2 + Kv*Ls*ws^2);
					*/

					A(0, 0) = - ws;
					A(0, 1) = 0;
					A(0, 2) = 0;

					A(1, 0) = 0;
					A(1, 1) = -ws;
					A(1, 2) = -Fm/Ls;

					A(2, 0) = 0;
					A(2, 1) = Fm / J;
					A(2, 2) = - Kv / J;
					EA1 = arma::expmat(A*model_period_sec);
					EA2.zeros(3, 3);
					EA2(2, 2) = 1;
				}
				void ideal2::set_phase_voltage(float _a_V, float _b_V, float _c_V){
					voltage.set_abc(_a_V, _b_V, _c_V, electro.position);
				}

				void ideal2::run(void){
					if (powerOn){
						if (actuator.state != joint::iactuator::istate::blocked){

							float w = electro.speed;
							float w2 = w*w;
							float ro = ro_ + Kv*Ls*w2;

							float KvwLs = Kv*w*Ls;
							float Fmw = Fm*w;

							/*
							IA = [...
							[  -Fm^2-Kv*ws*Ls, -Kv*w*Ls,           Fm*J*w];...
							[         Kv*w*Ls, -Kv*ws*Ls,          Fm*J*ws];...
							[         Fm*w*Ls, -Fm*ws*Ls,     -J*Ls*(w^2+ws^2)];...
							] /( Fm^2*ws + Kv*Ls*w^2 + Kv*Ls*ws^2);
							*/

							IA(0, 0) = IA0_0_0 / ro;
							IA(0, 1) = -KvwLs / ro;
							IA(0, 2) = Fmw*J / ro;

							IA(1, 0) = KvwLs / ro;
							IA(1, 1) = IA0_1_1 / ro;
							IA(1, 2) = IA0_1_2 / ro;

							IA(2, 0) = Fmw*Ls / ro;
							IA(2, 1) = IA0_2_1 / ro;
							IA(2, 2) = -J*Ls*(w2 + ws2) / ro;

							U(0) = voltage.dq.d / Ls;
							U(1) = voltage.dq.q / Ls;
							U(2) = -actuator.contr_torque/ p / J;
							Xmax = -IA*U;
							float wT = w*model_period_sec;
							EA2(0, 0) = cos(wT);
							EA2(0, 1) = sin(wT);
							EA2(1, 0) = -sin(wT);
							EA2(1, 1) = cos(wT);
							X = Xmax - EA1*EA2*(Xmax - X);



							float els = X(2);
							electro.position += (els + electro.speed)*model_period_sec / 2;
							electro.phase = fmod(electro.position + 3.14159265358979f, 2 * 3.14159265358979f) - 3.14159265358979f;
							electro.speed = els;

							actuator.position = electro.position / p;
							actuator.speed = electro.speed / p;
							actuator.position = electro.position / p;
							actuator.speed = electro.speed / p;
						}
						else {
							float idm = voltage.dq.d / Rs;
							X(0) = idm - EA1(0, 0)*(idm - X(0));
							float iqm = voltage.dq.q / Rs;
							X(1) = iqm - EA1(1, 1)*(iqm - X(1));
							X(2) = electro.speed = actuator.speed = 0.f;
						}
						current.set_dq(X(0), X(1), electro.position);
						actuator.driveng_torque = p*current.dq.q*Fm;
					}
					else {
						if (actuator.state != joint::iactuator::istate::blocked){
							X(2) -= (actuator.contr_torque / p + X(2)*Kv) / J*model_period_sec;
							float els = X(2);
							electro.position += (els + electro.speed)*model_period_sec / 2;
							electro.phase = fmod(electro.position + 3.14159265358979f, 2 * 3.14159265358979f) - 3.14159265358979f;
							electro.speed = els;
							actuator.position = electro.position / p;
							actuator.speed = electro.speed / p;
							X(0) = X(1) = 0.f;
						}
						else {
							X.zeros();
							electro.speed = actuator.speed = 0.f;
						}
						actuator.driveng_torque = 0.f;
						current.set_dq(0, 0, electro.position);
					}


				}

				bool ideal2::setup(void){
					float Un;
					float nxx;
					ROBO_BREAKEN_LOAD_FLOAT(ini_section, RS("Rs"), &Rs);
					ROBO_BREAKEN_LOAD_FLOAT(ini_section, RS("Ls"), &Ls);
					ROBO_BREAKEN_LOAD_FLOAT(ini_section, RS("J"), &J);
					ROBO_BREAKEN_LOAD_FLOAT(ini_section, RS("Kv"), &Kv);
					ROBO_BREAKEN_LOAD_FLOAT(ini_section, RS("p"), &p);
					ROBO_BREAKEN_LOAD_FLOAT(ini_section, RS("Un"), &Un);
					ROBO_BREAKEN_LOAD_FLOAT(ini_section, RS("nxx"), &nxx);
					Kv = Kv / p;
					J = J / p / p;
					Rs = Rs * 3.f / 2.f;
					Ls = Ls * 3.f / 2.f;
					Fm = Un / (nxx*p*3.14159265358979f/30.f);
					return true;
				}
			}
		}
	}
}
