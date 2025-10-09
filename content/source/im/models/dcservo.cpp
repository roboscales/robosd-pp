#include "im/models/dcservo.hpp"
#include "core/robosd_ini.hpp"
#include <math.h>
namespace robo{
	namespace edev{
		namespace joint {
			namespace dcservo {

				ideal::ideal(agent& _agent, cstr _name)
					: dcmachine::ideal2(_agent, _name)
				{}

				void ideal::do_reconfig() {

					Jo = load_inert_kgm2;
					Mmax = load_max_kgsm * 0.0981f;
					wn = speed_max_gps / 180.f * 3.1415f;
					float Um = voltage_max;
					
					float a = flux_score;
					float D = Um * Um - 4 * Mmax * Rs * wn / a;
					if (D < 0.f) D = 0.f;
					Ke = 2 * Mmax * Rs / (a * (Um - sqrt(D)));
					J = Jo * load_score;
					Km = Ke * a;
					Ko = Um * Km / Rs / Jo;
					double T = owner.sample_time;
					double b = T * beta_score;
					Kd = (float)((T + b) / (T * (b + T / 2)) / Ko);
					Kp = (float)(1 / (T * (b + T / 2)) / Ko * prop_score);
					Kp = Kp / Kd;
					Kd = Kd / 2.f;
					position_gain = (position_max_g - position_min_g) / (position_max_ms - position_min_ms);
					dcmachine::ideal2::do_reconfig();
				}

				bool ideal::do_load(cstr _specific_sect, cstr _common_sect) {
					ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("Rs"), Rs));
					ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("Ls"), Ls));
					ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("Kv"), Kv));

					ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("load_inert_kgm2"), load_inert_kgm2));
					ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("load_max_kgsm"), load_max_kgsm));
					ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("speed_max_gps"), speed_max_gps));
					ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("position_max_ms"), position_max_ms));
					ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("position_min_ms"), position_min_ms));
					ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("position_max_g"), position_max_g));
					ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("position_min_g"), position_min_g));
					ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("voltage_max"), voltage_max));
					ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("flux_score"), flux_score));
					ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("beta_score"), beta_score));
					ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("prop_score"), prop_score));
					ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("load_score"), load_score));
					return true;
				}
								

				void ideal::set_duty_ms(float  _position_ms) {
					if (_position_ms > position_max_ms) _position_ms = position_max_ms;
					if (_position_ms < position_min_ms) _position_ms = position_min_ms;
					float position_req_g = (position_min_g + (_position_ms - position_min_ms) * position_gain);
					position_req = position_req_g / 180.f * 3.1415f;
				}

				void ideal::do_run(void) {
					if (powerOn) {
						position_err = position_req - (float)actuator.position;
						speed_req = position_err * Kp;
						if (speed_req > speed_max_gps) speed_req = speed_max_gps;
						if (speed_req < -speed_max_gps) speed_req = -speed_max_gps;
						speed_err = speed_req - actuator.speed;

						if (!((speed_err < 0.f && (voltage <= -voltage_max)) || (speed_err > 0.f && (voltage >= voltage_max)))) {
							speed_errint += (float) ( owner.sample_time * speed_err * Ko);
						}
						voltage = (speed_err + speed_errint - actuator.speed) * Kd;
						if (voltage > voltage_max) voltage = voltage_max;
						if (voltage < -voltage_max) voltage = -voltage_max;
					}
					else {
						speed_errint = 0.f;
						voltage = 0.f;
					}
					dcmachine::ideal2::do_run();
				}
			}
		}
	}
}