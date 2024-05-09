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
			R raw[count];
			signal_t values[count];
			bool ready;
		};
		
		#define reg_arr(t,x,n) if(n < count) reg(t, x[n], RT(#n));
		#if ROBO_APP_BURST_VARTREE_ENABLED
		virtual void do_regvar_present(void) {
			using namespace burst::var;
			ACTOR_PRESENT_S(p);
			if (actual_mode >= mode::full) {
				reg(types::const_uint8, p.ready, RT("ready"));
				push(RT("raw"));
				auto t = (types)descriptor_enco(sizeof(R), false, false, false);

				reg_arr(t, p.raw, 0);
				reg_arr(t, p.raw, 1);
				reg_arr(t, p.raw, 2);
				reg_arr(t, p.raw, 3);
				reg_arr(t, p.raw, 4);
				reg_arr(t, p.raw, 5);
				reg_arr(t, p.raw, 6);
				reg_arr(t, p.raw, 7);
				reg_arr(t, p.raw, 8);
				reg_arr(t, p.raw, 10);
				reg_arr(t, p.raw, 11);
				reg_arr(t, p.raw, 12);
				reg_arr(t, p.raw, 13);
				reg_arr(t, p.raw, 14);
				reg_arr(t, p.raw, 15);
				pop();
				push(RT("val"));
				reg_arr(number::var::signal, p.raw, 0);
				reg_arr(number::var::signal, p.raw, 1);
				reg_arr(number::var::signal, p.raw, 2);
				reg_arr(number::var::signal, p.raw, 3);
				reg_arr(number::var::signal, p.raw, 4);
				reg_arr(number::var::signal, p.raw, 5);
				reg_arr(number::var::signal, p.raw, 6);
				reg_arr(number::var::signal, p.raw, 7);
				reg_arr(number::var::signal, p.raw, 8);
				reg_arr(number::var::signal, p.raw, 10);
				reg_arr(number::var::signal, p.raw, 11);
				reg_arr(number::var::signal, p.raw, 12);
				reg_arr(number::var::signal, p.raw, 13);
				reg_arr(number::var::signal, p.raw, 14);
				reg_arr(number::var::signal, p.raw, 15);
				pop();
			}
		}

		virtual void do_regvar_conf(void) {
			using namespace burst::var;
			ACTOR_CONFIG_S(c);
			if (actual_mode >= mode::config) {
				reg(types::uint8, c.init_count_bits, RT("icb"));
			}			
		}
		#endif

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