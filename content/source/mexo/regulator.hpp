#ifndef __regulator_hpp
#define __regulator_hpp
#include "mexo/mexo.hpp"
namespace mexo {

	template< typename q >  class  quazzy_adapt
		: public controller_handler< typename q::signal_t, typename q::discret_t > {
		typedef controller_handler<typename q::signal_t, typename q::discret_t> A;
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
			present.model = q::round_s(present.model_l, config.model_shift);
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
		: public controller_handler< typename q::signal_t, typename q::discret_t > {
		typedef controller_handler<typename q::signal_t, typename q::discret_t> A;
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

	/*
	#ifndef MODEL_VALUE_MAX
	#define MODEL_VALUE_MAX 0.9
	#endif

	#define regulator_settings_t struct regulator_settings_s
	#define regulator_settings_p regulator_settings_t *

	struct regulator_settings_s {
		mexo_parametr_t propGain;
		mexo_parametr_t modelGain;
		mexo_parametr_t diffGain;
		uint8_t controlShift;
		uint8_t modelShift;
	};

	#define regulator_t struct regulator_s
	#define regulator_p regulator_t *
	typedef enum { REGULATOR_SAT_NEG = -1, REGULATOR_SAT_NONE = 0, REGULATOR_SAT_POS = 1 } regulator_sat_flag_t;
	struct regulator_s {
		regulator_settings_p settings;
		mexo_signal_t* signal_req;
		mexo_signal_t* signal;
		mexo_signal_t* control_val;
		mexo_signal_t* controlMax;
		mexo_signal_t* controlMin;
		mexo_long_signal_t model;
		mexo_long_signal_t long_model;
		mexo_signal_t* signal_diff;
		mexo_long_signal_t control_diff;
		regulator_sat_flag_t sut_flag;
		regulator_sat_flag_t* master_sut_flag;
	};
	void regulator_run(regulator_p);
	void regulator_reset(regulator_p);


	#define limiter_t struct limiter_s
	#define limiter_p limiter_t *
	struct limiter_s {
		regulator_t r_hi;
		regulator_t r_low;
		mexo_signal_t zero_signal;
		mexo_signal_t signal_hi;
		mexo_signal_t signal_low;
		mexo_signal_t control_hi;
		mexo_signal_t control_low;
		mexo_signal_t control_des;
		regulator_sat_flag_t sut_flag;
		mexo_signal_t* control_req;
		mexo_signal_t* control_val;
		mexo_signal_t* signal;
		mexo_signal_t* controlMax;
		mexo_signal_t* controlMin;
		mexo_signal_t* signalMin;
		mexo_signal_t* signalMax;
		mexo_parametr_t* ramp;

	};

	#define limiter_config_t struct limiter_config_s
	#define limiter_config_p limiter_config_t *
	struct limiter_config_s {
		mexo_signal_t* control_req;
		mexo_signal_t* control_val;
		mexo_signal_t* controlMin;
		mexo_signal_t* controlMax;
		mexo_signal_t* signal;
		mexo_signal_t* signalMin;
		mexo_signal_t* signalMax;
		regulator_settings_p reg_settings;
		mexo_parametr_t* ramp;
	};

	#define motion_reg_settings_t struct motion_reg_settings_s
	#define motion_reg_settings_p motion_reg_settings_t *

	void limiter_run(limiter_p);
	void limiter_reset(limiter_p);
	void limiter_init(limiter_p _limiter, limiter_config_p _config);

	struct motion_reg_settings_s {
		mexo_parametr_t propGain;
		mexo_parametr_t modelGain;
		mexo_parametr_t diffGain;
		mexo_parametr_t forceGain;
		mexo_signal_t forceMax;
		uint8_t controlShift;
		uint8_t modelShift;
		mexo_parametr_t limitGain;
		uint8_t limitGainPresc;
	};

	#define motion_reg_t struct motion_reg_s
	#define motion_reg_p motion_reg_t *

	struct motion_reg_s {
		motion_reg_settings_p settings;
		mexo_signal_t* signal_req;
		mexo_signal_t* signal;
		mexo_signal_t* signal_diff;
		mexo_signal_t* signal_force_diff;
		mexo_signal_t* control_val;
		mexo_signal_t* controlMax;
		mexo_signal_t* controlMin;
		mexo_signal_t* voltage;
		mexo_long_signal_t model;
		mexo_long_signal_t force;
		mexo_long_signal_t long_model;
		mexo_long_signal_t limit_max;
		mexo_long_signal_t limit_min;
		mexo_long_signal_p reference;
		regulator_sat_flag_t sut_flag;
		regulator_sat_flag_t* master_sut_flag;
	};

	void motion_reg_reset(motion_reg_p r);
	void motion_reg_run(motion_reg_p r);


	#define posicioner_settings_t struct posicioner_settings_s
	#define posicioner_settings_p posicioner_settings_t *

	struct posicioner_settings_s {
		mexo_parametr_t propGain;
		mexo_parametr_t diffGain;
		mexo_parametr_t diffQuardGain;
		uint8_t controlShift;
		mexo_signal_t deadZone;
		mexo_signal_t crawlSpeed;
	};

	#define posicioner_t struct posicioner_s
	#define posicioner_p posicioner_t *

	struct posicioner_s {
		posicioner_settings_p settings;
		mexo_long_signal_t* signal_req;
		mexo_long_signal_t* signal;
		mexo_signal_t* signal_diff;
		mexo_signal_t* forceControl;
		mexo_signal_t* control_val;
		mexo_signal_t* controlMax;
		mexo_signal_t* controlMin;
	};

	void posicioner_run(posicioner_p);

	#include "__robosd_head_end.h"

	*/
}

#endif