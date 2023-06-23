#include "burst/modules/actuator.h"
void actuator_mode_fault_start(burst_dev_ref_p _ref){
	actuator_p actuator = (actuator_p)(_ref);
	actuator->ps->command =  burst_ps_command_off;
	actuator->ps->shutdown_begin(); //вырубаемся немедленно
}

void actuator_mode_fault_runB(burst_dev_ref_p _ref){
	actuator_p actuator = (actuator_p)(_ref);
	actuator_config_p cfg =(actuator_config_p)(_ref->config);
	if( (actuator->enco->counter.total>cfg->enco_fault_ticks.reset) && (actuator->enco->counter.fault < actuator->enco->counter.total ) ){
		_ref->mode = actuator->def_mode;
		actuator->enco->counter.fault = actuator->enco->counter.total = 0;
	}
}
burst_dev_mode_t actuator_mode_fault = {
	&burst_dev_idle_event
	,&actuator_mode_fault_start
	,&burst_dev_idle_event
	,&burst_dev_idle_event
	,&actuator_mode_fault_runB
	,&burst_dev_idle_event
	,&burst_dev_idle_event
};

void actuator_check_enco( burst_dev_ref_p _ref){
	actuator_p actuator = (actuator_p)(_ref);
	actuator_config_p cfg =(actuator_config_p)(_ref->config);
	if(actuator->enco->counter.total>0 && (actuator->enco->counter.total - actuator->enco->counter.fault ) >= cfg->enco_fault_ticks.set){
		_ref->mode = actuator_mode_fault_ix;
	}
	actuator->enco->counter.fault = actuator->enco->counter.total = 0;	
}

void actuator_set_voltage_range_(burst_dev_ref_p _ref){
	actuator_config_p cfg =(actuator_config_p)(_ref->config);
	actuator_p actuator = (actuator_p)(_ref);
	actuator_action_p action = (actuator_action_p)(_ref->action);
	burst_range_set(&(actuator->voltage.range),action->voltage, &(cfg->range.voltage));
}
	
void actuator_mode_speed_applay_action(burst_dev_ref_p _ref){
	actuator_action_p action =(actuator_action_p)(_ref->action);
	actuator_p actuator = (actuator_p)(_ref);
	actuator_config_p cfg =(actuator_config_p)(_ref->config);	
	burst_range_set(&(actuator->voltage.range),action->voltage, &(cfg->range.voltage));
	actuator->speed.req = burst_range_apply(action->speed, &(cfg->range.speed));
}

void actuator_mode_speed_start(burst_dev_ref_p _ref){
	actuator_p actuator = (actuator_p)(_ref);
	actuator->ps->command =  burst_ps_command_on;
	actuator_config_p cfg =(actuator_config_p)(_ref->config);
	actuator->motion->setup(
		 &actuator->speed.req
		, &actuator->speed.flt->value
		, &actuator->voltage.req
		, 0
		, &actuator->ps->satstate
		,	&actuator->voltage.range.hi
		, &actuator->voltage.range.lo
		, 0
		, 0
		, 0
		, 0
		, &(cfg->modes.motion)
	);
	actuator->motion->reset(actuator->speed.flt->value); 
}

void actuator_mode_speed_stop(burst_dev_ref_p _ref){
	actuator_p actuator = (actuator_p)(_ref);
	actuator->ps->command =  burst_ps_command_off;
}


void actuator_mode_speed_runB(burst_dev_ref_p _ref){
	actuator_p actuator = (actuator_p)(_ref);
	actuator->motion->run();
}

burst_dev_mode_t actuator_mode_speed = {
	&actuator_mode_speed_applay_action
	,&actuator_mode_speed_start
	,&actuator_mode_speed_stop
	,&burst_dev_idle_event
	,&actuator_mode_speed_runB
	,&burst_dev_idle_event
	,&burst_dev_idle_event
};

void actuator_mode_voltage_applay_action(burst_dev_ref_p _ref){
	actuator_action_p a =(actuator_action_p)(_ref->action);
	actuator_p actuator = (actuator_p)(_ref);
	//todo voltage ramp!  actuator->voltage.des =a->voltage
	actuator_config_p cfg =(actuator_config_p)(_ref->config);
	actuator->voltage.req = burst_range_apply(a->voltage, &(cfg->range.voltage));
}

void actuator_mode_voltage_start(burst_dev_ref_p _ref){
	actuator_p actuator = (actuator_p)(_ref);
	actuator_config_p cfg =(actuator_config_p)(_ref->config);	
	actuator->voltage.range.hi =  cfg->range.voltage.hi;
	actuator->voltage.range.lo =  cfg->range.voltage.lo;
	actuator->ps->command =  burst_ps_command_on;
}

