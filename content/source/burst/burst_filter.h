#ifndef burst_filter_h
#define burst_filter_h
#include "burst/burst_signal.h"
#include "stdint.h"

typedef struct  {
} burst_filter_config_t;

typedef burst_filter_config_t * burst_filter_config_p;

typedef struct  {
	burst_signal_t value;
	burst_filter_config_p config;
	burst_signal_p input;
	void (* run)(void);
	void (* setup)(burst_signal_p _input);
	void (* reset)(void);
} burst_filter_t;

typedef burst_filter_t * burst_filter_p;
void burst_filter_setup_(burst_signal_p _input);
#define burst_filter_impl( S, D ) \
BURST_WEAK  void S##_run(void){\
}\
BURST_WEAK  void S##_setup(burst_signal_p _input){\
	burst_filter_setup_( &D, _input);\
}\
BURST_WEAK  burst_bool_t S##_reset(void){\
}

#define burst_filter_setup(S)\
{ \
	, 0 \
	, 0 \
	, 0 \
	, S##_run \
	, S##_setup \
	, S##_reset \
}

#define BURST_FILTER( S ) BURST_OBJECT(burst_filter,S)

#define BURST_FILTER_CREATE( S ) BURST_OBJECT_CREATE(burst_filter,S)

#define BURST_FILTER_SUBCREATE( S,P ) BURST_OBJECT_SUBCREATE(burst_filter,S,P)

#define BURST_FILTER_SUBSETUP( S,P )  BURST_OBJECT_SUBSETUP(burst_filter,S,P)

#endif
