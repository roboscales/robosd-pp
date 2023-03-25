#include "burst/burst_pi.h"
void burst_pi_begin_(burst_pi_p _pi, burst_signal_t _start_control){
	//todo
	burst_pi_reset_(_pi,_start_control);
}
void burst_pi_run_(burst_pi_p _pi){
  burst_pi_config_p s =_pi->config;
	burst_long_signal_t Error;
	burst_long_signal_t tmp;
	burst_long_signal_t controlLong;

	burst_signal_t signal = *(_pi->signal);
	burst_signal_t control = *(_pi->control);

	burst_signal_t controlMax = *(_pi->controlMax);
	burst_signal_t controlMin = *(_pi->controlMin);
	burst_satstate_t master_sut_flag = *(_pi->master_sut_flag);
	burst_satstate_t sut_flag;
	burst_signal_t signal_req = *(_pi->signal_req);

	#ifndef MODEL_VALUE_MAX
	#define MODEL_VALUE_MAX 0.9
	#endif

 	Error = signal_req - signal;
	if( (Error>0) && ( (control>=controlMax) || (_pi->long_model>(BURST_LONG_SIGNAL_MAX*MODEL_VALUE_MAX)) || (master_sut_flag == burst_satstate_hi) ) ){
			sut_flag = burst_satstate_hi;
	} else {
			if( (Error<0) && ( (control<=controlMin) || (_pi->long_model<(BURST_LONG_SIGNAL_MIN*MODEL_VALUE_MAX)) || (master_sut_flag == burst_satstate_lo)  ) ){
					sut_flag = burst_satstate_lo;
			}
			else sut_flag =  master_sut_flag;
	}
	_pi->satstate = sut_flag;
	if ( sut_flag == burst_satstate_none )
	{
		burst_long_signal_t tmp;
		tmp = (_pi->long_model += (Error* s->modelGain) );
		tmp = BURST_RIGHT_SHIFT(tmp, s->modelShift);
		BURST_SATURATE(tmp, BURST_SIGNAL_MIN, BURST_SIGNAL_MAX);
		_pi->model = tmp;
		
  }
	
 	tmp = Error+ _pi->model - signal;
	

	controlLong = tmp* (s->propGain)  ;

  if (_pi->signal_force != 0) {
		burst_long_signal_t force = *(_pi->signal_force) * (s->forceGain);
		controlLong += force;
		_pi->force = force;
	}

  if (_pi->signal_diff != 0) {
		burst_long_signal_t diff = *(_pi->signal_diff) * (s->diffGain);
		controlLong -= diff;
		_pi->diff = diff;
	}

	controlLong = BURST_RIGHT_SHIFT(controlLong, s->controlShift);

	BURST_SATURATE(controlLong, controlMin, controlMax);
	control = BURST_LONG_TO_SIGNAL(controlLong);
  *(_pi->control) = control;
}

void burst_pi_reset_(burst_pi_p _pi, burst_signal_t _start_control){
	//todo _start_control
	_pi->model = 0;
	_pi->force = 0;
	_pi->long_model=0;
	*(_pi->control)=0;
	_pi->satstate=burst_satstate_both;
}
