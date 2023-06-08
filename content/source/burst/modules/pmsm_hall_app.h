#ifndef  pmsm_hall_app_h
#define pmsm_hall_app_h

#define PMSM_HALL_APP_EXTRA_TYPE_NONE 0 
#define PMSM_HALL_APP_EXTRA_TYPE_REGRESS 1
#define PMSM_HALL_APP_EXTRA_TYPE_QUBIC 2

#include "burst/burst.h"
#include "burst/burst_serial.h"
#include "burst/burst_inv3ph.h"
#include "burst/burst_hall.h"
#include "burst/burst_adc.h"
#include "burst/modules/nikitin.h"
#include "burst/burst_ps.h"
#include "burst/modules/enco_abs32.h"
#include "burst/modules/pmsm.h"

//силовой преобразователь
BURST_PS(power)

//датчик скорости - фильтh Никитина
NIKITIN(speedse)

ENCO_ABS32(enco)

BURST_MOTION(motion)

BURST_POSITIONER(positioner)

BURST_PI(c_lat_pi)

BURST_PI(c_cross_pi)

BURST_PI(c_hi_pi)

BURST_PI(c_lo_pi)

NIKITIN(c_lat_flt)

NIKITIN(c_cross_flt)

extern hall_t hall;
extern adc_t adc;
extern pmsm_t motor;

extern burst_long_signal_t RPM ;
typedef struct pmsm_hall_app_config_s {
	pmsm_config_t pmsm;
	hall_config_t hall;
	adc_config_t adc;
	enco_abs32_config_t enco;
	nikitin_config_t speedse;
	nikitin_config_t cross_flt;
	nikitin_config_t lat_flt;
	pmsm_angle_forcer_config_t angle_forcer;
	int controlPresc;
} pmsm_hall_app_config_t;
typedef  pmsm_hall_app_config_t * pmsm_hall_app_config_p;
 
void pmsm_hall_app_begin(pmsm_hall_app_config_p _config, pmsm_action_p _action, pmsm_feedback_p _feedback );
void pmsm_hall_app_start(void);
void pmsm_hall_app_realtime_loop(void);
void pmsm_hall_app_backend_loop(void);
void pmsm_hall_app_frontend_loop(void);
void pmsm_hall_app_control_step_1(void);
void pmsm_hall_app_control_step_2(void);
void pmsm_hall_app_control_step_3(void);

#define PMSM_HALL_APP_CONFIG()\
{\
	PMSM_CONFIG(motor)\
	,HALL_CONFIG(hall)\
	,ADC_CONFIG(adc)\
	,ENCO_ABS32_CONFIG(enco)\
	,NIKITIN_CONFIG(speedse)\
	,NIKITIN_CONFIG(c_cross_flt)\
	,NIKITIN_CONFIG(c_lat_flt)\
	,PMSM_ANGLE_FORCER_CONFIG(angle_forcer)\
	,motor_CONTROL_PRESC\
}

#ifndef hall_OFFSET_NATIVE
#define hall_OFFSET_NATIVE BURST_SIGNAL_T(175./180)
#endif

#ifndef hall_OFFSET_DYNAMIC
#define hall_OFFSET_DYNAMIC 0
#endif

#ifndef hall_INV
#define hall_INV burst_false
#endif

#ifndef adc_INDEX
#define adc_INDEX {0,1,2,3,4}
#endif

#ifndef adc_SCALE
#define adc_SCALE {1,1,1,1,1}
#endif

#ifndef adc_INIT_COUNT_BITS
#define adc_INIT_COUNT_BITS 10
#endif

#ifndef speedse_SHIFT
#define speedse_SHIFT 5
#endif

#ifndef speedse_PRESC_SHIFT
#define speedse_PRESC_SHIFT 0
#endif

#ifndef speedse_VALUE_SHIFT
#define speedse_VALUE_SHIFT 0
#endif

#ifndef c_cross_flt_SHIFT
#define c_cross_flt_SHIFT 4
#endif

#ifndef c_cross_flt_PRESC_SHIFT
#define c_cross_flt_PRESC_SHIFT 2
#endif

