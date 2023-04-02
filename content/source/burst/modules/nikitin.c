#include "burst/modules/nikitin.h"
void nikitin_begin(nikitin_p _filter, nikitin_config_p _config ){
	_filter->ref.config = &(_config->ref);
	_filter->shift = _config->shift;
	_filter->presc_shift = _config->presc_shift+_filter->shift;
	_filter->value_shift = _config->value_shift+_filter->shift;
	_filter->gain = (1<<_config->shift)-1;
	_filter->ref.reset();
}
void nikitin_run_(nikitin_p _filter){	
	burst_long_signal_t long_value= _filter->long_value;
	long_value = long_value  * _filter->gain + ((*_filter->ref.input)<<(_filter->presc_shift));
	if(long_value<0){
		long_value = -((-long_value) >> _filter->shift);
	} else {
		long_value = long_value >> _filter->shift;
	}
	_filter->long_value=long_value;
	if(long_value<0){
		long_value = -((-long_value) >> _filter->value_shift);
	} else {
		long_value = long_value >> _filter->value_shift;
	}
	BURST_SATURATE(long_value, BURST_SIGNAL_MIN, BURST_SIGNAL_MAX);
	_filter->ref.value =(burst_signal_t)long_value;
}

void nikitin_reset_(nikitin_p _filter){	
	nikitin_config_p cfg = (nikitin_config_p)(_filter->ref.config);
	burst_signal_t r = *_filter->ref.input;
	_filter->ref.value = r;
	_filter->long_value = (r << cfg->shift);
}
