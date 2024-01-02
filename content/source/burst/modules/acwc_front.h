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
	acwc_mode_voltage_cl_ix = actuator_mode_count+0			//5
	, acwc_mode_speed_cl_ix = actuator_mode_count+1			//6
	, acw_mode_position_cl_ix = actuator_mode_count+2		//7
	, acwc_mode_current_ix = actuator_mode_count+3				//8
	, acwc_mode_speed_ix = actuator_mode_count+3				//9
	, acw_mode_position_ix = actuator_mode_count+4			//10
	, acw_mode_count = actuator_mode_count+6
};

enum{
	burst_panic_acwc_overpower_bit				=	burst_panic_actuator_bits+1, burst_panic_acwc_overpower='P'
	, burst_panic_acwc_overcurrent_bit		=	burst_panic_actuator_bits+2, burst_panic_acwc_overcurrent='C'
	, burst_panic_acwc_circuit_break_bit	=	burst_panic_actuator_bits+3, burst_panic_acwc_circuit_break='c'
	, burst_panic_acwc_bits								= burst_panic_acwc_circuit_break_bit
};
	
#pragma pack(pop)

#if defined(__cplusplus)
}
#endif

#endif
