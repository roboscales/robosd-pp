#include "burst/modules/ps_dc.h"

void ps_dc_begin(ps_dc_p _dc,ps_dc_config_p _config){
	burst_ps_begin(&(_dc->machine), &(_config->machine));
	burst_scaler_begin(&(_dc->scaler), &(_config->voltage),&(_config->duty));
}


burst_satstate_t ps_dc_do_invert_(ps_dc_p _dc){
	burst_satstate_t tmp = burst_scaler_run(&(_dc->scaler),*_dc->required,&(_dc->duty) );	
	_dc->set(_dc->duty);
	return tmp;
}

void ps_dc_setup_(ps_dc_p _dc, burst_signal_p _required){
	static burst_signal_t standalone = 0;
	_dc->required  = _required ? _required :  &standalone;
}
