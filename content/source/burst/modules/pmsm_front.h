#ifndef burst_modules_pmsm_front_h
#define burst_modules_pmsm_front_h
#if defined(__cplusplus)
extern "C"
{
#endif

#include "burst/burst_common.h"
#include "burst/modules/acwc_front.h"
typedef struct pmsm_action_s{
	acwc_action_t cross;
	struct{
		int16_t voltage;
		int16_t current;
	} lateral; 
	struct{
		int32_t freq;
		int32_t angle;
	} synchro;
} pmsm_action_t;
typedef pmsm_action_t * pmsm_action_p;

typedef struct pmsm_feedback_s{
	acwc_feedback_t cross;
	struct{
		int16_t voltage;
		int16_t current;
	} lateral;
} pmsm_feedback_t;
typedef pmsm_feedback_t * pmsm_feedback_p;

enum {
	pmsm_mode_synchro_voltage_ix = acw_mode_count+0 //11
	, pmsm_mode_synchro_curent_ix = acw_mode_count+1 //12
	, pmsm_mode_estimate_ix = acw_mode_count+2 //13
	, pmsm_mode_count = acw_mode_count+3
};

#if defined(__cplusplus)
}
#endif
#endif
