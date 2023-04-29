#ifndef burst_modules_actuator_h
#define burst_modules_actuator_h
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
		burst_range_t voltage;
		burst_range_t speed;
		burst_long_range_t position;
	} range;
	struct {
		burst_motion_config_t motion;
		burst_positioner_config_t positioner;
	} modes;
} actuator_config_t;
typedef actuator_config_t * actuator_config_p;


typedef struct {
	burst_dev_ref_t ref;
	int def_mode;
	burst_ps_p ps;	
	burst_enco_p enco;
	burst_motion_p motion;
	burst_positioner_p positioner;
	struct {
		burst_signal_t des;
		burst_signal_t req;
		burst_range_t range;
	} voltage;
	struct {
		burst_signal_t req;
		burst_range_t range;
		burst_filter_p flt;
	} speed;
	struct {
		burst_long_signal_t req;
		burst_long_range_t range;
	} position;
} actuator_t;
typedef  actuator_t * actuator_p;

//void actuator_mode_fault_start(burst_dev_ref_p _ref);
//void actuator_mode_fault_runB(burst_dev_ref_p _ref);
extern burst_dev_mode_t actuator_mode_fault;

//void actuator_check_enco( burst_dev_ref_p _ref);
void actuator_mode_speed_applay_action(burst_dev_ref_p _ref);
//void actuator_mode_speed_start(burst_dev_ref_p _ref, burst_motion_config_p _config);
//void actuator_mode_speed_stop(burst_dev_ref_p _ref);
//void actuator_mode_speed_runB(burst_dev_ref_p _ref);
extern burst_dev_mode_t actuator_mode_speed;

void actuator_mode_voltage_applay_action(burst_dev_ref_p _ref);
//void actuator_mode_voltage_start(burst_dev_ref_p _ref);
//void actuator_mode_voltage_stop(burst_dev_ref_p _ref);
extern burst_dev_mode_t actuator_mode_voltage;

void actuator_mode_position_applay_action(burst_dev_ref_p _ref);
//void actuator_mode_position_start(burst_dev_ref_p _ref,  burst_motion_config_p _motion_config,  burst_positioner_config_p _config);
//void actuator_mode_position_stop(burst_dev_ref_p _ref);
//void actuator_mode_position_runB(burst_dev_ref_p _ref);
extern burst_dev_mode_t actuator_mode_position;

void actuator_event_update_feedback(burst_dev_ref_p _dev);

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
);
	
	
#endif