#ifndef c_cross_flt_VALUE_SHIFT
#define c_cross_flt_VALUE_SHIFT 2
#endif

#ifndef c_lat_flt_SHIFT
#define c_lat_flt_SHIFT 4
#endif

#ifndef c_lat_flt_PRESC_SHIFT
#define c_lat_flt_PRESC_SHIFT 2
#endif

#ifndef c_lat_flt_VALUE_SHIFT
#define c_lat_flt_VALUE_SHIFT 2
#endif


#ifndef angle_forcer_FORCE_GAIN
#define angle_forcer_FORCE_GAIN 0
#endif

#ifndef angle_forcer_FORCE_SHIFT
#define angle_forcer_FORCE_SHIFT 5
#endif

#ifndef angle_forcer_EDS_GAIN 
#define angle_forcer_EDS_GAIN 0
#endif

#ifndef angle_forcer_EDS_SHIFT1 
#define angle_forcer_EDS_SHIFT1 0
#endif

#ifndef angle_forcer_EDS_SHIFT2 
#define angle_forcer_EDS_SHIFT2 0
#endif

#ifndef angle_forcer_ANGLE_LIMIT 
#define angle_forcer_ANGLE_LIMIT 0
#endif

#ifndef enco_RESOLUTION_ROUND
#define enco_RESOLUTION_ROUND 32
#endif

#ifndef enco_RESOLUTION_RAW
#define enco_RESOLUTION_RAW 16
#endif

#ifndef enco_RESOLUTION_ACTUAL
#define enco_RESOLUTION_ACTUAL 8
#endif

#ifndef enco_INIT_COUNT_BITS
#define enco_INIT_COUNT_BITS 10
#endif

#ifndef enco_OFFSET_NATIVE
#define enco_OFFSET_NATIVE 0
#endif

#ifndef enco_OFFSET_POSITION
#define enco_OFFSET_POSITION 0
#endif

#ifndef enco_INVERCE
#define enco_INVERCE burst_false
#endif

#ifndef motor_INV3PH_NATIVE_RANGE_LO
#define motor_INV3PH_NATIVE_RANGE_LO 0
#endif

#ifndef motor_INV3PH_NATIVE_RANGE_HI
#define motor_INV3PH_NATIVE_RANGE_HI  0
#endif

#ifndef motor_CURRENT3PH_ADC_INDEX
#define motor_CURRENT3PH_ADC_INDEX {0,1,2}
#endif

#ifndef motor_CURRENT3PH_DEFORM
#define motor_CURRENT3PH_DEFORM {65536,0,0,0,65536,0,0,0,65536}
#endif

#ifndef motor_LATERAL_CURRENT_PI_PROP_GAIN
#define motor_LATERAL_CURRENT_PI_PROP_GAIN 0
#endif

#ifndef motor_LATERAL_CURRENT_PI_MODEL_GAIN
#define motor_LATERAL_CURRENT_PI_MODEL_GAIN 0
#endif

#ifndef motor_LATERAL_CURRENT_PI_DIFF_GAIN
#define motor_LATERAL_CURRENT_PI_DIFF_GAIN 0
#endif

#ifndef motor_LATERAL_CURRENT_PI_FORCE_GAIN
#define motor_LATERAL_CURRENT_PI_FORCE_GAIN 0
#endif

#ifndef motor_LATERAL_CURRENT_PI_CONTROL_SHIFT
#define motor_LATERAL_CURRENT_PI_CONTROL_SHIFT 0
#endif

#ifndef motor_LATERAL_CURRENT_PI_MODEL_SHIFT
#define motor_LATERAL_CURRENT_PI_MODEL_SHIFT 0
#endif

#ifndef motor_LATERAL_CURRENT_PI_RAMP
#define motor_LATERAL_CURRENT_PI_RAMP 0
#endif

#ifndef motor_LATERAL_CURRENT_RANGE_LO
#define  motor_LATERAL_CURRENT_RANGE_LO 0
#endif

