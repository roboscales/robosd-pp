#ifndef __regulator_hpp
#define __regulator_hpp
#include "mexo/mexo.hpp"
namespace mexo {
	template< typename q >  class  quazzy_adapt {
	public:
		typedef typename  q::signal_t signal_t;
		typedef typename  q::long_signal_t long_signal_t;
		typedef typename  q::parameter_t parameter_t;

		struct config_s {
			parameter_t propGain;
			parameter_t modelGain;
			parameter_t diffGain;
			uint8_t control_shift;
			uint8_t model_shift;
		};
		struct present_s {
			long_signal_t control;
			long_signal_t control_diff;
			long_signal_t model;
			long_signal_t model_l;
			long_signal_t error;
		};
	private:
		signal_t& output_;
		iblock::satstate& local_satstate_;
		iblock::satstate& satstate_;
		config_s& config_;
		present_s& present_;

	public:
		quazzy_adapt(
			signal_t& _output
			, iblock::satstate& _local_satstate
			, iblock::satstate& _satstate
			, config_s& _config
			, present_s& _present
		)
			: output_(_output)
			, local_satstate_(_local_satstate)
			, satstate_(_satstate)
			, config_(_config)
			, present_(_present) {

		}

		void do_execute(
			const signal_t& _deseired
			, iblock::satstate _master_sat
			, const range_s<signal_t>& _control_range
			, const signal_t& _actual
			, const signal_t& _diff
		) {
			//		iblock::satstate remote = controller_block_t<I, O>::master_satstate.value();
			//		present_s& present = iblock::present_cast<present_s>();
			if (_master_sat == iblock::satstate::both) {
				return;
			}

			present_.error = (long_signal_t)_deseired - _actual;

			long_signal_t tmp = present_.error + present_.model - _actual;

			present_.control = tmp * config_.propGain;

			present_.control_diff = _diff * config_.diffGain;
			present_.control += present_.control_diff;
			local_satstate_ = q::round_s(present_.control, _control_range, config_.control_shift, output_);

			if (_master_sat == iblock::satstate::none) {
				satstate_ = local_satstate_;
			}
			else {
				satstate_ = _master_sat;
			}

			long_signal_t model_inc = present_.error * config_.modelGain;

			if (model_inc > std::numeric_limits<long_signal_t>::epsilon()) {
				if (satstate_ != iblock::satstate::up) {
					long_signal_t d = q::long_max - present_.model_l;
					if (d < model_inc) {
						model_inc = d;
						present_.model_l = q::long_max;
					}
					else {
						present_.model_l += model_inc;
					}
				}
			}
			else {
				if (satstate_ != iblock::satstate::low) {
					if (model_inc < -std::numeric_limits<long_signal_t>::epsilon()) {
						long_signal_t d = q::long_min - present_.model_l;
						if (d > model_inc) {
							present_.model_l = q::long_min;
						}
						else {
							present_.model_l += model_inc;
						}
					}
				}
			}
			present_.model = q::round_s(present_.model_l, config_.model_shift);
		}

		void do_adjust(const signal_t& _def) {

			if (config_.propGain > (parameter_t)0) {
				present_.control = (long_signal_t)_def * (1 << config_.model_shift);
				output_ = _def;
				present_.model = present_.control / config_.propGain;
			}
			else {
				present_.control = (long_signal_t)0;
				output_ = (signal_t)0;
				present_.model = (long_signal_t)0;
			}
		}
	};

	template< typename q >  class  quazzy_adapt_b : public controller_block_t<typename q::signal_t, typename q::signal_t >, public quazzy_adapt<q> {
		typedef controller_block_t<typename q::signal_t, typename q::signal_t> B;
		typedef quazzy_adapt<q> QA;
	public:
		struct config_s {
			typename B::config_s cb;
			typename QA::config_s qa;
			typename q::signal_t standalone_actual;
			typename q::signal_t standalone_actual_diff;
		};
		struct present_s {
			typename B::present_s cb;
			typename QA::present_s qa;
		};

	private:

	protected:
		virtual void execute(void) {
			QA::do_execute(B::input.value(), B::master_satstate.value(), B::range.value(), actual.value(), actual_diff.value());
		}
		virtual void adjust(const typename q::signal_t& _def) {
			QA::do_adjust(_def);
		}
	public:
		iblock::input_t<typename q::signal_t> actual;
		iblock::input_t<typename q::signal_t> actual_diff;

		quazzy_adapt_b(isubsystem& _subsystem, cstr  _name, config_s& _config, present_s& _present)
			: B(_subsystem, _name, _config.cb, _present.cb)
			, QA(_present.cb.output
				 , _present.cb.local_satstate
				 , _present.cb.satstate
				 , _config.qa
				 , _present.qa
			)
			, actual(_config.standalone_actual)
			, actual_diff(_config.standalone_actual_diff) {}

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