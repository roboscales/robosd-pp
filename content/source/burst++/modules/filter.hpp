#ifndef burst_filter_hpp
#define burst_filter_hpp
#include "burst++/modules/actor.hpp"
#include "burst++/math.hpp"

namespace burst {
	template< class number>  class filter_t : public sink_t<typename number::signal_t> {
	public:
		using B = sink_t<typename number::signal_t>;
		#define FILTER_CONFIG(a) FILTER_CONFIG_(a)
		#define FILTER_CONFIG_(a)\
		{\
			ACTOR_CONFIG(a)\
		}		
		struct config_s {
			typename B::config_s tag;
		} ;
		struct present_s {
			typename B::present_s tag;
			typename number::signal_t value;
		} ;
		
		filter_t(const config_s& _config, present_s& _present)
			: B(_config.tag, _present.tag) {};
		filter_t(const config_s& _config, present_s& _present, subsystem& _subsystem)
			: B(_config.tag, _present.tag, _subsystem) {};
		virtual void begin(void) { 
			B::begin();
			present_s& p =  B::template present<present_s>();
			p.value = *B::input;
		};
		
	};

	template< class number, bool reset_acc_value> class nikitin_t : public filter_t<number> {
	public:
		using B = filter_t<number>;
		struct config_s {
			typename B::config_s flt;
			int8_t shift;
			int8_t presc_shift;
			int8_t value_shift;
		};
		#define NIKITIN_CONFIG(a) NIKITIN_CONFIG_(a)
		#define NIKITIN_CONFIG_(a)\
		{\
			FILTER_CONFIG(a)\
			, a##_SHIFT\
			, a##_PRESC_SHIFT\
			, a##_VALUE_SHIFT\
		}

		int8_t shift=0;
		int8_t presc_shift = 0;
		int8_t value_shift = 0;
		typename number::parameter_t gain=1;
		struct present_s {
			typename B::present_s flt;
			typename number::long_signal_t long_value;
		};
		
		nikitin_t(const config_s& _config, present_s& _present)
			: B(_config.flt, _present.flt) {};
		nikitin_t(const config_s& _config, present_s& _present, subsystem& _subsystem)
			: B(_config.flt, _present.flt, _subsystem) {};
		virtual void begin(void) { 
			B::begin();
			ACTOR_CONFIG_S(c);
			ACTOR_PRESENT_S(p);
			shift = c.shift;
			presc_shift = c.presc_shift + c.shift;
			value_shift = c.value_shift + c.shift;
			gain = (1 << shift) - 1;
			p.long_value = fast::lsh(p.flt.value , shift);
		};
		virtual void run (void) {
			ACTOR_PRESENT_S(p);
			auto long_value = p.long_value;
			long_value = long_value * gain + fast::lsh( * B::input , presc_shift );
			if (reset_acc_value) {
				*B::input = 0;
			}
			long_value = fast::rsh(long_value, shift);
			p.long_value = long_value;
			long_value = fast::rsh(long_value, value_shift);
			p.flt.value = number::s_sat(long_value);
		}
	};
}
#endif
