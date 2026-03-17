#include "im/models/pmsmr2.hpp"
#include "core/robosd_ini.hpp"
#include <iostream>
namespace robo {
	namespace edev {
		namespace joint {
			namespace pmsmr2 {
				ideal2::ideal2(agent &_agent, cstr _name)
					: agent::block(_agent, _name)
				{
				}
				void ideal2::do_reconfig(void){
					teylor_begin_();

				}
				void ideal2::set_line_voltage(double _a_V, double _b_V, double _c_V){
                    //Момент истины. Не умнажаем на 2/3 как это было бы при расчете реальных фазных напряжений
                    auto ua = (_a_V - (_b_V + _c_V) / 2);
                    auto ub = (_b_V - (_a_V + _c_V) / 2);
                    auto uc = (_c_V - (_a_V + _b_V) / 2);
					voltage.set_abc(ua, ub, uc, electro.position);
				}
                void ideal2::set_phase_voltage(double _a_V, double _b_V, double _c_V) {
                    voltage.set_abc(_a_V, _b_V, _c_V, electro.position);
                }
                void ideal2::teylor_begin_(void) {
                    a = Rs / Ls;
                    b = Psi_m / Ls;
                    c = Psi_m / J3 * p * p;
                    d = -Kv3 / J3;
                    g = - (double)p / J3;

                    T = owner.sample_time;
                    g_factor = g;               // g = -p/J
                    T2 = T * T;
                    T3 = T2 * T;                 // T^3 (может не использоваться, но сохранено для полноты)
                    LsT = Ls * T;
                    LsT2 = Ls * T2;
                    LsT3 = Ls * T3;
                    twoLs = 2.0 * Ls;
                    twoLsT = 2.0 * LsT;
                    a2 = a * a;

                    LsT2a = LsT2 * a;
                    LsT2a2 = LsT2 * a2;
                    LsT2b = LsT2 * b;
                    LsT2c = LsT2 * c;
                    LsT2d = LsT2 * d;

                    LsTa = LsT * a;
                    LsTb = LsT * b;
                    LsTc = LsT * c;
                    LsTd = LsT * d;

                    T2a = T2 * a;
                    T2c = T2 * c;

                    D = 2.0 * T - T2a;           // для ud и uq
                    C1 = LsT - LsT2a;             // для перекрёстных членов

                    LsT2ab = LsT2a * b;
                    LsT2ac = LsT2a * c;
                    LsT2bc = LsT2b * c;
                    LsT2bd = LsT2b * d;
                    LsT2cd = LsT2c * d;
                    LsT2d2 = LsT2d * d;

                    twoLsTa = 2.0 * LsTa;
                    twoLsTb = 2.0 * LsTb;
                    twoLsTd = 2.0 * LsTd;

                    A_id0 = twoLs + LsT2a2 - twoLsTa;
                    A_iq0 = -twoLs - LsT2a2 + twoLsTa + LsT2bc;


                    aT = a * T;
                    aT2 = aT * aT;
                    aT3 = aT2 * aT;
                    c0 = 1.0 - aT + aT2 / 2.0 - aT3 / 6.0;          // коэффициент при id
                    c1 = (T / Ls) * (1.0 - aT / 2.0 + aT2 / 6.0); // коэффициент при ud

                    aT = a * T;
                    aT2 = aT * aT;
                    aT3 = aT2 * aT;
                    c0 = 1.0 - aT + aT2 / 2.0 - aT3 / 6.0;          // коэффициент при id
                    c1 = (T / Ls) * (1.0 - aT / 2.0 + aT2 / 6.0); // коэффициент при ud

                }

