#ifndef enco_abs32_h
#define enco_abs32_h
#include "burst/burst_enco.h"

typedef struct  enco_abs32_config_s{
	burst_enco_config_t ref;
	struct{
		uint8_t round;
		uint8_t raw;
		uint8_t actual;
	} resolution;
	uint8_t init_count_bits;
	struct{
		uint32_t native;
		burst_long_signal_t position;
	} offset;
	burst_bool_t inverce;
} enco_abs32_config_t;
typedef enco_abs32_config_t * enco_abs32_config_p;

#define ENCO_ABS32_CONFIG(a) ENCO_ABS32_CONFIG_(a)
#define ENCO_ABS32_CONFIG_(a)\
{\
	ENCO_CONFIG(a)\
	,{\
		a##_RESOLUTION_ROUND\
		, a##_RESOLUTION_RAW\
		, a##_RESOLUTION_ACTUAL\
	}\
	, a##_INIT_COUNT_BITS\
	,{\
		a##_OFFSET_NATIVE\
		,a##_OFFSET_POSITION\
	}\
}

typedef struct  enco_abs32_s {
	burst_enco_t ref;
	void (* query)(void);
	uint32_t (* encode)(void);
	burst_bool_t (* error)(void);
	struct{
		burst_long_signal_t native;
		burst_long_signal_t position;
	} offset;
	uint32_t start_pause_tick;
	struct{
		int raw;
		int value;
	} shift;
	struct {
		uint32_t raw;
		uint32_t ceiled;
		int32_t delta;
	} native;
	burst_signal_t delta;
	burst_long_signal_t acc;
} enco_abs32_t;
typedef enco_abs32_t * enco_abs32_p;

void enco_abs32_begin(enco_abs32_p,enco_abs32_config_p);

void enco_abs32_run_(enco_abs32_p);
void enco_abs32_reset_(enco_abs32_p);

#define enco_abs32_impl( S, D ) \
BURST_WEAK  void S##_run(void){\
	return enco_abs32_run_(&D);\
}\
BURST_WEAK  void S##_reset(void){ \
	enco_abs32_reset_(&D); \
}\
BURST_WEAK  void S##_query(void){\
}\
BURST_WEAK  uint32_t S##_encode(void){\
	return 0;\
}\
BURST_WEAK  burst_bool_t S##_error(void){\
	return burst_false;\
}

#define enco_abs32_setup( S ) \
{ \
	{\
		0 \
		,{ \
		} \
		,0 \
		,0 \
		,burst_false \
		, S##_run\
		, S##_reset\
	}\
	, S##_query\
	, S##_encode\
	, S##_error\
} 


#define ENCO_ABS32( S ) BURST_OBJECT(enco_abs32,S)

#define ENCO_ABS32_CREATE( S ) BURST_OBJECT_CREATE(enco_abs32,S)

#define ENCO_ABS32_SUBCREATE( S,P ) BURST_OBJECT_SUBCREATE(enco_abs32,S,P)

#define ENCO_ABS32_SUBSETUP( S,P )  BURST_OBJECT_SUBSETUP(enco_abs32,S,P)

#endif
