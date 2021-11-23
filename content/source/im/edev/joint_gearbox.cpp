#include "im/edev/joint_gearbox.hpp"
#include "im/edev/edev.h"
#include "core/robosd_log.h"
#include "robosd_target_api.h"
#include "core/robosd_ini.hpp"
#include <math.h>
namespace robo{
	namespace im{
		namespace joint{
			namespace gearbox{
				namespace elastic{
					const float PI = 3.14159265359f;
					ideal::ideal(void) : link()
					{
					}
					void ideal::run(void){
						observer.position = actuator->position / gear_ratio;
						observer.speed = actuator->speed / gear_ratio;
						observer.tension_diff = observer.speed - load->speed;
						observer.tension = (float)(observer.position - load->position + observer.tension_diff * model_period_sec / 2);
						
						observer.elastic_torque = observer.tension * hook_gain;

						actuator->contr_torque = observer.elastic_torque  / gear_ratio;
						load->driveng_torque = observer.elastic_torque - load->speed * load_viscous_gain;
						load->driveng_position = observer.position;
						load->driveng_speed = observer.speed;
						actuator->state = joint::iactuator::istate::run;
					}
					bool ideal::setup(void){
						ROBO_BREAKEN_LOAD_FLOAT(ini_section, RS("hook_gain"), &hook_gain);
						ROBO_BREAKEN_LOAD_FLOAT(ini_section, RS("load_viscous_gain"), &load_viscous_gain);
						ROBO_BREAKEN_LOAD_FLOAT(ini_section, RS("gear_ratio"), &gear_ratio);
						return true;
					}
					void ideal::reconfig(void){
					}
					nonline::nonline(void) : link()
					{
						actuator->state = joint::iactuator::istate::run;
					}
					void nonline::run(void){
						driver.speed = actuator->speed / driver.gear_ratio;
						if (supply.enabled){
							if (load->position >= supply.max){
								supply.tension = (float)(load->position - supply.max);
								if (supply.tension > supply.tension_max){
									load->position = supply.max + supply.tension_max;
									if (load->speed > 0.f){
										load->speed = 0.f;
									}
								}
							}
							else {
								if (load->position <= supply.min){
									supply.tension = (float)(load->position - supply.min );
									if (supply.tension < -supply.tension_max){
										load->position = supply.min - supply.tension_max;
										if (load->speed < 0.f){
											load->speed = 0.f;
										}
									}
								}
								else {
									supply.tension = 0.f;
								}
							}

							if (fabs(supply.tension) > 0.000001f){
								supply.torque = supply.gain * supply.tension + supply.desipation_gain* driver.speed;
							}
							else {
								supply.torque = 0.f;
							}
						}
						else {
							supply.torque = 0.f;

						}
						supply.position = load->position;// +(load->speed * model_period_sec);

						driver.tension_diff = load->driveng_speed - load->speed;

						driver.tension = (float)(load->driveng_position - load->position   + driver.tension_diff * model_period_sec / 2 );
						if (driver.tension >= driver.dead_zone){
							driver.tension -= driver.dead_zone;
						}
						else {
							if (driver.tension <= -driver.dead_zone){
								driver.tension += driver.dead_zone;
							}
							else{
								driver.tension = 0.f;
							}
						}
						
						driver.position = actuator->position / driver.gear_ratio;

						driver.elastic_torque = (driver.tension /*+ driver.tension_diff(model_period_sec * 500*/ ) * driver.hook_gain ;
						
						actuator->contr_torque = (driver.elastic_torque + supply.torque) / driver.gear_ratio;
						load->driveng_position = driver.position;
						load->driveng_speed = driver.speed;
						load->driveng_torque = driver.elastic_torque - supply.torque - load->speed*driver.load_viscous_gain;
						load->tension = driver.tension;
						load->tension_diff = driver.tension_diff;
					}

