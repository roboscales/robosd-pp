#include "burst/modules/pmsm.h"
#include "burst/burst_sqrt.h"

void pmsm_begin (
	pmsm_p _pmsm
	, pmsm_config_p _config
	, pmsm_action_p _action
	, pmsm_feedback_p _feedback
	, burst_ps_p _ps
	, burst_enco_p _enco
	, burst_filter_p _spf
	, burst_motion_p _motion
	, burst_positioner_p _positioner	
	, int _mode_count
	, burst_dev_mode_p * _modes	
	, burst_filter_p _cross_curf
	, burst_filter_p _lateral_curf
	,	burst_pi_p _cross_pi
	,	burst_pi_p _lateral_pi
	,	burst_pi_p _cross_hi
	,	burst_pi_p _cross_lo
	, burst_signal_t * _raw
	, pmsm_estimate_p _estimate
	){

	acwc_begin(
		&(_pmsm->cross)//acwc_p _acwc
		, &(_config->cross)//acwc_config_p _config
		, &(_action->cross)//acwc_action_p _action
		, &(_feedback->cross)//acwc_feedback_p _feedback
		, _ps//burst_ps_p _ps
		, _enco//burst_enco_p _enco
		, _spf//burst_filter_p _spf
		, _motion//burst_motion_p _motion
		, _positioner//burst_positioner_p _positioner	
		, _mode_count//int _mode_count
		, _modes//burst_dev_mode_p * _modes	
		, _cross_curf//burst_filter_p _curf
		,	_cross_pi//burst_pi_p _cross
		,	_cross_hi//burst_pi_p _cross_hi
		,	_cross_lo//burst_pi_p _cross_lo
		, &(_pmsm->sensor.dq.cross)
	);

	inv3ph_begin(&(_pmsm->inverter),&(_config->inverter));
	current3ph_begin(&(_pmsm->sensor),&(_config->sensor), &(_pmsm->inverter), _raw);

	_pmsm->lateral.current.flt = _lateral_curf;
	_pmsm->lateral.current.raw = &(_pmsm->sensor.dq.lateral);
	_pmsm->lateral.current.pi = _lateral_pi;
	_pmsm->estimate = _estimate;
	
	_pmsm->cross.ac.ref.update_feedback = pmsm_event_update_feedback;
	_pmsm->lateral.current.flt->setup(_pmsm->lateral.current.raw,0);

	_lateral_pi->setup(
		&(_config->lateral.current.pi)
		,	&_pmsm->lateral.current.req 	//burst_signal_p				_signal_req
		, &_pmsm->lateral.current.flt->value 					//burst_signal_p				_signal
		, &_pmsm->lateral.voltage.req				//burst_signal_p			  _control
		, 0														//burst_signal_t 				_start_control
		, &_ps->satstate							//burst_satstate_t *		_master_sut_flag
		, 0														//burst_signal_p				_controlMax
		, 0														//burst_signal_p				_controlMin
		, 0														//burst_signal_p				_signal_diff
		, 0														//burst_signal_p				_signal_force
	);
	
}


void pmsm_mode_synchro_voltage_applay_action(burst_dev_ref_p _ref){	
	pmsm_p pmsm = (pmsm_p)(_ref);
	pmsm_action_p action = (pmsm_action_p)(_ref->action);
//	pmsm_config_p cfg =(pmsm_config_p)(_ref->config);	
	pmsm->cross.ac.voltage.req = burst_range_apply(action->cross.ac.voltage,&(pmsm->cross.ac.voltage.range));
	pmsm->lateral.voltage.req = burst_range_apply(action->lateral.voltage,&(pmsm->lateral.voltage.range));
	pmsm->synchro.freq = action->synchro.freq;
	pmsm->synchro.angle = action->synchro.angle;
}

void pmsm_mode_synchro_voltage_start(burst_dev_ref_p _ref){
	pmsm_p pmsm = (pmsm_p)(_ref);
	pmsm_config_p cfg =(pmsm_config_p)(_ref->config);	
	pmsm->cross.ac.voltage.range.hi =  cfg->cross.ac.range.voltage.hi;
	pmsm->cross.ac.voltage.range.lo =  cfg->cross.ac.range.voltage.lo;
	pmsm->lateral.voltage.range.hi =  cfg->lateral.voltage.range.hi;
	pmsm->lateral.voltage.range.lo =  cfg->lateral.voltage.range.lo;
	pmsm->cross.ac.ps->command =  burst_ps_command_on;
}
void pmsm_mode_synchro_voltage_stop(burst_dev_ref_p _ref){
	pmsm_p pmsm = (pmsm_p)(_ref);
	pmsm->cross.ac.ps->command =  burst_ps_command_off;
}
void pmsm_mode_synchro_voltage_runA(burst_dev_ref_p _ref){
	pmsm_p pmsm = (pmsm_p)(_ref);
	pmsm->synchro.angle += pmsm->synchro.freq;
	//current3ph_run(&(pmsm->sensor));
	inv3ph_run(&(pmsm->inverter),pmsm->cross.ac.voltage.req,pmsm->lateral.voltage.req, (pmsm->synchro.angle)>>16);	
}

