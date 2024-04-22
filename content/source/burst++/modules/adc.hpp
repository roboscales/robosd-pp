#ifndef burst_adc_hpp
#define burst_adc_hpp
#include "burst++/modules/actor.hpp"

namespace burst {
	template< class number, typename R, int N>  class adc_t : public actor {
	public:

		struct config_s {
			actor::config_s tag;
			unsigned init_count_bits;
		};
		struct present_s {
			actor::present_s tag;
			R raw[N];
			number::signal_t values[N];
			bool ready;
		};
		R offset[N];
		R acc[N];
		volatile burst_bool_t ready;
		adc_config_p config;
		int init_count;

		#define ADC_CONFIG(a) ADC_CONFIG_(a)
		#define ADC_CONFIG_(a)\
		{\
			ACTOR_CONFIG(a)\
			, a##_INDEX\
			, a##_SCALE\
			, a##_INIT_COUNT_BITS\
		}

		virtual void reset(void) {
			actor::reset();
			present_s& p = present<present_s>();
			config_s& cfg = config<config_s>();
			R * a = acc;
			for (int i = 0; i < N; ++i, ++a) {
				*a = 0;
			}
			init_count = 1 << cfg->init_count_bits;
			p.ready = burst_false;
		};

		virtual void operator ()(void) {
			present_s& p = present<present_s>();
			config_s& cfg = config<config_s>();

			if (p.ready) {
				number::signal_t * v = p.values;
				R * n = p.raw;
				R * o = p.offset;
				for (int i = 0; i < N; ++i, ++n, ++o, ++v) {
					*v = (number::signal_t)((number::long_signal_t)(*n - *o));
				}
			}
			else {
				R * a = acc;
				R * n = p.raw;
				for (int i = 0; i < N; ++i, ++a, ++n) {
					*a += *n;
				}
				init_count--;
				if (_adc->init_count == 0) {
					number::signal_t* v = p.values;
					R * n = p.raw;
					R * o = p.offset;
					R* a = _adc->acc;
					int shift = cfg->init_count_bits;
					for (int i = 0; i < N; ++i, ++v, ++n, ++o, ++a) {
						*o = (R)((*a + (1 << (shift - 1))) >> shift) + 1;
						*v = (number::signal_t)((number::long_signal_t)(*n - *o)  );
					}
					p.ready = true;
				}
			}
		}

	};
	#if 0
	template< class number, typename R, int N>  class adc_t : public actor {
	public:

		struct config_s {
			actor::config_s tag;
			unsigned int index[N];
			number::signal_t scale[N];
			unsigned init_count_bits;
		};
		struct present_s {
			actor::present_s tag;
			R raw[N];
			R native[N];
			R offset[N];
			R acc[N];
			number::signal_t values[N];
			bool ready;
		};
		volatile burst_bool_t ready;
		adc_config_p config;
		int init_count;

		#define ADC_CONFIG(a) ADC_CONFIG_(a)
		#define ADC_CONFIG_(a)\
		{\
			ACTOR_CONFIG(a)\
			, a##_INDEX\
			, a##_SCALE\
			, a##_INIT_COUNT_BITS\
		}

		virtual void reset(void) {
			actor::reset();
			present_s& p = present<present_s>();
			config_s& cfg = config<config_s>();
			R* a = p.acc;
			for (int i = 0; i < N; ++i, ++a) {
				*a = 0;
			}
			init_count = 1 << cfg->init_count_bits;
			p.ready = burst_false;
		};

		virtual void operator ()(void) {
			present_s& p = present<present_s>();
			config_s& cfg = config<config_s>();
			R* n = p.native;
			const unsigned int* ix = c.index;
			for (int i = 0; i < N; ++i, ++n, ++ix) {
				*n = _raw[*ix];
			}
			if (p.ready) {
				number::signal_t* v = p.values;
				R* n = p.native;
				const burst_signal_t* s = _adc->config->scale;
				BURST_ADC_TYPE* o = _adc->offset;
				for (int i = 0; i < BURST_ADC_CHANNEL_COUNT; ++i, ++v, ++n, ++s, ++o) {
					*v = (burst_signal_t)((burst_long_signal_t)(*n - *o) * *s);
				}
			}
			else {
				BURST_ADC_ACC_TYPE* a = _adc->acc;
				BURST_ADC_TYPE* n = _adc->native;
				for (int i = 0; i < BURST_ADC_CHANNEL_COUNT; ++i, ++a, ++n) {
					*a += *n;
				}
				_adc->init_count--;
				if (_adc->init_count == 0) {
					burst_signal_t* v = _adc->values;
					BURST_ADC_TYPE* n = _adc->native;
					const burst_signal_t* s = _adc->config->scale;
					BURST_ADC_TYPE* o = _adc->offset;
					BURST_ADC_ACC_TYPE* a = _adc->acc;
					int shift = _adc->config->init_count_bits;
					for (int i = 0; i < BURST_ADC_CHANNEL_COUNT; ++i, ++v, ++n, ++s, ++o, ++a) {
						*o = (BURST_ADC_TYPE)((*a + (1 << (shift - 1))) >> shift) + 1;
						*v = (burst_signal_t)((burst_long_signal_t)(*n - *o) * *s);
					}
					adc_reset(_adc);
					_adc->ready = burst_true;
				}
			}
		}

	};
	#endif
}
#endif