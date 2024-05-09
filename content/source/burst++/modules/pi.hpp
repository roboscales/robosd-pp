#ifndef burst_pi_hpp
#define burst_pi_hpp
#include "burst++/modules/actor.hpp"
namespace burst{
	template <class number> class pi_t {
		public:
			using parametr_t = typename number::parametr_t;
			using signal_t = typename number::signal_t;
			using long_min = typename number::long_min;
			using long_signal_t = typename number::long_signal_t;
			struct config_s{
				parametr_t	propGain;
				parametr_t	modelGain;
				parametr_t	diffGain;
				parametr_t	forceGain;
				uint8_t			controlShift;
				uint8_t			modelShift;
			} & config ;
			
			#define PI_CONFIG(a) PI_CONFIG_(a)
			#define PI_CONFIG_(a)\
			{\
				a##_PROP_GAIN\
				,a##_MODEL_GAIN\
				,a##_DIFF_GAIN\
				,a##_FORCE_GAIN\
				,a##_CONTROL_SHIFT\
				,a##_MODEL_SHIFT\
			}
			const signal_t &	signal_req;
			const signal_t &	signal;
			const signal_t *	signal_diff;
			const signal_t *	signal_force;
			const  signal_t &	controlMin;
			const  signal_t &	controlMax;
			signal_t &	control;
			const satstates &	master_sut_flag;

			struct present_s{
				long_signal_t model;
				long_signal_t long_model;
				long_signal_t diff;
				long_signal_t force;
				satstates		satstate;
			} & present;
			
			pi_t(
				const config_s & _config
				, present_s & _present
				, const signal_t &	_signal_req
				, const signal_t &	_signal
				, const signal_t *	_signal_diff
				, const signal_t *	_signal_force
				, const  signal_t &	_controlMin
				, const  signal_t &	_controlMax
				, signal_t &	_control
				, const satstates &	_master_sut_flag
			)
				: config(_config)
				, present(_present)
				, signal_req(_signal_req)
				, signal(_signal)
				, signal_diff(_signal_diff)
				, signal_force(_signal_force)
				, controlMin (_controlMin)
				, controlMax (_controlMax)
				, control(_control)
				, master_sut_flag(_master_sut_flag)
			{
			}
			

			virtual void run(void){
				long_signal_t Error;
				long_signal_t tmp;
				long_signal_t controlLong;

				satstates sut_flag;

				#ifndef MODEL_VALUE_MAX
				#define MODEL_VALUE_MAX number::long_frac(0.9)
				#endif

				Error = signal_req - signal;
				if( (Error>0) && ( (control>=controlMax) || (p.long_model>MODEL_VALUE_MAX) || (master_sut_flag == satstates::up) ) ){
					sut_flag = satstates::up;
				} else {
					if( (Error<0) && ( (control<=controlMin) || (p.long_model<-MODEL_VALUE_MAX) || (master_sut_flag == satstates::low)  ) ){
							sut_flag = satstates::low;
						}
						else {
							sut_flag =  satstates::none;
						}
				}
				p.satstate = sut_flag;
				if ( sut_flag == satstates::none )
				{
					long_signal_t tmp;
					tmp = (p.long_model += (Error* s->modelGain) );
					tmp = number::fast::rsh( tmp, s.modelShift );
					p.model = tmp = number::saturate(tmp, number::min, number::max );
				}
				
				tmp = Error+ p.model - signal;
				tmp = number::fast::rsh( tmp , s.controlShift );

				controlLong = tmp* (s->propGain)  ;

				if (signal_force != 0) {
					long_signal_t force = *(p.signal_force) * (s->forceGain);
					controlLong += force;
					p.force = force;
				}

				if (signal_diff != 0) {
					p.diff =  - *(signal_diff) * s.diffGain;
					controlLong += p.diff;
				}

				controlLong = number::fast::rsh(controlLong, s.controlShift);
				controlLong = number::saturate(controlLong, controlMin,controlMax );
				p.contol = number::s_extract(controlLong);
			}
			
			virtual void begin(void){
				present = {};
			}
	};
	template <class number> class limiter_t {
		public:
			using parametr_t = typename number::parametr_t;
			using signal_t = typename number::signal_t;
			using long_min = typename number::long_min;
			using long_signal_t = typename number::long_signal_t;
			signal_t zero_signal = 0;
			struct present_s{
				typename pi_t<number>::present_s r_hi;
				typename pi_t<number>::present_s r_low;
				signal_t signal_hi;
				signal_t signal_low;
				signal_t control_hi;
				signal_t control_low;
				signal_t control_des;
				satstates sut_flag;
			};
			struct config_s{
				actor::config_s tag;
				parametr_t ramp;
			};
			#define LIMMITER_CONFIG(a) LIMMITER_CONFIG_(a)
			#define LIMMITER_CONFIG_(a)\
			{\
				ACTOR_CONFIG(a)\
				, a##_RAMP\
			}
			pi_t<number> r_hi;
			pi_t<number> r_low;
			const signal_t & control_req;
			signal_t & control_val;
			const signal_t & signal;
			const range_s< signal_t >  & signalRange;
			const range_s< signal_t >  & controlRange;
			limiter_t(
				const config_s & _config
				, present_s & _present
				, const typename pi_t<number>::config_s & _pi_config
				, const signal_t & _control_req
				, signal_t & _control_val
				, const signal_t & _signal
				, const range_s< signal_t >  & _signalRange
				, const range_s< signal_t >  & _controlRange
			)
				: config(_config)
				, present(_present)
				, r_hi(
					_pi_config
					,_present.r_hi
					, zero_signal//const signal_t & _control_req
					, _present.signal_hi
					, nullptr
					, nullptr
					, _controlRange.lo
					, zero_signal
					,_present.control_hi
					,_present.sut_flag
				)
				, r_low(
					_pi_config
					,_present.r_low
					, zero_signal//const signal_t & _control_req
					, _present.signal_lo
					, nullptr
					, nullptr
					, zero_signal
					, controlRange.hi
					,_present.control_low
					,_present.sut_flag
				)
				, control_req(_control_req)
				, control_val(_control_val)
				, signal(_signal)
				, signalRange(_signalRange)
				, controlRange(_controlRange)
			{
			}
			virtual void run(void){
				ACTOR_CONFIG_S(s);
				ACTOR_PRESENT_S(p);
				long_signal_t test_lim_control;
				p.signal_hi = signal - signalRange.hi;
				p.signal_low = signal - signalRange.lo;
				r_hi.run();
				r_low.run();

				if( p.control_des < control_req){
					p.control_des = s_inc(p.control_des, s.ramp, controlRange.min, control_req);
				} else {
					if( p.control_des > control_req){
						p.control_des = s_inc(p.control_des, -s.ramp, control_req, controlRange.max);
					}
				}

				test_lim_control = p.control_hi +  p.control_low + p.control_des;    
    
				if(test_lim_control > controlRange.max){
					p.control_val =  controlRange.max;
					p.sut_flag = satstates::up;
				} else if(test_lim_control <  controlRange.min){
					p.control_val =   controlRange.min;        
					p.sut_flag = satstates::low;
				} else {
					p.sut_flag = satstates::none;
					p.control_val =  test_lim_control;                
				}			
			}
			
			virtual void begin(void){
				r_hi.reset();
				r_low.reset();
				ACTOR_PRESENT_S(p);
				p = {};

			}
	};
} 
#endif