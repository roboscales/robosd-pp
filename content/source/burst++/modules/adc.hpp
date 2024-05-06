#ifndef burst_adc_hpp
#define burst_adc_hpp
#include "burst++/modules/actor.hpp"

namespace burst {
	template< class number, class driver>  class adc_t : public actor {
		using R = typename driver::raw_t;
		enum {N = driver::channel_count};
		using signal_t = typename number::signal_t;
		using long_signal_t = typename number::long_signal_t;

	public:
		enum { count = N};
		struct config_s {
			actor::config_s tag;
			unsigned init_count_bits;
		};
		struct present_s {
			actor::present_s tag;
			R raw[N];
			signal_t values[N];
			bool ready;
		};
		R offset[N] = {};
		R acc[N] = {};
		int init_count = 0;

		#define ADC_CONFIG(a) ADC_CONFIG_(a)
		#define ADC_CONFIG_(a)\
		{\
			ACTOR_CONFIG(a)\
			, a##_INIT_COUNT_BITS\
		}

		virtual void begin(void) {
			actor::begin();
			ACTOR_CONFIG_S(cfg);
			ACTOR_PRESENT_S(p);
			R * a = acc;
			for (int i = 0; i < N; ++i, ++a) {
				*a = 0;
			}
			init_count = 1 << cfg.init_count_bits;
			p.ready = false;
		};

		virtual void run(void) {
			ACTOR_CONFIG_S(cfg);
			ACTOR_PRESENT_S(p);
			driver::query(p.raw);
			if (p.ready) {
				signal_t * v = p.values;
				R * n = p.raw;
				R * o = offset;
				for (int i = 0; i < N; ++i, ++n, ++o, ++v) {
					*v = (signal_t)((long_signal_t)(*n - *o));
				}
			}
			else {
				R * a = acc;
				R * n = p.raw;
				for (int i = 0; i < N; ++i, ++a, ++n) {
					*a += *n;
				}
				init_count--;
				if (init_count == 0) {
					typename number::signal_t * v = p.values;
					R * n = p.raw;
					R * o = offset;
					R* a = acc;
					int shift = cfg.init_count_bits;
					for (int i = 0; i < N; ++i, ++v, ++n, ++o, ++a) {
						*o = (R)((*a + (1 << (shift - 1))) >> shift) + 1;
						*v = (signal_t)((long_signal_t)(*n - *o)  );
					}
					p.ready = true;
				}
			}
		}

		adc_t(const config_s& _config, present_s& _present)
			: actor(_config.tag, _present.tag) {};
		adc_t(const config_s& _config, present_s& _present, subsystem& _subsystem)
			: actor(_config.tag, _present.tag, _subsystem) {};

	};	
}
#endif