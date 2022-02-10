#include "im/models/gearbox.hpp"
#include "core/robosd_ini.hpp"

#include <math.h>
namespace robo{
	namespace edev{
		namespace joint {
			namespace gearbox {
				namespace elastic {
					ideal::ideal(agent& _agent, cstr _name) : joint::link(_agent,_name) {}
					void ideal::do_run(void) {
						observer.position = actuator->position / gear_ratio;
						observer.speed = actuator->speed / gear_ratio;
						observer.tension_diff = observer.speed - load->speed;
						observer.tension = (float)(observer.position - load->position + observer.tension_diff * owner.sample_time / 2);

						observer.elastic_torque = observer.tension * hook_gain;

						actuator->contr_torque = observer.elastic_torque / gear_ratio;
						load->driveng_torque = observer.elastic_torque - load->speed * load_viscous_gain;
						load->driveng_position = observer.position;
						load->driveng_speed = observer.speed;
						actuator->state = joint::iactuator::istate::run;
					}
					bool ideal::do_load(cstr _specific_sect, cstr _common_sect) {
						ROBO_LBREAKN(link::do_load(_specific_sect, _common_sect));
						ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("hook_gain"), hook_gain));
						ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("load_viscous_gain"), load_viscous_gain));
						ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("gear_ratio"), gear_ratio));
						return true;
					}
					void ideal::do_reconfig(void) {
					}

					nonline::nonline(agent& _agent, cstr _name) : link(_agent,_name) {
						actuator->state = joint::iactuator::istate::run;
					}
					void nonline::do_run(void) {
						driver.speed = actuator->speed / driver.gear_ratio;
						if (supply.enabled) {
							if (load->position >= supply.max) {
								supply.tension = (float)(load->position - supply.max);
								if (supply.tension > supply.tension_max) {
									load->position = supply.max + supply.tension_max;
									if (load->speed > 0.f) {
										load->speed = 0.f;
									}
								}
							}
							else {
								if (load->position <= supply.min) {
									supply.tension = (float)(load->position - supply.min);
									if (supply.tension < -supply.tension_max) {
										load->position = supply.min - supply.tension_max;
										if (load->speed < 0.f) {
											load->speed = 0.f;
										}
									}
								}
								else {
									supply.tension = 0.f;
								}
							}

							if (fabs(supply.tension) > 0.000001f) {
								supply.torque = supply.gain * supply.tension + supply.desipation_gain * driver.speed;
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

						driver.tension = (float)(load->driveng_position - load->position + driver.tension_diff * owner.sample_time / 2);
						if (driver.tension >= driver.dead_zone) {
							driver.tension -= driver.dead_zone;
						}
						else {
							if (driver.tension <= -driver.dead_zone) {
								driver.tension += driver.dead_zone;
							}
							else {
								driver.tension = 0.f;
							}
						}

						driver.position = actuator->position / driver.gear_ratio;

						driver.elastic_torque = (driver.tension  + driver.tension_diff * driver.dissipation_gain) * driver.hook_gain;

						actuator->contr_torque = (driver.elastic_torque + supply.torque   ) / driver.gear_ratio;
						load->driveng_position = driver.position;
						load->driveng_speed = driver.speed;
						load->driveng_torque = driver.elastic_torque - supply.torque -  load->speed * driver.load_viscous_gain;
						load->tension = driver.tension;
						load->tension_diff = driver.tension_diff;
					}

					bool nonline::do_load(cstr _specific_sect, cstr _common_sect) {
						ROBO_LBREAKN(link::do_load(_specific_sect, _common_sect));
						ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("driver_tension_max"), driver.config.tension_max));
						ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("driver_torque_max"), driver.config.torque_max));
						ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("driver_dead_zone"), driver.config.dead_zone));
						ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("load_viscous_gain"), driver.load_viscous_gain));
						ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("driver_gear_ratio"), driver.gear_ratio));
						ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("supply_enabled"), supply.enabled));
						ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("driver_dissipation_gain"), driver.dissipation_gain));
						

						if (supply.enabled == 1) {
							ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("supply_min"), supply.config.min));
							ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("supply_max"), supply.config.max));
							ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("supply_tension_max"), supply.config.tension_max));
							ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("supply_torque_max"), supply.config.torque_max));
							ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("supply_desipation_gain"), supply.desipation_gain));
						}
						return true;
					}
					void nonline::do_reconfig(void) {
						driver.tension_max = driver.config.tension_max * grad2rad<float>;
						if (driver.tension_max > 1e-6) {
							driver.hook_gain = driver.config.torque_max / driver.tension_max;
						}
						else {
							driver.hook_gain = 0;
						}
						driver.dead_zone = driver.config.dead_zone * grad2rad<float>;
						if (supply.enabled == 1) {
							supply.max = supply.config.max * grad2rad<float>;
							supply.min = supply.config.min * grad2rad<float>;
							supply.tension_max = supply.config.tension_max * grad2rad<float>;
							supply.gain = supply.config.torque_max / supply.tension_max;
						}

					}

					friction::friction(agent& _agent, cstr _name) : nonline(_agent,_name) {}
					void friction::do_reconfig(void) {
						nonline::do_reconfig();
						base.crawl_speed = base.crawl_speed_grad * grad2rad<float> * driver.gear_ratio;
					}

					bool friction::do_load(cstr _specific_sect, cstr _common_sect) {
						ROBO_LBREAKN(nonline::do_load(_specific_sect, _common_sect));
						ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("friction_rest"), base.rest));
						ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("friction_dry"), base.dry));
						ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("friction_tension_gain"), base.tension_gain));
						ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("friction_crawl_speed_grad"), base.crawl_speed_grad));
						return true;
					}
					friction::result friction::run_(float _fric) {
						if (fabs(actuator->speed) < crawl_speed) {
							// мы в потенциальной яме
							if (torque.total > 0.f) {
								if (torque.total < rest) {
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
								if (torque.total > -rest) {
									//момент недостаточен, чтобы вылезти из ямы
									torque.friction = torque.total;
									return result::STOP;
								}
								else {
									torque.friction = -rest;
									return result::START;
								}
							}
							else {
								torque.friction = 0.f;
							}
							return result::STOP;
						}
						else {
							//нормально катимся
							if (actuator->speed > 0.f) {
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

					void friction::do_run(void) {
						nonline::do_run();
						scale = 1.f + (float)fabs(base.tension_gain * driver.tension);
						crawl_speed = base.crawl_speed * scale;
						rest = base.rest * scale;
						dry = base.dry * scale;
						torque.total = actuator->driveng_torque * driver.gear_ratio - driver.elastic_torque;

						switch (actuator->state) {
						case iactuator::istate::blocked:
						switch (run_(rest)) {
						case result::STOP:
						break;
						case result::START:
						case result::RUN:
						actuator->state = iactuator::istate::start;
						break;
						}
						break;

						case iactuator::istate::start:
						switch (run_(rest)) {
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
						switch (run_(dry)) {
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

					brake::brake(agent& _agent, cstr _name)
						: friction(_agent,_name)
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
						, set_gama(1.f) {

					}

					bool brake::do_load(cstr _specific_sect, cstr _common_sect) {
						ROBO_LBREAKN(friction::do_load(_specific_sect, _common_sect));
						ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("brake_friction_dry"), set_dry));
						ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("brake_friction_rest"), set_rest));
						ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("brake_set_time_us"), set_time_us));
						ROBO_LBREAKN(ini::load(_specific_sect, _common_sect, RT("brake_release_time_us"), release_time_us));
						return true;
					}

					void brake::do_reconfig(void) {
						friction::do_reconfig();
						const float tau_scale = 4.6f;
						release_dry = base.dry;
						base.dry = set_dry;
						release_rest = base.rest;
						base.rest = set_rest;
						set_time = set_time_us / 1000000.f;

						if (set_time < owner.sample_time) {
							fast_set = true;
							set_beta = 0.f;
							set_gama = 1.f;
						}
						else {
							fast_set = false;
							set_beta = expf((float)(-owner.sample_time / (set_time / tau_scale)));
							set_gama = 1.f - set_beta;
						}

						release_time = release_time_us / 1000000.f;

						if (release_time < owner.sample_time) {
							fast_set = true;
							release_beta = 0.f;
							release_gama = 1.f;
						}
						else {
							fast_set = false;
							release_beta = (float)exp((float)(-owner.sample_time / (release_time / tau_scale)));
							release_gama = 1.f - release_beta;
						}
					}
					void brake::do_run(void) {
						if (state_ == state::release) {
							if (fast_release) {
								base.dry = release_dry;
								base.rest = release_rest;
							}
							else {
								base.dry = base.dry * release_beta + release_dry * release_gama;
								base.rest = base.rest * release_beta + release_rest * release_gama;
							}
						}
						else {
							if (fast_set) {
								base.dry = set_dry;
								base.rest = set_rest;
							}
							else {
								base.dry = base.dry * set_beta + set_dry * set_gama;
								base.rest = base.rest * set_beta + set_rest * release_gama;
							}
						}
						friction::do_run();
					}
					void brake::set(void) {
						state_ = state::set;
					}
					void brake::release(void) {
						state_ = state::release;
					}
				}
			}
		}
	}
}
