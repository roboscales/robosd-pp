#include "im/models/pmsm.hpp"
#include "core/robosd_ini.hpp"
#include <iostream>
namespace robo {
	namespace edev {
		namespace joint {
			namespace pmsm {
				ideal2::ideal2(agent &_agent, cstr _name)
					: agent::block(_agent, _name)
#if ARMADILLO_ENABLED
					, A(3,3)
                    , IA(3,3)
                    , EA1(3, 3)
                    , EA2(3, 3)
                    , X(3, 1)
                    , Xmax(3, 1)
                    , U(3, 1)
#endif																						
				{
#if ARMADILLO_ENABLED
					X(0) = 0;
					X(1) = 0;
					X(2) = 0;
#else
					X[0] = 0.;
					X[1] = 0.;
					X[2] = 0.;
#endif
				}
				void ideal2::do_reconfig(void){
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
#if ARMADILLO_ENABLED

					A(0, 0) = - ws;
					A(0, 1) = 0;
					A(0, 2) = 0;

					A(1, 0) = 0;
					A(1, 1) = -ws;
					A(1, 2) = -Fm/Ls;

					A(2, 0) = 0;
					A(2, 1) = Fm / J;
					A(2, 2) = - Kv / J;
					EA1 = arma::expmat(A * (float)owner.sample_time);
					EA2.zeros(3, 3);
					EA2(2, 2) = 1;
#else
					A(0, 0) = -ws;
					A(0, 1) = 0;
					A(0, 2) = 0;

					A(1, 0) = 0;
					A(1, 1) = -ws;
					A(1, 2) = -Fm / Ls;

					A(2, 0) = 0;
					A(2, 1) = Fm / J;
					A(2, 2) = -Kv / J;

					EA1 = robo::expm(A * owner.sample_time, 1e-10);
					#if 0
					{1, 0, 0, 0, 1, 0, 0, 0, 1};					
					auto AT = A * owner.sample_time;
					auto ATN = AT;
					EA1 += AT;
					double nf = 1;
					for (int i = 2; i <= 6;++i) {
						ATN = ATN * AT;
						nf = nf * i;
						EA1 += ATN / nf;
					}

					#endif
					EA2 = {0, 0, 0, 0, 0, 0, 0, 0, 1};

#endif
				}
				void ideal2::set_phase_voltage(float _a_V, float _b_V, float _c_V){
					voltage.set_abc(_a_V, _b_V, _c_V,(float) electro.position);
				}

