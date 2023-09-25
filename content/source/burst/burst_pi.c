#include "burst/burst_pi.h"

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
			else {
				sut_flag =  burst_satstate_none;
			}
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


void burst_limiter_run(burst_limiter_p _limiter){
	burst_long_signal_t test_lim_control;
	burst_signal_t control_des = _limiter->control_des;
    burst_signal_t  control_req = *(_limiter->control_req);
    _limiter->signal_hi = *(_limiter->signal) - *(_limiter->signalMax);
    _limiter->signal_low = *(_limiter->signal) - *(_limiter->signalMin);
    burst_pi_run_( _limiter->r_hi);
    burst_pi_run_( _limiter->r_low);

    if( control_des < control_req){
		control_des = s_inc(control_des, *(_limiter-> ramp), *(_limiter->controlMin), control_req);
    } else {
    	if( control_des > control_req){
				control_des = s_inc(control_des, -*(_limiter->ramp), control_req, *(_limiter->controlMax));
      }
    }


    test_lim_control =  _limiter -> control_hi +  _limiter -> control_low + control_des;
    _limiter->control_des = control_des;
    
    
    if(test_lim_control > *(_limiter->controlMax)){
       *(_limiter->control_val) =  *(_limiter->controlMax);
        _limiter ->sut_flag = burst_satstate_hi;
    } else if(test_lim_control < *(_limiter->controlMin)){
        *(_limiter->control_val) =  *(_limiter->controlMin);        
        _limiter ->sut_flag = burst_satstate_lo;
    } else {
        _limiter ->sut_flag = burst_satstate_none;
        *(_limiter->control_val) =  test_lim_control;                
    }
}

void burst_limiter_reset(burst_limiter_p _limiter, burst_signal_t _def){
	burst_pi_reset_(_limiter->r_hi,_def);
	burst_pi_reset_(_limiter->r_low,_def);
	*(_limiter->control_val)=_def;
}
void burst_limiter_setup(burst_limiter_p _limiter, burst_limiter_config_p _config, burst_signal_t _def ){
	_limiter->r_hi = _config->r_hi;
	_limiter->r_low = _config->r_lo;

	_limiter->r_hi->config = _config->reg_config;
	_limiter->r_hi->config = _config->reg_config;
	_limiter->r_hi->controlMin =  _config->controlMin; 
	_limiter->r_hi->controlMax = &(_limiter->zero_signal);
	_limiter->r_hi->signal = &(_limiter -> signal_hi);
	_limiter->r_hi->signal_req = &(_limiter->zero_signal);
	_limiter->r_hi->control = &(_limiter -> control_hi);
	_limiter->r_hi->master_sut_flag = &(_limiter ->sut_flag);

	_limiter->r_low->config = _config->reg_config;
	_limiter->r_low->controlMin =  &(_limiter->zero_signal); 
	_limiter->r_low->controlMax = _config->controlMax;
	_limiter->r_low->signal = &(_limiter -> signal_low);
	_limiter->r_low->signal_req = &(_limiter->zero_signal);
	_limiter->r_low->control = &(_limiter -> control_low);
	_limiter->r_low->master_sut_flag = &(_limiter ->sut_flag);

	_limiter->control_req = _config->control_req;
	_limiter->control_val=  _config->control_val;
	_limiter->signal=  _config->signal;
	_limiter->controlMax = _config->controlMax;
	_limiter->controlMin = _config->controlMin;
	_limiter->signalMin = _config->signalMin;
	_limiter->signalMax = _config->signalMax;

	_limiter->ramp = &(_config->reg_config->ramp);
	
	burst_limiter_reset(_limiter, _def);
}
