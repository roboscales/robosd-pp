#include "burst/modules/pmsm.h"

void pmsm_begin (
	pmsm_p _pmsm
	, pmsm_config_p _config
	,	pmsm_modes_config_p _modes_config
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
	, burst_signal_p _cross_current_raw
	, burst_signal_p _lat_current_raw
){
	acwc_begin(
		&(_pmsm->acwc)//acwc_p _acwc
		, &(_config->acwc)//acwc_config_p _config
		,	&(_modes_config->acwc)//acwc_modes_config_p _modes_config
		, &(_action->acwc)//acwc_action_p _action
		, &(_feedback->acwc)//acwc_feedback_p _feedback
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
		, _cross_current_raw
	);
	_pmsm->lateral.current.flt = _lateral_curf;
	_pmsm->lateral.current.raw = _lat_current_raw;
	_pmsm->lateral.current.pi = _lateral_pi;
}


void pmsm_mode_synchro_voltage_applay_action(burst_dev_ref_p _ref){	
	acwc_p acwc = (acwc_p)(_ref);
	acwc_config_p cfg =(acwc_config_p)(_ref->config);	
	acwc->voltage.range.hi =  cfg->acwc.voltage.hi;
	actuator->voltage.range.lo =  cfg->range.voltage.lo;
	actuator->ps->command =  burst_ps_command_on;
}
extern burst_dev_mode_t acwc_synchro_voltage;

void pmsm_mode_synchro_current_applay_action(burst_dev_ref_p _ref){
}
extern burst_dev_mode_t acwc_synchro_current;
