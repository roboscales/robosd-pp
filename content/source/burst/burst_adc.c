#include "burst/burst_adc.h"
void adc_reset(adc_p _adc){
	BURST_ADC_ACC_TYPE * a = _adc->acc;
	for (int i = 0; i < BURST_ADC_CHANNEL_COUNT; ++i, ++a) {
		*a = 0;
	}
	_adc->init_count = 1 << _adc->config->init_count_shift;
	_adc->ready = burst_false;
}
void adc_begin(adc_p _adc,adc_config_p _config){
	_adc->config = _config;
	adc_reset(_adc);
}

void adc_update( adc_p _adc, const BURST_ADC_TYPE *  _raw){
	BURST_ADC_TYPE * n = _adc->native;
	const unsigned int* ix = _adc->config->index;
	for (int i = 0; i < BURST_ADC_CHANNEL_COUNT; ++i, ++n, ++ix) {
		*n = _raw[*ix];
	}
	if (_adc->ready) {
		burst_signal_t* v = _adc->values;
		BURST_ADC_TYPE* n = _adc->native;
		const burst_signal_t* s = _adc->config->scale;
		BURST_ADC_TYPE * o = _adc->offset;
		for (int i = 0; i < BURST_ADC_CHANNEL_COUNT; ++i, ++v, ++n, ++s, ++o) {
			*v =  (burst_signal_t) ((burst_long_signal_t) (*n - *o) * *s);
		}
	}
	else {
		BURST_ADC_ACC_TYPE* a = _adc->acc;
		BURST_ADC_TYPE * n = _adc->native;
		for (int i = 0; i < BURST_ADC_CHANNEL_COUNT; ++i, ++a, ++n) {
			*a += *n;
		}
		_adc->init_count--;
		if (_adc->init_count == 0) {
			burst_signal_t* v = _adc->values;
			BURST_ADC_TYPE* n = _adc->native;
			const burst_signal_t* s = _adc->config->scale;
			BURST_ADC_TYPE* o = _adc->offset;
			BURST_ADC_ACC_TYPE* a =_adc->acc;
			int shift = _adc->config->init_count_shift;
			for (int i = 0; i < BURST_ADC_CHANNEL_COUNT; ++i, ++v, ++n, ++s, ++o, ++a) {
				*o = (BURST_ADC_TYPE)((*a + (1 << (shift - 1))) >> shift)+1;
				*v =  (burst_signal_t) ((burst_long_signal_t) (*n - *o) * *s);
			}
			adc_reset(_adc);
			_adc->ready = burst_true;
		}
	}				
}