#ifndef motor_LATERAL_CURRENT_RANGE_HI
#define  motor_LATERAL_CURRENT_RANGE_HI 0
#endif

#ifndef motor_LATERAL_VOLTAGE_RANGE_LO
#define  motor_LATERAL_VOLTAGE_RANGE_LO 0
#endif

#ifndef motor_LATERAL_VOLTAGE_RANGE_HI
#define  motor_LATERAL_VOLTAGE_RANGE_HI 0
#endif

//=====================================
#ifndef motor_CROSS_CURRENT_PI_PROP_GAIN
#define motor_CROSS_CURRENT_PI_PROP_GAIN 0
#endif

#ifndef motor_CROSS_CURRENT_PI_MODEL_GAIN
#define motor_CROSS_CURRENT_PI_MODEL_GAIN 0
#endif

#ifndef motor_CROSS_CURRENT_PI_DIFF_GAIN
#define motor_CROSS_CURRENT_PI_DIFF_GAIN 0
#endif

#ifndef motor_CROSS_CURRENT_PI_FORCE_GAIN
#define motor_CROSS_CURRENT_PI_FORCE_GAIN 0
#endif

#ifndef motor_CROSS_CURRENT_PI_CONTROL_SHIFT
#define motor_CROSS_CURRENT_PI_CONTROL_SHIFT 0
#endif

#ifndef motor_CROSS_CURRENT_PI_MODEL_SHIFT
#define motor_CROSS_CURRENT_PI_MODEL_SHIFT 0
#endif

#ifndef motor_CROSS_CURRENT_PI_RAMP
#define motor_CROSS_CURRENT_PI_RAMP 0
#endif

#ifndef motor_CROSS_CURRENT_RANGE_LO
#define motor_CROSS_CURRENT_RANGE_LO 0
#endif

#ifndef motor_CROSS_CURRENT_RANGE_HI
#define motor_CROSS_CURRENT_RANGE_HI 0
#endif

//=====================================
#ifndef motor_MOTION_OV_VOLTAGE_CL_PROP_GAIN
#define motor_MOTION_OV_VOLTAGE_CL_PROP_GAIN 0
#endif

#ifndef motor_MOTION_OV_VOLTAGE_CL_MODEL_GAIN
#define motor_MOTION_OV_VOLTAGE_CL_MODEL_GAIN 0
#endif

#ifndef motor_MOTION_OV_VOLTAGE_CL_DIFF_GAIN
#define motor_MOTION_OV_VOLTAGE_CL_DIFF_GAIN 0
#endif

#ifndef motor_MOTION_OV_VOLTAGE_CL_FORCE_GAIN
#define motor_MOTION_OV_VOLTAGE_CL_FORCE_GAIN 0
#endif

#ifndef motor_MOTION_OV_VOLTAGE_CL_FORCE_MAX
#define motor_MOTION_OV_VOLTAGE_CL_FORCE_MAX 0
#endif

#ifndef motor_MOTION_OV_VOLTAGE_CL_CONTROL_SHIFT
#define motor_MOTION_OV_VOLTAGE_CL_CONTROL_SHIFT 0
#endif

#ifndef motor_MOTION_OV_VOLTAGE_CL_MODEL_SHIFT
#define motor_MOTION_OV_VOLTAGE_CL_MODEL_SHIFT 0
#endif

#ifndef motor_MOTION_OV_VOLTAGE_CL_LIMIT_GAIN
#define motor_MOTION_OV_VOLTAGE_CL_LIMIT_GAIN 0
#endif

#ifndef motor_MOTION_OV_VOLTAGE_CL_LIMIT_GAIN_PRESC
#define motor_MOTION_OV_VOLTAGE_CL_LIMIT_GAIN_PRESC 0
#endif

//===============================================
#ifndef motor_POSITIONER_OV_VOLTAGE_CL_PROP_GAIN
#define motor_POSITIONER_OV_VOLTAGE_CL_PROP_GAIN 0
#endif

