#ifndef mexo_math_hpp
#define mexo_math_hpp
#include "mexo/mexo.hpp" 
#include <limits>  
#include <math.h>
namespace mexo {
	constexpr signal_t pi = robo::pi<signal_t>;
	constexpr signal_t one_div_sqrt3 = robo::one_div_sqrt3<signal_t>;
	constexpr signal_t sqrt3_div_2 = robo::sqrt3_div_2<signal_t>;


	template <typename F, typename D>  void round(const F& _f, D& _d) {
		F tmp = _f;
		if (tmp > ((F)0)) tmp += ((F)0.5);
		else
			if (tmp < ((F)0)) tmp -= ((F)0.5);
		_d = (D)tmp;
	}

	template <typename I, typename  O, typename  P>  void scale(const I& _x, const P& _scale, const O& _offset, O& _y) {
		_y = (O)(_scale * _x + _offset);
	}

	template <typename I, typename O, typename F> class scale_b : public function_block_t<I, O> {
		typedef function_block_t<I, O> B;
	public:
		struct config_s {
			typename B::config_s fb;
			F scale;
			O offset;
		};
		struct present_s {
			typename B::present_s fb;
		};

	protected:
		virtual void execute(void) {
			present_s& present = iblock::present_cast<present_s>();
			const config_s& config = iblock::config_cast<config_s>();
			scale(B::input.value(), config.scale, config.offset, present.fb.output);
		}
	public:
		scale_b(isubsystem& _subsystem, cstr  _name, config_s& _config, present_s& _present)
			: B(_subsystem, _name, _config, _present) {}
	};

	template <typename I, typename O, typename F> class control_scale_b : public controller_block_t<I, O> {
		typedef controller_block_t<I, O> B;
	public:
		struct config_s {
			typename B::config_s cb;
			F scale;
			O offset;
		};

		struct present_s {
			typename B::present_s cb;
		};
	protected:
		virtual void execute(void) {
			present_s& present = iblock::present_cast<present_s>();
			const config_s& config = iblock::config_cast<config_s>();
			scale(B::input.value(), config.scale, config.offset, present.cb.output);
			B::saturate();
			B::update_satstate();
		};
	public:
		control_scale_b(isubsystem& _subsystem, cstr  _name, config_s& _config, present_s& _present)
			: controller_block_t<I, O>(_subsystem, _name, _config.cb, _present.cb) {}
	};


	template <typename I, typename O, typename F> class to_parrot_scale_b : public controller_block_t<I, O> {
		typedef controller_block_t<I, O> B;
	public:
		struct config_s {
			typename B::config_s cb;
			F scale;
		};
		struct present_s {
			typename controller_block_t<I, O>::present_s cb;
		};
	protected:
		virtual void execute(void) {
			round(iblock::config_cast<config_s>().scale * B::input.value(), iblock::present_cast<present_s>().cb.output);
			B::saturate();
			B::update_satstate();
		}
	public:
		to_parrot_scale_b(isubsystem& _subsystem, cstr  _name, config_s& _config, present_s& _present)
			: B(_subsystem, _name, _config.cb, _present.cb) {}
	};

	template <typename I, typename O = I> class ramp_b : public controller_block_t<I, O> {
		typedef controller_block_t<I, O> B;
	public:
		struct config_s {
			typename B::config_s cb;
			O rampStep;
		};
		struct present_s {
			typename B::present_s cb;
		};
	protected:

		virtual void execute(void) {

			iblock::satstate remote = controller_block_t<I, O>::master_satstate.value();
			present_s& present = iblock::present_cast<present_s>();
			const config_s& config = iblock::config_cast<config_s>();
			if (remote == iblock::satstate::both) {
				present.cb.satstate = iblock::satstate::both;
				return;
			}

			I inp = B::input.value();
			const range_s<O>& r = B::range.value();
			iblock::satstate inp_sut;
			if (inp >= r.hi) {
				inp = r.hi;
				inp_sut = iblock::satstate::up;
			}
			else if (inp <= r.low) {
				inp = r.low;
				inp_sut = iblock::satstate::low;
			}
			else {
				inp_sut = iblock::satstate::none;
			}

			O out = present.cb.output;
			O delta = inp - out;

			if (delta > std::numeric_limits<O>::epsilon()) {
				if (remote != iblock::satstate::up) {
					if (delta < config.rampStep) {
						present.cb.output = inp;
						present.cb.local_satstate = inp_sut;
					}
					else {
						present.cb.output += config.rampStep;
						present.cb.local_satstate = iblock::satstate::none;
					}
				}
				else {
					present.cb.local_satstate = iblock::satstate::none;
				}
			}
			else {
				if (delta < -std::numeric_limits<O>::epsilon()) {
					if (remote != iblock::satstate::low) {
						if ((-delta) < config.rampStep) {
							present.cb.output = inp;
							present.cb.local_satstate = inp_sut;
						}
						else {
							present.cb.output -= config.rampStep;
							present.cb.local_satstate = iblock::satstate::none;
						}
					}
					else {
						present.cb.local_satstate = iblock::satstate::none;
					}
				}
				else {
					present.cb.local_satstate = inp_sut;
				}
			}
			B::update_satstate();
		}
	public:
		ramp_b(isubsystem& _subsystem, cstr  _name, config_s& _config, present_s& _present)
			: controller_block_t<I, O>(_subsystem, _name, _config.cb, _present.cb) {}
	};

