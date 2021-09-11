#ifndef mexo_math_hpp
#define mexo_math_hpp
#include "mexo/mexo.hpp" 
#include <limits>  
#include <math.h>
namespace mexo {
	constexpr signal_t pi = robo::pi<signal_t>;
	constexpr signal_t one_div_sqrt3 = robo::one_div_sqrt3<signal_t>;
	constexpr signal_t sqrt3_div_2 = robo::sqrt3_div_2<signal_t>;

	template <typename F, typename T> struct fdc {
		F scale;
		iblock::satstate dirrect(const F& _float, const range_s<T> _range, T& _digit) {
			F tmp = _float * scale;
			if (tmp > ((F)0)) tmp += ((F)0.5);
			else
				if (tmp < ((F)0)) tmp -= ((F)0.5);

			_digit = (T)tmp;

			if (_digit >= _range.hi) {
				_digit = _range.hi;
				return iblock::satstate::up;
			}
			else if (tmp <= _range.low) {
				_digit = _range.low;
				return iblock::satstate::low;
			}
			else {
				return iblock::satstate::none;
			}
		}
		void revert(const T& _digit, F& _float) {
			_float = _digit / scale;
		}
	};



	template< typename A>  class  ramp {
	public:
		typedef A input_t;
		typedef A output_t;

		struct config_s {
			iblock::config_s	block;
			range_s<output_t>	range;
			output_t			rampGain;
			output_t			def;
		};

		class math {
		public:

			output_t rampStep = (output_t)0;
			output_t output_value = (output_t)0;

			iblock::satstate  perform(const input_t& _input, iblock::satstate _master_sat, const range_s<output_t>& _range) {
				if (_master_sat == iblock::satstate::both) return iblock::satstate::both;
				input_t input = _input;
				if (input >= _range.hi) {
					input = _range.hi;
				}
				else if (input <= _range.low) {
					input = _range.low;
				}

				output_t delta = input - output_value;
				if (delta > (output_t)0) {
					if (_master_sat != iblock::satstate::up) {
						if (delta < rampStep) {
							output_value = input;
							return iblock::satstate::up;
						}
						else {
							output_value += rampStep;
						}
					}
					else {
						return iblock::satstate::up;
					}
				}
				else {
					if (delta < (output_t)0) {
						if (_master_sat != iblock::satstate::low) {
							if ((-delta) < rampStep) {
								output_value = input;
								return iblock::satstate::low;
							}
							else {
								output_value -= rampStep;
							}
						}
						else {
							return iblock::satstate::low;
						}
					}
				}
				return _master_sat;
			}
		};

	private:
		math math_;
	protected:
		iblock::satstate execute(const input_t& _input, iblock::satstate _master_sat, const range_s<output_t>& _range) {
			return math_.perform(_input, _master_sat, _range);
		}
	public:
		const output_t& output_value(void) {
			return math_.output_value;
		}

		ramp(void) {}
		bool applay(const config_s& _config) {
			ROBO_LBREAKN(_config.range.low <= _config.range.hi);
			math_.rampStep = _config.rampGain;
			math_.output_value = _config.def;
			return true;
		}

	};

	template< typename A, typename P>  class  filter {
	public:
		typedef A input_t;
		typedef A output_t;

		struct config_s {
			iblock::config_s	block;
			P					gain;
			output_t			def;
		};

		P gain1 = (P)0;
		P gain2 = (P)0;
		output_t value = (output_t)0;

	protected:
	public:
		void execute(const input_t& _input) {
			value = value * gain1 + _input * gain2;
		}

		const output_t& output_value(void) {
			return value;
		}

		filter(void) {}
		bool applay(const config_s& _config) {
			gain1 = _config.gain;
			gain2 = (P)1 - _config.gain;
			value = _config.def;
			return true;
		}

	};

	template< typename S, typename L, typename P>  class  quazzy_adapt {
	public:
		typedef S input_t;
		typedef S output_t;

		struct config_s {
			iblock::config_s	block;
			range_s<output_t>	range;
			P propGain;
			P modelGain;
			P diffGain;
			S def;
		};

		class math {
		public:
			P propGain;
			P modelGain;
			P diffGain;

			L control = (L)0;
			L controlDiff = (L)0;

			L model = (L)0;

			S error = (L)0;
			S controlValue = (S)0;

			iblock::satstate perform(
				const input_t& _deseired
				, iblock::satstate _master_sat
				, const range_s<output_t>& _control_range
				, const input_t& _actual
				, const input_t& _diff
			) {
				if (_master_sat == iblock::satstate::both) return iblock::satstate::both;

				iblock::satstate sut_flag;

				error = _deseired - _actual;
				L tmp = error + model - _actual;

				control = tmp * propGain;

				controlDiff = _diff * diffGain;
				control += controlDiff;

				if (control >= _control_range.hi) {
					control = _control_range.hi;
					sut_flag = iblock::satstate::up;
				}
				else if (tmp <= _control_range.low) {
					control = _control_range.low;
					sut_flag = iblock::satstate::low;
				}
				else {
					if (_master_sat == iblock::satstate::up) {
						sut_flag = iblock::satstate::up;
					}
					else if (_master_sat == iblock::satstate::low) {
						sut_flag = iblock::satstate::low;
					}
					else {
						sut_flag = iblock::satstate::none;
					}
				}

				if (!(
					((error > (L)0) && (sut_flag == iblock::satstate::up))
					||
					((error < (L)0) && (sut_flag == iblock::satstate::low))
					)) {
					model += error * modelGain;
				}
				controlValue = (S)control;
				return sut_flag;
			}
		};


		S standalone_actual = (S)0;
		iblock::input_t<S> actual;

		S standalone_actual_diff = (S)0;
		iblock::input_t<S> actual_diff;

	private:
		math math_;
	protected:
		iblock::satstate execute(const input_t& _input, const iblock::satstate _master_sat, const range_s<output_t>& _range) {
			return math_.perform(_input, _master_sat, _range, actual.value(), actual_diff.value());
		}
	public:
		const output_t& output_value(void) {
			return math_.controlValue;
		}

		quazzy_adapt(void)
			: actual(standalone_actual)
			, actual_diff(standalone_actual_diff) {}

		bool applay(const config_s& _config) {
			ROBO_LBREAKN(_config.range.low <= _config.def && _config.def <= _config.range.hi);

			math_.propGain = _config.propGain;
			math_.modelGain = _config.modelGain;
			math_.diffGain = _config.diffGain;
			math_.control = _config.def;

			return true;
		}
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