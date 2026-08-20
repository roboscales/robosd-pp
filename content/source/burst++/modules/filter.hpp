#ifndef burst_filter_hpp
#define burst_filter_hpp
#include "burst++/burst.hpp"
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
		#if ROBO_APP_BURST_VARTREE_ENABLED
		virtual void do_regvar_present(void) {
			using namespace burst::var;
			if (actual_mode >= mode::full) {
				ACTOR_PRESENT_S(p);
				reg(number::var::signal, p.value, RT("val"));
			}
		}
		virtual void do_regvar_conf(void) {}
		#endif	
		#if ROBO_APP_ULTRACOMPACT == 0
		filter_t(const config_s& _config, present_s& _present)
			: B(_config.tag, _present.tag) {};
		filter_t(const config_s& _config, present_s& _present, subsystem& _subsystem)
			: B(_config.tag, _present.tag, _subsystem) {};
		#else
		filter_t(const config_s& _config, present_s& _present, const typename number::signal_t & _in)
			: B(_config.tag, _present.tag, _in) {};
		#endif
		virtual void begin(void) { 
			B::begin();
			present_s& p =  B::template present<present_s>();
			#if ROBO_APP_ULTRACOMPACT == 0
			p.value = *B::input;
			#else
			p.value = B::input;
			#endif
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
		#if ROBO_APP_BURST_VARTREE_ENABLED
		virtual void do_regvar_present(void) {
			B::do_regvar_present();
			using namespace burst::var;
			if (actual_mode >= mode::full) {
				ACTOR_PRESENT_S(p);
				reg(number::var::const_long_signal, p.long_value, RT("lval"));
			}
		}
		virtual void do_regvar_conf(void) {
			B::do_regvar_conf();
			using namespace burst::var;
			if (actual_mode >= mode::config) {
				ACTOR_CONFIG_S(s);
				push(RT("shift"));
				reg(types::int8, s.shift, RT("flt"));
				reg(types::int8, s.presc_shift, RT("presc"));
				reg(types::int8, s.value_shift, RT("val"));
				pop();
			}
		}
		#endif	
		#if ROBO_APP_ULTRACOMPACT == 0
		nikitin_t(const config_s& _config, present_s& _present)
			: B(_config.flt, _present.flt) {};
		nikitin_t(const config_s& _config, present_s& _present, subsystem& _subsystem)
			: B(_config.flt, _present.flt, _subsystem) {};
		#else
		nikitin_t(const config_s& _config, present_s& _present, const typename number::signal_t& _in)
			: B(_config.flt, _present.flt, _in) {};
		#endif
		virtual void begin(void) { 
			B::begin();
			ACTOR_CONFIG_S(c);
			ACTOR_PRESENT_S(p);
			shift = c.shift;
			presc_shift = c.presc_shift + c.shift;
			value_shift = c.value_shift + c.shift;
			gain = (1 << shift) - 1;
			p.long_value = robo::digit::lsh((typename number::long_signal_t)p.flt.value , shift);
		};
		virtual void run (void) {
			ACTOR_PRESENT_S(p);
			auto long_value = p.long_value;
			#if ROBO_APP_ULTRACOMPACT == 0
			auto tmp = *B::input;
			#else
			auto tmp = B::input;
			#endif	
			long_value = long_value * gain + robo::digit::lsh( (typename number::long_signal_t ) tmp , presc_shift );
			if (reset_acc_value) {
				#if ROBO_APP_ULTRACOMPACT == 0
				*B::input = 0;
				#else
				B::input = 0;
				#endif
			}
			long_value = robo::digit::rsh(long_value, shift);
			p.long_value = long_value;
			if (*B::input == 0) {
				if (p.long_value > 0) {
					p.long_value--;
				}
				else if (p.long_value < 0) {
					p.long_value++;
				}
			}
			long_value = robo::digit::rsh(long_value, value_shift);
			p.flt.value = number::s_sat(long_value);
		}
	};
}
#endif
