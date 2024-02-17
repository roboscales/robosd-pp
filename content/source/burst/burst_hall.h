#include "burst/burst_signal.h"
#ifndef burst_hall_h
#define burst_hall_h

enum { hall_extra_mode_qubic = 1, hall_extra_mode_regress = 2, hall_extra_mode_fuzzy = 3, hall_extra_mode_none = 0 };

struct hall_extra_s;
typedef struct hall_extra_s  * hall_extra_p;
typedef struct hall_extra_s {
	burst_long_signal_t angle32;
	burst_signal_t angle;
	burst_long_signal_t delta32;
	burst_long_signal_t speed32;
	struct{
		burst_signal_t prev;
		burst_signal_t * pactual;
	} hall;
	uint32_t ticks;
	uint32_t begin;
	uint32_t lost;
	uint32_t period;
	burst_bool_t on_sign_reset_flag;
	burst_bool_t lost_flag;
	void (* aproxx)(hall_extra_p);
	void (* interp)(hall_extra_p);
	burst_long_signal_t * speeds_history;
	int rank;
} hall_extra_t;

typedef struct hall_config_s {
	struct {
		burst_signal_t native;
		burst_signal_t dynamic;
	} offset;
	burst_bool_t inv;
	uint8_t extra_mode;
} hall_config_t;
typedef hall_config_t * hall_config_p;

#define HALL_VAR_REG(t,h,n) HALL_VAR_REG_(t,h,n)
#define HALL_VAR_REG_(t,h,n)\
BURST_VAR_PUSH(t, n)\
BURST_VAR_PUSH(t, "ofs")\
BURST_VAR_REG(t,h->offset.native,"native",burst_signal_var)\
BURST_VAR_REG(t,h->offset.native,"dynamic",burst_signal_var)\
BURST_VAR_POP(t )\
BURST_VAR_REG(t,h->inv,"inv",int8)\
BURST_VAR_REG(t,h->extra_mode,"extra_mode",uint8)\
BURST_VAR_POP(t )


#define HALL_CONFIG(h) HALL_CONFIG_(h)
#define HALL_CONFIG_(h)\
{\
	{\
		h##_OFFSET_NATIVE\
		,h##_OFFSET_DYNAMIC\
	}\
	,h##_INV\
	,h##_EXTRA_MODE\
}

typedef union hall_pins_s {
	struct{
		unsigned int A:1;
		unsigned int B:1;
		unsigned int C:1;
	};
	unsigned int index;
} hall_pins_t;
typedef hall_pins_t * hall_pins_p;


typedef struct hall_s {
	hall_config_p config;
	burst_signal_t angle_offset_prev;
	burst_signal_t position_offset_prev;
	struct {
		unsigned fault;
		unsigned total;
	} counter;
	int sector;
	burst_signal_t raw;
	burst_signal_t angle;
	burst_signal_t extra_angle;
	burst_signal_t delta;
	burst_signal_t delta_acc;
	int true_diff;
	int sector_prev;
	hall_pins_t pins;
	hall_extra_p extra;
	uint8_t extra_mode;
} hall_t;
typedef hall_t * hall_p;

void hall_begin(hall_p _hall, hall_config_p _config);
void hall_update(hall_p _hall, const hall_pins_p _pins);
void hall_begin(hall_p _hall, hall_config_p _config);
void hall_interp(hall_p _hall);
void hall_poll(hall_p _hall);


//--void hall_dummy_interp(hall_qubic_p _qubic);
#endif