					bool nonline::setup(void){
						ROBO_BREAKEN_LOAD_FLOAT(ini_section, RS("driver_tension_max"), &driver.config.tension_max);
						ROBO_BREAKEN_LOAD_FLOAT(ini_section, RS("driver_torque_max"), &driver.config.torque_max);
						ROBO_BREAKEN_LOAD_FLOAT(ini_section, RS("driver_dead_zone"), &driver.config.dead_zone);
						ROBO_BREAKEN_LOAD_FLOAT(ini_section, RS("load_viscous_gain"), &driver.load_viscous_gain);
						ROBO_BREAKEN_LOAD_FLOAT(ini_section, RS("driver_gear_ratio"), &driver.gear_ratio);

						ROBO_BREAKEN_LOAD_INT(ini_section, RS("supply_enabled"), &supply.enabled);

						if (supply.enabled == 1){
							ROBO_BREAKEN_LOAD_FLOAT(ini_section, RS("supply_min"), &supply.config.min);
							ROBO_BREAKEN_LOAD_FLOAT(ini_section, RS("supply_max"), &supply.config.max);
							ROBO_BREAKEN_LOAD_FLOAT(ini_section, RS("supply_tension_max"), &supply.config.tension_max);
							ROBO_BREAKEN_LOAD_FLOAT(ini_section, RS("supply_torque_max"), &supply.config.torque_max);
							ROBO_BREAKEN_LOAD_FLOAT(ini_section, RS("supply_desipation_gain"), &supply.desipation_gain);
						}

						driver.tension_max = driver.config.tension_max * PI / 180.f;
						driver.hook_gain = driver.config.torque_max / driver.tension_max;
						driver.dead_zone = driver.config.dead_zone * PI / 180.f;

						if (supply.enabled == 1){
							supply.max = supply.config.max * PI / 180.f;
							supply.min = supply.config.min * PI / 180.f;
							supply.tension_max = supply.config.tension_max * PI / 180.f;
							supply.gain = supply.config.torque_max / supply.tension_max;
						}

						return true;
					}
					void nonline::reconfig(void){
					}

					friction::friction(void) : nonline()
					{
					}
					void friction::reconfig(void){
					}
					bool friction::setup(void){
						ROBO_BREAKN(nonline::setup());
						ROBO_BREAKEN_LOAD_FLOAT(ini_section, RS("friction_rest"), &base.rest);
						ROBO_BREAKEN_LOAD_FLOAT(ini_section, RS("friction_dry"), &base.dry);
						ROBO_BREAKEN_LOAD_FLOAT(ini_section, RS("friction_tension_gain"), &base.tension_gain);
						ROBO_BREAKEN_LOAD_FLOAT(ini_section, RS("friction_crawl_speed_grad"), &base.crawl_speed_grad);
						base.crawl_speed = base.crawl_speed_grad * PI / 180.f*driver.gear_ratio;
						return true;
					}
					friction::result friction::run_(float _fric){
						if (fabs(actuator->speed) < crawl_speed){
							// мы в потенциальной яме
							if (torque.total > 0.f){
								if (torque.total < rest){
									//момент недостаточен, чтобы вылезти из ямы
									torque.friction = torque.total;
									return result::STOP;
								}
								else {
									torque.friction = rest;
									return result::START;
								}
							}
							else if (torque.total < 0.f) {
								if (torque.total > -rest){
									//момент недостаточен, чтобы вылезти из ямы
									torque.friction = torque.total;
									return result::STOP;
								}
								else {
									torque.friction = -rest;
									return result::START;
								}
							}
							else{
								torque.friction = 0.f;
							}
							return result::STOP;
						}
						else{
							//нормально катимся
							if (actuator->speed> 0.f){
								torque.friction = _fric;
							}
							else if (actuator->speed < 0.f) {
								torque.friction = -_fric;
							}
							else {
								torque.friction = 0.f;
							}
							return result::RUN;
						}
					}

