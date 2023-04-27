#ifndef burst_positioner_h
#define burst_positioner_h
#include "burst/burst_signal.h"
#include "burst/burst.h"

typedef struct {
	burst_parametr_t propGain;
	burst_parametr_t diffGain;
	burst_parametr_t diffQuardGain;
	uint8_t controlShift;
  burst_signal_t deadZone;
  burst_signal_t crawlSpeed;
} burst_positioner_config_t;

typedef burst_positioner_config_t * burst_positioner_config_p;

typedef struct burst_positioner_s{
	void (* run)(void);
	void (* setup)(			
		burst_long_signal_p				_signal_req
		, burst_long_signal_p				_signal
		,	burst_signal_p						_signal_diff
		,	burst_signal_p						_forceControl
		, burst_signal_p						_control
		,	burst_signal_p						_controlMax
		, burst_signal_p						_controlMin
		, burst_positioner_config_p _config
	);
	void (* reset)( void );
	burst_positioner_config_p config;
	burst_long_signal_p signal_req;
	burst_long_signal_p signal;
	burst_signal_p signal_diff;
	burst_signal_p forceControl;
	burst_signal_p control;
	burst_signal_p controlMax;
	burst_signal_p controlMin;
} burst_positioner_t;
typedef burst_positioner_t * burst_positioner_p;

void burst_positioner_setup_(
	burst_positioner_p _positioner
	, burst_long_signal_p _signal_req
	, burst_long_signal_p _signal
	, burst_signal_p _signal_diff
	, burst_signal_p _forceControl
	, burst_signal_p _control
	, burst_signal_p _controlMax
	, burst_signal_p _controlMin
	, burst_positioner_config_p _config
);
void burst_positioner_run_(burst_positioner_p _positioner);
void burst_positioner_reset_(burst_positioner_p _positioner);


#define burst_positioner_impl( S, D ) \
BURST_WEAK  void S##_run(void){\
	burst_positioner_run_(&D);\
}\
BURST_WEAK  void S##_setup(\
		burst_long_signal_p				_signal_req\
	, burst_long_signal_p				_signal\
	, burst_signal_p				_signal_diff\
	,	burst_signal_p				_forceControl\
	, burst_signal_p			  _control\
	,	burst_signal_p				_controlMax\
	, burst_signal_p				_controlMin\
	, burst_positioner_config_p _config\
){ \
	burst_positioner_setup_(\
		&D\
		, _signal_req\
		, _signal\
		, _signal_diff\
		, _forceControl\
		, _control\
		, _controlMax\
		, _controlMin\
		, _config\
	);\
}\
BURST_WEAK  void S##_reset(void){\
	burst_positioner_reset_(&S);\
}


#define burst_positioner_setup( S ) \
{ \
	 S##_run\
	, S##_setup\
	, S##_reset\
}; 

#define BURST_POSITIONER( S ) BURST_OBJECT(burst_positioner,S)

#define BURST_POSITIONER_CREATE( S ) BURST_OBJECT_CREATE(burst_positioner,S)

#define BURST_POSITIONER_SUBCREATE( S,P ) BURST_OBJECT_SUBCREATE(burst_positioner,S,P)

#define BURST_POSITIONER_SUBSETUP( S,P )  BURST_OBJECT_SUBSETUP(burst_positioner,S,P)

#endif
