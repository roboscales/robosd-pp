#ifndef dc_estimate_hpp
#define dc_estimate_hpp

#include "burst++/modules/actor.hpp"

#ifndef BURST_PANICS_ACTUATOR_TEMPER_ENABLED
#define BURST_PANICS_ACTUATOR_TEMPER_ENABLED 0
#endif

namespace burst{
	template <class number> class dcme_t: public actor {
	public:
		using signal_t = typename number::signal_t;
		using long_signal_t = typename number::long_signal_t;
		using fpn_t = typename number::signal_t;
		struct config_s {
			actor::config_s tag;
			struct{
				signal_t voltage8;
				signal_t current8;
			} scale;
			struct{
				range_s<fpn_t> L;
				range_s<fpn_t> R;
				range_s<fpn_t> E;
			} range;
			long_signal_t mu;
		};
		#define DCME_CONFIG(a)DCME_CONFIG_(a)
		#define DCME_CONFIG_(a)\
		{\
			ACTOR_CONFIG(a)\
			, a##_SCALE_VOLTAGE8\
			, a##_SCALE_CURRENT8\
			, {\
				BURST_RANGE_CONFIG(a##_RANGE_L)\
				, BURST_RANGE_CONFIG(a##_RANGE_R)\
				, BURST_RANGE_CONFIG(a##_RANGE_E)\
			}\
			, a##_MU\
		}

		
		struct present_s{
			actor::present_s tag;
			struct{
				fpn_t actual;
				fpn_t prev;
			} voltage;
			struct{
				fpn_t actual;
				fpn_t prev;
				fpn_t delta;
			} current;
			struct{
//				fpn_t voltage;
//				fpn_t L;
				fpn_t R;
				fpn_t E;
				fpn_t E10;
				fpn_t error;
				//fpn_t scale;
				//fpn_t factor;
				//long_signal_t L32;
				//long_signal_t R32;
				long_signal_t E32;
			} estimate;
		};
		protected:
			signal_t * pvoltage = &standby<signal_t>();
			signal_t * pcurrent = &standby<signal_t>();
	public:
		virtual void connect(signal_t* _voltage,signal_t* _current) {
			connectto(pvoltage, _voltage );
			connectto(pcurrent, _current );
		}		
		virtual void begin(void) {
			ACTOR_CONFIG_S(c);
			ACTOR_PRESENT_S(p);
			p.estimate.E = 0;
			//p.estimate.L = c.range.L.lo;
			p.estimate.R = c.range.R.hi;
		}
		virtual void run(void) {
			ACTOR_CONFIG_S(c);
			ACTOR_PRESENT_S(p);
			p.voltage.prev = p.voltage.actual;
			p.voltage.actual = (*pvoltage* c.scale.voltage8)>>8;
			
			p.current.prev = p.current.actual;
			p.current.actual = (signal_t)((((long_signal_t)*pcurrent)* c.scale.current8)>>8);
			p.estimate.E32 = p.voltage.prev - ( (p.current.actual + p.current.prev) >> 1 )* p.estimate.R;
			p.estimate.E = range_apply(p.estimate.E32,c.range.E);
			p.estimate.E10 = p.estimate.E>>5;
			//p.current.delta = p.current.actual-p.current.prev;
			
			//p.estimate.voltage = p.estimate.E + ( (long_signal_t)p.current.prev * p.estimate.R );
			
			/*
			p.estimate.voltage = (((long_signal_t)p.current.prev * p.estimate.R)>>16) + (((long_signal_t)p.current.delta *  p.estimate.L)>>16) + p.estimate.E ;
			
			p.estimate.error = p.voltage.prev - p.estimate.voltage;
			
			p.estimate.scale = (  (long_signal_t)p.current.delta*p.current.delta + c.mu + (long_signal_t)p.current.prev*p.current.prev + 65536) >>16;
			p.estimate.factor = p.estimate.error/p.estimate.scale ;
			long_signal_t E =  p.estimate.E32 =  p.estimate.E32 + p.estimate.factor*65536;
			p.estimate.E = range_apply(E>>16,c.range.E);
			
			long_signal_t L =  p.estimate.L32 = p.estimate.L32 + (p.estimate.factor * p.current.delta);
			p.estimate.L = range_apply(L>>16,c.range.L);
			
			long_signal_t R =  p.estimate.R32 =  p.estimate.R32 + (p.estimate.factor * p.current.prev);
			p.estimate.R = range_apply(R>>16,c.range.R);
			*/
		}
		
		dcme_t(const config_s& _config, present_s& _present)
			: actor(_config.tag, _present.tag) {};
		dcme_t(const config_s& _config, present_s& _present, subsystem& _subsystem)
			: actor(_config.tag, _present.tag, _subsystem) {};
	};
}

#endif