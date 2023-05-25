#ifndef burst_adc_h
#define burst_adc_h

#include "burst/burst_signal.h"
#ifndef BURST_ADC_CHANNEL_COUNT 
#define BURST_ADC_CHANNEL_COUNT  1
#endif
#ifndef BURST_ADC_TYPE 
#define BURST_ADC_TYPE  uint32_t 
#endif
#ifndef BURST_ADC_ACC_TYPE 
#define BURST_ADC_ACC_TYPE  uint32_t 
#endif

typedef struct adc_config_s {
		unsigned int index[BURST_ADC_CHANNEL_COUNT ];
		burst_signal_t scale[BURST_ADC_CHANNEL_COUNT ];
		unsigned init_count_bits;
} adc_config_t;
typedef adc_config_t * adc_config_p;

#define ADC_CONFIG(a) ADC_CONFIG_(a)
#define ADC_CONFIG_(a)\
{\
	a##_INDEX\
	,a##_SCALE\
	,a##_INIT_COUNT_BITS\
}

typedef struct adc_s {
	BURST_ADC_TYPE native[BURST_ADC_CHANNEL_COUNT];
	BURST_ADC_TYPE offset[BURST_ADC_CHANNEL_COUNT];
	BURST_ADC_ACC_TYPE acc[BURST_ADC_CHANNEL_COUNT];
	burst_signal_t values[BURST_ADC_CHANNEL_COUNT];
	volatile burst_bool_t ready;
	adc_config_p config;
	int init_count;

} adc_t;
typedef adc_t * adc_p;

void adc_begin(adc_p,adc_config_p);
void adc_reset(adc_p);
void adc_update( adc_p, const BURST_ADC_TYPE * );

#endif
