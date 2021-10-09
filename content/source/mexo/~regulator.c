#include "mexo/mexo_regulator.h"
#include "mexo/mexo_signal.h"

void regulator_reset(regulator_p p){
	p->model = 0;
	p->long_model = 0;
	*(p->control_val)=0;
    p->sut_flag = REGULATOR_SAT_NONE;
}

void regulator_run(regulator_p r){
    regulator_settings_p s =r->settings;
	mexo_long_signal_t Error;
	mexo_long_signal_t tmp;
	mexo_long_signal_t controlLong;

//  	mexo_signal_t signal_req = *(r->signal_req);
	mexo_signal_t signal = *(r->signal);
	mexo_signal_t control_val ;
//	mexo_signal_t control_lim ;
	mexo_signal_t controlMax = *(r->controlMax);
	mexo_signal_t controlMin = *(r->controlMin);
	
	regulator_sat_flag_t master_sut_flag = r->master_sut_flag? *(r->master_sut_flag) : REGULATOR_SAT_NONE;
	regulator_sat_flag_t sut_flag;


 	Error = *(r->signal_req) - signal;
 	tmp = Error+ r->model - signal;

	controlLong = tmp* (s->propGain) ;

	if(r->signal_diff){
		mexo_long_signal_t controlDiff= -((mexo_long_signal_t)(*r->signal_diff))*(s->diffGain) ;
		r->control_diff = controlDiff;	
		controlLong	+=controlDiff;
	}

	controlLong = MEXO_RIGHT_SHIFT(controlLong,s->controlShift);
	MEXO_SATURATE(controlLong, controlMin, controlMax);
	control_val = MEXO_LONG_TO_SIGNAL(controlLong);
    *(r->control_val) = control_val;

	if ((Error>0) && ((control_val >= controlMax) || (r->long_model>(MEXO_LONG_SIGNAL_MAX*MODEL_VALUE_MAX)) || (master_sut_flag == REGULATOR_SAT_POS))){
        sut_flag = REGULATOR_SAT_POS;
    } else {
		if ((Error<0) && ((control_val <= controlMin) || (r->long_model<(MEXO_LONG_SIGNAL_MIN*MODEL_VALUE_MAX)) || (master_sut_flag == REGULATOR_SAT_NEG))){
            sut_flag = REGULATOR_SAT_NEG;
        }
        else sut_flag =  REGULATOR_SAT_NONE;
    }
    r->sut_flag = sut_flag;
	if ( sut_flag == REGULATOR_SAT_NONE )
	{
		mexo_long_signal_t tmp;
		tmp = (r->long_model += (Error* s->modelGain) );
		tmp = MEXO_RIGHT_SHIFT(tmp, s->modelShift);
		r->model = tmp;
	}
}


void motion_reg_reset(motion_reg_p p){
	p->model = 0;
	p->long_model = 0;
	*(p->control_val) = 0;
    p->force = 0;
    p->sut_flag = REGULATOR_SAT_NONE;
}

