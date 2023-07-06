#ifndef irga_dpt_h
#define irga_dpt_h
#include "burst/modules/actuator.h"
#include "burst/modules/enco_abs32.h"
#include "burst/modules/nikitin.h"
#include "burst/modules/ps_dc.h"

#define pitch_ENCO_OFFSET_NATIVE ((uint32_t)-BURST_LONG_SIGNAL_T(136.9/180.))
#define yaw_ENCO_OFFSET_NATIVE ((uint32_t) BURST_LONG_SIGNAL_T( 53.493/180.))

typedef struct irga_dpt_config_s{
	actuator_config_t  actuator;
	enco_abs32_config_t enco;
	nikitin_config_t spf;
} irga_dpt_config_t;
typedef irga_dpt_config_t * irga_dpt_config_p;

#define IRGA_DPT_CONFIG(a) IRGA_DPT_CONFIG_(a)
#define IRGA_DPT_CONFIG_(a)\
{\
	ACTUATOR_CONFIG(a)\
	, ENCO_ABS32_CONFIG(a##_ENCO)\
	, NIKITIN_CONFIG(a##_SPF)\
}


typedef struct irga_dpt_s{
	actuator_t  actuator;
	actuator_action_t  action;
	actuator_feedback_t  feedback;
} irga_dpt_t;
typedef irga_dpt_t * irga_dpt_p;

/*
	
#define irga_dpt_impl( S, D )\
ps_dc_impl(S##_ps_dc,S.ps)\
nikitin_impl(S##_spf,S.spf)\
enco_abs32_impl(S##_enco,S.enco)\
burst_dev_mode_p S##_modes[ actuator_mode_count] = {\
	&burst_idle_mode\
	, &actuator_mode_voltage\
	, &actuator_mode_speed\
	, &actuator_mode_position\
	, &actuator_mode_fault\
};

#define irga_dpt_setup( S ) \
{\
	{\
		{\
			actuator_event_begin\
			, burst_dev_idle_event\
			, burst_dev_idle_event\
			, burst_dev_idle_event\
			, actuator_update_feedback\
		}\
		,actuator_mode_idle_ix\
		,{\
		}\
		,&S.ps	\
		,&S.enco\
		,&S.spf\
		,&S.motion\
		,&S.positioner;\
	}\
	, ps_dc_setup(S##_ps)\
	, enco_abs32_setup(S##_enco)\
	, nikitin_setup(S##_spf)\
	, {}\
	, {}\
} 

#define IRGA_DPT( S ) BURST_OBJECT(irga_dpt,S)
#define IRGA_DPT_CREATE( S ) BURST_OBJECT_CREATE(irga_dpt,S)
#define IRGA_DPT_SUBCREATE( S,P ) BURST_OBJECT_SUBCREATE(irga_dpt,S,P)
#define IRGA_DPT_SUBSETUP( S,P )  BURST_OBJECT_SUBSETUP(irga_dpt,S,P)
*/




#ifndef ps_DUTY_LIM_LO
#define ps_DUTY_LIM_LO (-(burst_signal_t)IRGA1_PWM_MODULO)
#endif

#ifndef ps_DUTY_LIM_HI
#define ps_DUTY_LIM_HI ((burst_signal_t)IRGA1_PWM_MODULO)
#endif

#ifndef ps_VOLTAGE_LIM_LO
#define ps_VOLTAGE_LIM_LO  BURST_SIGNAL_MIN
#endif

#ifndef ps_VOLTAGE_LIM_HI
#define ps_VOLTAGE_LIM_HI  BURST_SIGNAL_MAX
#endif


//=================================================
#ifndef yaw_REF_TAG
#define yaw_REF_TAG 2
#endif

#ifndef yaw_ENCO_FAULT_TICKS_RESET
#define yaw_ENCO_FAULT_TICKS_RESET 100
#endif

#ifndef yaw_ENCO_FAULT_TICKS_SET
#define yaw_ENCO_FAULT_TICKS_SET 0
#endif

#ifndef yaw_RANGE_VOLTAGE_LO
#define yaw_RANGE_VOLTAGE_LO BURST_SIGNAL_MIN
#endif

#ifndef yaw_RANGE_VOLTAGE_HI
#define yaw_RANGE_VOLTAGE_HI BURST_SIGNAL_MAX
#endif

#ifndef yaw_RANGE_SPEED_LO
#define yaw_RANGE_SPEED_LO -60
#endif

#ifndef yaw_RANGE_SPEED_HI
#define yaw_RANGE_SPEED_HI 60
#endif

#ifndef yaw_RANGE_POSITION_LO
#define yaw_RANGE_POSITION_LO (-BURST_SIGNAL_T( 175./180.)>>3) 
#endif

#ifndef yaw_RANGE_POSITION_HI
#define yaw_RANGE_POSITION_HI (BURST_SIGNAL_T( 175./180.)>>3)
#endif
//=================================================
#ifndef yaw_MOTION_OV_VOLTAGE_PROP_GAIN
#define yaw_MOTION_OV_VOLTAGE_PROP_GAIN 30
#endif

#ifndef yaw_MOTION_OV_VOLTAGE_MODEL_GAIN
#define yaw_MOTION_OV_VOLTAGE_MODEL_GAIN 80
#endif

#ifndef yaw_MOTION_OV_VOLTAGE_DIFF_GAIN
#define yaw_MOTION_OV_VOLTAGE_DIFF_GAIN 0
#endif

#ifndef yaw_MOTION_OV_VOLTAGE_FORCE_GAIN
#define yaw_MOTION_OV_VOLTAGE_FORCE_GAIN 100
#endif

#ifndef yaw_MOTION_OV_VOLTAGE_FORCE_MAX
#define yaw_MOTION_OV_VOLTAGE_FORCE_MAX 10000
#endif

#ifndef yaw_MOTION_OV_VOLTAGE_CONTROL_SHIFT
#define yaw_MOTION_OV_VOLTAGE_CONTROL_SHIFT 0
#endif

#ifndef yaw_MOTION_OV_VOLTAGE_MODEL_SHIFT
#define yaw_MOTION_OV_VOLTAGE_MODEL_SHIFT 10
#endif

#ifndef yaw_MOTION_OV_VOLTAGE_LIMIT_GAIN
#define yaw_MOTION_OV_VOLTAGE_LIMIT_GAIN 0
#endif

#ifndef yaw_MOTION_OV_VOLTAGE_LIMIT_GAIN_PRESC
#define yaw_MOTION_OV_VOLTAGE_LIMIT_GAIN_PRESC 0
#endif

//===============================================
#ifndef yaw_POSITIONER_OV_VOLTAGE_PROP_GAIN
#define yaw_POSITIONER_OV_VOLTAGE_PROP_GAIN 15
#endif

#ifndef yaw_POSITIONER_OV_VOLTAGE_DIFF_GAIN
#define yaw_POSITIONER_OV_VOLTAGE_DIFF_GAIN 50
#endif

#ifndef yaw_POSITIONER_OV_VOLTAGE_DIFF_QUARD_GAIN
#define yaw_POSITIONER_OV_VOLTAGE_DIFF_QUARD_GAIN 0
#endif

#ifndef yaw_POSITIONER_OV_VOLTAGE_CONTROL_SHIFT
#define yaw_POSITIONER_OV_VOLTAGE_CONTROL_SHIFT 7
#endif

#ifndef yaw_POSITIONER_OV_VOLTAGE_DEAD_ZONE
#define yaw_POSITIONER_OV_VOLTAGE_DEAD_ZONE 1
#endif

#ifndef yaw_POSITIONER_OV_VOLTAGE_CRAWL_SPEED
#define yaw_POSITIONER_OV_VOLTAGE_CRAWL_SPEED 1
#endif

#ifndef yaw_SPF_SHIFT
#define yaw_SPF_SHIFT 5
#endif

#ifndef yaw_SPF_PRESC_SHIFT
#define yaw_SPF_PRESC_SHIFT 5
#endif

#ifndef yaw_SPF_VALUE_SHIFT
#define yaw_SPF_VALUE_SHIFT 0
#endif


#ifndef yaw_ENCO_RESOLUTION_ROUND
#define yaw_ENCO_RESOLUTION_ROUND 32
#endif

#ifndef yaw_ENCO_RESOLUTION_RAW
#define yaw_ENCO_RESOLUTION_RAW 13
#endif

#ifndef yaw_ENCO_RESOLUTION_ACTUAL
#define yaw_ENCO_RESOLUTION_ACTUAL 13
#endif

#ifndef yaw_ENCO_INIT_COUNT_BITS
#define yaw_ENCO_INIT_COUNT_BITS 10
#endif

#ifndef yaw_ENCO_OFFSET_NATIVE
#define yaw_ENCO_OFFSET_NATIVE 0
#endif

#ifndef yaw_ENCO_OFFSET_POSITION
#define yaw_ENCO_OFFSET_POSITION 0
#endif

#ifndef yaw_ENCO_INVERCE
#define yaw_ENCO_INVERCE burst_false
#endif
//===================================================
//=================================================
#ifndef pitch_REF_TAG
#define pitch_REF_TAG 2
#endif

#ifndef pitch_ENCO_FAULT_TICKS_RESET
#define pitch_ENCO_FAULT_TICKS_RESET 100
#endif

#ifndef pitch_ENCO_FAULT_TICKS_SET
#define pitch_ENCO_FAULT_TICKS_SET 0
#endif

#ifndef pitch_RANGE_VOLTAGE_LO
#define pitch_RANGE_VOLTAGE_LO BURST_SIGNAL_MIN
#endif

#ifndef pitch_RANGE_VOLTAGE_HI
#define pitch_RANGE_VOLTAGE_HI BURST_SIGNAL_MAX
#endif

#ifndef pitch_RANGE_SPEED_LO
#define pitch_RANGE_SPEED_LO -60
#endif

#ifndef pitch_RANGE_SPEED_HI
#define pitch_RANGE_SPEED_HI 60
#endif

#ifndef pitch_RANGE_POSITION_LO
#define pitch_RANGE_POSITION_LO -BURST_SIGNAL_T( 15./180.)>>3
#endif

#ifndef pitch_RANGE_POSITION_HI
#define pitch_RANGE_POSITION_HI BURST_SIGNAL_T( 90./180.)>>3
#endif
//=================================================
#ifndef pitch_MOTION_OV_VOLTAGE_PROP_GAIN
#define pitch_MOTION_OV_VOLTAGE_PROP_GAIN 300
#endif

#ifndef pitch_MOTION_OV_VOLTAGE_MODEL_GAIN
#define pitch_MOTION_OV_VOLTAGE_MODEL_GAIN 15
#endif

#ifndef pitch_MOTION_OV_VOLTAGE_DIFF_GAIN
#define pitch_MOTION_OV_VOLTAGE_DIFF_GAIN 0
#endif

#ifndef pitch_MOTION_OV_VOLTAGE_FORCE_GAIN
#define pitch_MOTION_OV_VOLTAGE_FORCE_GAIN 50
#endif

#ifndef pitch_MOTION_OV_VOLTAGE_FORCE_MAX
#define pitch_MOTION_OV_VOLTAGE_FORCE_MAX 3000
#endif

#ifndef pitch_MOTION_OV_VOLTAGE_CONTROL_SHIFT
#define pitch_MOTION_OV_VOLTAGE_CONTROL_SHIFT 0
#endif

#ifndef pitch_MOTION_OV_VOLTAGE_MODEL_SHIFT
#define pitch_MOTION_OV_VOLTAGE_MODEL_SHIFT 10
#endif

#ifndef pitch_MOTION_OV_VOLTAGE_LIMIT_GAIN
#define pitch_MOTION_OV_VOLTAGE_LIMIT_GAIN 0
#endif

#ifndef pitch_MOTION_OV_VOLTAGE_LIMIT_GAIN_PRESC
#define pitch_MOTION_OV_VOLTAGE_LIMIT_GAIN_PRESC 0
#endif

//===============================================
#ifndef pitch_POSITIONER_OV_VOLTAGE_PROP_GAIN
#define pitch_POSITIONER_OV_VOLTAGE_PROP_GAIN 120
#endif

#ifndef pitch_POSITIONER_OV_VOLTAGE_DIFF_GAIN
#define pitch_POSITIONER_OV_VOLTAGE_DIFF_GAIN 150
#endif

#ifndef pitch_POSITIONER_OV_VOLTAGE_DIFF_QUARD_GAIN
#define pitch_POSITIONER_OV_VOLTAGE_DIFF_QUARD_GAIN 0
#endif

#ifndef pitch_POSITIONER_OV_VOLTAGE_CONTROL_SHIFT
#define pitch_POSITIONER_OV_VOLTAGE_CONTROL_SHIFT 7
#endif

#ifndef pitch_POSITIONER_OV_VOLTAGE_DEAD_ZONE
#define pitch_POSITIONER_OV_VOLTAGE_DEAD_ZONE 1
#endif

#ifndef pitch_POSITIONER_OV_VOLTAGE_CRAWL_SPEED
#define pitch_POSITIONER_OV_VOLTAGE_CRAWL_SPEED 1
#endif



#ifndef pitch_SPF_SHIFT
#define pitch_SPF_SHIFT 5
#endif

#ifndef pitch_SPF_PRESC_SHIFT
#define pitch_SPF_PRESC_SHIFT 5
#endif

#ifndef pitch_SPF_VALUE_SHIFT
#define pitch_SPF_VALUE_SHIFT 0
#endif


#ifndef pitch_ENCO_RESOLUTION_ROUND
#define pitch_ENCO_RESOLUTION_ROUND 32
#endif

#ifndef pitch_ENCO_RESOLUTION_RAW
#define pitch_ENCO_RESOLUTION_RAW 13
#endif

#ifndef pitch_ENCO_RESOLUTION_ACTUAL
#define pitch_ENCO_RESOLUTION_ACTUAL 13
#endif

#ifndef pitch_ENCO_INIT_COUNT_BITS
#define pitch_ENCO_INIT_COUNT_BITS 10
#endif

#ifndef pitch_ENCO_OFFSET_NATIVE
#define pitch_ENCO_OFFSET_NATIVE 0
#endif

#ifndef pitch_ENCO_OFFSET_POSITION
#define pitch_ENCO_OFFSET_POSITION 0
#endif

#ifndef pitch_ENCO_INVERCE
#define pitch_ENCO_INVERCE burst_true
#endif


void irga_dpt_begin (
	irga_dpt_p _actuator
	, irga_dpt_config_p _config
	, ps_dc_config_p _ps_config
	, ps_dc_p _ps
	, enco_abs32_p _enco
	, nikitin_p _spf
	, burst_motion_p _motion
	, burst_positioner_p _positioner	
);
#endif




