#include "burst/modules/acwc.h"

void acwc_mode_voltage_cl_applay_action(burst_dev_ref_p _ref){
	acwc_action_p action =(acwc_action_p)(_ref->action);
	acwc_p acwc = (acwc_p)(_ref);
	acwc_config_p cfg =(acwc_config_p)(_ref->config);	
	acwc->ac.voltage.des =  burst_range_apply(action->ac.voltage, &(cfg->ac.range.voltage)) ;
	burst_range_set(&(acwc->current.range),action->current, &(cfg->current.range));
}

void acwc_mode_voltage_cl_start(burst_dev_ref_p _ref){
	acwc_p acwc = (acwc_p)(_ref);
	acwc_config_p cfg =(acwc_config_p)(_ref->config);	
	acwc->ac.voltage.range.hi =  cfg->ac.range.voltage.hi;
	acwc->ac.voltage.range.lo =  cfg->ac.range.voltage.lo;
	burst_limiter_reset( &(acwc->current.limiter), 0  );
	acwc->ac.ps->command =  burst_ps_command_on;
}

void acwc_mode_voltage_cl_stop(burst_dev_ref_p _ref){
	acwc_p acwc = (acwc_p)(_ref);
	acwc->ac.ps->command =  burst_ps_command_off;
}

void acwc_mode_voltage_cl_runA(burst_dev_ref_p _ref){
	acwc_p acwc = (acwc_p)(_ref);
	burst_limiter_run( &(acwc->current.limiter) );
}

burst_dev_mode_t acwc_mode_voltage_cl = {
	&acwc_mode_voltage_cl_applay_action
	,&acwc_mode_voltage_cl_start
	,&acwc_mode_voltage_cl_stop
	,&acwc_mode_voltage_cl_runA
	,&burst_dev_idle_event
	,&burst_dev_idle_event
	,&burst_dev_idle_event
};


void acwc_mode_speed_cl_applay_action(burst_dev_ref_p _ref){
	acwc_action_p action =(acwc_action_p)(_ref->action);
	acwc_p acwc = (acwc_p)(_ref);
	acwc_config_p cfg =(acwc_config_p)(_ref->config);	
	burst_range_set(&(acwc->current.range),action->current, &(cfg->current.range));

	actuator_mode_speed_applay_action(_ref);
}
void acwc_mode_speed_cl_start(burst_dev_ref_p _ref){
	acwc_p acwc = (acwc_p)(_ref);
	actuator_p actuator = (actuator_p)(_ref);
	acwc_config_p cfg =(acwc_config_p)(_ref->config);	
	actuator->motion->setup(
			&actuator->speed.req
		, &actuator->speed.flt->value
		, &actuator->voltage.des
		, 0
		, &acwc->current.limiter.sut_flag
		,	&actuator->voltage.range.hi
		, &actuator->voltage.range.lo
		, 0
		, 0
		, 0
		, 0
		, &(cfg->modes.voltage_cl.motion)
	);

	actuator->motion->reset(actuator->speed.flt->value); 
	acwc_mode_voltage_cl_start(_ref);
}
void acwc_mode_speed_cl_stop(burst_dev_ref_p _ref){
	acwc_p acwc = (acwc_p)(_ref);
	acwc->ac.ps->command =  burst_ps_command_off;	
}
void acwc_mode_speed_cl_runA(burst_dev_ref_p _ref){
	acwc_p acwc = (acwc_p)(_ref);
	burst_limiter_run( &(acwc->current.limiter) );
}
void acwc_mode_speed_cl_runB(burst_dev_ref_p _ref){
	actuator_p actuator = (actuator_p)(_ref);
	actuator->motion->run();
}

burst_dev_mode_t acwc_mode_cl_speed = {
	&acwc_mode_speed_cl_applay_action
	,&acwc_mode_speed_cl_start
	,&acwc_mode_speed_cl_stop
	,&acwc_mode_speed_cl_runA
	,&acwc_mode_speed_cl_runB
	,&burst_dev_idle_event
	,&burst_dev_idle_event
};