void actuator_mode_voltage_stop(burst_dev_ref_p _ref){
	actuator_p actuator = (actuator_p)(_ref);
	actuator->ps->command =  burst_ps_command_off;
}

burst_dev_mode_t actuator_mode_voltage = {
	&actuator_mode_voltage_applay_action
	,&actuator_mode_voltage_start
	,&actuator_mode_voltage_stop
	,&burst_dev_idle_event
	,&burst_dev_idle_event
	,&burst_dev_idle_event
	,&burst_dev_idle_event
};

void actuator_mode_position_applay_action(burst_dev_ref_p _ref){
	actuator_action_p action =(actuator_action_p)(_ref->action);
	actuator_p actuator = (actuator_p)(_ref);
	actuator_config_p cfg =(actuator_config_p)(_ref->config);
	
	burst_range_set(&(actuator->voltage.range),action->voltage, &(cfg->range.voltage));
	burst_range_set(&(actuator->speed.range),action->speed, &(cfg->range.speed));

	if( action->position > cfg->range.position.hi ){
		actuator->position.req = cfg->range.position.hi;
	} else if( action->position < cfg->range.position.lo ){
		actuator->position.req = cfg->range.position.lo;
	} else {
		actuator->position.req =  action->position;
	}
}

void actuator_mode_position_start(burst_dev_ref_p _ref){
	actuator_p actuator = (actuator_p)(_ref);
	actuator->ps->command =  burst_ps_command_on;
	actuator_config_p cfg =(actuator_config_p)(_ref->config);
	actuator->motion->setup(
			&actuator->speed.req
		, &actuator->speed.flt->value
		, &actuator->voltage.req
		, 0
		, &actuator->ps->satstate
		,	&actuator->voltage.range.hi
		, &actuator->voltage.range.lo
		, 0
		, 0
		, 0
		, 0
		, &(cfg->modes.motion)
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
		, &(cfg->modes.positioner)
	);
	actuator->positioner->reset(); 	

}

void actuator_mode_position_stop(burst_dev_ref_p _ref){
	actuator_p actuator = (actuator_p)(_ref);
	actuator->ps->command =  burst_ps_command_off;
}
void actuator_mode_position_runB(burst_dev_ref_p _ref){
	actuator_p actuator = (actuator_p)(_ref);
	actuator->positioner->run();
	actuator->motion->run();
}
burst_dev_mode_t actuator_mode_position = {
	&actuator_mode_position_applay_action
	,&actuator_mode_position_start
	,&actuator_mode_position_stop
	,&burst_dev_idle_event
	,&actuator_mode_position_runB
	,&burst_dev_idle_event
	,&burst_dev_idle_event
};

void actuator_event_update_feedback(burst_dev_ref_p _dev){
	actuator_feedback_p fb =(actuator_feedback_p)(_dev->feedback);
	actuator_p actuator = (actuator_p)(_dev);
	_dev->feedback->mode = _dev->mode;
	fb->voltage = actuator->voltage.req;
	fb->speed = actuator->speed.flt->value;
	fb->position = actuator->enco->position;
}


void actuator_begin (
	actuator_p _actuator
	, actuator_config_p _config
	, actuator_action_p _action
	, actuator_feedback_p _feedback
	, burst_ps_p _ps
	, burst_enco_p _enco
	, burst_filter_p _spf
	, burst_motion_p _motion
	, burst_positioner_p _positioner	
	, int _mode_count
	, burst_dev_mode_p * _modes	
){
	_actuator->ref.reset = burst_dev_idle_event;
	_actuator->ref.start = burst_dev_idle_event;
	_actuator->ref.realtime_loop = burst_dev_idle_event;
	_actuator->ref.frontend_loop = burst_dev_idle_event;
	_actuator->ref.update_feedback = actuator_event_update_feedback;
	_actuator->ref.config = &(_config->ref);
	_actuator->ref.action = &(_action->ref);
	_actuator->ref.feedback = &(_feedback->ref);
	_actuator->ref.mode_count = _mode_count;
	_actuator->ref.modes = _modes;
	_actuator->def_mode = burst_dev_mode_idle;
	_actuator->ps = _ps;
	_actuator->enco = _enco;
	_actuator->speed.flt = _spf;
	_actuator->motion = _motion;
	_actuator->positioner = _positioner;
	burst_dev_attach(&(_actuator->ref));
	_spf->setup(&_enco->delta_acc,1);
}

