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

typedef struct {
	burst_positioner_config_p config;
	burst_long_signal_p signal_req;
	burst_long_signal_p signal;
	burst_signal_p signal_diff;
	burst_signal_p forceControl;
	burst_signal_p control;
	burst_signal_p controlMax;
	burst_signal_p controlMin;
	void (* run)(void);
	void (* begin)(	
		burst_positioner_config_p _config
		,	burst_long_signal_p				_signal_req
		, burst_long_signal_p				_signal
		,	burst_signal_p						_signal_diff
		,	burst_signal_p						_forceControl
		, burst_signal_p						_control
		,	burst_signal_p						_controlMax
		, burst_signal_p						_controlMin
	);
	void (* reset)( void );
} burst_posicioner_t;
typedef burst_posicioner_t * burst_posicioner_p;

void burst_positioner_begin_( burst_posicioner_p _positioner);
void burst_positioner_run_(burst_posicioner_p _positioner);
void burst_positioner_reset_(burst_posicioner_p _positioner);

#define BURST_POSITIONER( S ) BURST_POSITIONER_( S )
#define BURST_POSITIONER_( S ) \
extern burst_posicioner_t  S;

#define BURST_POSITIONER_CREATE( S ) BURST_POSITIONER_CREATE_( S )
#define BURST_POSITIONER_CREATE_( S ) \
BURST_WEAK  void S##_run(void){\
	burst_positioner_run_(&S);\
}\
BURST_WEAK  void S##_begin(\
	burst_positioner_config_p _config\
	,	burst_long_signal_p				_signal_req\
	, burst_long_signal_p				_signal\
	, burst_signal_p				_signal_diff\
	,	burst_signal_p				_forceControl\
	, burst_signal_p			  _control\
	,	burst_signal_p				_controlMax\
	, burst_signal_p				_controlMin\
){ \
	S.config = _config;\
	S.signal_req = _signal_req;\
	S.signal = _signal;\
	S.signal_diff = _signal_diff;\
	S.forceControl = _forceControl;\
	S.control = _control;\
	S.controlMax = _controlMax;\
	S.controlMin = _controlMin;\
	burst_positioner_begin_(&S); \
}\
BURST_WEAK  void S##_reset(void){\
	burst_positioner_reset_(&S);\
}\
burst_posicioner_t S ={ \
	0 \
	, 0\
	, 0\
	, 0\
	, 0\
	, 0\
	, 0\
	, 0\
	, S##_run\
	, S##_begin\
	, S##_reset\
}; 
#endif
