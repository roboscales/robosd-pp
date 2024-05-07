#ifndef burst_motion_hpp
#define burst_motion_hpp
#include "burst++/modules/actor.hpp"
namespace burst{
	template < class number > class motion_t {
		public:
			using parametr_t = typename number::parametr_t;
			using signal_t = typename number::signal_t;
			using long_min = typename number::long_min;
			using long_signal_t = typename number::long_signal_t;
			struct config_s{
				actor::config_s tag;
				parametr_t	propGain;
				parametr_t	modelGain;
				parametr_t	diffGain;
				parametr_t	forceGain;
				signal_t		forceMax;
				uint8_t			controlShift;
				uint8_t			modelShift;
				parametr_t	limitGain;
				uint8_t			limitGainPresc;
				bool 				elasticLimmiterEnabled;
			} ;
			
			#define MOTION_CONFIG(a) MOTION_CONFIG_(a)
			#define MOTION_CONFIG_(a)\
			{\
				a##_PROP_GAIN\
				, a##_MODEL_GAIN\
				, a##_DIFF_GAIN\
				, a##_FORCE_GAIN\
				, a##_FORCE_MAX\
				, a##_CONTROL_SHIFT\
				, a##_MODEL_SHIFT\
				, a##_LIMIT_GAIN\
				, a##_LIMIT_GAIN_PRESC\
				, a##_ELASTIC_LIMMITER_ENABLED\
			}
			
				//satstate_t		satstate;
	//motion_config_p config;
	//signal_p			signal_req;
	//signal_p			signal;
	//signal_p			signal_diff;
	//signal_p			controlMax;
	//signal_p			controlMin;
	//long_signal_p reference;
	//long_signal_p reference_max;
	//long_signal_p reference_min;
	//satstate_t *	master_sut_flag;
	//long_signal_t model;
	//long_signal_t force;
	//long_signal_t long_model;
	//signal_t			* control;
			const signal_t &	signal_req;
			const signal_t &	signal;
		private:
			const config_s * config_;
			const signal_t *	signal_diff_;
			const signal_t *	signal_force_;
			const signal_t *	controlMin_;
			const signal_t *	controlMax_;
			long_signal_t * reference_;
			long_signal_t * reference_max_;
			long_signal_t * reference_min_;
			signal_t *	control_;
			const satstates *	master_sut_flag_;
		public:

			struct present_s{
				actor::present_s tag;
				long_signal_t model;
				long_signal_t long_model;
				long_signal_t diff;
				long_signal_t force;
				satstates		satstate;
			} & present;
			
			virtual void run(void){
				//ACTOR_CONFIG_S(s);
				//ACTOR_PRESENT_S(p);
				long_signal_t Error;
				long_signal_t tmp;
				long_signal_t controlLong;
				
				signal_t controlMax = *controlMax_;
				signal_t controlMin = *controlMin_;
				
				satstates sut_flag;
				if ( reference_ != 0) {
					long_signal_t r_max = *reference_max_;
					long_signal_t r_min = *reference_min_;
					long_signal_t r = *reference_;
				
					if (config_->elasticLimmiterEnabled) {
						if (r > r_max) {
								long_signal_t tmp = (r_max - r);
								tmp = number::fast::rsh( tmp, config_->limitGainPresc );
								tmp *= config_->limitGain;
								if (tmp < 0) {
										tmp = 0;
								}
								if (signal_req > tmp) {
										signal_req = tmp;
								}            
						}	else {
								if (r < r_min) {
										long_signal_t tmp = (r_min - r);
										tmp = number::fast::rsh( tmp, config_->limitGainPresc );
										tmp *= config_->limitGain;
										if (tmp > 0) {
												tmp = 0;
										}
										if (signal_req < tmp) {
												signal_req = tmp;
										}
								}
						}
					} else {
						if (r > r_max) {
							long_signal_t tmp = (r_max - r);
							tmp = number::fast::rsh( tmp, config_->limitGainPresc );
							tmp *= config_->limitGain;
							controlMax = controlMax + number::fast::rsh( tmp, config_->controlShift);
							if (controlMax < 0) {
									controlMax = 0;
							}
						}
						else {
							if (r < r_min) {
								long_signal_t tmp = (r_min - r);
								tmp = number::fast::rsh( tmp, config_->limitGainPresc );
								tmp *= config_->limitGain;
								controlMin = controlMin +number::fast::rsh(tmp, config_->controlShift);
								if (controlMin > 0)
										controlMin = 0;
							}
						}
					}
				}

				#ifndef MODEL_VALUE_MAX
				#define MODEL_VALUE_MAX number::long_frac(0.9)
				#endif
				signal_t control = * control_;
				Error = signal_req - signal;
				if( (Error>0) && ( (control>=controlMax) || (present.long_model> MODEL_VALUE_MAX) || (*master_sut_flag_ == satstates::up) ) ){
						sut_flag = satstates::up;
				} else {
					if( (Error<0) && ( (control<=controlMin) || (present.long_model<-MODEL_VALUE_MAX) || (*master_sut_flag_ == satstates::low)  ) ){
								sut_flag = satstates::low;
						}
						else sut_flag =  *master_sut_flag_;
				}
				present.satstate = sut_flag;
				if ( sut_flag == satstates::none )
				{
					long_signal_t tmp;
					tmp = (present.long_model += (Error* config_->modelGain) );
					tmp = number::fast::rsh( tmp, config_->modelShift );
					present.model = tmp = number::saturate(tmp, number::min, number::max );

					if (  signal ==0 ){
						if(Error>0){
							parametr_t fm = config_->forceMax;
							present.force+=config_->forceGain;
							if(present.force>fm)
									present.force=fm;
						}else if (Error<0){
							parametr_t fm = -config_->forceMax;
							present.force-=config_->forceGain;
							if(present.force<fm)
									present.force=fm;
						}
					}else {
						present.force = 0;
					}
				}
				
				tmp = Error+ present.model - signal;
				
				if (signal_diff_ != 0) {
					present.diff =  - *(signal_diff_) * config_->diffGain;
					controlLong += present.diff;
				}
				
				controlLong = tmp* config_->propGain;
				controlLong = number::fast::rsh(controlLong, config_->controlShift) + present.force;
				controlLong = number::saturate(controlLong, controlMin,controlMax );
				 * control_ = number::s_extract(controlLong);				
			}
			
			motion_t(
				present_s & _present
				, const signal_t &	_signal_req
				, const signal_t &	_signal
			)
				: present(_present)
				, signal_req(_signal_req)
				, signal(_signal){
				setup(nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr);
			}
				
			virtual void setup(
				const config_s * _config
				, const signal_t *	_signal_diff
				, const signal_t *	_signal_force
				, const signal_t *	_controlMin
				, const signal_t *	_controlMax
				, long_signal_t * _reference
				, long_signal_t * _reference_max
				, long_signal_t * _reference_min
				, signal_t *	_control
				, const satstates *	_master_sut_flag){
					
					connectto(config_,_config);
					connectto(signal_diff_,_signal_diff);
					connectto(signal_force_,_signal_force);
					connectto(controlMin_,_controlMin);
					connectto(controlMax_,_controlMax);
					connectto(reference_,_reference);
					connectto(reference_max_,_reference_max);
					connectto(reference_min_,_reference_min);
					connectto(control_,_control);
					connectto(master_sut_flag_,_master_sut_flag);
			}
			virtual void begin(void){
				present = {};
			}
	};
}
#endif
