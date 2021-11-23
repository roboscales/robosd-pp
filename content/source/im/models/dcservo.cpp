#include "im/models/dcservo.hpp"
#include "core/robosd_ini.hpp"
namespace robo{
	namespace im{
		namespace dcservo{
			ideal::isettings::isettings(ideal & _owner)
				: owner_(_owner)
				, load_inert_kgm2(_owner, _owner.load_inert_kgm2)
				, load_max_kgsm(_owner, _owner.load_max_kgsm)
				, speed_max_gps(_owner, _owner.speed_max_gps)
				, position_max_ms(_owner, _owner.position_max_ms)
				, position_min_ms(_owner, _owner.position_min_ms)
				, position_max_g(_owner, _owner.position_max_g)
				, position_min_g(_owner, _owner.position_min_g)
				, voltage_max(_owner, _owner.voltage_max)
				, flux_score(_owner, _owner.flux_score)
				, beta_score(_owner, _owner.beta_score)
				, prop_score(_owner, _owner.prop_score)
			{

			}

			ideal::ideal() 
				: dcmachine::ideal2()
				, settings(*this)

			{
			}

			void ideal::reconfig(){

				Jo = load_inert_kgm2; 
				Mmax = load_max_kgsm * 0.0981f;
				wn = speed_max_gps / 180.f * 3.1415f;
				float Um = voltage_max;
				float R = dcmachine::ideal2::settings.Rs;
				float a = flux_score;
				float D = Um*Um - 4 * Mmax*R * wn/a;
				if (D < 0.f) D = 0.f;
				Ke = 2 * Mmax * R / (a*(Um - sqrt(D)));
				J = Jo*load_score;
				Km = Ke * a;
				Ko = Um * Km / R / Jo;
				float T = model_period_sec;
				float b = T* beta_score;
				Kd = (T + b) / (T*(b + T / 2)) / Ko;
				Kp = 1 / (T*(b + T / 2)) / Ko * prop_score;
				Kp = Kp / Kd;
				Kd = Kd / 2.f;
				position_gain = (position_max_g - position_min_g) / (position_max_ms - position_min_ms);					
				dcmachine::ideal2::reconfig();
			}

			bool ideal::setup(void){
				ROBO_BREAKEN_LOAD_FLOAT(ini_section, RS("Rs"), &Rs); 
				ROBO_BREAKEN_LOAD_FLOAT(ini_section, RS("Ls"), &Ls); 
				ROBO_BREAKEN_LOAD_FLOAT(ini_section, RS("Kv"), &Kv); 

				ROBO_BREAKEN_LOAD_FLOAT(ini_section, RS("load_inert_kgm2"), &load_inert_kgm2);
				ROBO_BREAKEN_LOAD_FLOAT(ini_section, RS("load_max_kgsm"), &load_max_kgsm);
				ROBO_BREAKEN_LOAD_FLOAT(ini_section, RS("speed_max_gps"), &speed_max_gps);
				ROBO_BREAKEN_LOAD_FLOAT(ini_section, RS("position_max_ms"), &position_max_ms);
				ROBO_BREAKEN_LOAD_FLOAT(ini_section, RS("position_min_ms"), &position_min_ms);
				ROBO_BREAKEN_LOAD_FLOAT(ini_section, RS("position_max_g"), &position_max_g);
				ROBO_BREAKEN_LOAD_FLOAT(ini_section, RS("position_min_g"), &position_min_g);
				ROBO_BREAKEN_LOAD_FLOAT(ini_section, RS("voltage_max"), &voltage_max);
				ROBO_BREAKEN_LOAD_FLOAT(ini_section, RS("flux_score"), &flux_score);
				ROBO_BREAKEN_LOAD_FLOAT(ini_section, RS("beta_score"), &beta_score);
				ROBO_BREAKEN_LOAD_FLOAT(ini_section, RS("prop_score"), &prop_score);
				ROBO_BREAKEN_LOAD_FLOAT(ini_section, RS("load_score"), &load_score);
				return true;
			}

			void ideal::set_duty_ms(float  _position_ms){
				if (_position_ms > position_max_ms) _position_ms = position_max_ms;
				if (_position_ms < position_min_ms) _position_ms = position_min_ms;
				float position_req_g = (position_min_g + (_position_ms - position_min_ms)*position_gain);
				position_req = position_req_g / 180.f*3.1415f;
			}

			void ideal::run(void){
				if (powerOn){
					position_err = position_req - (float)actuator.position;
					speed_req = position_err * Kp;
					if (speed_req > speed_max_gps) speed_req = speed_max_gps;
					if (speed_req < -speed_max_gps) speed_req = -speed_max_gps;
					speed_err = speed_req - actuator.speed;

					if (!((speed_err<0.f && (voltage<=-voltage_max)) || (speed_err>0.f && (voltage >= voltage_max)))){
						speed_errint += model_period_sec*speed_err*Ko;
					}
					voltage = (speed_err + speed_errint - actuator.speed)*Kd;
					if (voltage > voltage_max) voltage = voltage_max;
					if (voltage < -voltage_max) voltage = -voltage_max;
				}
				else{
					speed_errint = 0.f;
					voltage = 0.f;
				}
				dcmachine::ideal2::run();
			}
		}
	}
}