#include "burst/burst_motion.h"
void burst_motion_begin(burst_motion_p _motion,burst_motion_config_p _config){
	_motion->config = _config;
}
void burst_motion_run_(burst_motion_p _motion){
  burst_motion_config_p s =_motion->config;
	burst_long_signal_t Error;
	burst_long_signal_t tmp;
	burst_long_signal_t controlLong;

	burst_signal_t signal = *(_motion->signal);
	burst_signal_t control = *(_motion->control);
	burst_signal_t force = _motion->force;

	burst_signal_t controlMax = *(_motion->controlMax);
	burst_signal_t controlMin = *(_motion->controlMin);
	burst_satstate_t master_sut_flag = *(_motion->master_sut_flag);
	burst_satstate_t sut_flag;
	burst_signal_t signal_req = *(_motion->signal_req);
  if (_motion->reference != 0) {
		burst_long_signal_t reference_max = *(_motion->reference_max);
		burst_long_signal_t reference_min = *(_motion->reference_min);
		burst_long_signal_t reference = *(_motion->reference);
	
		#if BURST_MOTION_VIRTUAL_ELASTIC_ENABLED == 1
		if (reference > reference_max) {
				mexo_long_signal_t tmp = (reference_max - reference);
				tmp = MEXO_RIGHT_SHIFT(tmp, s->limitGainPresc);
				tmp *= (s->limitGain);
				
				if (tmp < 0) {
						tmp = 0;
				}
				if (signal_req > tmp) {
						signal_req = tmp;
				}            
		}
		else {
				if (reference < reference_min) {
						mexo_long_signal_t tmp = (reference_min - reference);
						tmp = MEXO_RIGHT_SHIFT(tmp, s->limitGainPresc);
						tmp *= (s->limitGain);
						if (tmp > 0) {
								tmp = 0;
						}
						if (signal_req < tmp) {
								signal_req = tmp;
						}
				}
		}
		#else
		if (reference > reference_max) {
			burst_long_signal_t tmp = (reference_max - reference);
			tmp = BURST_RIGHT_SHIFT(tmp, s->limitGainPresc);
			tmp *= (s->limitGain);
			controlMax = controlMax + BURST_RIGHT_SHIFT(tmp, s->controlShift);
			if (controlMax < 0) {
					controlMax = 0;
			}
		}
		else {
			if (reference < reference_min) {
				burst_long_signal_t tmp = (reference_min - reference);
				tmp = BURST_RIGHT_SHIFT(tmp, s->limitGainPresc);
				tmp *= (s->limitGain);
				controlMin = controlMin + BURST_RIGHT_SHIFT(tmp, s->controlShift);
				if (controlMin > 0)
						controlMin = 0;
			}
		}
		#endif
	}

	#ifndef MODEL_VALUE_MAX
	#define MODEL_VALUE_MAX 0.9
	#endif

 	Error = signal_req - signal;
	if( (Error>0) && ( (control>=controlMax) || (_motion->long_model>(BURST_LONG_SIGNAL_MAX*MODEL_VALUE_MAX)) || (master_sut_flag == burst_satstate_hi) ) ){
			sut_flag = burst_satstate_hi;
	} else {
			if( (Error<0) && ( (control<=controlMin) || (_motion->long_model<(BURST_LONG_SIGNAL_MIN*MODEL_VALUE_MAX)) || (master_sut_flag == burst_satstate_lo)  ) ){
					sut_flag = burst_satstate_lo;
			}
			else sut_flag =  master_sut_flag;
	}
	_motion->satstate = sut_flag;
	if ( sut_flag == burst_satstate_none )
	{
		burst_long_signal_t tmp;
		tmp = (_motion->long_model += (Error* s->modelGain) );
		tmp = BURST_RIGHT_SHIFT(tmp, s->modelShift);
		BURST_SATURATE(tmp, BURST_SIGNAL_MIN, BURST_SIGNAL_MAX);
		_motion->model = tmp;

		if (  signal ==0 ){
			if(Error>0){
					burst_parametr_t fm = s->forceMax;
					force+=s->forceGain;
					if(force>fm)
							force=fm;
			}else if (Error<0){
					burst_parametr_t fm = -s->forceMax;
					force-=s->forceGain;
					if(force<fm)
							force=fm;
			}
		}else {
			force = 0;
		}
		_motion->force = force;
  }
	
 	tmp = Error+ _motion->model - signal;

	controlLong = tmp* (s->propGain)  ;
	controlLong = BURST_RIGHT_SHIFT(controlLong, s->controlShift) + force;

	BURST_SATURATE(controlLong, controlMin, controlMax);
	control = BURST_LONG_TO_SIGNAL(controlLong);
  *(_motion->control) = control;
}

void burst_motion_reset_(burst_motion_p _motion, burst_signal_t _start_control){
	//todo _start_control
	_motion->model = 0;
	_motion->force = 0;
	_motion->long_model=0;
	*(_motion->control)=0;
	_motion->satstate=burst_satstate_both;
}


void burst_motion_setup_(
	  burst_motion_p 				_motion
	,	burst_signal_p				_signal_req
	, burst_signal_p				_signal
	, burst_signal_p			  _control
	, burst_signal_t 				_start_control
	, burst_satstate_t *		_master_sut_flag
	,	burst_signal_p				_controlMax
	, burst_signal_p				_controlMin
	, burst_signal_p				_signal_diff
	, burst_long_signal_p 	_reference
	, burst_long_signal_p 	_reference_max
	, burst_long_signal_p 	_reference_min
){
	_motion->signal_req = _signal_req;
	_motion->signal = _signal;
	_motion->control = _control;
	_motion->master_sut_flag = _master_sut_flag;
	_motion->controlMax = _controlMax;
	_motion->controlMin = _controlMin;
	_motion->signal_diff = _signal_diff;
	_motion->reference = _reference;
	_motion->reference_max = _reference_max;
	_motion->reference_min = _reference_min;
	burst_motion_reset_(_motion,_start_control);
}