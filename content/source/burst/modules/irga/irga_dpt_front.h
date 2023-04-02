#ifndef _irga_dpt_front_h
#define _irga_dpt_front_h
#include <stdint.h>
#if defined(__cplusplus)
extern "C"
{
#endif

#include "burst/burst_common.h"
typedef struct irga_dpt_action_s{
	burst_dev_action_t dev;
	int16_t voltage;
	int16_t speed;
	int32_t position;
} irga_dpt_action_t;
typedef irga_dpt_action_t * irga_dpt_action_p;

typedef struct irga_dpt_feedback_s{
	burst_dev_feedback_t dev;
	int16_t voltage;
	int16_t speed;
	int32_t position;
} irga_dpt_feedback_t;
typedef irga_dpt_feedback_t * irga_dpt_feedback_p;

#if defined(__cplusplus)
}
#endif

#endif