                void ideal2::teylor_run_(void)
                {
                    // Предварит

                    auto omega = electro.speed;
                    auto theta = electro.phase;

                    // Напряжения на текущем шаге
  //                  ux = ux_ref[k];
    //                uy = uy_ref[k];

                    auto id = current.dq.d;
                    auto iq = current.dq.q;

                    auto ud = voltage.dq.d;
                    auto uq = voltage.dq.q;

                    // Прогноз скорости на середину шага
                    auto domega_dt = c * iq - (Kv3 / J3) * omega - (double)actuator.contr_torque / J3 * p;
                    auto omega_pred = omega + domega_dt * (T / 2.0);
                    auto w = omega_pred;
                    auto w0 = omega;

                    // Промежуточные величины
                    auto w2 = w * w;
                    auto udT = ud * T;            // не используется напрямую, но оставлено для симметрии
                    auto uqT = uq * T;
                    auto udT2 = ud * T2;
                    auto uqT2 = uq * T2;
                    auto LsT2w2 = LsT2 * w2;
                    auto T2w = T2 * w;

                    auto LsM_g = Ls * actuator.contr_torque * g_factor;       // эквивалент Ls * M_g
                    auto LsM_gT = LsM_g * T;
                    auto LsM_gT2 = LsM_g * T2;
                    auto LsM_gT2b = LsM_gT2 * b;
                    auto LsM_gT2d = LsM_gT2 * d;

                    auto id_w = id * w;
                    auto iq_w = iq * w;

                    // --- Вычисление id_new ---
                    auto A_id = A_id0 - LsT2w2;
                    auto term1_id = id * A_id + 2.0 * iq_w * C1 + ud * D + uq * T2w - LsT2b * w * w0;
                    auto id_new = term1_id / twoLs;

                    // --- Вычисление iq_new ---
                    auto A_iq = A_iq0 + LsT2w2;
                    auto const_iq_w0 = w0 * (twoLsTb - LsT2ab + LsT2bd);
                    auto term1_iq = iq * A_iq + 2.0 * id_w * C1 + uq * (-D) + ud * T2w + const_iq_w0 + LsM_gT2b;
                    auto iq_new = -term1_iq / twoLs;          // минус согласно исходной формуле

                        // --- Вычисление omega_new ---
                    auto temp1 = twoLsT - LsT2a + LsT2d;
                    auto iq_coef_omega = c * temp1;
                    auto const_omega_w0 = w0 * (twoLs + LsT2d2 + twoLsTd - LsT2bc);
                    auto const_omega_M = 2.0 * LsM_gT + LsM_gT2d;
                    auto term1_omega = -LsT2c * id_w + iq * iq_coef_omega + const_omega_w0 + T2c * uq + const_omega_M;
                    auto omega_new = term1_omega / twoLs;

                    // --- Обновление угла ---
                    auto deltaTheta = (T * (2.0 * omega + actuator.contr_torque * g_factor * T + T * c * iq + T * d * omega)) / 2.0;
                    auto theta_new = theta + deltaTheta;
                    electro.position += deltaTheta;
                    electro.phase = (double)fmod(theta_new + pi, 2 * pi) - pi;
                    electro.speed = (double)omega_new;
                    current.set_dq((double)id_new, (double)iq_new, electro.phase);

                    actuator.position = electro.position / p;
                    actuator.speed = (float)electro.speed / p;
                    actuator.driveng_torque = (float) (current.dq.q * Psi_m * p);

                }
                void ideal2::locked_rotor_taylor_(void)
                {
                    // Предварительные коэффициенты для ряда Тейлора 3-го порядка
                    double aT = a * T;
                    double aT2 = aT * aT;
                    double aT3 = aT2 * aT;
                    double c0 = 1.0 - aT + aT2 / 2.0 - aT3 / 6.0;          // коэффициент при id
                    double c1 = (T / Ls) * (1.0 - aT / 2.0 + aT2 / 6.0); // коэффициент при ud

                    auto id = current.dq.d;
                    auto iq = current.dq.q;

                    auto ud = voltage.dq.d;
                    auto uq = voltage.dq.d;

                    // Обновление по методу Тейлора (независимо для d и q)
                    double id_new = id * c0 + ud * c1;
                    double iq_new = iq * c0 + uq * c1;

                    current.set_dq(id_new, iq_new, electro.phase);
                    actuator.driveng_torque = (float)(current.dq.q * Psi_m * p);
                    electro.speed = 0;
                    actuator.speed = 0;

                }

                void ideal2::taylor_step_no_current(void)
                {
                    auto w = electro.speed;
                    auto th = electro.phase;

                    // Производная скорости (dw/dt)
                    auto dw_dt = -(Kv3 / J3) * w - (p / J3) * actuator.contr_torque;
                    // Вторая производная скорости (d²w/dt²)
                    auto d2w_dt2 = -(Kv3 / J3) * dw_dt;   // так как dw_dt линейно по w

                    // Новое значение скорости по формуле Тейлора 2-го порядка
                    auto w_new = w + T * dw_dt + 0.5 * T * T * d2w_dt2;

                    // Новое значение угла (интегрирование скорости)
                    auto deltaTheta = T * w + 0.5 * T * T * dw_dt;
                    auto theta_new = th + deltaTheta;
                    electro.position += deltaTheta;
                    electro.phase = (double)fmod(theta_new + robo::pi<double>, 2 * robo::pi<double>) - robo::pi<double>;
                    electro.speed = w_new;

                    current.set_dq(0, 0, electro.phase);
                    actuator.driveng_torque = 0;
                    actuator.position = electro.position / p;
                    actuator.speed = (float)electro.speed / p;

                }
				void ideal2::do_run(void){

					if (powerOn) {
                        teylor_run_();
#if 0
                        if (actuator.state != joint::iactuator::istate::blocked) {
							teylor_run_();
						} else {
							locked_rotor_taylor_();
						}
#endif
					}
					else {
						if (actuator.state != joint::iactuator::istate::blocked) {
							taylor_step_no_current();
						}
						else {
							electro.speed = actuator.speed = 0.f;
							current.set_dq(0, 0, (double)electro.phase);
							actuator.driveng_torque = 0.f;
						}
					}
				}

				bool ideal2::do_load(cstr _specific_sect, cstr _common_sect) {
					double Un;
					double nxx;
					ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("Rf"), Rf3));
					ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("Lf"), Lf3));
					ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("J"), J3));
					ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("Kv"), Kv3));
					ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("kL"), kL));
					ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("p"), p));
					ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("Un"), Un));
					ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("nxx"), nxx));
					auto Fm = Un / (nxx * pi / 30.f);
					Ls3 = Lf3 * kL;
					Lm3 = Lf3 - Ls3;

					Rs = Rf3 * 3.f / 2.f;
					auto Lm = Lm3 * 3.f / 2.f;
					auto Ls1 = 1.5f * Ls3 + 0.75f * Lm3;
					Ls = Ls1 + Lm;
					Psi_e = 1.5 * Fm / p;
					Psi_m = Psi_e*(1.f-kL);
					return true;
				}
			}
		}
	}
}
