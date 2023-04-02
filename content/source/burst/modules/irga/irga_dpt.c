#include "burst/modules/irga/irga_dpt.h"

burst_dev_mode_p irga_dpt_actuator_modes[ actuator_mode_count] = {
	&burst_idle_mode
	, &actuator_mode_fault
	, &actuator_mode_voltage
	, &actuator_mode_speed
	, &actuator_mode_position
};

void irga_dpt_begin (
	irga_dpt_p _dpt
	, irga_dpt_config_p _config
	, ps_dc_p _ps
	, enco_abs32_p _enco
	, nikitin_p _spf
	, burst_motion_p _motion
	, burst_positioner_p _positioner	
){
	actuator_begin(
	&(_dpt->actuator)//actuator_p _actuator
	,&( _config->actuator)//actuator_config_p _config
	, &(_dpt->action)//actuator_action_p _action
	, &(_dpt->feedback)//actuator_feedback_p _feedback
	, &(_ps->machine)//burst_ps_p _ps
	, &(_enco->ref)//burst_enco_p _enco
	, &(_spf->ref)//burst_filter_p _spf
	, _motion//burst_motion_p _motion
	, _positioner//burst_positioner_p _positioner	
	, actuator_mode_count//int _mode_count
	, irga_dpt_actuator_modes//burst_dev_mode_p * _modes	
	, &(_config->ps.voltage)//burst_range_p _voltage_range
	);
	ps_dc_begin(_ps,&(_config->ps));
	_ps->setup( &(_dpt->actuator.voltage.req) );
	_dpt->actuator.voltage_range = &(_config->ps.voltage);
	enco_abs32_begin(_enco,&(_config->enco));
	nikitin_begin(_spf,&(_config->spf));
	burst_motion_begin(_motion, &(_config->motion) );
	burst_positioner_begin(_positioner, &(_config->positioner));
}
