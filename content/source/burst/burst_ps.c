#include "burst/burst_ps.h"
burst_bool_t burst_ps_active_(burst_ps_p _ps){
	return (burst_bool_t)(_ps->status == burst_ps_status_on); 
}
void burst_ps_begin_(burst_ps_p _ps, burst_ps_config_p _config){
	_ps->config = _config;
	_ps->status = burst_ps_status_off;
}
void burst_ps_poll_(burst_ps_p _ps){
	switch (_ps->status) {
		case burst_ps_status_unknown:
			break;
		case burst_ps_status_off:
			if (_ps->command == burst_ps_command_on) {
				_ps->boot_begin();
				_ps->status = burst_ps_status_boot;
			}		else {
				break;
			}

		case burst_ps_status_boot:
			if (_ps->do_boot()) {
				_ps->satstate = burst_satstate_none;
				_ps->boot_complete(/*present.inverter.duty*/);
				_ps->satstate =_ps->do_invert();
				_ps->status = burst_ps_status_on;
				return;
			}	else {
				break;
			}
		case burst_ps_status_on:
			if (_ps->command == burst_ps_command_on) {
				_ps->satstate =_ps->do_invert();
				return;
			}	else {
				_ps->satstate = burst_satstate_both;
				_ps->status = burst_ps_status_shutdown;
				_ps->shutdown_begin();
			}
		case burst_ps_status_shutdown:
			if (_ps->do_shutdown()) {
				_ps->shutdown_complete();
				_ps->status = burst_ps_status_off;
			} else {
					break;
			}
	}
}

void burst_ps_dc_begin_(burst_ps_dc_p _dc,burst_ps_dc_config_p _config){
	_dc->ps.begin(&(_config->ps));
	_dc->required = &_dc->standalone;
	burst_scaler_begin(&(_dc->scaler), &(_config->voltage),&(_config->duty));
}


burst_satstate_t burst_ps_dc_do_invert_(burst_ps_dc_p _dc){
	burst_satstate_t tmp = burst_scaler_run(&(_dc->scaler),*_dc->required,&(_dc->duty) );	
	//_dc->set(_dc->duty);
	return tmp;
}