void acwc_mode_position_cl_applay_action(burst_dev_ref_p _ref){
	acwc_action_p action =(acwc_action_p)(_ref->action);
	acwc_p acwc = (acwc_p)(_ref);
	acwc_config_p cfg =(acwc_config_p)(_ref->config);	
	burst_range_set(&(acwc->current.range),action->current, &(cfg->current.range));

	actuator_mode_position_applay_action(_ref);
}

void acwc_mode_position_cl_start(burst_dev_ref_p _ref){
	acwc_p acwc = (acwc_p)(_ref);
	actuator_p actuator = (actuator_p)(_ref);

	acwc_mode_voltage_cl_start(_ref);
	acwc_config_p cfg =(acwc_config_p)(_ref->config);
	actuator->ps->command =  burst_ps_command_on;
	actuator->motion->setup(
			&actuator->speed.req
		, &actuator->speed.flt->value
		, &actuator->voltage.des
		, 0
		, &acwc->current.limiter.sut_flag
		,	&actuator->voltage.range.hi
		, &actuator->voltage.range.lo
		, 0
		, 0
		, 0
		, 0
		, &(cfg->modes.voltage_cl.motion)
	);

	actuator->motion->reset(actuator->speed.flt->value); 
	
	actuator->positioner->setup(
			&actuator->position.req
		, &actuator->enco->position
		,	&actuator->speed.flt->value
		,	0
		, &actuator->speed.req
		,	&actuator->speed.range.hi
		, &actuator->speed.range.lo
		, &(cfg->modes.voltage_cl.positioner)
	);
	actuator->positioner->reset(); 

	acwc_mode_voltage_cl_start(_ref);
}

void acwc_mode_position_cl_stop(burst_dev_ref_p _ref){
	acwc_p acwc = (acwc_p)(_ref);
	acwc->ac.ps->command =  burst_ps_command_off;	
}

void acwc_mode_position_cl_runA(burst_dev_ref_p _ref){
	acwc_p acwc = (acwc_p)(_ref);
	burst_limiter_run( &(acwc->current.limiter) );
}

void acwc_mode_position_cl_runB(burst_dev_ref_p _ref){
	actuator_p actuator = (actuator_p)(_ref);
	actuator->positioner->run();
	actuator->motion->run();
}


burst_dev_mode_t acwc_mode_cl_position = {
	&acwc_mode_position_cl_applay_action
	,&acwc_mode_position_cl_start
	,&acwc_mode_position_cl_stop
	,&acwc_mode_position_cl_runA
	,&acwc_mode_position_cl_runB
	,&burst_dev_idle_event
	,&burst_dev_idle_event
};



void acwc_mode_current_applay_action(burst_dev_ref_p _ref){
	acwc_p acwc = (acwc_p)(_ref);
	acwc_action_p action =(acwc_action_p)(_ref->action);
	acwc_config_p config =(acwc_config_p)(_ref->config);
	burst_range_set(&(acwc->ac.voltage.range),action->ac.voltage, &(config->ac.range.voltage));
	acwc->current.req = burst_range_apply(action->current,&(config->current.range));
}
void acwc_mode_current_start(burst_dev_ref_p _ref){
	acwc_p acwc = (acwc_p)(_ref);
	acwc_config_p cfg =(acwc_config_p)(_ref->config);
//	acwc->ac.voltage.range.hi =  cfg->ac.range.voltage.hi;
//	acwc->ac.voltage.range.lo =  cfg->ac.range.voltage.lo;
	acwc->current.range.hi =  cfg->current.range.hi;
	acwc->current.range.lo =  cfg->current.range.lo;

	//todo согласовать
	acwc->current.dir->reset(acwc->current.flt->value);
	acwc->ac.ps->command =  burst_ps_command_on;
}
void acwc_mode_current_stop(burst_dev_ref_p _ref){
	acwc_p acwc = (acwc_p)(_ref);
	acwc->ac.ps->command =  burst_ps_command_off;
}
void acwc_mode_current_runA(burst_dev_ref_p _ref){
	acwc_p acwc = (acwc_p)(_ref);
	acwc->current.dir->run();
}

