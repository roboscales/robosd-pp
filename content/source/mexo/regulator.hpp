#ifndef __regulator_hpp
#define __regulator_hpp
#include "mexo/mexo.hpp"
namespace mexo {
	template< typename I, typename O, typename P, typename L = O>  class  quazzy_adapt {
	public:
		struct config_s {
			P propGain;
			P modelGain;
			P diffGain;
			uint8_t control_shift;
			uint8_t model_shift;
		};
		struct present_s {
			L control;
			L control_diff;
			L model;
			L model_l;
			L error;
		};
	private:
		O& output_;
		iblock::satstate& local_satstate_;
		iblock::satstate& satstate_;
		config_s& config_;
		present_s& present_;

	public:
		quazzy_adapt(
			O& _output
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
			const I& _deseired
			, iblock::satstate _master_sat
			, const range_s<O>& _control_range
			, const I& _actual
			, const I& _diff
		) {
			//		iblock::satstate remote = controller_block_t<I, O>::master_satstate.value();
			//		present_s& present = iblock::present_cast<present_s>();
			if (_master_sat == iblock::satstate::both) {
				return;
			}

			present_.error = (L)_deseired - _actual;

			L tmp = present_.error + present_.model - _actual;

			present_.control = tmp * config_.propGain;

			present_.control_diff = _diff * config_.diffGain;
			present_.control += present_.control_diff;

			if (present_.control >= _control_range.hi) {
				present_.control = _control_range.hi;
				local_satstate_ = iblock::satstate::up;
			}
			else if (tmp <= _control_range.low) {
				present_.control = _control_range.low;
				local_satstate_ = iblock::satstate::low;
			}
			else {
				local_satstate_ = iblock::satstate::none;
			}

			L model_inc = present_.error * config_.modelGain;
			if (model_inc > std::numeric_limits<L>::epsilon()) {
				L d = std::numeric_limits<float>::max() - present_.model_l;
				if (d < model_inc) {
					model_inc = d;
				}
			}
			else
				if (model_inc < -std::numeric_limits<L>::epsilon()) {
					L d = std::numeric_limits<float>::lowest() - present_.model_l;
					if (d > model_inc) {
						model_inc = d;
					}
				}
				else {
					model_inc = (L)0;
				}


			if (_master_sat == iblock::satstate::none) {
				satstate_ = local_satstate_;
			}
			else {
				satstate_ = _master_sat;
			}

			if (!(
				((present_.error > (L)0) && ((satstate_ == iblock::satstate::up) || (present_.model_l > integrator_limit_max<L, O>())))
				||
				((present_.error < (L)0) && (satstate_ == iblock::satstate::low) || (present_.model_l < integrator_limit_min<L, O>()))
				)) {
				present_.model_l += ;
			}
			extract<L, O>(present_.model_l, present_.model, config_.model_shift);
			extract<L, O>(present_.control, output_, config_.control_shift);
		}

		void do_adjust(const O& _def) {

			if (config_.propGain > (O)0) {
				present_.control = (L)0;
				output_ = _def;
				present_.model = present_.control / config_.propGain;
			}
			else {
				present_.control = (L)0;
				output_ = (O)0;
				present_.model = (L)0;
			}
		}
	};

	template< typename I, typename O, typename P, typename L = O>  class  quazzy_adapt_b : public controller_block_t<I, O>, public quazzy_adapt<I, O, P, L> {
		typedef controller_block_t<I, O> B;
		typedef quazzy_adapt<I, O, P, L> QA;
	public:
		struct config_s {
			typename B::config_s cb;
			typename QA::config_s qa;
			I standalone_actual;
			I standalone_actual_diff;
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
		virtual void adjust(const O& _def) {
			QA::do_adjust(_def);
		}
	public:
		iblock::input_t<I> actual;
		iblock::input_t<I> actual_diff;

		quazzy_adapt_b(isubsystem& _subsystem, cstr  _name, config_s& _config, present_s& _present)
			: controller_block_t<I, O>(_subsystem, _name, _config.cb, _present.cb)
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