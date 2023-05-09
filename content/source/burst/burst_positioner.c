#include "burst/burst_positioner.h"
#ifndef  BURTS_POSITINER_MAX_ERR
#define BURTS_POSITINER_MAX_ERR BURST_SIGNAL_T(0.9)
#endif
void burst_positioner_run_(burst_positioner_p _positioner){
	burst_positioner_config_p s = _positioner->config;
	burst_long_signal_t err = *(_positioner->signal_req) - *(_positioner->signal);
	burst_signal_t deadZone = s->deadZone;
	if (err==0){
			if (_positioner->forceControl) {
				*(_positioner->control)= *(_positioner->forceControl);
			} else{
				*(_positioner->control)= 0;
			}
			return;
	}else{
			if(err > s->deadZone){
					if (err>BURTS_POSITINER_MAX_ERR){
							err = BURTS_POSITINER_MAX_ERR;
					} else {
							if(s->crawlSpeed == 0) {
									err -= deadZone;
							}
					}
			} else if(err > 0){
				if (_positioner->forceControl) {
					*(_positioner->control)= *(_positioner->forceControl);
				} else{
					*(_positioner->control)= 0;
				}
				return;
			}
			if(err < -s->deadZone){
					if (err<-BURTS_POSITINER_MAX_ERR){
							err = -BURTS_POSITINER_MAX_ERR;
					} else {
							if(s->crawlSpeed == 0) {
									err += deadZone;
							}
					}
			} else if(err < 0){
				if (_positioner->forceControl) {
					*(_positioner->control)= *(_positioner->forceControl);
				} else{
					*(_positioner->control)= 0;
				}
				return;
			}
	}
	{        
		burst_long_signal_t control_val = err*(s->propGain);
		burst_signal_t controlMin = *(_positioner->controlMin);
		burst_signal_t controlMax = *(_positioner->controlMax);
		if(_positioner->signal_diff){
			control_val -= (*(_positioner->signal_diff))*(s->diffGain);
		}
		control_val = BURST_RIGHT_SHIFT(control_val, s->controlShift);
		if (_positioner->forceControl) control_val += *(_positioner->forceControl);
		BURST_SATURATE(control_val, controlMin, controlMax);
		if(control_val == 0 && s->crawlSpeed >0 ){
			if(err>s->deadZone){
				control_val =   s->crawlSpeed;				
			} else if( err < -s->deadZone) {
				control_val =   -s->crawlSpeed;				
			}
		}
		*(_positioner->control) = control_val;
	}

}

void burst_positioner_reset_(burst_positioner_p _motion){
}

void burst_positioner_setup_(
	burst_positioner_p _positioner
	,	burst_long_signal_p				_signal_req
	, burst_long_signal_p				_signal
	, burst_signal_p				_signal_diff
	,	burst_signal_p				_forceControl
	, burst_signal_p			  _control
	,	burst_signal_p				_controlMax
	, burst_signal_p				_controlMin
	, burst_positioner_config_p _config
){ 
	_positioner->config = _config;
	_positioner->signal_req = _signal_req;
	_positioner->signal = _signal;
	_positioner->signal_diff = _signal_diff;
	_positioner->forceControl = _forceControl;
	_positioner->control = _control;
	_positioner->controlMax = _controlMax;
	_positioner->controlMin = _controlMin;
	burst_positioner_reset_(_positioner);

}
