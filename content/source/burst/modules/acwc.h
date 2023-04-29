#ifndef burst_modules_front_h
#define burst_modules_front_h
#include "burst/modules/actuator.h"
#include "burst/modules/acwc_front.h"

#include "burst/burst_pi.h"

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
} acwc_config_t;
typedef acwc_config_t * acwc_config_p;


	
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


#endif
