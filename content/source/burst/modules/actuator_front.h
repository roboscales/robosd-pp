#ifndef burst_modules_actuator_front_h
#define burst_modules_actuator_front_h
#include <stdint.h>
#if defined(__cplusplus)
extern "C"
{
#endif

#include "burst/burst_common.h"

#pragma pack(push, 1)

typedef struct actuator_action_s{
	burst_dev_action_t ref;
	int16_t voltage;
	int16_t speed;
	int32_t position;
} actuator_action_t;
typedef actuator_action_t * actuator_action_p;

typedef struct actuator_feedback_s{
	burst_dev_feedback_t ref;
	int16_t voltage;
	int16_t speed;
	int32_t position;
} actuator_feedback_t;
typedef actuator_feedback_t * actuator_feedback_p;

enum {actuator_mode_idle_ix = 0, actuator_mode_fault_ix = 1, actuator_mode_voltage_ix = 2, actuator_mode_speed_ix = 3, actuator_mode_position_ix = 4, actuator_mode_count = 5};

#pragma pack(pop)

#if defined(__cplusplus)
}
#endif

#endif
