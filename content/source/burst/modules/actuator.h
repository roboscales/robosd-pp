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

#ifndef BURST_PANICS_ACTUATOR_TEMPER_ENABLED
#define BURST_PANICS_ACTUATOR_TEMPER_ENABLED 0
#endif

typedef struct actuator_config_s{
	burst_dev_config_t ref;
	struct{
		burst_time_us_t reset;
		burst_time_us_t set;
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
	struct {
		#if BURST_PROTECTION_ENABLED == 1
		#if BURST_PANICS_ACTUATOR_TEMPER_ENABLED == 1
		burst_hyst_t temper_pp;		
		#endif
		#endif
	} panic;
} actuator_config_t;
typedef actuator_config_t * actuator_config_p;

#define ACTUATOR_VAR_REG(t,h,n) ACTUATOR_VAR_REG_(t,h,n)

#if BURST_PROTECTION_ENABLED == 1
#if BURST_PANICS_ACTUATOR_TEMPER_ENABLED == 1
#define ACTUATOR_VAR_REG_(t,h,n)\
BURST_VAR_PUSH(t, n)\
	BURST_VAR_PUSH(t, "enco_fault_ticks")\
		BURST_VAR_REG(t,h->enco_fault_ticks.reset,"reset",uint32)\
		BURST_VAR_REG(t,h->enco_fault_ticks.set,"set",uint32)\
	BURST_VAR_POP(t)\
	BURST_VAR_PUSH(t, "range")\
		RANGE_VAR_REG(t,(&(h->voltage)),"voltage")\
		RANGE_VAR_REG(t,(&(h->speed)),"speed")\
		LONG_RANGE_VAR_REG(t,(&(h->position)),"position")\
	BURST_VAR_POP(t)\
	BURST_VAR_PUSH(t, "modes")\
		MOTION_VAR_REG(t,(&(h->modes.motion)),"mo")\
		POSITIONER_VAR_REG(t,(&(h->modes.positioner)),"po")\
	BURST_VAR_POP(t)\
	BURST_VAR_PUSH(t, "panic")\
		burst_hyst_t temper_pp;		
	BURST_VAR_POP(t)\
BURST_VAR_POP(t)
#endif
#endif

#ifndef ACTUATOR_VAR_REG_
#define ACTUATOR_VAR_REG_(t,h,n)\
BURST_VAR_PUSH(t, n)\
	BURST_VAR_PUSH(t, "eft")\
		BURST_VAR_REG(t,h->enco_fault_ticks.reset,"reset",uint32)\
		BURST_VAR_REG(t,h->enco_fault_ticks.set,"set",uint32)\
	BURST_VAR_POP(t)\
	BURST_VAR_PUSH(t, "r")\
		RANGE_VAR_REG(t,(&(h->range.voltage)),"v")\
		RANGE_VAR_REG(t,(&(h->range.speed)),"sp")\
		LONG_RANGE_VAR_REG(t,(&(h->range.position)),"po")\
	BURST_VAR_POP(t)\
	BURST_VAR_PUSH(t, "modes")\
		MOTION_VAR_REG(t,(&(h->modes.motion)),"mo")\
		POSITIONER_VAR_REG(t,(&(h->modes.positioner)),"po")\
	BURST_VAR_POP(t)\
BURST_VAR_POP(t)
#endif

#if BURST_PANICS_ACTUATOR_TEMPER_ENABLED == 1 && BURST_PROTECTION_ENABLED == 1

#define BURST_PANICS_ACTUATOR_TEMPER_CO(a)\
{\
	a##_PANICS_ACTUATOR_TEMPER_OVERHI_PP\
	, a##_PANICS_ACTUATOR_TEMPER_HI_PP\
	, a##_PANICS_ACTUATOR_TEMPER_LO_PP\
	, a##_PANICS_ACTUATOR_TEMPER_ULTRALO_PP\
}
#else
#define BURST_PANICS_ACTUATOR_TEMPER_CO(a)
#endif

#define ACTUATOR_CONFIG(a) ACTUATOR_CONFIG_(a)
#define ACTUATOR_CONFIG_(a)\
{\
	DEV_CONFIG(a##_REF)\
	,{\
		a##_ENCO_FAULT_TICKS_RESET\
		,a##_ENCO_FAULT_TICKS_SET\
	}\
	,{\
		RANGE_CONFIG(a##_RANGE_VOLTAGE)\
		, RANGE_CONFIG(a##_RANGE_SPEED)\
		, RANGE_CONFIG(a##_RANGE_POSITION)\
	}\
	,{\
			MOTION_CONFIG(a##_MOTION_OV_VOLTAGE)\
			,POSITIONER_CONFIG(a##_POSITIONER_OV_VOLTAGE)\
	}\
	,{\
		BURST_PANICS_ACTUATOR_TEMPER_CO(a)\
	}\
}
struct actuator_s;
typedef struct actuator_s actuator_t;
typedef actuator_t * actuator_p;
typedef burst_signal_t ( * burst_actuator_prf)(void);

struct actuator_s {
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
	#if BURST_PANICS_ACTUATOR_TEMPER_ENABLED == 1 && BURST_PROTECTION_ENABLED == 1
	burst_actuator_prf temper_pp;
	#endif
} ;


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

#if BURST_PROTECTION_ENABLED == 1
void burst_actuator_realtime_protection(burst_dev_ref_p _ref);
void burst_actuator_frontend_protection(burst_dev_ref_p _ref);	
#endif 
	
#endif
