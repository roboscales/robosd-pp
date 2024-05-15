#ifndef burst_pi_hpp
#define burst_pi_hpp
#include "burst++/modules/actor.hpp"
namespace burst{
	template <class number> class pi_t {
	public:
		using parameter_t = typename number::parameter_t;
		using signal_t = typename number::signal_t;
		using long_signal_t = typename number::long_signal_t;
		const struct config_s{
			parameter_t	propGain;
			parameter_t	modelGain;
			parameter_t	diffGain;
			parameter_t	forceGain;
			uint8_t		controlShift;
			uint8_t		modelShift;
		} & config ;
		#if ROBO_APP_BURST_VARTREE_ENABLED
		static void regvar_config(robo::cstr _name, const config_s& _config) {
			var::push(_name);				{
				var::reg(number::var::parameter, _config.propGain, RT("prop"));
				var::reg(number::var::parameter, _config.modelGain, RT("model"));
				var::reg(number::var::parameter, _config.diffGain, RT("diff"));
				var::reg(number::var::parameter, _config.forceGain, RT("forceGain"));
				var::reg(var::types::uint8, _config.controlShift, RT("csh"));
				var::reg(var::types::uint8, _config.modelShift, RT("msh"));
			} var::pop();
		}
		#endif
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
	private:
		const signal_t &	signal_req_;
		const signal_t &	signal_;
		const signal_t *	signal_diff_;
		const signal_t *	signal_force_;
		const  signal_t &	controlMin_;
		const  signal_t &	controlMax_;
		signal_t &	control_;
		const satstates &	master_sut_flag_;
	public:
		struct present_s{
			long_signal_t model;
			long_signal_t long_model;
			long_signal_t diff;
			long_signal_t force;
			satstates		satstate;
		} & present;
		#if ROBO_APP_BURST_VARTREE_ENABLED
		static void regvar_present(robo::cstr _name, present_s& _present) {
			var::push(_name);
			var::reg(number::var::const_long_signal, _present.model, RT("model"));
			var::reg(number::var::const_long_signal, _present.long_model, RT("lmodel"));
			var::reg(number::var::const_long_signal, _present.diff, RT("diff"));
			var::reg(number::var::const_long_signal, _present.force, RT("force"));
			var::reg(var::types::const_uint8, _present.satstate, RT("satstate"));
			var::pop();
		}
		#endif
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
			, signal_req_(_signal_req)
			, signal_(_signal)
			, signal_diff_(_signal_diff)
			, signal_force_(_signal_force)
			, controlMin_(_controlMin)
			, controlMax_(_controlMax)
			, control_(_control)
			, master_sut_flag_(_master_sut_flag)
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

			Error = signal_req_ - signal_;
			if( (Error>0) && ( (control_ >=controlMax_) || (present.long_model>MODEL_VALUE_MAX) || (master_sut_flag_ == satstates::up) ) ){
				sut_flag = satstates::up;
			} else {
				if( (Error<0) && ( (control_ <=controlMin_) || (present.long_model<-MODEL_VALUE_MAX) || (master_sut_flag_ == satstates::low)  ) ){
						sut_flag = satstates::low;
					}
					else {
						sut_flag =  satstates::none;
					}
			}
			present.satstate = sut_flag;
			if ( sut_flag == satstates::none )
			{
				long_signal_t tmp;
				tmp = (present.long_model += (Error* config.modelGain) );
				tmp = fast::rsh( tmp, config.modelShift );
				present.model = tmp = saturate(tmp, number::min, number::max );
			}
				
			tmp = Error+ present.model - signal_;
			tmp = fast::rsh( tmp , config.controlShift );

			controlLong = tmp* (config.propGain)  ;

			if (signal_force_ != 0) {
				long_signal_t force = (*signal_force_) * (config.forceGain);
				controlLong += force;
				present.force = force;
			}

			if (signal_diff_ != 0) {
				present.diff =  - *(signal_diff_) * config.diffGain;
				controlLong += present.diff;
			}

			controlLong = fast::rsh(controlLong, config.controlShift);
			controlLong = saturate(controlLong, controlMin_,controlMax_);
			control_ = (signal_t)controlLong;
		}
			
		virtual void begin(void){
			present = {};
		}
	};
	template <class number> class limiter_t {
	public:
		using parameter_t = typename number::parameter_t;
		using signal_t = typename number::signal_t;
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
		} & present;
		#if ROBO_APP_BURST_VARTREE_ENABLED
		static void regvar_present(robo::cstr _name, present_s& _present) {
			var::push(_name);
			{
				pi_t<number>::regvar_present(RT("r_hi"), _present.r_hi);
				pi_t<number>::regvar_present(RT("r_low"), _present.r_low);
				var::push(RT("sig"));
				{
					var::reg(number::var::const_signal, _present.signal_hi, RT("hi"));
					var::reg(number::var::const_signal, _present.signal_low, RT("lo"));
				} var::pop();
				var::push(RT("co"));
				{
					var::reg(number::var::const_signal, _present.control_hi, RT("hi"));
					var::reg(number::var::const_signal, _present.control_low, RT("lo"));
				} var::pop();
			} var::pop();
		}
		#endif
		const struct config_s{
			parameter_t ramp;
		} & config;
		#define LIMMITER_CONFIG(a) LIMMITER_CONFIG_(a)
		#define LIMMITER_CONFIG_(a)\
		{\
			ACTOR_CONFIG(a)\
			, a##_RAMP\
		}
		#if ROBO_APP_BURST_VARTREE_ENABLED
		static void regvar_config(robo::cstr _name, const config_s& _config) {
			var::push(_name);
			{
				var::reg(number::var::parameter, _config.ramp, RT("ramp"));
			} var::pop();
		}
		#endif
	private:
		pi_t<number> r_hi_;
		pi_t<number> r_low_;
		const signal_t & control_req_;
		signal_t & control_val_;
		const signal_t & signal_;
		const range_s< signal_t >  & signalRange_;
		const range_s< signal_t >  & controlRange_;
	public:
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
			, r_hi_(
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
			, r_low_(
				_pi_config
				,_present.r_low
				, zero_signal//const signal_t & _control_req
				, _present.signal_low
				, nullptr
				, nullptr
				, zero_signal
				, controlRange_.hi
				,_present.control_low
				,_present.sut_flag
			)
			, control_req_(_control_req)
			, control_val_(_control_val)
			, signal_(_signal)
			, signalRange_(_signalRange)
			, controlRange_(_controlRange)
		{
		}
		virtual void run(void){
			long_signal_t test_lim_control;
			present.signal_hi = signal_ - signalRange_.hi;
			present.signal_low = signal_ - signalRange_.lo;
			r_hi_.run();
			r_low_.run();

			if( present.control_des < control_req_){
				present.control_des = number::s_inc(present.control_des, config.ramp, controlRange_.lo, control_req_);
			} else {
				if( present.control_des > control_req_){
					present.control_des = number::s_inc(present.control_des, -config.ramp, control_req_, controlRange_.hi);
				}
			}

			test_lim_control = present.control_hi +  present.control_low + present.control_des;    
    
			if(test_lim_control > controlRange_.hi){
				control_val_ =  controlRange_.hi;
				present.sut_flag = satstates::up;
			} else if(test_lim_control <  controlRange_.lo){
				control_val_ =   controlRange_.lo;
				present.sut_flag = satstates::low;
			} else {
				present.sut_flag = satstates::none;
				control_val_ =  test_lim_control;
			}			
		}
			
		virtual void begin(void){
			r_hi_.begin();
			r_low_.begin();
			present = {};

		}
	};
} 
#endif