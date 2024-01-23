#ifndef burst_modules_front_h
#define burst_modules_front_h
#include "burst/modules/actuator.h"
#include "burst/modules/acwc_front.h"

#include "burst/burst_pi.h"

#ifndef BURST_PANICS_ACWC_OVERCURRENT_ENABLED
#define BURST_PANICS_ACWC_OVERCURRENT_ENABLED 0
#endif

typedef struct acwc_config_s{
	actuator_config_t ac;
	struct{
		burst_pi_config_t pi;
		burst_range_t range;
	} current;
	struct {
		struct{
			burst_motion_config_t motion;
			burst_positioner_config_t positioner;
		} voltage_cl;
		struct{
			burst_motion_config_t motion;
			burst_positioner_config_t positioner;
		} current;
	} modes;
	#if BURST_PROTECTION_ENABLED == 1
	#if BURST_PANICS_ACWC_OVERCURRENT_ENABLED ==1
	struct{
		burst_signal_t  overcurrent_pp;
		burst_signal_t  overpower_pp;
		burst_signal_t  normpower_pp;
		burst_time_us_t  overpower_tm_us;
	} panic;
	#endif
	#endif
} acwc_config_t;
typedef acwc_config_t * acwc_config_p;

#define ACWC_VAR_REG(t,h,n) ACWC_VAR_REG_(t,h,n)
#define ACWC_VAR_REG_(t,h,n)\
BURST_VAR_PUSH(t, n)\
	ACTUATOR_VAR_REG(t,(&(h->ac)),"ac")\
	BURST_VAR_PUSH(t, "c")\
		PI_VAR_REG(t,(&(h->current.pi)),"pi")\
		RANGE_VAR_REG(t,(&(h->current.range)),"range")\
	BURST_VAR_POP(t)\
	BURST_VAR_PUSH(t, "modes")\
		BURST_VAR_PUSH(t, "c")\
			MOTION_VAR_REG(t,(&(h->modes.current.motion)),"mo")\
			POSITIONER_VAR_REG(t,(&(h->modes.current.positioner)),"po")\
		BURST_VAR_POP(t)\
		BURST_VAR_PUSH(t, "cl")\
			MOTION_VAR_REG(t,(&(h->modes.voltage_cl.motion)),"mo")\
			POSITIONER_VAR_REG(t,(&(h->modes.voltage_cl.positioner)),"po")\
		BURST_VAR_POP(t)\
	BURST_VAR_POP(t)\
BURST_VAR_POP(t)

//			POSITIONER_VAR_RERG(t,(&(h->modes.current.positioner)),"po")\
	//		MOTION_VAR_RERG(t,(&(h->modes.current.motion)),"mo")\

#if BURST_PANICS_ACWC_OVERCURRENT_ENABLED ==1 && 	BURST_PROTECTION_ENABLED == 1

#define BURST_PANICS_ACWC_OVERCURRENT_CO(a)\
,{\
	a##_PANICS_ACWC_OWERCURRENT_PP\
	, a##_PANICS_ACWC_OWERPOWER_PP\
	, a##_PANICS_ACWC_NORMPOWER_PP\
	, a##_PANICS_ACWC_OWERPOWER_TM_US\
}
#else
#define BURST_PANICS_ACWC_OVERCURRENT_CO(a)
#endif

#define ACWC_CONFIG(a,b) ACWC_CONFIG_(a,b)
#define ACWC_CONFIG_(a,b)\
{\
	ACTUATOR_CONFIG(a)\
	,{\
		PI_CONFIG(b##_CURRENT_PI)\
		, RANGE_CONFIG(b##_CURRENT_RANGE)\
	}\
	,{\
		{\
			MOTION_CONFIG(a##_MOTION_OV_VOLTAGE_CL)\
			,POSITIONER_CONFIG(a##_POSITIONER_OV_VOLTAGE_CL)\
		}\
		,{\
			MOTION_CONFIG(a##_MOTION_OV_CURRENT)\
			,POSITIONER_CONFIG(a##_POSITIONER_OV_CURRENT)\
		}\
	}\
	BURST_PANICS_ACWC_OVERCURRENT_CO(a)\
}
	
typedef struct {
	actuator_t ac;
	struct {
		burst_signal_t req;
		burst_range_t range;
		burst_pi_p dir;
		burst_pi_p hi;
		burst_pi_p lo;
		burst_limiter_t limiter;
		burst_filter_p flt;
		burst_signal_p raw;
		struct {
			#if BURST_PANICS_ACWC_OVERCURRENT_ENABLED ==1 && 	BURST_PROTECTION_ENABLED == 1
			burst_actuator_prf get;
			#endif
			burst_signal_t actual;
			burst_signal_t delta;
			burst_time_us_t us;
		} magnitude;
	} current;	
} acwc_t;
typedef  acwc_t * acwc_p;


void acwc_mode_voltage_cl_applay_action(burst_dev_ref_p _ref);
extern burst_dev_mode_t acwc_mode_voltage_cl;

void acwc_mode_speed_cl_applay_action(burst_dev_ref_p _ref);
extern burst_dev_mode_t acwc_mode_cl_speed;

void acwc_mode_position_cl_applay_action_(burst_dev_ref_p _ref);
extern burst_dev_mode_t acwc_mode_cl_position;


void acwc_mode_current_applay_action(burst_dev_ref_p _ref);
extern burst_dev_mode_t acwc_mode_current;

void acwc_mode_speed_applay_action(burst_dev_ref_p _ref);
extern burst_dev_mode_t acwc_mode_speed;

void acwc_mode_position_applay_action_(burst_dev_ref_p _ref);
extern burst_dev_mode_t acwc_mode_position;


void acwc_event_update_feedback(burst_dev_ref_p _dev);

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
);

#if BURST_PROTECTION_ENABLED == 1
void burst_acwc_realtime_protection(burst_dev_ref_p _ref);
void burst_acwc_frontend_protection(burst_dev_ref_p _ref);	
#if BURST_PANICS_ACWC_OVERCURRENT_ENABLED ==1
burst_signal_t burst_acwc_magnitude_get(acwc_p _acwc);
#endif
#endif
	
#endif
