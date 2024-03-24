#ifndef burst_inv3ph_h
#define burst_inv3ph_h
#include "burst/burst_signal.h"

typedef struct abc_s {
	burst_signal_t A;
	burst_signal_t B;
	burst_signal_t C;
} abc_t;

typedef struct ab_s {
	burst_long_signal_t alfa;
	burst_long_signal_t beta;
} ab_t;

typedef struct dq_s {
	burst_signal_t lateral;
	burst_signal_t cross;
} dq_t;

typedef struct inv3ph_deform_s{
	burst_bool_t enabled;
	burst_signal_t level;
	burst_long_signal_t hi_gain_16;
	burst_long_signal_t lo_gain_16;
	burst_long_signal_t lo_bevel_16;
} inv3ph_deform_t;
typedef inv3ph_deform_t * inv3ph_deform_p;

typedef struct inv3ph_config_s{
	burst_long_range_t native;
	burst_long_signal_t pwm_force;
	inv3ph_deform_t deform;
} inv3ph_config_t;
typedef inv3ph_config_t * inv3ph_config_p;


#define INV3PH_CONFIG(a) INV3PH_CONFIG_(a)
#define INV3PH_CONFIG_(a)\
{\
	RANGE_CONFIG(a##_NATIVE_RANGE)\
	,a##_PWM_FORCE\
	,{\
		a##_DEFORM_ENABLED\
		,a##_DEFORM_LEVEL\
		,a##_DEFORM_HI_GAIN_16\
		,a##_DEFORM_LO_GAIN_16\
		,a##_DEFORM_LO_BEVEL_16\
	}\
}

typedef struct inv3ph_s{
	abc_t duty;
	abc_t pwm;
	ab_t ab;
	rot_t rot;
	dq_t dq;
	uint8_t swm;
	burst_signal_t angle;
	burst_long_signal_t scale_gain ;
	burst_signal_t discret_lo;
	burst_signal_t discret_hi;
	burst_signal_t discret_delta_lo;
	burst_signal_t discret_delta_hi;
	burst_long_signal_t pwm_force;
	inv3ph_deform_p deform;
} inv3ph_t;
typedef inv3ph_t * inv3ph_p;

void inv3ph_run(inv3ph_p _inverter, burst_signal_t _cross, burst_signal_t _lateral, burst_signal_t _angle);
void inv3ph_begin(inv3ph_p _inverter, inv3ph_config_p _config);

typedef struct current3ph_config_s{
	int adc_index[3];
	struct{
		burst_long_signal_t matrix[9];
		burst_bool_t enable;
	} deform;
} current3ph_config_t;
typedef current3ph_config_t * current3ph_config_p;


#define CURRENT3PH_CONFIG(a) CURRENT3PH_CONFIG_(a)
#define CURRENT3PH_CONFIG_(a)\
{\
	a##_ADC_INDEX\
	,{\
		a##_DEFORM\
		,(burst_bool_t)a##_DEFORM_ENABLE\
	}\
}

typedef struct current3ph_s{
	volatile abc_t abc;
	ab_t ab;
	dq_t dq;
	inv3ph_p inverter;
	struct{
		burst_signal_t * A;
		burst_signal_t * B;
		burst_signal_t * C;
	} raw;
	burst_long_signal_t * deform;
} current3ph_t;
typedef current3ph_t * current3ph_p;

void current3ph_begin(current3ph_p _sensor, current3ph_config_p _config, inv3ph_p _inverter, burst_signal_t * _raw );
void current3ph_run(current3ph_p _sensor);

#endif