					void friction::run(void){
						nonline::run();
                        scale = 1.f + (float)fabs(base.tension_gain*driver.tension);
						crawl_speed = base.crawl_speed * scale;
						rest = base.rest * scale;
						dry = base.dry * scale;
						torque.total = actuator->driveng_torque * driver.gear_ratio - driver.elastic_torque;

						switch (actuator->state){
						case iactuator::istate::blocked:
							switch (run_(rest)){
							case result::STOP:
								break;
							case result::START:
							case result::RUN:
								actuator->state	= iactuator::istate::start;
								break;
							}
							break;

						case iactuator::istate::start:
							switch (run_(rest)){
							case result::STOP:
								actuator->state = iactuator::istate::blocked;
								break;
							case result::START:
							case result::RUN:
								actuator->state = iactuator::istate::run;
								break;
							}
							break;
						case iactuator::istate::run:
							switch (run_(dry)){
							case result::STOP:
								actuator->state = iactuator::istate::blocked;
								break;
							case result::START:
								actuator->state = iactuator::istate::start;
								break;
							case result::RUN:
								break;
							}
							break;
						}
						actuator->contr_torque += torque.friction / driver.gear_ratio;
					}

					brake::brake(void)
						: friction()
						, set_time_us(0.f)
						, release_time_us(0.f)
						, set_time(0.f)
						, release_time(0.f)
						, set_dry(0.f)
						, release_dry(0.f)
						, set_rest(0.f)
						, release_rest(0.f)
						, fast_set(true)
						, fast_release(true)
						, release_beta(0.f)
						, release_gama(1.f)
						, set_beta(0.f)
						, set_gama(1.f)
					{

					}

					bool brake::setup(void){
						const float tau_scale = 4.6f;
						ROBO_BREAKN(friction::setup());
						ROBO_BREAKEN_LOAD_FLOAT(ini_section, RS("brake_friction_dry"), &set_dry);
						ROBO_BREAKEN_LOAD_FLOAT(ini_section, RS("brake_friction_rest"), &set_rest);
						ROBO_BREAKEN_LOAD_FLOAT(ini_section, RS("brake_set_time_us"), &set_time_us);
						ROBO_BREAKEN_LOAD_FLOAT(ini_section, RS("brake_release_time_us"), &release_time_us);
						release_dry = base.dry;
						base.dry = set_dry;
						release_rest = base.rest;
						base.rest = set_rest;
						set_time = set_time_us / 1000000.f;

						if (set_time < model_period_sec){
							fast_set = true;
							set_beta = 0.f;
							set_gama = 1.f;
						}
						else{
							fast_set = false;
							set_beta = expf(-model_period_sec / (set_time / tau_scale));
							set_gama = 1.f - set_beta;
						}
						
						release_time = release_time_us / 1000000.f;

						if (release_time < model_period_sec){
							fast_set = true;
							release_beta = 0.f;
							release_gama = 1.f;
						}
						else{
							fast_set = false;
                            release_beta = (float)exp(-model_period_sec / (release_time / tau_scale));
							release_gama = 1.f - release_beta;
						}
						return true;

					}
					void brake::reconfig(void){

					}
					void brake::run(void){
						if (state_ == state::release){
							if (fast_release){
								base.dry = release_dry;
								base.rest = release_rest;
							}
							else{
								base.dry = base.dry*release_beta + release_dry*release_gama;
								base.rest = base.rest*release_beta + release_rest*release_gama;
							}
						}
						else {
							if (fast_set){
								base.dry = set_dry;
								base.rest = set_rest;
							}
							else{
								base.dry = base.dry*set_beta + set_dry*set_gama;
								base.rest = base.rest*set_beta + set_rest*release_gama;
							}
						}
						friction::run();
					}
					void brake::set(void){
						state_ = state::set;
					}
					void brake::release(void){
						state_ = state::release;
					}
				}
			}
		}
	}
}