burst_dev_mode_t acwc_mode_current = {
	&acwc_mode_current_applay_action
	,&acwc_mode_current_start
	,&acwc_mode_current_stop
	,&acwc_mode_current_runA
	,&burst_dev_idle_event
	,&burst_dev_idle_event
	,&burst_dev_idle_event
};

void acwc_mode_speed_applay_action(burst_dev_ref_p _ref){
	acwc_action_p action =(acwc_action_p)(_ref->action);
	acwc_p acwc = (acwc_p)(_ref);
	acwc_config_p cfg =(acwc_config_p)(_ref->config);	
	burst_range_set(&(acwc->current.range),action->current, &(cfg->current.range));
	actuator_mode_speed_applay_action(_ref);
}
void acwc_mode_speed_start(burst_dev_ref_p _ref){
	acwc_p acwc = (acwc_p)(_ref);
	actuator_p actuator = (actuator_p)(_ref);
	acwc_config_p cfg =(acwc_config_p)(_ref->config);	
	actuator->motion->setup(
			&actuator->speed.req
		, &actuator->speed.flt->value
		, &acwc->current.req
		, 0
		, &acwc->current.dir->satstate
		,	&actuator->voltage.range.hi
		, &actuator->voltage.range.lo
		, 0
		, 0
		, 0
		, 0
		, &(cfg->modes.current.motion)
	);

	actuator->motion->reset(actuator->speed.flt->value); 
	acwc_mode_current_start(_ref);
}
void acwc_mode_speed_stop(burst_dev_ref_p _ref){
	acwc_p acwc = (acwc_p)(_ref);
	acwc->ac.ps->command =  burst_ps_command_off;	
}
void acwc_mode_speed_runA(burst_dev_ref_p _ref){
	acwc_p acwc = (acwc_p)(_ref);
	acwc->current.dir->run();
}
void acwc_mode_speed_runB(burst_dev_ref_p _ref){
	actuator_p actuator = (actuator_p)(_ref);
	actuator->motion->run();
}
burst_dev_mode_t acwc_mode_speed = {
	&acwc_mode_speed_applay_action
	,&acwc_mode_speed_start
	,&acwc_mode_speed_stop
	,&acwc_mode_speed_runA
	,&acwc_mode_speed_runB
	,&burst_dev_idle_event
	,&burst_dev_idle_event
};

void acwc_mode_position_applay_action(burst_dev_ref_p _ref){
	acwc_action_p action =(acwc_action_p)(_ref->action);
	acwc_p acwc = (acwc_p)(_ref);
	acwc_config_p cfg =(acwc_config_p)(_ref->config);	
	burst_range_set(&(acwc->current.range),action->current, &(cfg->current.range));
	actuator_mode_position_applay_action(_ref);
}

void acwc_mode_position_start(burst_dev_ref_p _ref){
	acwc_p acwc = (acwc_p)(_ref);
	actuator_p actuator = (actuator_p)(_ref);

	acwc_mode_voltage_cl_start(_ref);
	acwc_config_p cfg =(acwc_config_p)(_ref->config);	
	actuator->ps->command =  burst_ps_command_on;
	actuator->motion->setup(
			&actuator->speed.req
		, &actuator->speed.flt->value
		, &acwc->current.req
		, 0
		, &acwc->current.dir->satstate
		,	&actuator->voltage.range.hi
		, &actuator->voltage.range.lo
		, 0
		, 0
		, 0
		, 0
		, &(cfg->modes.current.motion)
	);

	actuator->motion->reset(actuator->speed.flt->value); 
	
	actuator->positioner->setup(
			&actuator->position.req
		, &actuator->enco->position
		,	&actuator->speed.flt->value
		,	0
		, &actuator->speed.req
		,	&actuator->speed.range.hi
		, &actuator->speed.range.lo
		,&(cfg->modes.current.positioner)
	);
	actuator->positioner->reset(); 

	acwc_mode_current_start(_ref);
}
void acwc_mode_position_stop(burst_dev_ref_p _ref){
	acwc_p acwc = (acwc_p)(_ref);
	acwc->ac.ps->command =  burst_ps_command_off;	
}