				void ideal2::do_run(void){
#if ARMADILLO_ENABLED
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
							float wT = (float)( owner.sample_time * w ) ;
							EA2(0, 0) = cos(wT);
							EA2(0, 1) = sin(wT);
							EA2(1, 0) = -sin(wT);
							EA2(1, 1) = cos(wT);
							X = Xmax - EA1*EA2*(Xmax - X);

							float els = X(2);
							electro.position += ( owner.sample_time *(els + electro.speed) / 2. );							
							electro.phase = (float)fmod(electro.position + robo::pi<float>, 2 * robo::pi<float>) - robo::pi<float>;
							electro.speed = els;

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
						current.set_dq(X(0), X(1), electro.phase);
						actuator.driveng_torque = p*current.dq.q*Fm;
					}
					else {
						if (actuator.state != joint::iactuator::istate::blocked){
							X(2) -= (float)(owner.sample_time * (actuator.contr_torque / p + X(2)*Kv) / J);
							float els = X(2);
							electro.position += (float) ( owner.sample_time *(els + electro.speed)/ 2. );
							electro.phase = (float)(fmod(electro.position + robo::pi<float>, 2 * robo::pi<float>) - robo::pi<float>);
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
						current.set_dq(0, 0, (float)electro.position);
					}
#else
					if (powerOn) {
						if (actuator.state != joint::iactuator::istate::blocked) {

							float w = electro.speed;
							float w2 = w * w;
							float ro = ro_ + Kv * Ls * w2;

							float KvwLs = Kv * w * Ls;
							float Fmw = Fm * w;

							/*
							IA = [...
							[  -Fm^2-Kv*ws*Ls, -Kv*w*Ls,           Fm*J*w];...
							[         Kv*w*Ls, -Kv*ws*Ls,          Fm*J*ws];...
							[         Fm*w*Ls, -Fm*ws*Ls,     -J*Ls*(w^2+ws^2)];...
							] /( Fm^2*ws + Kv*Ls*w^2 + Kv*Ls*ws^2);
							*/

							IA(0, 0) = IA0_0_0 / ro;
							IA(0, 1) = -KvwLs / ro;
							IA(0, 2) = Fmw * J / ro;

							IA(1, 0) = KvwLs / ro;
							IA(1, 1) = IA0_1_1 / ro;
							IA(1, 2) = IA0_1_2 / ro;

							IA(2, 0) = Fmw * Ls / ro;
							IA(2, 1) = IA0_2_1 / ro;
							IA(2, 2) = -J * Ls * (w2 + ws2) / ro;

							U[0] = voltage.dq.d / Ls;
							U[1] = voltage.dq.q / Ls;
							U[2] = -actuator.contr_torque / p / J;
							Xmax = IA * U *(-1.);
							float wT = (float)(owner.sample_time * w);
							EA2(0, 0) = cos(wT);
							EA2(0, 1) = sin(wT);
							EA2(1, 0) = -sin(wT);
							EA2(1, 1) = cos(wT);
							X = Xmax - EA1 * EA2 * (Xmax - X);

							float els = (float)X[2];
							electro.position += (owner.sample_time * (els + electro.speed) / 2.);
							electro.phase = (float)fmod(electro.position + robo::pi<float>, 2 * robo::pi<float>) - robo::pi<float>;
							electro.speed = els;

							actuator.position = electro.position / p;
							actuator.speed = electro.speed / p;
						}
						else {
							float idm = voltage.dq.d / Rs;
							X[0] = idm - EA1(0, 0) * (idm - X[0]);
							float iqm = voltage.dq.q / Rs;
							X[1] = iqm - EA1(1, 1) * (iqm - X[1]);
							X[2] = electro.speed = actuator.speed = 0.f;
						}
						current.set_dq((float)X[0], (float)X[1], electro.phase);
						actuator.driveng_torque = p * current.dq.q * Fm;
					}
					else {
						if (actuator.state != joint::iactuator::istate::blocked) {
							X[2] -= (float)(owner.sample_time * (actuator.contr_torque / p + X[2] * Kv) / J);
							float els = (float)X[2];
							electro.position += (float)(owner.sample_time * (els + electro.speed) / 2.);
							electro.phase = (float)(fmod(electro.position + robo::pi<float>, 2 * robo::pi<float>) - robo::pi<float>);
							electro.speed = els;
							actuator.position = electro.position / p;
							actuator.speed = electro.speed / p;
							X[0] = X[1] = 0.f;
						}
						else {
							X.zeros();
							electro.speed = actuator.speed = 0.f;
						}
						actuator.driveng_torque = 0.f;
						current.set_dq(0, 0, (float)electro.position);
					}
#endif
				}

				bool ideal2::do_load(cstr _specific_sect, cstr _common_sect) {
					float Un;
					float nxx;
					ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("Rs"), Rs));
					ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("Ls"), Ls));
					ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("J"), J));
					ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("Kv"), Kv));
					ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("p"), p));
					ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("Un"), Un));
					ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("nxx"), nxx));
					Kv = Kv / p;
					J = J / p / p;
					Rs = Rs * 3.f / 2.f;
					Ls = Ls * 3.f / 2.f;
					Fm = Un / (nxx * p * pi<float> / 30.f);
					return true;
				}
			}
		}
	}
}