#ifndef motor_POSITIONER_OV_VOLTAGE_CL_DIFF_GAIN
#define motor_POSITIONER_OV_VOLTAGE_CL_DIFF_GAIN 0
#endif

#ifndef motor_POSITIONER_OV_VOLTAGE_CL_DIFF_QUARD_GAIN
#define motor_POSITIONER_OV_VOLTAGE_CL_DIFF_QUARD_GAIN 0
#endif

#ifndef motor_POSITIONER_OV_VOLTAGE_CL_CONTROL_SHIFT
#define motor_POSITIONER_OV_VOLTAGE_CL_CONTROL_SHIFT 0
#endif

#ifndef motor_POSITIONER_OV_VOLTAGE_CL_DEAD_ZONE
#define motor_POSITIONER_OV_VOLTAGE_CL_DEAD_ZONE 0
#endif

#ifndef motor_POSITIONER_OV_VOLTAGE_CL_CRAWL_SPEED
#define motor_POSITIONER_OV_VOLTAGE_CL_CRAWL_SPEED 0
#endif

//=================================================
#ifndef motor_MOTION_OV_CURRENT_PROP_GAIN
#define motor_MOTION_OV_CURRENT_PROP_GAIN 0
#endif

#ifndef motor_MOTION_OV_CURRENT_DIFF_GAIN
#define motor_MOTION_OV_CURRENT_DIFF_GAIN 0
#endif

#ifndef motor_MOTION_OV_CURRENT_MODEL_GAIN
#define motor_MOTION_OV_CURRENT_MODEL_GAIN 0
#endif

#ifndef motor_MOTION_OV_CURRENT_FORCE_GAIN
#define motor_MOTION_OV_CURRENT_FORCE_GAIN 0
#endif

#ifndef motor_MOTION_OV_CURRENT_FORCE_MAX
#define motor_MOTION_OV_CURRENT_FORCE_MAX 0
#endif

#ifndef motor_MOTION_OV_CURRENT_CONTROL_SHIFT
#define motor_MOTION_OV_CURRENT_CONTROL_SHIFT 0
#endif

#ifndef motor_MOTION_OV_CURRENT_MODEL_SHIFT
#define motor_MOTION_OV_CURRENT_MODEL_SHIFT 0
#endif

#ifndef motor_MOTION_OV_CURRENT_LIMIT_GAIN
#define motor_MOTION_OV_CURRENT_LIMIT_GAIN 0
#endif

#ifndef motor_MOTION_OV_CURRENT_LIMIT_GAIN_PRESC
#define motor_MOTION_OV_CURRENT_LIMIT_GAIN_PRESC 0
#endif

//===============================================
#ifndef motor_POSITIONER_OV_CURRENT_PROP_GAIN
#define motor_POSITIONER_OV_CURRENT_PROP_GAIN 0
#endif

#ifndef motor_POSITIONER_OV_CURRENT_DIFF_GAIN
#define motor_POSITIONER_OV_CURRENT_DIFF_GAIN 0
#endif

#ifndef motor_POSITIONER_OV_CURRENT_DIFF_QUARD_GAIN
#define motor_POSITIONER_OV_CURRENT_DIFF_QUARD_GAIN 0
#endif

#ifndef motor_POSITIONER_OV_CURRENT_CONTROL_SHIFT
#define motor_POSITIONER_OV_CURRENT_CONTROL_SHIFT 0
#endif

#ifndef motor_POSITIONER_OV_CURRENT_DEAD_ZONE
#define motor_POSITIONER_OV_CURRENT_DEAD_ZONE 0
#endif

#ifndef motor_POSITIONER_OV_CURRENT_CRAWL_SPEED
#define motor_POSITIONER_OV_CURRENT_CRAWL_SPEED 0
#endif

//=================================================
#ifndef motor_REF_TAG
#define motor_REF_TAG 0
#endif

#ifndef motor_ENCO_FAULT_TICKS_RESET
#define motor_ENCO_FAULT_TICKS_RESET 0
#endif

#ifndef motor_ENCO_FAULT_TICKS_SET
#define motor_ENCO_FAULT_TICKS_SET 0
#endif

