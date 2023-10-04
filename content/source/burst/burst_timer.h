#ifndef __burst_timer_h
#define __burst_timer_h

#include "burst/burst.h"
#ifndef BURST_TIMER_ENABLED
#define BURST_TIMER_ENABLED 0
#endif

#if BURST_TIMER_ENABLED == 1
typedef unsigned int burst_time_us_t;
typedef unsigned int burst_time_ms_t;

typedef struct burst_timer_s {
	burst_time_us_t last;
	burst_time_us_t period;
	burst_run_t (* on_tick)(void);
	burst_bool_t (* start)(void);
	void (* stop)(void);
} burst_timer_t;

typedef burst_timer_t * burst_timer_p;

burst_bool_t burst_timer_start(burst_timer_p _timer);
void burst_timer_stop(burst_timer_p _timer);
void burst_timer_poll(void);
burst_time_us_t burst_time_us(void);
burst_time_ms_t burst_time_ms(void);
#define BURST_TIMER( S ) BURST_TIMER_( S )

#define BURST_TIMER_( S ) \
extern burst_timer_t  S;

#define BURST_TIMER_CREATE( S ) BURST_TIMER_CREATE_( S )
#define BURST_TIMER_CREATE_( S ) \
BURST_WEAK burst_run_t  S##_tick(void){ return burst_continue; }\
extern burst_timer_t  S;\
BURST_WEAK burst_bool_t  S##_start(void){ return burst_timer_start(&S); }\
BURST_WEAK void  S##_stop(void){ burst_timer_stop(&S); }\
burst_timer_t  S={ \
	0\
	, 0\
	, S##_tick \
	, S##_start \
	, S##_stop \
};

typedef struct burst_packet_s {
	burst_time_us_t timeout;
	burst_bool_t result;
	uint8_t repeat_count;
} burst_packet_t;
typedef burst_packet_t * burst_packet_p;

#endif
#endif
