#ifndef burst_pi_h
#define burst_pi_h
#include "burst/burst_signal.h"
#include "burst/burst.h"

typedef struct {
	burst_parametr_t	propGain;
	burst_parametr_t	modelGain;
	burst_parametr_t	diffGain;
	burst_parametr_t	forceGain;
	uint8_t						controlShift;
	uint8_t						modelShift;
	burst_parametr_t	ramp;
} burst_pi_config_t;

typedef burst_pi_config_t * burst_pi_config_p;

#define PI_CONFIG(a) PI_CONFIG_(a)
#define PI_CONFIG_(a)\
{\
	a##_PROP_GAIN\
	,a##_MODEL_GAIN\
	,a##_DIFF_GAIN\
	,a##_FORCE_GAIN\
	,a##_CONTROL_SHIFT\
	,a##_MODEL_SHIFT\
	,a##_RAMP\
}

typedef struct {
	burst_pi_config_p config;
	burst_signal_p			signal_req;
	burst_signal_p			signal;
	burst_signal_p			signal_diff;
	burst_signal_p			signal_force;
	burst_signal_p			controlMax;
	burst_signal_p			controlMin;
	burst_satstate_t *	master_sut_flag;
	burst_long_signal_t model;
	burst_long_signal_t long_model;
	burst_long_signal_t diff;
	burst_long_signal_t force;
	burst_signal_p			control;
	burst_satstate_t		satstate;
	void (* run)(void);
	void (* setup)(	
		burst_pi_config_p _config
		,	burst_signal_p				_signal_req
		, burst_signal_p				_signal
		, burst_signal_p				_control
		, burst_signal_t 				_start_control
		, burst_satstate_t *		_master_sut_flag
		,	burst_signal_p				_controlMax
		, burst_signal_p				_controlMin
		, burst_signal_p			 	_signal_diff
		, burst_signal_p			 	_signal_force
	);
	void (* reset)(burst_signal_t _start_control );
} burst_pi_t;
typedef burst_pi_t * burst_pi_p;

void burst_pi_run_(burst_pi_p _pi);
void burst_pi_reset_(burst_pi_p _pi, burst_signal_t _start_control);

#define BURST_PI( S ) BURST_PI_( S )
#define BURST_PI_( S ) \
extern burst_pi_t  S;

#define BURST_PI_CREATE( S ) BURST_PI_CREATE_( S )
#define BURST_PI_CREATE_( S ) \
BURST_WEAK  void S##_run(void){\
	return burst_pi_run_(&S);\
}\
burst_signal_t S##_def_control_min=BURST_SIGNAL_MIN;\
burst_signal_t S##_def_control_max=BURST_SIGNAL_MAX;\
BURST_WEAK  void S##_setup(\
	burst_pi_config_p _config\
	,	burst_signal_p				_signal_req\
	, burst_signal_p				_signal\
	, burst_signal_p			  _control\
	, burst_signal_t 				_start_control\
	, burst_satstate_t *		_master_sut_flag\
	,	burst_signal_p				_controlMax\
	, burst_signal_p				_controlMin\
	, burst_signal_p				_signal_diff\
	, burst_signal_p				_signal_force\
){ \
	S.config = _config;\
	S.signal_req = _signal_req;\
	S.signal = _signal;\
	S.control = _control;\
	S.master_sut_flag = _master_sut_flag;\
	S.controlMax = _controlMax?_controlMax:&S##_def_control_max;\
	S.controlMin = _controlMin?_controlMin:&S##_def_control_min;\
	S.signal_diff = _signal_diff;\
	S.signal_force = _signal_force;\
	burst_pi_reset_(&S,_start_control); \
}\
BURST_WEAK  void S##_reset(burst_signal_t _start_control){\
	return burst_pi_reset_(&S,_start_control);\
}\
burst_pi_t S ={ \
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
	, burst_satstate_both\
	, S##_run\
	, S##_setup\
	, S##_reset\
}; 


typedef struct burst_limiter_s{
    burst_pi_p r_hi;
    burst_pi_p r_low;
    burst_signal_t zero_signal;
    burst_signal_t signal_hi;
    burst_signal_t signal_low;
    burst_signal_t control_hi;
    burst_signal_t control_low;
    burst_signal_t control_des;
    burst_satstate_t sut_flag;
    burst_signal_p control_req;
    burst_signal_p control_val;
    burst_signal_p signal;
    burst_signal_p controlMax;
    burst_signal_p controlMin;
    burst_signal_p signalMin;
    burst_signal_p signalMax;
    burst_signal_p ramp;
} burst_limiter_t;
typedef burst_limiter_t * burst_limiter_p;
void burst_limiter_run(burst_limiter_p _limiter);
void burst_limiter_reset(burst_limiter_p _limiter, burst_signal_t _def);

typedef struct burst_limiter_config_s{
    burst_pi_p r_hi;
    burst_pi_p r_lo;
    burst_signal_p control_req;
    burst_signal_p control_val;
    burst_signal_p controlMin;
    burst_signal_p controlMax;
    burst_signal_p signal;
    burst_signal_p signalMin;
    burst_signal_p signalMax;
    burst_pi_config_p reg_config;
} burst_limiter_config_t;
typedef burst_limiter_config_t * burst_limiter_config_p;
void burst_limiter_setup(burst_limiter_p _limiter, burst_limiter_config_p _config, burst_signal_t _def );
#endif
