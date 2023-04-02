#ifndef _actuator_h
#define _actuator_h
//#include "burst/modules/actuator_front.h"
#include "burst/burst_signal.h"
#include "burst/burst.h"
#include "burst/burst_ps.h"
#include "burst/burst_filter.h"
#include "burst/burst_enco.h"
#include "burst/burst_motion.h"
#include "burst/burst_positioner.h"
#include "burst/modules/actuator_front.h"
typedef struct actuator_config_s{
	burst_dev_config_t ref;
	struct{
		int reset;
		int set;
	} enco_fault_ticks;
	struct {
		burst_range_t speed;
		burst_long_range_t position;
	} range;
} actuator_config_t;
typedef actuator_config_t * actuator_config_p;


typedef struct {
	burst_dev_ref_t ref;
	int def_mode;
	burst_range_p voltage_range;
	burst_ps_p ps;	
	burst_enco_p enco;
	burst_filter_p spf;
	burst_motion_p motion;
	burst_positioner_p positioner;
	struct {
		burst_signal_t req;
		burst_range_t range;
	} voltage;
	struct {
		burst_signal_t req;
		burst_range_t range;
	} speed;
	struct {
		burst_long_signal_t req;
		burst_long_range_t range;
	} position;
} actuator_t;
typedef  actuator_t * actuator_p;

void actuator_mode_fault_start(burst_dev_ref_p _ref);
void actuator_mode_fault_runB(burst_dev_ref_p _ref);
extern burst_dev_mode_t actuator_mode_fault;

void actuator_check_enco( burst_dev_ref_p _ref);
void actuator_modee_speed_applay_action(burst_dev_ref_p _ref);
void actuator_mode_speed_start(burst_dev_ref_p _ref);
void actuator_mode_speed_stop(burst_dev_ref_p _ref);
void actuator_mode_speed_runB(burst_dev_ref_p _ref);
extern burst_dev_mode_t actuator_mode_speed;

void actuator_modee_voltage_applay_action(burst_dev_ref_p _ref);
void actuator_mode_voltage_start(burst_dev_ref_p _ref);
void actuator_mode_voltage_stop(burst_dev_ref_p _ref);
extern burst_dev_mode_t actuator_mode_voltage;

void actuator_mode_position_applay_action_(burst_dev_ref_p _ref);
void actuator_mode_position_start_(burst_dev_ref_p _ref);
void actuator_mode_position_stop_(burst_dev_ref_p _ref);
void actuator_mode_position_runB_(burst_dev_ref_p _ref);
extern burst_dev_mode_t actuator_mode_position;

void actuator_event_begin (burst_dev_ref_p _dev);
void actuator_update_feedback(burst_dev_ref_p _dev);

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
	, burst_range_p _voltage_range
);
#endif
