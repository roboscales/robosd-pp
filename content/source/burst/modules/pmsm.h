#ifndef burst_modules_pmsm_h
#define burst_modules_pmsm_h
#include "burst/modules/pmsm_front.h"
#include "burst/modules/acwc.h"
typedef struct pmsm_config_s{
	acwc_config_t acwc;
	struct{
		struct{
			burst_pi_config_t pi;
			burst_range_t range;
		} current;
		struct{
			burst_pi_config_t pi;
			burst_range_t range;
		} voltage;
	} lateral;
} pmsm_config_t;
typedef pmsm_config_t * pmsm_config_p;

typedef struct pmsm_modes_config_s{
	acwc_modes_config_t acwc;
	burst_pi_config_p lc_pi;
} pmsm_modes_config_t;
typedef pmsm_modes_config_t * pmsm_modes_config_p;

typedef struct pmsm_s {
	acwc_t acwc;
	struct{
		struct {
			burst_signal_t req;
			burst_range_t range;
			burst_pi_p pi;
			burst_filter_p flt;
			burst_signal_p raw;
		} current;
		struct {
			burst_signal_t req;
			burst_range_t range;
		} voltage;
	}lateral;	
} pmsm_t;
typedef  pmsm_t * pmsm_p;

void pmsm_mode_synchro_voltage_applay_action(burst_dev_ref_p _ref);
extern burst_dev_mode_t pmsm_synchro_voltage;

void pmsm_mode_synchro_current_applay_action(burst_dev_ref_p _ref);
extern burst_dev_mode_t pmsm_synchro_current;


void pmsm_begin (
	pmsm_p _acwc
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
	);

#endif