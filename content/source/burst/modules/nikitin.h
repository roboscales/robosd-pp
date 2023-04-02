#ifndef nikitin_h
#define nikitin_h
#include "burst/burst_filter.h"

typedef struct  {
	burst_filter_config_t ref;
	int8_t shift;
	int8_t presc_shift;
	int8_t value_shift;
	int8_t unused;
} nikitin_config_t;

typedef nikitin_config_t * nikitin_config_p;

typedef struct  {
	burst_filter_t ref;
	int8_t shift;
	int8_t presc_shift;
	int8_t value_shift;
	burst_parametr_t gain;
	burst_long_signal_t long_value;
} nikitin_t;

typedef nikitin_t * nikitin_p;

void nikitin_begin(nikitin_p _filter, nikitin_config_p _config );
void nikitin_run_(nikitin_p _filter);
void nikitin_reset_(nikitin_p _filter);

#define nikitin_impl( S, D ) \
BURST_WEAK  void S##_run(void){\
	return nikitin_run_(&D);\
}\
BURST_WEAK  void S##_setup(burst_signal_p _input){ \
	D.ref.input = _input;\
	nikitin_reset_(&D);\
}\
BURST_WEAK  void S##_reset(void){ \
	nikitin_reset_(&D);\
}

#define nikitin_setup( S ) \
{\
	{\
		0\
		,0\
		,0\
		, S##_run\
		, S##_setup\
		, S##_reset\
	}\
} 

#define NIKITIN( S ) BURST_OBJECT(nikitin,S)

#define NIKITIN_CREATE( S ) BURST_OBJECT_CREATE(nikitin,S)

#define NIKITIN_SUBCREATE( S,P ) BURST_OBJECT_SUBCREATE(nikitin,S,P)

#define NIKITIN_SUBSETUP( S,P )  BURST_OBJECT_SUBSETUP(nikitin,S,P)

#endif



