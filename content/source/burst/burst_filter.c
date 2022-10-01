#include "burst/burst_filter.h"
void burst_filter_begin_(burst_filter_p _filter, burst_signal_t _value){
	_filter->shift = _filter->config->shift;
	_filter->presc_shift = _filter->config->presc_shift+_filter->shift;
	_filter->value_shift = _filter->config->value_shift+_filter->shift;

	_filter->gain = (1<<_filter->config->shift)-1;
	_filter->value = _value;
	_filter->long_value = (_value<<_filter->config->shift);
}
void burst_filter_run_(burst_filter_p _filter){	
	burst_long_signal_t long_value= _filter->long_value;
	long_value = long_value  * _filter->gain + ((*_filter->input)<<(_filter->presc_shift));
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
	_filter->value =(burst_signal_t)long_value;
}
