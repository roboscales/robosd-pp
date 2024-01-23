#ifndef burst_motion_h
#define burst_motion_h
#include "burst/burst_signal.h"
#include "burst/burst.h"
#ifndef BURST_MOTION_VIRTUAL_ELASTIC_ENABLED
#define BURST_MOTION_VIRTUAL_ELASTIC_ENABLED 0
#endif
typedef struct {
	burst_parametr_t	propGain;
	burst_parametr_t	modelGain;
	burst_parametr_t	diffGain;
	burst_parametr_t	forceGain;
	burst_signal_t		forceMax;
	uint8_t						controlShift;
	uint8_t						modelShift;
	burst_parametr_t	limitGain;
	uint8_t						limitGainPresc;    
} burst_motion_config_t;

typedef burst_motion_config_t * burst_motion_config_p;

#define MOTION_VAR_REG(t,h,n) MOTION_VAR_REG_(t,h,n)
#define MOTION_VAR_REG_(t,h,n)\
BURST_VAR_PUSH(t, n)\
	BURST_VAR_REG(t,h->propGain,"prop",burst_parametr_var)\
	BURST_VAR_REG(t,h->modelGain,"model",burst_parametr_var)\
	BURST_VAR_REG(t,h->diffGain,"diff",burst_parametr_var)\
	BURST_VAR_PUSH(t, "force")\
		BURST_VAR_REG(t,h->forceGain,"gain",burst_parametr_var)\
		BURST_VAR_REG(t,h->forceMax,"max",burst_signal_var)\
	BURST_VAR_POP(t)\
	BURST_VAR_REG(t,h->controlShift,"cs",uint8)\
	BURST_VAR_REG(t,h->modelShift,"ms",uint8)\
	BURST_VAR_PUSH(t, "limit")\
		BURST_VAR_REG(t,h->limitGain,"gain",burst_parametr_var)\
		BURST_VAR_REG(t,h->limitGainPresc,"presc",uint8)\
	BURST_VAR_POP(t)\
BURST_VAR_POP(t)

#define MOTION_CONFIG(a) MOTION_CONFIG_(a)
#define MOTION_CONFIG_(a)\
{\
	a##_PROP_GAIN\
	, a##_MODEL_GAIN\
	, a##_DIFF_GAIN\
	, a##_FORCE_GAIN\
	, a##_FORCE_MAX\
	, a##_CONTROL_SHIFT\
	, a##_MODEL_SHIFT\
	, a##_LIMIT_GAIN\
	, a##_LIMIT_GAIN_PRESC\
}

typedef struct {
	void (* run)(void);
	void (* setup)(	
			burst_signal_p				_signal_req
		, burst_signal_p				_signal
		, burst_signal_p				_control
		, burst_signal_t 				_start_control
		, burst_satstate_t *		_master_sut_flag
		,	burst_signal_p				_controlMax
		, burst_signal_p				_controlMin
		, burst_signal_p				_signal_diff
		, burst_long_signal_p 	_reference
		, burst_long_signal_p 	_reference_max
		, burst_long_signal_p 	_reference_min
		, burst_motion_config_p _config
	);
	void (* reset)(burst_signal_t _start_control );
	burst_satstate_t		satstate;
	burst_motion_config_p config;
	burst_signal_p			signal_req;
	burst_signal_p			signal;
	burst_signal_p			signal_diff;
	burst_signal_p			controlMax;
	burst_signal_p			controlMin;
	burst_long_signal_p reference;
	burst_long_signal_p reference_max;
	burst_long_signal_p reference_min;
	burst_satstate_t *	master_sut_flag;
	burst_long_signal_t model;
	burst_long_signal_t force;
	burst_long_signal_t long_model;
	burst_signal_t			* control;

} burst_motion_t;
typedef burst_motion_t * burst_motion_p;

void burst_motion_run_(burst_motion_p _motion);
void burst_motion_reset_(burst_motion_p _motion, burst_signal_t _start_control);
void burst_motion_setup_(
	  burst_motion_p 				_motion
	,	burst_signal_p				_signal_req
	, burst_signal_p				_signal
	, burst_signal_p			  _control
	, burst_signal_t 				_start_control
	, burst_satstate_t *		_master_sut_flag
	,	burst_signal_p				_controlMax
	, burst_signal_p				_controlMin
	, burst_signal_p				_signal_diff
	, burst_long_signal_p 	_reference
	, burst_long_signal_p 	_reference_max
	, burst_long_signal_p 	_reference_min
	, burst_motion_config_p _config
);

#define burst_motion_impl( S, D ) \
BURST_WEAK  void S##_run(void){\
	return burst_motion_run_(&D);\
}\
BURST_WEAK  void S##_setup(\
	burst_signal_p				_signal_req\
	, burst_signal_p				_signal\
	, burst_signal_p			  _control\
	, burst_signal_t 				_start_control\
	, burst_satstate_t *		_master_sut_flag\
	,	burst_signal_p				_controlMax\
	, burst_signal_p				_controlMin\
	, burst_signal_p				_signal_diff\
	, burst_long_signal_p 	_reference\
	, burst_long_signal_p 	_reference_max\
	, burst_long_signal_p 	_reference_min\
	, burst_motion_config_p _config\
){ \
	burst_motion_setup_(\
		&D\
		,_signal_req\
		,_signal\
		,_control\
		,_start_control\
		,_master_sut_flag\
		,_controlMax\
		,_controlMin\
		,_signal_diff\
		,_reference\
		,_reference_max\
		,_reference_min\
		,_config\
	);\
}\
BURST_WEAK  void S##_reset(burst_signal_t _start_control){\
	return burst_motion_reset_(&D,_start_control);\
}

#define burst_motion_setup( S ) \
{\
	S##_run\
	, S##_setup\
	, S##_reset\
	, burst_satstate_both\
}; 

#define BURST_MOTION( S ) BURST_OBJECT(burst_motion,S)

#define BURST_MOTION_CREATE( S ) BURST_OBJECT_CREATE(burst_motion,S)

#define BURST_MOTION_SUBCREATE( S,P ) BURST_OBJECT_SUBCREATE(burst_motion,S,P)

#define BURST_MOTION_SUBSETUP( S,P )  BURST_OBJECT_SUBSETUP(burst_motion,S,P)


#endif
