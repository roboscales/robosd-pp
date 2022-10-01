#ifndef burst_filter_h
#define burst_filter_h
#include "burst/burst_signal.h"
#include "stdint.h"

typedef struct  {
	int8_t shift;
	int8_t presc_shift;
	int8_t value_shift;
	int8_t unused;
} burst_filter_config_t;

typedef burst_filter_config_t * burst_filter_config_p;

typedef struct  {
	int8_t shift;
	int8_t presc_shift;
	int8_t value_shift;
	burst_parametr_t gain;
	burst_long_signal_t long_value;
	burst_signal_t value;
	burst_filter_config_p config;
	burst_signal_p input;
	void (* run)(void);
	void (* begin)(burst_filter_config_p, burst_signal_p _input, burst_signal_t);
} burst_filter_t;

typedef burst_filter_t * burst_filter_p;

void burst_filter_begin_(burst_filter_p _filter, burst_signal_t _value);
void burst_filter_run_(burst_filter_p _filter);

#define BURST_FILTER( S ) BURST_FILTER_( S )
#define BURST_FILTER_( S ) \
extern burst_filter_t  S;

#define BURST_FILTER_CREATE( S ) BURST_FILTER_CREATE_( S )
#define BURST_FILTER_CREATE_( S ) \
BURST_WEAK  void S##_run(void){\
	return burst_filter_run_(&S);\
}\
BURST_WEAK  void S##_begin(burst_filter_config_p _config, burst_signal_p _input, burst_signal_t _value){ \
	S.input = _input;\
	S.config = _config;\
	burst_filter_begin_(&S,_value); \
}\
burst_filter_t S ={ \
	0 \
	, 0 \
	, 0 \
	, 1 \
	, 0 \
	, 0 \
	, 0 \
	, 0 \
	, S##_run\
	, S##_begin\
}; 
#endif
