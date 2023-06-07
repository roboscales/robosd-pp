#include "burst/burst_signal.h"
#ifndef burst_hall_h
#define burst_hall_h

typedef struct hall_config_s {
	struct {
		burst_signal_t native;
		burst_signal_t dynamic;
	} offset;
	burst_bool_t inv;
} hall_config_t;
typedef hall_config_t * hall_config_p;

#define HALL_CONFIG(h) HALL_CONFIG_(h)
#define HALL_CONFIG_(h)\
{\
	{\
		h##_OFFSET_NATIVE\
		,h##_OFFSET_DYNAMIC\
	}\
	,h##_INV\
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
	burst_signal_t delta;
	burst_signal_t delta_acc;
	int true_diff;
	int sector_prev;
	unsigned int index;	
} hall_t;
typedef hall_t * hall_p;

void hall_update(hall_p _hall, const hall_pins_p _pins);
void hall_begin(hall_p _hall, hall_config_p _config);

enum { hall_qubic_rank = 14, hall_regress_rank = 6, hall_fuzzy_rank = 6 };
typedef struct hall_qubic_s {
	burst_long_signal_t angle32;
	burst_signal_t angle;
	burst_long_signal_t speed32;
	struct{
		burst_signal_t prev;
		burst_signal_t * pactual;
	} hall;
	uint32_t ticks;
	uint32_t begin;
	uint32_t lost;
	uint32_t period;
	burst_bool_t lost_flag;
	burst_long_signal_t speeds32[hall_qubic_rank];
	burst_long_signal_t A;
	burst_long_signal_t B;
	burst_long_signal_t C;
	burst_long_signal_t D;
	burst_bool_t fuzzy;
} hall_extra_t;

typedef hall_extra_t  * hall_qubic_p;
void hall_dummy_interp(hall_qubic_p _qubic);
void hall_qubic_poll(hall_qubic_p _qubic);
void hall_regres_poll(hall_qubic_p _qubic);
void hall_qubic_interp(hall_qubic_p _qubic);
#endif