void acwc_mode_position_runA(burst_dev_ref_p _ref){
	acwc_p acwc = (acwc_p)(_ref);
	acwc->current.dir->run();
}

void acwc_mode_position_runB(burst_dev_ref_p _ref){
	actuator_p actuator = (actuator_p)(_ref);
	actuator->positioner->run();
	actuator->motion->run();
}

burst_dev_mode_t acwc_mode_position = {
	&acwc_mode_position_applay_action
	,&acwc_mode_position_start
	,&acwc_mode_position_stop
	,&acwc_mode_position_runA
	,&acwc_mode_position_runB
	,&burst_dev_idle_event
	,&burst_dev_idle_event
};

void acwc_begin (
	acwc_p _acwc
	, acwc_config_p _config
	, acwc_action_p _action
	, acwc_feedback_p _feedback
	, burst_ps_p _ps
	, burst_enco_p _enco
	, burst_filter_p _spf
	, burst_motion_p _motion
	, burst_positioner_p _positioner	
	, int _mode_count
	, burst_dev_mode_p * _modes	
	, burst_filter_p _curf
	,	burst_pi_p _dir
	,	burst_pi_p _hi
	,	burst_pi_p _lo
	,	burst_signal_p _current_raw
	){
	_acwc->current.flt = _curf;
	_acwc->current.raw = _current_raw;
	_acwc->ac.ref.update_feedback = acwc_event_update_feedback;
	_curf->setup(	_acwc->current.raw,0);

		actuator_begin(
		&(_acwc->ac)
		, &(_config->ac)
		, &(_action->ac)
		, &(_feedback->ac)
		, _ps
		, _enco
		, _spf
		, _motion
		, _positioner	
		, _mode_count
		, _modes	
	);

		burst_limiter_config_t  cfg = {
		_hi
		,_lo
    , &(_acwc->ac.voltage.des)//burst_signal_p control_req;
    ,&(_acwc->ac.voltage.req)//burst_signal_p control_val;
    ,&(_acwc->ac.voltage.range.lo)//burst_signal_p controlMin;
    ,&(_acwc->ac.voltage.range.hi)//burst_signal_p controlMax;
    ,&(_acwc->current.flt->value)//burst_signal_p signal;
    ,&(_acwc->current.range.lo)//burst_signal_p signalMin;
    ,&(_acwc->current.range.hi)//burst_signal_p signalMax;
    ,&(_config->current.pi)//burst_pi_config_p reg_config;
	};
	burst_limiter_setup( &(_acwc->current.limiter), &cfg,0 );
	_acwc->current.dir = _dir;
	
	_acwc->current.dir->setup(
		&(_config->current.pi) //_config
		,	&(_acwc->current.req) //_signal_req
		, &(_acwc->current.flt->value) //_signal
		, &(_acwc->ac.voltage.req) //control
		, 0//burst_signal_t 				_start_control
		, &(_acwc->ac.ps->satstate) //master_sutstate
		,	&(_acwc->ac.voltage.range.hi)// _controlMax
		, &(_acwc->ac.voltage.range.lo)//_controlMin
		, 0//				_signal_diff
		, 0//				_signal_force
	);
}
	

void acwc_event_update_feedback(burst_dev_ref_p _dev){
	actuator_event_update_feedback(_dev);
	acwc_feedback_p fb =(acwc_feedback_p)(_dev->feedback);
	acwc_p acwc = (acwc_p)(_dev);
	fb->current = acwc->current.flt->value;
}