burst_dev_mode_t pmcm_synchro_voltage = {
	&pmsm_mode_synchro_voltage_applay_action
	,&pmsm_mode_synchro_voltage_start
	,&pmsm_mode_synchro_voltage_stop
	,&pmsm_mode_synchro_voltage_runA
	,&burst_dev_idle_event
	,&burst_dev_idle_event
	,&burst_dev_idle_event
};

void pmsm_mode_synchro_current_applay_action(burst_dev_ref_p _ref){	
	pmsm_p pmsm = (pmsm_p)(_ref);
	pmsm_action_p action = (pmsm_action_p)(_ref->action);
	pmsm_config_p cfg =(pmsm_config_p)(_ref->config);	
	burst_range_set(&(pmsm->cross.ac.voltage.range),action->cross.ac.voltage, &(cfg->cross.ac.range.voltage));
	burst_range_set(&(pmsm->lateral.voltage.range),action->lateral.voltage, &(cfg->lateral.voltage.range));
	
	pmsm->cross.current.req = burst_range_apply(action->cross.current,&(pmsm->cross.current.range));
	pmsm->lateral.current.req = burst_range_apply(action->lateral.current,&(pmsm->lateral.current.range));
	pmsm->synchro.freq = action->synchro.freq;
	pmsm->synchro.angle = action->synchro.angle;
}

void pmsm_mode_synchro_current_start(burst_dev_ref_p _ref){
	pmsm_p pmsm = (pmsm_p)(_ref);
	pmsm_config_p cfg =(pmsm_config_p)(_ref->config);	
	pmsm->cross.current.range.hi =  cfg->cross.current.range.hi;
	pmsm->cross.current.range.lo =  cfg->cross.current.range.lo;
	pmsm->lateral.current.range.hi =  cfg->lateral.current.range.hi;
	pmsm->lateral.current.range.lo =  cfg->lateral.current.range.lo;

	pmsm->cross.ac.ps->command =  burst_ps_command_on;
	pmsm->lateral.current.pi->reset(pmsm->lateral.current.flt->value);
	pmsm->cross.current.dir->reset(pmsm->cross.current.flt->value);

}
void pmsm_mode_synchro_current_stop(burst_dev_ref_p _ref){
	pmsm_p pmsm = (pmsm_p)(_ref);
	pmsm->cross.ac.ps->command =  burst_ps_command_off;
}
void pmsm_mode_synchro_current_runA(burst_dev_ref_p _ref){
	pmsm_p pmsm = (pmsm_p)(_ref);
	pmsm->synchro.angle += pmsm->synchro.freq;
	//
	pmsm->lateral.current.pi->run();
	pmsm->cross.current.dir->run();
	inv3ph_run(&(pmsm->inverter),pmsm->cross.ac.voltage.req,pmsm->lateral.voltage.req, (pmsm->synchro.angle)>>16);	
}

burst_dev_mode_t pmcm_synchro_current = {
	&pmsm_mode_synchro_current_applay_action
	,&pmsm_mode_synchro_current_start
	,&pmsm_mode_synchro_current_stop
	,&pmsm_mode_synchro_current_runA
	,&burst_dev_idle_event
	,&burst_dev_idle_event
	,&burst_dev_idle_event
};


void pmsm_event_update_feedback(burst_dev_ref_p _dev){
	acwc_event_update_feedback(_dev);
	pmsm_feedback_p fb =(pmsm_feedback_p)(_dev->feedback);
	pmsm_p pmsm = (pmsm_p)(_dev);
	fb->lateral.current = pmsm->lateral.current.flt->value;
	fb->lateral.voltage = pmsm->lateral.voltage.req;
}

void pmsm_sence_run (pmsm_p _pmsm){
	current3ph_run(&(_pmsm->sensor));
	_pmsm->cross.current.flt->run();
	_pmsm->lateral.current.flt->run();
}

