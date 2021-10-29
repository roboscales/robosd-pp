#ifndef __regulator_hpp
#define __regulator_hpp
#include "mexo/mexo.hpp"
namespace mexo {

	template< typename q >  class  quazzy_adapt
		: public controller_handler< typename q::signal_t, typename q::signal_t > {
		typedef controller_handler<typename q::signal_t, typename q::signal_t> A;
	public:
		typedef typename  q::signal_t signal_t;
		typedef typename  q::long_signal_t long_signal_t;
		typedef typename  q::parameter_t parameter_t;

		struct config_s {
			typename A::config_s cb;
			signal_t adjust_value;
			parameter_t propGain;
			parameter_t modelGain;
			parameter_t diffGain;
			uint8_t control_shift;
			uint8_t model_shift;
		};
		struct present_s {
			typename A::present_s cb;
			long_signal_t control;
			long_signal_t control_diff;
			long_signal_t model;
			long_signal_t model_l;
			long_signal_t error;
		};
		const signal_t& actual;
		const signal_t& actual_diff;
	protected:
		virtual void do_handler_create_vars(var::record::list& _vars, int _master_key) {
			A::do_handler_create_vars(_vars, _master_key);
			const config_s& config = handler::config_cast<config_s>();
			present_s& present = handler::present_cast<present_s>();
			if (var::machine::actual_mode() >= var::machine::mode::tuning) {
				var::record::create(q::var::parameter, config.propGain, RT("pg"), _master_key, _vars);
				var::record::create(q::var::parameter, config.modelGain, RT("mg"), _master_key, _vars);
				var::record::create(q::var::parameter, config.diffGain, RT("dg"), _master_key, _vars);
				var::record::create(var::uint8, config.control_shift, RT("co_sh"), _master_key, _vars);
				var::record::create(var::uint8, config.model_shift, RT("mo_sh"), _master_key, _vars);
			}
		};

	public:
		quazzy_adapt(const config_s& _config
					 , present_s& _present
					 , const range_s<signal_t>& _range
					 , const satstate_t& _master_satstate
					 , const signal_t& _actual
					 , const signal_t& _actual_diff
		)
			: A(_config.cb, _present.cb, _range, _master_satstate)
			, actual(_actual), actual_diff(_actual_diff) {}

		void execute(void) {
			//		satstate remote = controller_block_t<I, O>::master_satstate.value();

			if (A::master_satstate == satstate_t::both) {
				return;
			}

			present_s& present = handler::present_cast<present_s>();
			const config_s& config = handler::config_cast<config_s>();

			present.error = (long_signal_t) * A::deseired - actual;

			long_signal_t tmp = present.error + present.model - actual;

			present.control = tmp * config.propGain;

			present.control_diff = actual_diff * config.diffGain;
			present.control += present.control_diff;
			present.cb.satstate.local = q::round_s(present.control, A::range, config.control_shift, *A::output);

			if (A::master_satstate == satstate_t::none) {
				present.cb.satstate.actual = present.cb.satstate.local;
			}
			else {
				present.cb.satstate.actual = A::master_satstate;
			}

			long_signal_t model_inc = present.error * config.modelGain;

			if (model_inc > std::numeric_limits<long_signal_t>::epsilon()) {
				if (present.cb.satstate.actual != satstate_t::up) {
					if (present.model_l > 0) {
						long_signal_t d = q::long_max;
						d -= present.model_l;
						if (d < model_inc) {
							model_inc = d;
							present.model_l = q::long_max;
						}
						else {
							present.model_l += model_inc;
						}
					}
					else {
						present.model_l += model_inc;
					}
				}
			}
			else {
				if (model_inc < -std::numeric_limits<long_signal_t>::epsilon()) {
					if (present.cb.satstate.actual != satstate_t::low) {
						if (present.model_l < 0) {
							long_signal_t d = q::long_min;
							d -= present.model_l;
							if (d > model_inc) {
								present.model_l = q::long_min;
							}
							else {
								present.model_l += model_inc;
							}
						}
						else {
							present.model_l += model_inc;
						}
					}
				}
			}
			present.model = q::round_l(present.model_l, config.model_shift);
		}

		void do_handler_adjust(void) {
			present_s& present = handler::present_cast<present_s>();
			const config_s& config = handler::config_cast<config_s>();

			if (config.propGain > (parameter_t)0) {
				present.control = (long_signal_t)config.adjust_value * (1 << config.control_shift);
				*A::output = config.adjust_value;
				present.model = present.control / config.propGain;
				present.model_l = present.model * (1 << config.model_shift);
			}
			else {
				present.control = (long_signal_t)0;
				*A::output = (signal_t)0;
				present.model = present.model_l = (long_signal_t)0;
			}
		}
	};


