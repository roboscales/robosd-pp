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

typedef struct inv3ph_config_s{
	burst_long_signal_t hi;
	burst_long_signal_t lo;
} inv3ph_config_t;
typedef inv3ph_config_t * inv3ph_config_p;

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
} inv3ph_t;
typedef inv3ph_t * inv3ph_p;
void inv3ph_run(inv3ph_p _inverter, burst_signal_t _angle);
void inv3ph_begin(inv3ph_p _inverter, inv3ph_config_p _config);

typedef struct current3ph_config_s{
	int adc_index[3];
	burst_long_signal_t deform[9];
} current3ph_config_t;
typedef current3ph_config_t * current3ph_config_p;

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
