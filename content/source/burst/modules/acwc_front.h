#ifndef burst_modules_acwc_front_h
#define burst_modules_acwc_front_h
#include <stdint.h>
#include "burst/modules/actuator_front.h"

#if defined(__cplusplus)
extern "C"
{
#endif

#pragma pack(push, 1)

typedef struct acwc_action_s{
	actuator_action_t ac;
	int16_t current;
} acwc_action_t;
typedef acwc_action_t * acwc_action_p;

typedef struct acwc_feedback_s{
	actuator_feedback_t ac;
	int16_t current;
} acwc_feedback_t;
typedef acwc_feedback_t * acwc_feedback_p;

enum {
	acwc_mode_voltage_cl_ix = actuator_mode_count+1
	, acwc_mode_speed_cl_ix = actuator_mode_count+2
	, acw_mode_position_cl_ix = actuator_mode_count+3
	, acwc_mode_speed_ix = actuator_mode_count+4
	, acw_mode_position_ix = actuator_mode_count+5
	, acw_mode_count = actuator_mode_count+6
};
	
#pragma pack(pop)

#if defined(__cplusplus)
}
#endif

#endif
