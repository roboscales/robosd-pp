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

typedef struct {
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
	burst_satstate_t		satstate;
	void (* run)(void);
	void (* begin)(	burst_motion_config_p _config
		,	burst_signal_p				_signal_req
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
	);
	void (* reset)(burst_signal_t _start_control );
} burst_motion_t;
typedef burst_motion_t * burst_motion_p;

void burst_motion_begin_( burst_motion_p _motion, burst_signal_t _start_control );
void burst_motion_run_(burst_motion_p _motion);
void burst_motion_reset_(burst_motion_p _motion, burst_signal_t _start_control);

#define BURST_MOTION( S ) BURST_MOTION_( S )
#define BURST_MOTION_( S ) \
extern burst_motion_t  S;

#define BURST_MOTION_CREATE( S ) BURST_MOTION_CREATE_( S )
#define BURST_MOTION_CREATE_( S ) \
BURST_WEAK  void S##_run(void){\
	return burst_motion_run_(&S);\
}\
BURST_WEAK  void S##_begin(\
	burst_motion_config_p _config\
	,	burst_signal_p				_signal_req\
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
){ \
	S.config = _config;\
	S.signal_req = _signal_req;\
	S.signal = _signal;\
	S.control = _control;\
	S.master_sut_flag = _master_sut_flag;\
	S.controlMax = _controlMax;\
	S.controlMin = _controlMin;\
	S.signal_diff = _signal_diff;\
	S.reference = _reference;\
	S.reference_max = _reference_max;\
	S.reference_min = _reference_min;\
	burst_motion_begin_(&S,_start_control); \
}\
BURST_WEAK  void S##_reset(burst_signal_t _start_control){\
	return burst_motion_reset_(&S,_start_control);\
}\
burst_motion_t S ={ \
	0 \
	, 0\
	, 0\
	, 0\
	, 0\
	, 0\
	, 0\
	, 0\
	, 0\
	, 0\
	, 0\
	, 0\
	, 0\
	, 0\
	, burst_satstate_both\
	, S##_run\
	, S##_begin\
	, S##_reset\
}; 
#endif
