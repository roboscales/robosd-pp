#include "burst/burst_positioner.h"
void burst_positioner_begin_(burst_posicioner_p _posicioner){
	burst_positioner_reset_(_posicioner);
}
#ifndef  BURTS_POSITINER_MAX_ERR
#define BURTS_POSITINER_MAX_ERR BURST_SIGNAL_T(0.9)
#endif
void burst_positioner_run_(burst_posicioner_p _posicioner){
	burst_positioner_config_p s = _posicioner->config;
	burst_long_signal_t err = *(_posicioner->signal_req) - *(_posicioner->signal);
	burst_signal_t deadZone = s->deadZone;
	if (err==0){
			*(_posicioner->control)=0;
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
			}
			if(err < -s->deadZone){
					if (err<-BURTS_POSITINER_MAX_ERR){
							err = -BURTS_POSITINER_MAX_ERR;
					} else {
							if(s->crawlSpeed == 0) {
									err += deadZone;
							}
					}
			}
	}
	{        
		burst_long_signal_t control_val = err*(s->propGain);
		burst_signal_t controlMin = *(_posicioner->controlMin);
		burst_signal_t controlMax = *(_posicioner->controlMax);
		if(_posicioner->signal_diff){
			control_val -= (*(_posicioner->signal_diff))*(s->diffGain);
		}
		control_val = BURST_RIGHT_SHIFT(control_val, s->controlShift);
		if (_posicioner->forceControl) control_val += *(_posicioner->forceControl);
		BURST_SATURATE(control_val, controlMin, controlMax);
		if(control_val == 0 && s->crawlSpeed >0 ){
			if(err>s->deadZone){
				control_val =   s->crawlSpeed;				
			} else if( err < -s->deadZone) {
				control_val =   -s->crawlSpeed;				
			}
		}
		*(_posicioner->control) = control_val;
	}
}

void burst_positioner_reset_(burst_posicioner_p _motion){
}
