#ifndef burst_enco_h
#define burst_enco_h
#include "burst/burst_signal.h"

typedef struct  {
	struct{
		uint8_t round;
		uint8_t raw;
		uint8_t actual;
	} resolution;
	uint8_t init_count_shift;
	struct{
		uint32_t native;
		burst_long_signal_t position;
	} offset;
	burst_bool_t inverce;
} burst_enco_config_t;
typedef burst_enco_config_t * burst_enco_config_p;

typedef struct  {
			burst_enco_config_p config;
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
			struct{
				unsigned fault;
				unsigned total;
			} counter;
			burst_signal_t delta;
			burst_signal_t delta_acc;
			burst_long_signal_t acc;
			burst_long_signal_t position;
			void (* run)(void);
			void (* begin)(burst_enco_config_p);
			void (* query)(void);
			uint32_t (* encode)(void);
			burst_bool_t (* error)(void);
} burst_enco_t;
typedef burst_enco_t * burst_enco_p;
void burst_enco_run_(burst_enco_p);
void burst_enco_begin_(burst_enco_p,burst_enco_config_p);

#define BURST_ENCO( S ) BURST_ENCO_( S )
#define BURST_ENCO_( S ) \
extern burst_enco_t  S;

#define BURST_ENCO_CREATE( S ) BURST_ENCO_CREATE_( S )
#define BURST_ENCO_CREATE_( S ) \
BURST_WEAK  void S##_run(void){\
	return burst_enco_run_(&S);\
}\
BURST_WEAK  void S##_begin(burst_enco_config_p _config){ \
	burst_enco_begin_(&S,_config); \
}\
BURST_WEAK  void S##_query(void){\
}\
BURST_WEAK  uint32_t S##_encode(void){\
	return 0;\
}\
BURST_WEAK  burst_bool_t S##_error(void){\
	return burst_false;\
}\
burst_enco_t S ={ \
		0 \
		,{ \
		} \
		,0 \
		,{ \
		} \
		,{ \
		} \
		,{ \
		} \
		,0 \
		,0 \
		,0 \
		,0 \
		, S##_run\
		, S##_begin\
		, S##_query\
		, S##_encode\
		, S##_error\
	}; 
#endif