void  motion_reg_run( motion_reg_p r){
     motion_reg_settings_p s =r->settings;
	mexo_long_signal_t Error;
	mexo_long_signal_t tmp;
	mexo_long_signal_t controlLong;

//  	mexo_signal_t signal_req = *(r->signal_req);
	mexo_signal_t signal = *(r->signal);
	mexo_signal_t control_val = *(r->control_val);
	mexo_signal_t force = r->force;
//	mexo_signal_t control_lim ;
	mexo_signal_t controlMax = *(r->controlMax);
	mexo_signal_t controlMin = *(r->controlMin);
	regulator_sat_flag_t master_sut_flag = r->master_sut_flag? *(r->master_sut_flag) : REGULATOR_SAT_NONE;
	regulator_sat_flag_t sut_flag;


 	Error = *(r->signal_req) - signal;

    if (r->reference != 0){
    	mexo_long_signal_t lim_max = r->limit_max;
    	mexo_long_signal_t lim_min = r->limit_min;
    	mexo_long_signal_t reference =  *(r->reference);
    	if(reference>lim_max){
	    	mexo_long_signal_t tmp = (lim_max-reference);
			tmp = MEXO_RIGHT_SHIFT(tmp, s->limitGainPresc);
			tmp*=(s->limitGain);
			controlMax = controlMax + MEXO_RIGHT_SHIFT(tmp, s->controlShift);
    		if (controlMax<0){
	    		controlMax = 0;
		    }
    	}else{
	    	if(reference<lim_min){
				mexo_long_signal_t tmp = (lim_max-reference);
				tmp = MEXO_RIGHT_SHIFT(tmp, s->limitGainPresc);
				tmp*=(s->limitGain);
				controlMin = controlMin + MEXO_RIGHT_SHIFT(tmp, s->controlShift);
    			if (controlMin>0)
	    			controlMin = 0;
		    }
    	}
    }


    if( (Error>0) && ( (control_val>=controlMax) || (r->long_model>(MEXO_LONG_SIGNAL_MAX*MODEL_VALUE_MAX)) || (master_sut_flag == REGULATOR_SAT_POS) ) ){
        sut_flag = REGULATOR_SAT_POS;
    } else {
        if( (Error<0) && ( (control_val<=controlMin) || (r->long_model<(MEXO_LONG_SIGNAL_MIN*MODEL_VALUE_MAX)) || (master_sut_flag == REGULATOR_SAT_NEG)  ) ){
            sut_flag = REGULATOR_SAT_NEG;
        }
        else sut_flag =  REGULATOR_SAT_NONE;
    }
    r->sut_flag = sut_flag;
	if ( sut_flag == REGULATOR_SAT_NONE )
	{
		mexo_long_signal_t tmp;
		tmp = (r->long_model += (Error* s->modelGain) );
		tmp = MEXO_RIGHT_SHIFT(tmp, s->modelShift);
		MEXO_SATURATE(tmp, MEXO_SIGNAL_MIN, MEXO_SIGNAL_MAX);
		r->model = tmp;//(mexo_signal_t)tmp;


        if ( *(r->signal_force_diff) == 0  && ( signal ==0)){
            if(Error>0){
                mexo_parametr_t fm = s->forceMax;
                force+=s->forceGain;
                if(force>fm)
                    force=fm;
            }else if (Error<0){
                mexo_parametr_t fm = -s->forceMax;
                force-=s->forceGain;
                if(force<fm)
                    force=fm;
            }
        }else {
        	force = 0;
        }
       	r->force = force;
    }
 	tmp = Error+ r->model - signal;

	controlLong = tmp* (s->propGain)  ;
	controlLong = MEXO_RIGHT_SHIFT(controlLong, s->controlShift) + force;

	MEXO_SATURATE(controlLong, controlMin, controlMax);
	control_val = MEXO_LONG_TO_SIGNAL(controlLong);
    *(r->control_val) = control_val;


}
mexo_long_signal_t test_lim_control;
void limiter_run(limiter_p _limmiter){   
    mexo_signal_t control_des = _limmiter->control_des;
    mexo_signal_t  control_req = *(_limmiter->control_req);
    _limmiter->signal_hi = *(_limmiter->signal) - *(_limmiter->signalMax);
    _limmiter->signal_low = *(_limmiter->signal) - *(_limmiter->signalMin);
    regulator_run( & ( _limmiter->r_hi));
    regulator_run( & ( _limmiter->r_low));

    if( control_des < control_req){
		control_des = s_inc(control_des, *(_limmiter->ramp), *(_limmiter->controlMin), control_req);
    } else {
    	if( control_des > control_req){
			control_des = s_inc(control_des, -*(_limmiter->ramp), control_req, *(_limmiter->controlMax));
        }
    }


    test_lim_control =  _limmiter -> control_hi +  _limmiter -> control_low + control_des;
    _limmiter->control_des = control_des;
    
    
    if(test_lim_control > *(_limmiter->controlMax)){
       *(_limmiter->control_val) =  *(_limmiter->controlMax);
        _limmiter ->sut_flag = REGULATOR_SAT_POS;
    } else if(test_lim_control < *(_limmiter->controlMin)){
        *(_limmiter->control_val) =  *(_limmiter->controlMin);        
        _limmiter ->sut_flag = REGULATOR_SAT_NEG;
    } else {
        _limmiter ->sut_flag = REGULATOR_SAT_NONE;
        *(_limmiter->control_val) =  test_lim_control;                
    }
    
    /*if( _limmiter ->sut_flag == REGULATOR_SAT_NONE){
        if( _limmiter -> control_hi <0  ){
            _limmiter ->sut_flag = REGULATOR_SAT_POS;            
        }else if( _limmiter -> control_low > 0  ){
            _limmiter ->sut_flag = REGULATOR_SAT_NEG;                        
        }
    }*/
}
void limiter_reset(limiter_p _limmiter){
    regulator_reset( & ( _limmiter->r_hi));
    regulator_reset( & ( _limmiter->r_low));
    _limmiter->control_des = 0;
	*(_limmiter->control_val)=0;
    _limmiter ->sut_flag = REGULATOR_SAT_NONE;
}
void limiter_init(limiter_p _limiter, limiter_config_p _config){
    _limiter->r_hi.settings = _config->reg_settings;
    _limiter->r_hi.controlMin =  _config->controlMin; 
    _limiter->r_hi.controlMax = &(_limiter->zero_signal);
    _limiter->r_hi.signal = &(_limiter -> signal_hi);
    _limiter->r_hi.signal_req = &(_limiter->zero_signal);
    _limiter->r_hi.control_val = &(_limiter -> control_hi);
    _limiter->r_hi.master_sut_flag = &(_limiter ->sut_flag);

    _limiter->r_low.settings = _config->reg_settings;
    _limiter->r_low.controlMin =  &(_limiter->zero_signal); 
    _limiter->r_low.controlMax = _config->controlMax;
    _limiter->r_low.signal = &(_limiter -> signal_low);
    _limiter->r_low.signal_req = &(_limiter->zero_signal);
    _limiter->r_low.control_val = &(_limiter -> control_low);
    _limiter->r_low.master_sut_flag = &(_limiter ->sut_flag);
    
    _limiter->control_req = _config->control_req;
    _limiter->control_val=  _config->control_val;
    _limiter->signal=  _config->signal;
    _limiter->controlMax = _config->controlMax;
    _limiter->controlMin = _config->controlMin;
    _limiter->signalMin = _config->signalMin;
    _limiter->signalMax = _config->signalMax;
	_limiter->ramp = _config->ramp;
}
#ifndef POSITINER_MAX_ERR
#define POSITINER_MAX_ERR  MEXO_SIGNAL_T(0.9)
#endif
void posicioner_run(posicioner_p p){
    posicioner_settings_p s = p->settings;
	mexo_long_signal_t err = *(p->signal_req) - *(p->signal);
	mexo_signal_t deadZone = s->deadZone;
    if (err==0){
        *(p->control_val)=0;
        return;
    }else{
        if(err > s->deadZone){
            if (err>POSITINER_MAX_ERR){
                err = POSITINER_MAX_ERR;
            } else {
                if(s->crawlSpeed == 0) {
                    err -= deadZone;
                }
            }
        }
        if(err < -s->deadZone){
            if (err<-POSITINER_MAX_ERR){
                err = -POSITINER_MAX_ERR;
            } else {
                if(s->crawlSpeed == 0) {
                    err += deadZone;
                }
            }
        }
    }
    {        
    	mexo_long_signal_t control_val = err*(s->propGain);
        mexo_signal_t controlMin = *(p->controlMin);
        mexo_signal_t controlMax = *(p->controlMax);
		if(p->signal_diff){
			control_val -= (*p->signal_diff)*(s->diffGain);
		}
		control_val = MEXO_RIGHT_SHIFT(control_val, s->controlShift);
		if (p->forceControl) control_val += *(p->forceControl);
 		MEXO_SATURATE(control_val, controlMin, controlMax);
        if(control_val == 0 && s->crawlSpeed >0 ){
            if(err>s->deadZone){
                control_val =   s->crawlSpeed;				
			} else if( err < -s->deadZone) {
                control_val =   -s->crawlSpeed;				
            }
        }
        

         *(p->control_val) = control_val;
    }
}
