#ifndef burst_filter_hpp
#define burst_filter_hpp
#include "burst++/modules/actor.hpp"

namespace burst {
	template< class number>  class filter_t : public actor {
	public:
		#define FILTER_CONFIG(a) FILTER_CONFIG_(a)
		#define FILTER_CONFIG_(a)\
		{\
			ACTOR_CONFIG(a)\
		}		
		struct config_s {
			actor::config_s tag;
		} ;
		struct present_s {
			actor::present_s tag;
			number::signal_t value;
		} ;
		filter_t(const config_s& _config, present_s& _present)
			: actor(_config.tag, _present.tag) {};
		filter_t(const config_s& _config, present_s& _present, subsystem& _subsystem)
			: actor(_config.tag, _present.tag, _subsystem) {};
		virtual void reset(void) { 
			ROBO_APP_ASSERT(input);
			actor::reset();
			present_s& p = present<present_s>();
			p.value = *input;
		};
		number::signal_t * input = nullptr;

	};

	template< class number> class nikitin_t : public filter_t<number> {
	public:
		using B = filter_t<number>;
		struct config_s {
			B::config_s flt;
			int8_t shift;
			int8_t presc_shift;
			int8_t value_shift;
		};
		int8_t shift;
		int8_t presc_shift;
		int8_t value_shift;
		typename number::parameter_t gain;
		struct present_s {
			B::present_s flt;
			number::long_signal_t long_value;
		};
		nikitin_t(const config_s& _config, present_s& _present)
			: B(_config.flt, _present.flt) {};
		nikitin_t(const config_s& _config, present_s& _present, subsystem& _subsystem)
			: B(_config.flt, _present.flt, _subsystem) {};
		virtual void reset(void) { 
			B::reset();
			const config_s& c = actor::config<config_s>();
			present_s& p = actor::present<present_s>();

			shift = c.shift;
			presc_shift = c.presc_shift + c.shift;
			value_shift = c.value_shift + c.shift;
			gain = (1 << shift) - 1;
			p.long_value = number::l_lsh(p.flt.value , shift);
		};
		virtual void operator ()(void) {
			present_s& p = actor::present<present_s>();
			auto long_value = p.long_value;
			long_value = long_value * gain + number::l_lsh( * B::input , presc_shift );
			long_value = number::l_rsh(long_value, shift);
			p.long_value = long_value;
			long_value = number::l_rsh(long_value, value_shift);
			p.flt.value = number::s_sat(long_value);
		}
	};
}
#endif