void pmsm_inverter_run (pmsm_p _pmsm){
	if(_pmsm->cross.ac.ps->command ==  burst_ps_command_on){
		int mode = ((burst_dev_ref_p)(_pmsm))->mode;
		if (mode != burst_dev_mode_idle) {
			if(_pmsm->mode_prev!=mode){	
				_pmsm->mode_prev = mode;		
				switch(mode){
					//в этих режимах контур тока не используется или используется напрямую
					case pmsm_mode_synchro_voltage_ix:
					case pmsm_mode_synchro_curent_ix:
					case pmsm_mode_estimate_ix:
						break;
					default:
						{
							pmsm_config_p cfg =(pmsm_config_p)(_pmsm->cross.ac.ref.config);	
							//резетим контур продольного тока
							_pmsm->lateral.current.req = 0; 
							_pmsm->lateral.current.range.hi =  cfg->lateral.current.range.hi;
							_pmsm->lateral.current.range.lo =  cfg->lateral.current.range.lo;			
							_pmsm->lateral.voltage.range.hi =  cfg->lateral.voltage.range.hi;
							_pmsm->lateral.voltage.range.lo =  cfg->lateral.voltage.range.lo;			
							_pmsm->lateral.current.pi->reset(_pmsm->lateral.current.flt->value);
						}
				}
			}
			switch(mode){
				case pmsm_mode_synchro_voltage_ix:
				case pmsm_mode_synchro_curent_ix:
				break;
				case pmsm_mode_estimate_ix:
					//работаем без регулятора продольного тока
					inv3ph_run(
						&(_pmsm->inverter)
						,_pmsm->cross.ac.voltage.req
						,_pmsm->lateral.voltage.req
						,_pmsm->estimate->electro.angle
					);			
					break;
				default:
					//для систем управления, аналогичных ДПТ включаем регулятор продоьного тока
					_pmsm->lateral.current.pi->run();
					inv3ph_run(
						&(_pmsm->inverter)
						,_pmsm->cross.ac.voltage.req
						,_pmsm->lateral.voltage.req
						,_pmsm->estimate->electro.angle
					);			
			}
		}
	}
}

void pmsm_angle_forcer_begin(pmsm_angle_forcer_p _forcer, pmsm_angle_forcer_config_p _config,burst_signal_p _angle,  burst_signal_p _speed, burst_signal_p _current){
	_forcer->config = _config;
	_forcer->angle.raw = _angle;
	_forcer->speed = _speed;
	_forcer->current = _current;
	
}
void pmsm_angle_forcer_run(pmsm_angle_forcer_p _forcer){
	pmsm_angle_forcer_config_p config = _forcer->config;
	burst_long_signal_t speed = *(_forcer->speed);
//	if(speed<0) speed=-speed;
	burst_long_signal_t total = _forcer->angle.force = (( speed * config->force.gain)>>config->force.shift);
	if(_forcer->current){
		burst_long_signal_t current = *(_forcer->current);
		if(current<0) current=-current;
		burst_long_signal_t eds =  ((speed * config->eds.gain)>>config->eds.shift1);
		eds *= current;
		eds >>= config->eds.shift2;
		_forcer->angle.eds = eds;
		total += eds;
	} else{
		_forcer->angle.eds = 0;
	}
	if(total>config->angle_lim){
		total = config->angle_lim;
	}	else if(total<-config->angle_lim){
		total = -config->angle_lim;
	}
	_forcer->ref.electro.angle = *(_forcer->angle.raw) + (burst_signal_t)total;
	_forcer->angle.total = total;
}



/*
struct{
		struct{
			burst_usignal_t hi;
			burst_usignal_t lo;
			burst_time_us_t us;
		}	voltage;
		struct{
			burst_usignal_t panic;
			burst_usignal_t level;
			burst_time_us_t us;
		}	current;
		struct{
			burst_usignal_t hi;
			burst_usignal_t lo;
			burst_time_us_t us;
		}	temper;
	} fault;
*/
/*
*/
void pmsm_protector_run (pmsm_p _pmsm){

	pmsm_config_p cfg = (pmsm_config_p)(_pmsm->cross.ac.ref.config);
	burst_signal_t ix = *_pmsm->cross.current.raw;
	burst_signal_t iy = *_pmsm->lateral.current.raw;
	burst_signal_t magnitude = (burst_signal_t) burst_sqrt( (uint32_t)(ix*ix + iy*iy) );
	burst_signal_t delta = magnitude -_pmsm ->protector.current.magnitude;
	burst_time_us_t now = burst_time_us();
	_pmsm ->protector.current.delta = delta;
	_pmsm ->protector.current.magnitude = magnitude;
	
	if( 
		magnitude > cfg->fault.current.panic 
		|| (magnitude + delta) > cfg->fault.current.panic  
	){
		burst_raise_panic(&(_pmsm->cross.ac.ref),1);
	} else {
		if( 
			magnitude > cfg->fault.current.level
		){
			//burst_time_us_t last = _pmsm ->protector.current.us;
			//if(last == 0){
			//	_pmsm ->protector.current.us = now;
			//} else {
				if( now -  _pmsm ->protector.current.us > cfg->fault.current.us){
					burst_raise_panic(&(_pmsm->cross.ac.ref),2);
					_pmsm ->protector.current.us = 0;
				}
		} else{
			_pmsm ->protector.current.us = now;
		}
	}
}

