#include "burst/modules/enco_abs32.h"
void enco_abs32_run_(enco_abs32_p _enco){
	enco_abs32_config_p conf = (enco_abs32_config_p)_enco->ref.config;
	if(conf == 0) return;
	if (_enco->ref.ready && (conf->offset.native != _enco->offset.native || conf->offset.position != _enco->offset.position) ) {
		_enco->ref.reset();
		return;
	}
	_enco->ref.counter.total++;
	_enco->native.raw = _enco->encode();
	_enco->query();
	if (_enco->ref.ready) {
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
				_enco->delta = _enco->ref.delta_acc = BURST_SIGNAL_MAX;
			} else if(dtmp < BURST_SIGNAL_MIN) {
				_enco->delta = _enco->ref.delta_acc = BURST_SIGNAL_MIN;
			} else {
				_enco->delta = dtmp;
				burst_long_signal_t adtmp = _enco->ref.delta_acc + _enco->delta;
				if(dtmp > BURST_SIGNAL_MAX){
					_enco->ref.delta_acc = BURST_SIGNAL_MAX;
				} else if(dtmp < BURST_SIGNAL_MIN) {
					_enco->ref.delta_acc = BURST_SIGNAL_MIN;
				}	else {						
					_enco->ref.delta_acc = adtmp;
				}
			}
		}
		else {
			_enco->ref.counter.fault++;
			_enco->native.delta = _enco->ref.delta_acc = 0;
			//to do так делать нельзя, та как накапливается ошибка!
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
		_enco->ref.position = _enco->acc >> _enco->shift.value;
		_enco->ref.position += conf->offset.position;
	}
	else {
		if ( !_enco->error() )  {
			_enco->native.ceiled = _enco->native.raw << _enco->shift.raw;
			uint32_t tmp = _enco->native.ceiled + conf->offset.native;
			if (conf->inverce) {
				tmp = 0xFFFFFFFF - tmp;
			}
			_enco->acc = ((burst_long_signal_t)tmp)>>_enco->shift.raw;
			_enco->ref.position = (_enco->acc >> _enco->shift.value );
			_enco->ref.position += conf->offset.position;
			_enco->start_pause_tick--;
			if(_enco->start_pause_tick==0){
				_enco->ref.ready = burst_true;
			}
		}
		else {
				_enco->ref.counter.fault++;
		}
	}
}
void enco_abs32_begin(enco_abs32_p _enco,enco_abs32_config_p _config){
	_enco->ref.config = &(_config->ref);
	_enco->shift.raw = (_config->resolution.round - _config->resolution.raw);
	_enco->shift.value = (_config->resolution.raw - _config->resolution.actual);
	_enco->ref.counter.fault = 0;
	_enco->ref.counter.total = 0;
	_enco->native.raw = 0;
	_enco->native.delta = 0;
	_enco->native.ceiled = 0;
	_enco->delta = 0;
	_enco->ref.delta_acc = 0;
	_enco->acc = 0;
	_enco->ref.position = 0;
	_enco->offset.position =		_config->offset.position;
	_enco->offset.native =		_config->offset.native;
	_enco->query();
	enco_abs32_reset_(_enco);
}

void enco_abs32_reset_(enco_abs32_p _enco){
	enco_abs32_config_p conf = (enco_abs32_config_p)_enco->ref.config;
	_enco->start_pause_tick = 1 <<  conf->init_count_shift;
	_enco->ref.ready = burst_false;
	_enco->offset.position =		conf->offset.position;
	_enco->offset.native =		conf->offset.native;
}
