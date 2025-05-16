#ifndef burst_positioner_hpp
#define burst_positioner_hpp
#include "burst++/modules/actor.hpp"
namespace burst{
	template < class number > class positioner_t {
		public:
			using parameter_t = typename number::parameter_t;
			using signal_t = typename number::signal_t;
			using long_signal_t = typename number::long_signal_t;
			struct config_s{
				parameter_t	propGain;
				parameter_t	diffGain;
				parameter_t diffQuardGain;
				uint8_t			diffQuardPreShift;
				uint8_t			controlShift;
				signal_t deadZone;
				signal_t crawlSpeed;
			};
			#if ROBO_APP_BURST_VARTREE_ENABLED
			static void regvar_config(robo::cstr _name, const config_s& _config) {
				var::push(_name);

				var::reg(number::var::parameter, _config.propGain, RT("prop"));
				var::reg(number::var::parameter, _config.diffGain, RT("diff"));
				var::push(RT("quard"));
				var::reg(number::var::parameter, _config.diffQuardGain, RT("gain"));
				var::reg(var::types::uint8, _config.diffQuardPreShift, RT("sh"));
				var::pop();
				
				var::reg(number::var::signal, _config.deadZone, RT("dz"));
				var::reg(number::var::signal, _config.crawlSpeed, RT("cs"));
				var::reg(var::types::uint8, _config.controlShift, RT("sh"));

				var::pop();
			}
			#endif

			#define POSITIONER_CONFIG(a) POSITIONER_CONFIG_(a)
			#define POSITIONER_CONFIG_(a)\
			{\
				a##_PROP_GAIN\
				, a##_DIFF_GAIN\
				, a##_DIFF_QUARD_GAIN\
				, a##_DIFF_QUARD_PRE_SHIFT\
				, a##_CONTROL_SHIFT\
				, a##_DEAD_ZONE\
				, a##_CRAWL_SPEED\
			}
			
			struct present_s{
				long_signal_t diff;
				long_signal_t quadDiff;
			};
			#if ROBO_APP_BURST_VARTREE_ENABLED
			static void regvar_present(robo::cstr _name, present_s& _present) {
				var::push(_name);
				var::reg(number::var::const_long_signal, _present.diff, RT("diff"));
				var::reg(number::var::const_long_signal, _present.diff, RT("quad"));
				var::pop();
			}
			#endif
			present_s & 			present;
			const long_signal_t &	signal_req;
			const long_signal_t &	signal;
			private:
			const config_s *  config_;
			const signal_t *	signal_diff_;
			const signal_t *	forceControl_;
			const signal_t *	controlMin_;
			const signal_t *	controlMax_;
			signal_t *	control_;
			public:
			
			positioner_t(
				present_s & 			_present
				, const long_signal_t &	_signal_req
				, const long_signal_t &	_signal
			) 
			: present(_present)
			, signal_req(_signal_req)
			, signal(_signal)
			{
				setup( nullptr,nullptr,nullptr,nullptr,nullptr,nullptr );
			}
			
			void setup(
				const config_s *  _config
				, const signal_t *	_signal_diff
				, const signal_t *	_forceControl
				, const signal_t *	_controlMin
				, const signal_t *	_controlMax
				, signal_t *	_control
			){
				connectto(config_,_config);
				connectto(signal_diff_,_signal_diff);
				connectto(forceControl_,_forceControl);
				connectto(controlMin_,_controlMin);
				connectto(controlMax_,_controlMax);
				connectto(control_,_control);
			}
			virtual void run( void ){
				long_signal_t err = signal_req - signal;
				signal_t deadZone = config_->deadZone;
				if (err==0){
						if (forceControl_) {
							*control_= *(forceControl_);
						} else{
							*control_= 0;
						}
						return;
				}else{
						#ifndef BURTS_POSITINER_MAX_ERR
						#define BURTS_POSITINER_MAX_ERR number::l_frac(0.9)
						#endif
						if(err > config_->deadZone){
								if (err>BURTS_POSITINER_MAX_ERR){
										err = BURTS_POSITINER_MAX_ERR;
								} else {
										if(config_->crawlSpeed == 0) {
												err -= deadZone;
										}
								}
						} else if(err > 0){
							if (forceControl_) {
								*control_= *(forceControl_);
							} else{
								*control_= 0;
							}
							return;
						}
						if(err < -config_->deadZone){
								if (err<-BURTS_POSITINER_MAX_ERR){
										err = -BURTS_POSITINER_MAX_ERR;
								} else {
										if(config_->crawlSpeed == 0) {
												err += deadZone;
										}
								}
						} else if(err < 0){
							if (forceControl_) {
								*control_= *(forceControl_);
							} else{
								*control_= 0;
							}
							return;
						}
				}
				{        
					long_signal_t control_val = err*(config_->propGain);
					if(signal_diff_){
						auto d =*signal_diff_;
						present.diff = -d * config_->diffGain;
						control_val += present.diff;
						if(config_->diffQuardGain && d != 0){
							long_signal_t d2 = d*d;							
							d2 = robo::digit::rsh(d2, config_->diffQuardPreShift);
							d2*=config_->diffQuardGain;
							if(d>0){
								present.quadDiff =  -d2;
							}  else {
								present.quadDiff =  d2;
							}
							control_val += present.quadDiff;
						}
					}
					if (control_val > 0) {
						control_val = robo::digit::rsh(control_val, config_->controlShift);
						if (control_val < config_->crawlSpeed ) {
							control_val = config_->crawlSpeed;
						}
					}
					else if(control_val < 0) {
						control_val = robo::digit::rsh(control_val, config_->controlShift);
						if (control_val > -config_->crawlSpeed) {
							control_val = -config_->crawlSpeed;
						}
					}
					
					if (forceControl_) control_val += *(forceControl_);
					control_val = robo::saturate(control_val, *controlMin_, *controlMax_);
					*control_ = control_val;
				}

			}
	};
}
#endif