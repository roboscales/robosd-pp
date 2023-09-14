#ifndef burst_modules_pmsm_h
#define burst_modules_pmsm_h

#include "burst/modules/pmsm_front.h"
#include "burst/modules/acwc.h"
#include "burst/burst_inv3ph.h"
#include "burst/burst_timer.h"

typedef struct pmsm_config_s{
	acwc_config_t cross;
	inv3ph_config_t inverter;
	current3ph_config_t sensor;
	struct{
		struct{
			burst_pi_config_t pi;
			burst_range_t range;
		} current;
		struct{
			burst_range_t range;
		} voltage;
	} lateral;	
	struct{
		/*struct{
			burst_usignal_t hi;
			burst_usignal_t lo;
			burst_time_us_t us;
		}	voltage;*/
		struct{
			burst_signal_t panic;
			burst_signal_t level;
			burst_time_us_t us;
		}	current;
		/*struct{
			burst_usignal_t hi;
			burst_usignal_t lo;
			burst_time_us_t us;
		}	temper;*/
	} fault;
} pmsm_config_t;
typedef pmsm_config_t * pmsm_config_p;

#define PMSM_CONFIG(a) PMSM_CONFIG_(a)
#define PMSM_CONFIG_(a)\
{\
	ACWC_CONFIG(a,a##_CROSS)\
	,INV3PH_CONFIG(a##_INV3PH)\
	,CURRENT3PH_CONFIG(a##_CURRENT3PH)\
	,{\
		{\
			PI_CONFIG(a##_LATERAL_CURRENT_PI)\
			, RANGE_CONFIG(a##_LATERAL_CURRENT_RANGE)\
		}\
		,{\
			RANGE_CONFIG(a##_LATERAL_VOLTAGE_RANGE)\
		}\
	}\
	,{\
		{\
			a##_FAULT_CURRENT_PANIC\
			, a##_FAULT_CURRENT_LEVEL\
			, a##_FAULT_CURRENT_US\
		}\
	}\
}

typedef struct pmsm_estimate_s{
	struct{
		burst_signal_t speed;
		burst_signal_t angle;
	} electro;
} pmsm_estimate_t;
typedef pmsm_estimate_t * pmsm_estimate_p;



typedef struct pmsm_angle_forcer_config_s{
	struct{
		burst_signal_t gain;
		uint8_t shift;
	} force;
	struct{
		burst_signal_t gain;
		uint8_t shift1;
		uint8_t shift2;
	} eds;
	burst_signal_t angle_lim;
} pmsm_angle_forcer_config_t;
typedef pmsm_angle_forcer_config_t * pmsm_angle_forcer_config_p;

#define PMSM_ANGLE_FORCER_CONFIG(a) PMSM_ANGLE_FORCER_CONFIG_(a)
#define PMSM_ANGLE_FORCER_CONFIG_(a)\
{\
	{\
		a##_FORCE_GAIN\
		, a##_FORCE_SHIFT\
	}\
	,{\
		 a##_EDS_GAIN\
		, a##_EDS_SHIFT1\
		, a##_EDS_SHIFT2\
	}\
	, a##_ANGLE_LIMIT\
}

typedef struct pmsm_angle_forcer_s{
	pmsm_estimate_t ref;
	pmsm_angle_forcer_config_p config;
	struct {
		burst_signal_p raw;
		burst_long_signal_t force;
		burst_long_signal_t eds;
		burst_long_signal_t total;
	} angle;
	burst_signal_p speed;
	burst_signal_p current;
} pmsm_angle_forcer_t;
typedef pmsm_angle_forcer_t * pmsm_angle_forcer_p;

void pmsm_angle_forcer_begin(pmsm_angle_forcer_p, pmsm_angle_forcer_config_p, burst_signal_p _angle, burst_signal_p _speed, burst_signal_p _current);
void pmsm_angle_forcer_run(pmsm_angle_forcer_p);

typedef struct pmsm_s {
	acwc_t cross;
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
	struct{
		burst_long_signal_t freq;
		burst_long_signal_t angle;
	} synchro;
	inv3ph_t inverter;
	current3ph_t sensor;
	pmsm_estimate_p estimate;
	int mode_prev;
	struct{
		/*struct{
			burst_usignal_t value;
			burst_signal_t delta;
			burst_time_us_t us;
			uint32_t count;
		} voltage;*/
		struct{
			burst_signal_t magnitude;
			burst_signal_t delta;
			burst_time_us_t us;
			uint32_t count;
		} current;
		/*struct{			
			burst_signal_t value;
			uint8_t status;
		} temper;*/
		uint32_t panic;
	} protector;
} pmsm_t;
typedef  pmsm_t * pmsm_p;

void pmsm_mode_synchro_voltage_applay_action(burst_dev_ref_p _ref);
extern burst_dev_mode_t pmcm_synchro_voltage;

void pmsm_mode_synchro_current_applay_action(burst_dev_ref_p _ref);
extern burst_dev_mode_t pmcm_synchro_current;


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
	, burst_signal_p _raw
	, pmsm_estimate_p _estimate
	);
	
	
void pmsm_event_update_feedback(burst_dev_ref_p _dev);
void pmsm_sence_run (pmsm_p _pmsm);
void pmsm_inverter_run (pmsm_p _pmsm);
void pmsm_protector_run (pmsm_p _pmsm);
#endif