	template< typename q >  class  limmiter
		: public controller_handler< typename q::signal_t, typename q::signal_t > {
		typedef controller_handler<typename q::signal_t, typename q::signal_t> A;
	public:
		typedef typename  q::signal_t signal_t;
		typedef typename  q::long_signal_t long_signal_t;
		typedef typename  q::parameter_t parameter_t;
		typedef quazzy_adapt<q> quazzy_adapt;

		struct config_s {
			typename A::config_s cb;
			typename quazzy_adapt::config_s qa;
			signal_t rampStep;
		};

		struct req_signals {
			signal_t signal;
			signal_t control;
			satstate_t local_satstate;
			satstate_t satstate;
		};

		struct present_s {
			typename A::present_s cb;
			struct {
				typename quazzy_adapt::present_s qa;
				req_signals lm;
			}  hi;
			struct {
				typename quazzy_adapt::present_s qa;
				req_signals lm;
			}  low;
			long_signal_t control;
			signal_t control_des;
		};
		const signal_t& actual;
		const signal_t& actual_diff;
	protected:
		virtual void do_handler_create_vars(var::record::list& _vars, int _master_key) {
			A::do_handler_create_vars(_vars, _master_key);
			const config_s& config = handler::config_cast<config_s>();
			present_s& present = handler::present_cast<present_s>();
			if (var::machine::actual_mode() >= var::machine::mode::tuning) {
				var::record::create(q::var::parameter, config.qa.propGain, RT("pg"), _master_key, _vars);
				var::record::create(q::var::parameter, config.qa.modelGain, RT("mg"), _master_key, _vars);
				var::record::create(q::var::parameter, config.qa.diffGain, RT("dg"), _master_key, _vars);
				var::record::create(var::uint8, config.qa.control_shift, RT("co_sh"), _master_key, _vars);
				var::record::create(var::uint8, config.qa.model_shift, RT("mo_sh"), _master_key, _vars);
				var::record::create(q::var::signal, config.rampStep, RT("st"), _master_key, _vars);
			}
		};

	private:
		quazzy_adapt r_hi_;
		quazzy_adapt r_low_;
		range_s<signal_t> range_hi_;
		range_s<signal_t> range_low_;
		const range_s<signal_t>& signal_range_;
	protected:
		void execute(void) {
			present_s& present = handler::present_cast<present_s>();
			const config_s& config = handler::config_cast<config_s>();

			present.hi.lm.signal = actual - signal_range_.hi;
			present.low.lm.signal = actual - signal_range_.low;
			range_hi_.low = A::range.low;
			range_low_.hi = A::range.hi;

			r_hi_.execute();
			r_low_.execute();

			signal_t inp = *A::deseired;
			long_signal_t delta = inp - present.control_des;

			if (delta > std::numeric_limits<long_signal_t>::epsilon()) {
				if (delta < config.rampStep) {
					present.control_des = inp;
				}
				else {
					present.control_des += config.rampStep;
				}
			}
			else {
				if (delta < -std::numeric_limits<long_signal_t>::epsilon()) {
					if ((-delta) < config.rampStep) {
						present.control_des = inp;
					}
					else {
						present.control_des -= config.rampStep;
					}
				}
			}

			present.control = present.hi.lm.control + present.low.lm.control + present.control_des;

			present.cb.satstate.local = q::round_s(present.control, A::range, 0, *A::output);

			if (A::master_satstate == satstate_t::none) {
				present.cb.satstate.actual = present.cb.satstate.local;
			}
			else {
				present.cb.satstate.actual = A::master_satstate;
			}
		}
		virtual void do_handler_adjust(void) {
			r_hi_.do_handler_adjust();
			r_low_.do_handler_adjust();
			handler::present_cast<present_s>().control_des = 0;
			*A::output = 0;
		}
	public:
		limmiter(
			const config_s& _config
			, present_s& _present
			, const range_s<signal_t>& _range
			, const satstate_t& _master_satstate
			, const signal_t& _actual
			, const signal_t& _actual_diff
			, const range_s<signal_t>& _signal_range
		) : A(_config.cb, _present.cb, _range, _master_satstate)
			, actual(_actual), actual_diff(_actual_diff)
			, r_hi_(
				_config.qa
				, _present.hi.qa
				, range_hi_
				, _master_satstate
				, _present.hi.lm.signal
				, _actual_diff
			)
			, r_low_(
				_config.qa
				, _present.low.qa
				, range_low_
				, _master_satstate
				, _present.low.lm.signal
				, _actual_diff
			)
			, signal_range_(_signal_range) {
			r_hi_.set_output(&_present.hi.lm.control);
			r_low_.set_output(&_present.low.lm.control);
		}
	};

	
		template< typename q >  class  motion
		: public controller_handler< typename q::signal_t, typename q::signal_t > {
		typedef controller_handler<typename q::signal_t, typename q::signal_t> A;
	public:
		typedef typename  q::signal_t signal_t;
		typedef typename  q::long_signal_t long_signal_t;
		typedef typename  q::parameter_t parameter_t;

		struct config_s {
			typename A::config_s cb;
			signal_t adjust_value;
			parameter_t propGain;
			parameter_t modelGain;
			uint8_t controlShift;
			uint8_t modelShift;
			signal_t forceStep;
			signal_t forceLim;
		};
		struct present_s {
			typename A::present_s cb;
			long_signal_t control;
			long_signal_t model;
			long_signal_t model_l;
			long_signal_t error;
			long_signal_t force;
		};
		const signal_t& actual;
		protected:
		virtual void do_handler_create_vars(var::record::list& _vars, int _master_key) {
			A::do_handler_create_vars(_vars, _master_key);
			const config_s& config = handler::config_cast<config_s>();
			present_s& present = handler::present_cast<present_s>();
			if (var::machine::actual_mode() >= var::machine::mode::tuning) {
				var::record::create(q::var::parameter, config.propGain, RT("pg"), _master_key, _vars);
				var::record::create(q::var::parameter, config.modelGain, RT("mg"), _master_key, _vars);
				var::record::create(var::uint8, config.controlShift, RT("co_sh"), _master_key, _vars);
				var::record::create(var::uint8, config.modelShift, RT("mo_sh"), _master_key, _vars);
				var::record::create(q::var::signal, config.forceStep, RT("f_st"), _master_key, _vars);
				var::record::create(q::var::signal, config.forceLim, RT("f_lim"), _master_key, _vars);
			}
		};

	public:
		motion(const config_s& _config
					 , present_s& _present
					 , const range_s<signal_t>& _range
					 , const satstate_t& _master_satstate
					 , const signal_t& _actual
		)
			: A(_config.cb, _present.cb, _range, _master_satstate)
			, actual(_actual) {}

		void execute(void) {
			//		satstate remote = controller_block_t<I, O>::master_satstate.value();

			if (A::master_satstate == satstate_t::both) {
				return;
			}

			present_s& present = handler::present_cast<present_s>();
			const config_s& config = handler::config_cast<config_s>();

			present.error = (long_signal_t) * A::deseired - actual;

			
			if (  actual ==0 ){
				if(present.error>0){
					if(present.force>config.forceLim-config.forceStep)
							present.force=config.forceLim;
					else {
						present.force+=config.forceStep;
					}
				}else if (present.error<0){                
					signal_t fm = -config.forceLim;
					if(present.force < fm+config.forceStep){
						present.force=fm;
					} else{
						present.force-=config.forceStep;	
					}
				}
			}else {
				present.force = 0;
			}
				
			long_signal_t tmp = present.error + present.model - actual;

			present.control = tmp * config.propGain;

			present.cb.satstate.local = q::round_s(present.control, A::range, config.controlShift, *A::output);

			if (A::master_satstate == satstate_t::none) {
				present.cb.satstate.actual = present.cb.satstate.local;
			}
			else {
				present.cb.satstate.actual = A::master_satstate;
			}

			long_signal_t model_inc = present.error * config.modelGain;

			if (model_inc > std::numeric_limits<long_signal_t>::epsilon()) {
				if (present.cb.satstate.actual != satstate_t::up) {
					if (present.model_l > 0) {
						long_signal_t d = q::long_max;
						d -= present.model_l;
						if (d < model_inc) {
							model_inc = d;
							present.model_l = q::long_max;
						}
						else {
							present.model_l += model_inc;
						}
					}
					else {
						present.model_l += model_inc;
					}
				}
			}
			else {
				if (model_inc < -std::numeric_limits<long_signal_t>::epsilon()) {
					if (present.cb.satstate.actual != satstate_t::low) {
						if (present.model_l < 0) {
							long_signal_t d = q::long_min;
							d -= present.model_l;
							if (d > model_inc) {
								present.model_l = q::long_min;
							}
							else {
								present.model_l += model_inc;
							}
						}
						else {
							present.model_l += model_inc;
						}
					}
				}
			}
			present.model = q::round_l(present.model_l, config.modelShift);
		}

		void do_handler_adjust(void) {
			present_s& present = handler::present_cast<present_s>();
			const config_s& config = handler::config_cast<config_s>();

			if (config.propGain > (parameter_t)0) {
				present.control = (long_signal_t)config.adjust_value * (1 << config.controlShift);
				*A::output = config.adjust_value;
				present.model = present.control / config.propGain;
				present.model_l = present.model * (1 << config.modelShift);
			}
			else {
				present.control = (long_signal_t)0;
				*A::output = (signal_t)0;
				present.model = present.model_l = (long_signal_t)0;
			}
		}
	};

