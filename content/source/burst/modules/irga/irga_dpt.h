#ifndef irga_dpt_h
#define irga_dpt_h
#include "burst/modules/actuator.h"
#include "burst/modules/enco_abs32.h"
#include "burst/modules/nikitin.h"
#include "burst/modules/ps_dc.h"

typedef struct irga_dpt_config_s{
	actuator_config_t  actuator;
	ps_dc_config_t ps;
	enco_abs32_config_t enco;
	nikitin_config_t spf;
	burst_motion_config_t motion;
	burst_positioner_config_t positioner;	
} irga_dpt_config_t;
typedef irga_dpt_config_t * irga_dpt_config_p;

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

void irga_dpt_begin (
	irga_dpt_p _actuator
	, irga_dpt_config_p _config
	, ps_dc_p _ps
	, enco_abs32_p _enco
	, nikitin_p _spf
	, burst_motion_p _motion
	, burst_positioner_p _positioner	
);
#endif