	template< typename I, typename O, typename P>  class  filter_b : public function_block_t<I, O> {
		typedef function_block_t<I, O> B;
	public:
		struct config_s {
			typename B::config_s fb;
			P	gain;
		};
		struct present_s {
			typename B::present_s fb;
		};
		P gain1 = (P)0;
		P gain2 = (P)0;

	protected:
		virtual void execute(void) {
			present_s& present = iblock::present_cast<present_s>();
			present.fb.output = (O)(gain1 * present.fb.output + gain2 * B::input.value());
		}

		virtual bool reconfig(void) {
			const config_s& config = iblock::config_cast<config_s>();
			gain1 = config.gain;
			gain2 = (P)1 - config.gain;
			B::reconfig();
			return true;
		};

	public:
		filter_b(isubsystem& _subsystem, cstr  _name, config_s& _config, present_s& _present)
			: function_block_t<I, O>(_subsystem, _name, _config.fb, _present.fb) {}
	};

	template< typename I, typename O, typename P, typename L = O>  class  quazzy_adapt_b : public controller_block_t<I, O> {
		typedef controller_block_t<I, O> B;
	public:
		struct config_s {
			typename B::config_s cb;
			P propGain;
			P modelGain;
			P diffGain;
			O standalone_actual;
			O standalone_actual_diff;
		};
		struct present_s {
			typename B::present_s cb;
			L control;
			L control_diff;
			L model;
			L error;
		};

	private:
		void execute_(
			const I& _deseired
			, iblock::satstate _master_sat
			, const range_s<O>& _control_range
			, const I& _actual
			, const I& _diff
		) {
			iblock::satstate remote = controller_block_t<I, O>::master_satstate.value();
			present_s& present = iblock::present_cast<present_s>();
			if (remote == iblock::satstate::both) {
				present.cb.satstate = iblock::satstate::both;
			}

			const config_s& config = iblock::config_cast<config_s>();

			present.error = _deseired - _actual;
			L tmp = present.error + present.model - _actual;

			present.control = tmp * config.propGain;

			present.control_diff = _diff * config.diffGain;
			present.control += present.control_diff;

			if (present.control >= _control_range.hi) {
				present.control = _control_range.hi;
				present.cb.local_satstate = iblock::satstate::up;
			}
			else if (tmp <= _control_range.low) {
				present.control = _control_range.low;
				present.cb.local_satstate = iblock::satstate::low;
			}
			else {
				present.cb.local_satstate = iblock::satstate::none;
			}
			B::update_satstate();
			if (!(
				((present.error > (L)0) && (present.cb.satstate == iblock::satstate::up))
				||
				((present.error < (L)0) && (present.cb.satstate == iblock::satstate::low))
				)) {
				present.model += present.error * config.modelGain;
			}
			present.cb.output = (O)present.control;
		}

	protected:
		virtual void execute(void) {
			execute_(B::input.value(), B::master_satstate.value(), B::range.value(), actual.value(), actual_diff.value());
		}
		virtual void adjust(const O& _def) {
			present_s& present = iblock::present_cast<present_s>();
			const config_s& config = iblock::config_cast<config_s>();

			if (config.propGain > (O)0) {
				present.control = (L)0;
				present.cb.output = _def;
				present.model = present.control / config.propGain;
			}
			else {
				present.control = (L)0;
				present.cb.output = (O)0;
				present.model = (L)0;
			}
		}
	public:
		iblock::input_t<I> actual;
		iblock::input_t<I> actual_diff;


		quazzy_adapt_b(isubsystem& _subsystem, cstr  _name, config_s& _config, present_s& _present)
			: controller_block_t<I, O>(_subsystem, _name, _config.cb, _present.cb)
			, actual(_config.standalone_actual)
			, actual_diff(_config.standalone_actual_diff) {}

	};

	/*
	struct signal2ph_s {
		signal_t cross;
		signal_t lateral;
	};

	struct signal3ph_s {
		signal_t A;
		signal_t B;
		signal_t C;
	};


	class transform {
	private:
		signal_t angle_;
		signal_t sin_;
		signal_t cos_;
	public:
		const signal_t& angle(void) { return angle_; };
		const signal_t& sin(void) { return sin_; };
		const signal_t& cos(void) { return cos_; };
		void set_angle(const signal_t& _angle) {
			angle_ = _angle;
			sin_ = (signal_t)::sin(_angle);
			cos_ = (signal_t)::cos(_angle);
		}
		void forward(const signal2ph_s& _src, signal2ph_s& _dst);
		void backward(const signal2ph_s& _src, signal2ph_s& _dst);
	};


	signal2ph_s& operator >> (const signal3ph_s& _s3, signal2ph_s& _s2);
	signal3ph_s& operator >> (const signal2ph_s& _s2, signal3ph_s& _s3);
	*/

}
#endif