	template< typename q >  class  positioner
		: public controller_handler< typename q::long_signal_t, typename q::signal_t > {
		typedef controller_handler<typename q::long_signal_t, typename q::signal_t> A;
	public:
		typedef typename  q::signal_t signal_t;
		typedef typename  q::long_signal_t long_signal_t;
		typedef typename  q::parameter_t parameter_t;

		struct config_s {
			typename A::config_s cb;
			long_signal_t adjust_value;
			parameter_t propGain;
			parameter_t diffGain;
			uint8_t controlShift;
			parameter_t diffQuadrGain;
			uint8_t diffQuadrShift;
			signal_t deadZone;
			signal_t crawlSpeed;
		};
		struct present_s {
			typename A::present_s cb;
			long_signal_t control;
			long_signal_t controlDiff;
			long_signal_t controlDiffQuadr;
			long_signal_t error;
		};
		const long_signal_t& actual;
		const signal_t& actualDiff;
		const signal_t& force;
	protected:
		virtual void do_handler_create_vars(var::record::list& _vars, int _master_key) {
			A::do_handler_create_vars(_vars, _master_key);
			const config_s& config = handler::config_cast<config_s>();
			present_s& present = handler::present_cast<present_s>();
			if (var::machine::actual_mode() >= var::machine::mode::tuning) {
				var::record::create(q::var::parameter, config.propGain, RT("pg"), _master_key, _vars);
				var::record::create(q::var::parameter, config.diffGain, RT("dg"), _master_key, _vars);
				var::record::create(var::uint8, config.controlShift, RT("co_sh"), _master_key, _vars);
				var::record::create(q::var::parameter, config.diffQuadrGain, RT("qg"), _master_key, _vars);
				var::record::create(q::var::signal, config.deadZone, RT("dz"), _master_key, _vars);
				var::record::create(q::var::signal, config.crawlSpeed, RT("cr_sp"), _master_key, _vars);
			}
		};
	public:
		positioner(
			const config_s& _config
			 , present_s& _present
			 , const range_s<signal_t>& _range
			 , const satstate_t& _master_satstate
			 , const long_signal_t& _actual
			 , const signal_t& _actualDiff
			 , const signal_t& _force
		)
			: A(_config.cb, _present.cb, _range, _master_satstate)
			, actual(_actual)
			, actualDiff(_actualDiff)
			, force(_force) 
{}

		void execute(void) {
			if (A::master_satstate == satstate_t::both) {
				return;
			}

			present_s& present = handler::present_cast<present_s>();
			const config_s& config = handler::config_cast<config_s>();

			present.error = *A::deseired - actual;
			const signal_t max_err = std::numeric_limits<signal_t>::max();
			
			if (present.error==0){
				present.control = (long_signal_t)0;
				*A::output = (signal_t)0;					return;
			}else{
					if(present.error > config.deadZone){
							if (present.error>max_err){
									present.error = max_err;
							} else {
									if(config.crawlSpeed == 0) {
											present.error -= config.deadZone;
									}
							}
					}
					if(present.error < -config.deadZone){
							if (present.error<-max_err){
									present.error = -max_err;
							} else {
									if(config.crawlSpeed == 0) {
											present.error += config.deadZone;
									}
							}
					}
			}
			present.control = present.error*config.propGain;
			
			if(config.diffQuadrGain>0){
				present.controlDiffQuadr = actualDiff*actualDiff;
				present.controlDiffQuadr = q::round_l(present.controlDiffQuadr, config.diffQuadrShift);
				present.controlDiffQuadr *= config.diffQuadrGain;
			} else {
				present.controlDiffQuadr = 0;
			}
				
			present.controlDiff = -actualDiff*config.diffGain;

			present.control += present.controlDiff;
			present.control += present.controlDiffQuadr;
			present.control = q::round_l(present.control, config.controlShift);
			present.control += force;

			if(present.control == 0 && config.crawlSpeed >0 ){
				if(present.error > config.deadZone){
					present.control =   config.crawlSpeed;				
				} else if( present.error < -config.deadZone) {
					present.control =   -config.crawlSpeed;				
				}
			}
				
			present.cb.satstate.local = q::round_s(present.control, A::range, 0, *A::output);

			if (A::master_satstate == satstate_t::none) {
				present.cb.satstate.actual = present.cb.satstate.local;
			}
			else {
				present.cb.satstate.actual = A::master_satstate;
			}
		}
			

		void do_handler_adjust(void) {
			present_s& present = handler::present_cast<present_s>();
			const config_s& config = handler::config_cast<config_s>();

			if (config.propGain > (parameter_t)0) {
				present.control = (long_signal_t)config.adjust_value * (1 << config.controlShift);
				*A::output = config.adjust_value;
			}
			else {
				present.control = (long_signal_t)0;
				*A::output = (signal_t)0;
			}
		}
	};	
}

#endif