#ifndef motor_RANGE_VOLTAGE_LO
#define motor_RANGE_VOLTAGE_LO 0
#endif

#ifndef motor_RANGE_VOLTAGE_HI
#define motor_RANGE_VOLTAGE_HI 0
#endif

#ifndef motor_RANGE_SPEED_LO
#define motor_RANGE_SPEED_LO 0
#endif

#ifndef motor_RANGE_SPEED_HI
#define motor_RANGE_SPEED_HI 0
#endif

#ifndef motor_RANGE_POSITION_LO
#define motor_RANGE_POSITION_LO 0
#endif

#ifndef motor_RANGE_POSITION_HI
#define motor_RANGE_POSITION_HI 0
#endif
//=================================================
#ifndef motor_MOTION_OV_VOLTAGE_PROP_GAIN
#define motor_MOTION_OV_VOLTAGE_PROP_GAIN 0
#endif

#ifndef motor_MOTION_OV_VOLTAGE_MODEL_GAIN
#define motor_MOTION_OV_VOLTAGE_MODEL_GAIN 0
#endif

#ifndef motor_MOTION_OV_VOLTAGE_DIFF_GAIN
#define motor_MOTION_OV_VOLTAGE_DIFF_GAIN 0
#endif

#ifndef motor_MOTION_OV_VOLTAGE_FORCE_GAIN
#define motor_MOTION_OV_VOLTAGE_FORCE_GAIN 0
#endif

#ifndef motor_MOTION_OV_VOLTAGE_FORCE_MAX
#define motor_MOTION_OV_VOLTAGE_FORCE_MAX 0
#endif

#ifndef motor_MOTION_OV_VOLTAGE_CONTROL_SHIFT
#define motor_MOTION_OV_VOLTAGE_CONTROL_SHIFT 0
#endif

#ifndef motor_MOTION_OV_VOLTAGE_MODEL_SHIFT
#define motor_MOTION_OV_VOLTAGE_MODEL_SHIFT 0
#endif

#ifndef motor_MOTION_OV_VOLTAGE_LIMIT_GAIN
#define motor_MOTION_OV_VOLTAGE_LIMIT_GAIN 0
#endif

#ifndef motor_MOTION_OV_VOLTAGE_LIMIT_GAIN_PRESC
#define motor_MOTION_OV_VOLTAGE_LIMIT_GAIN_PRESC 0
#endif

//===============================================
#ifndef motor_POSITIONER_OV_VOLTAGE_PROP_GAIN
#define motor_POSITIONER_OV_VOLTAGE_PROP_GAIN 0
#endif

#ifndef motor_POSITIONER_OV_VOLTAGE_DIFF_GAIN
#define motor_POSITIONER_OV_VOLTAGE_DIFF_GAIN 0
#endif

#ifndef motor_POSITIONER_OV_VOLTAGE_DIFF_QUARD_GAIN
#define motor_POSITIONER_OV_VOLTAGE_DIFF_QUARD_GAIN 0
#endif

#ifndef motor_POSITIONER_OV_VOLTAGE_CONTROL_SHIFT
#define motor_POSITIONER_OV_VOLTAGE_CONTROL_SHIFT 0
#endif

#ifndef motor_POSITIONER_OV_VOLTAGE_DEAD_ZONE
#define motor_POSITIONER_OV_VOLTAGE_DEAD_ZONE 0
#endif

#ifndef motor_POSITIONER_OV_VOLTAGE_CRAWL_SPEED
#define motor_POSITIONER_OV_VOLTAGE_CRAWL_SPEED 0
#endif
//===============================================
#ifndef motor_CONTROL_PRESC
#define motor_CONTROL_PRESC 16
#endif

//#ifndef motor_ANGLE_FORCE_ENABLE
//#define motor_ANGLE_FORCE_ENABLE 1
//#endif

#ifndef PMSM_HALL_APP_EXTRA_TYPE
#define PMSM_HALL_APP_EXTRA_TYPE PMSM_HALL_APP_EXTRA_TYPE_NONE
#endif

#endif
