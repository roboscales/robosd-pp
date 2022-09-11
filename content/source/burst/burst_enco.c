#include "burst/burst_enco.h"
void burst_enco_poll_(burst_enco_p _enco){
	burst_enco_config_p conf = _enco->config;
	if(conf == 0) return;
	if (conf->offset.native != _enco->offset.native || conf->offset.position != _enco->offset.position) {
		_enco->begin(conf);
		return;
	}
	_enco->counter.total++;
	_enco->native.raw = _enco->encode();
	_enco->query();
	if (_enco->start_pause_tick == 0) {
		if ( !_enco->error() ) {
			uint32_t tmp = _enco->native.raw << _enco->shift.raw;
			int32_t  tmp_delta = (int32_t)(tmp - _enco->native.ceiled);
			_enco->native.ceiled = tmp;
			
			//todo проверить на всехли компиляторах shift будет арифметический
			_enco->native.delta = (tmp_delta >> _enco->shift.raw);// (((native_t)(tmp_delta)) >> shift);
			burst_signal_t dtmp;
			if (conf->inverce) {
				dtmp = - _enco->native.delta >> _enco->shift.value;
			}
			else {
				dtmp =  _enco->native.delta >> _enco->shift.value;
			}
			
			if(dtmp > BURST_SIGNAL_MAX){
				_enco->delta = _enco->delta_acc = BURST_SIGNAL_MAX;
			} else if(dtmp < BURST_SIGNAL_MIN) {
				_enco->delta = _enco->delta_acc = BURST_SIGNAL_MIN;
			} else {
				_enco->delta = dtmp;
				burst_long_signal_t adtmp = _enco->delta_acc + _enco->delta;
				if(dtmp > BURST_SIGNAL_MAX){
					_enco->delta_acc = BURST_SIGNAL_MAX;
				} else if(dtmp < BURST_SIGNAL_MIN) {
					_enco->delta_acc = BURST_SIGNAL_MIN;
				}	else {						
					_enco->delta_acc = adtmp;
				}
			}
		}
		else {
			_enco->counter.fault++;
			//to do так делать нельзя, та как накапливается ошибка!
			_enco->native.delta = _enco->delta_acc = 0;
			/*present.native.raw += present.native.delta;
			present.native.ceiled += (present.native.delta << shift);
			present.delta_acc += present.delta;*/
		}
		if (conf->inverce) {
			_enco->acc -= _enco->native.delta;
		}
		else {
			_enco->acc += _enco->native.delta;
		}
		//todo round_l не катит
		_enco->position = _enco->acc >> _enco->shift.value;
		_enco->position += conf->offset.position;
	}
	else {
		if ( !_enco->error() )  {
			_enco->native.ceiled = _enco->native.raw << _enco->shift.raw;
			uint32_t tmp = _enco->native.ceiled + conf->offset.native;
			if (conf->inverce) {
				tmp = 0xFFFFFFFF - tmp;
			}
			_enco->acc = ((burst_long_signal_t)tmp)>>_enco->shift.raw;
			_enco->position = (_enco->acc >> _enco->shift.value );
			_enco->position += conf->offset.position;
			_enco->start_pause_tick--;
		}
		else {
				_enco->counter.fault++;
		}
	}
}
void burst_enco_begin_(burst_enco_p _enco,burst_enco_config_p _config){
	_enco->config = _config;
	_enco->shift.raw = (_config->resolution.round - _config->resolution.raw);
	_enco->shift.value = (_config->resolution.raw - _config->resolution.actual);
	_enco->counter.fault = 0;
	_enco->counter.total = 0;
	_enco->native.raw = 0;
	_enco->native.delta = 0;
	_enco->native.ceiled = 0;
	_enco->delta = 0;
	_enco->delta_acc = 0;
	_enco->acc = 0;
	_enco->position = 0;
	_enco->start_pause_tick = 1 <<  _config->init_count_shift;
	_enco->offset.position =		_config->offset.position;
	_enco->offset.native =		_config->offset.native;
	_enco->query();
}
