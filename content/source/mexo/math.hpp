#ifndef mexo_math_hpp
#define mexo_math_hpp
#include "mexo/mexo.hpp" 
#include <limits>  
#include <math.h>
namespace mexo {
	enum class  dsignal_bits {
		i7 = 8
		, i13 = 13
		, i15 = 16
		, i31 = 32
		, i63 = 64
	};
	template<dsignal_bits bits, dsignal_bits long_bits> struct dsigna {
		enum {
			max = (1 << ((int)bits - 1)) - 1
			, min = (-max)
			, long_max = (1UL << ((int)long_bits - 1)) - 1
			, long_min = (-long_max)
			, ones = max
		};
	};


	/*
	struct int7 : public dsigna<signal_bits::i7> {
		typedef int8_t discret_t;
		typedef int16_t long_discret_t;
		typedef int8_t signal_t;
		typedef uint8_t usignal_t;
		typedef int8_t parameter_t;
		typedef int16_t long_signal_t;
	};


	struct int31 : public dsigna<signal_bits::i31> {
		typedef int32_t discret_t;
		typedef int64_t long_discret_t;
		typedef int32_t signal_t;
		typedef uint32_t usignal_t;
		typedef int32_t parameter_t;
		typedef int64_t long_signal_t;
	};


	struct real31 {
		typedef int32_t discret_t;
		typedef int64_t long_discret_t;
		typedef float signal_t;
		typedef float parameter_t;
		typedef double long_signal_t;
	};

	struct real63 {
		typedef double signal_t;
		typedef double parameter_t;
		typedef double long_signal_t;
	};*/


	struct int15 : public dsigna<dsignal_bits::i15, dsignal_bits::i31> {
		typedef int16_t discret_t;
		typedef int32_t long_discret_t;
		typedef discret_t signal_t;
		typedef discret_t parameter_t;
		typedef long_discret_t long_signal_t;
	};

	struct real15 {
		typedef int16_t discret_t;
		typedef int32_t long_discret_t;
		typedef float signal_t;
		typedef float parameter_t;
		typedef float long_signal_t;
		constexpr static signal_t max = std::numeric_limits<float>::max();
		constexpr static signal_t min = std::numeric_limits<float>::lowest();
		constexpr static signal_t long_max = std::numeric_limits<float>::max();
		constexpr static signal_t long_min = std::numeric_limits<float>::lowest();
	};

	template< typename digit > struct fixed_point {

		typedef typename  digit::discret_t discret_t;
		typedef typename  digit::long_discret_t long_discret_t;
		typedef typename  digit::signal_t signal_t;
		typedef typename  digit::long_signal_t long_signal_t;
		typedef typename  digit::parameter_t parameter_t;
		constexpr static signal_t max = digit::max;
		constexpr static signal_t min = digit::min;
		constexpr static signal_t long_max = digit::long_max;
		constexpr static signal_t long_min = digit::long_min;
		constexpr static signal_t ones = digit::ones;
		constexpr static signal_t pi = digit::max;
		constexpr static signal_t one_div_sqrt3 = (digit::max * robo::one_div_sqrt3<signal_t>);
		constexpr static signal_t sqrt3_div_2 = (digit::max * robo::sqrt3_div_2<signal_t>);


		template <typename T> static iblock::satstate round_s(const long_signal_t& _src, const range_s <T>& _range, unsigned int _shift, T& _output) {
			if (_range.hi == _range.low) {
				_output = _range.hi;
				return iblock::satstate::both;
			}
			long_signal_t tmp;
			if (_src == 0) {
				tmp = (long_signal_t)0;
			}
			else {
				if (_src > 0) {
					if (_shift > 0) {
						int r = 1 << (_shift - 1);
						if (digit::long_max - _src < r) {
							tmp = digit::long_max >> _shift;
						}
						else {
							tmp = (_src + r) >> _shift;
						}
					}
					else {
						tmp = _src;
					}
				}
				else {
					if (_shift > 0) {
						int r = 1 << (_shift - 1);
						if (_src - digit::long_min < r) {
							tmp = -((-digit::long_min) >> _shift);
						}
						else {
							tmp = -((r - _src) >> _shift);
						}
					}
					else {
						tmp = _src;
					}
				}
			}
			if (tmp > _range.hi) {
				_output = _range.hi;
				return iblock::satstate::up;
			}
			else if (tmp < _range.low) {
				_output = _range.low;
				return iblock::satstate::low;
			}
			else {
				_output = (T)tmp;
				return iblock::satstate::none;
			}
		}
		static long_signal_t round_s(const long_signal_t& _src, unsigned int _shift) {
			if (_src == 0) {
				return (long_signal_t)0;
			}
			else {
				if (_src > 0) {
					if (_shift > 0) {
						int r = 1 << (_shift - 1);
						if (digit::long_max - _src < r) {
							return digit::long_max >> _shift;
						}
						else {
							return (_src + r) >> _shift;
						}
					}
					else {
						return _src;
					}
				}
				else {
					if (_shift > 0) {
						int r = 1 << (_shift - 1);
						if (_src - digit::long_min < r) {
							return -((-digit::long_min) >> _shift);
						}
						else {
							return -((r - _src) >> _shift);
						}
					}
					else {
						return _src;
					}
				}
			}
		}
	};



	template< typename q > struct float_point {

		typedef typename  q::discret_t discret_t;
		typedef typename  q::long_discret_t long_discret_t;
		typedef typename  q::signal_t signal_t;
		typedef typename  q::long_signal_t long_signal_t;
		typedef typename  q::parameter_t parameter_t;

		constexpr static signal_t pi = robo::pi<signal_t>;
		constexpr static signal_t one_div_sqrt3 = robo::one_div_sqrt3<signal_t>;
		constexpr static signal_t sqrt3_div_2 = robo::sqrt3_div_2<signal_t>;
		constexpr static signal_t ones = (signal_t)1;
		constexpr static signal_t max = q::max;
		constexpr static signal_t min = q::min;
		constexpr static signal_t long_max = q::long_max;
		constexpr static signal_t long_min = q::long_min;

		template <typename T> static iblock::satstate round_s(const long_signal_t& _src, const range_s<T>& _range, unsigned int _shift, T& _output) {
			if (_range.hi == _range.low) {
				_output = _range.hi;
				return iblock::satstate::both;
			}
			long_signal_t tmp;
			if (_src == 0) {
				tmp = (T)0;
			}
			else {
				if (_src > 0) {
					if (_shift > 0) {
						T r = (T)(1L << (_shift - 1));
						T d = (T)(1L << _shift);
						if (q::long_max - _src < r) {
							tmp = q::long_max / d;
						}
						else {
							tmp = (_src + r) / d;
						}
					}
					else {
						tmp = _src;
					}
				}
				else {
					if (_shift > 0) {
						T r = (T)(1L << (_shift - 1));
						T d = (T)(1L << _shift);
						if (_src - q::long_min < r) {
							tmp = q::long_min / d;
						}
						else {
							tmp = (_src - r) / d;
						}
					}
					else {
						tmp = _src;
					}
				}
			}
			if (tmp > _range.hi) {
				_output = _range.hi;
				return iblock::satstate::up;
			}
			else if (tmp < _range.low) {
				_output = _range.low;
				return iblock::satstate::low;
			}
			else {
				_output = tmp;
				return iblock::satstate::none;
			}
		}

		static  long_signal_t round_s(const long_signal_t& _src, unsigned int _shift) {
			if (_src == 0) {
				return (long_signal_t)0;
			}
			else {
				if (_src > 0) {
					if (_shift > 0) {
						signal_t r = (signal_t)(1L << (_shift - 1));
						signal_t d = (signal_t)(1L << _shift);
						if (q::long_max - _src < r) {
							return q::long_max / d;
						}
						else {
							return (_src + r) / d;
						}
					}
					else {
						return _src;
					}
				}
				else {
					if (_shift > 0) {
						signal_t r = (signal_t)(1L << (_shift - 1));
						signal_t d = (signal_t)(1L << _shift);
						if (_src - q::long_min < r) {
							return q::long_min / d;
						}
						else {
							return (_src - r) / d;
						}
					}
					else {
						return _src;
					}
				}
			}
		}
	};


	template<typename q> class to_digit_scale_b : public controller_block_t< typename q::signal_t, typename q::discret_t > {
		typedef controller_block_t<typename q::signal_t, typename q::discret_t> B;
	public:
		typedef typename q::signal_t signal_t;
		typedef typename q::long_signal_t long_signal_t;
		typedef typename q::parameter_t parameter_t;
		typedef typename q::discret_t discret_t;
		struct config_s {
			typename B::config_s cb;
			parameter_t scale;
			unsigned shift;
		};
		struct present_s {
			typename B::present_s cb;
		};
	protected:
		virtual void execute(void) {
			present_s& present = iblock::present_cast<present_s>();
			const config_s& config = iblock::config_cast<config_s>();

			long_signal_t tmp = (long_signal_t)config.scale * B::input.value();
			present.cb.local_satstate =  q::round_s(tmp, B::range.value(), config.shift,present.cb.output);
			
			iblock::present_cast<present_s>().cb.output = q::round_s(tmp, iblock::config_cast<config_s>().shift);
			//B::saturate();
			B::update_satstate();
		}
	public:
		to_digit_scale_b(isubsystem& _subsystem, cstr  _name, config_s& _config, present_s& _present)
			: B(_subsystem, _name, _config.cb, _present.cb) {}
	};

	template<typename q> class ramp_b : public controller_block_t<typename  q::signal_t, typename  q::signal_t> {
		typedef controller_block_t<typename q::signal_t, typename  q::signal_t> B;
	public:
		typedef typename q::signal_t signal_t;
		typedef typename q::long_signal_t long_signal_t;
		typedef typename q::parameter_t parameter_t;

		struct config_s {
			typename B::config_s cb;
			signal_t rampStep;
		};
		struct present_s {
			typename B::present_s cb;
		};
	protected:

		virtual void execute(void) {

			iblock::satstate remote = B::master_satstate.value();
			present_s& present = iblock::present_cast<present_s>();
			const config_s& config = iblock::config_cast<config_s>();
			if (remote == iblock::satstate::both) {
				present.cb.satstate = iblock::satstate::both;
				return;
			}

			signal_t inp = B::input.value();
			const range_s<signal_t>& r = B::range.value();
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

			signal_t out = present.cb.output;
			signal_t delta = inp - out;

			if (delta > std::numeric_limits<signal_t>::epsilon()) {
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
				if (delta < -std::numeric_limits<signal_t>::epsilon()) {
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
			: B(_subsystem, _name, _config.cb, _present.cb) {}
	};

	template<typename q>  class  filter_b : public function_block_t<typename  q::signal_t, typename  q::signal_t> {
		typedef function_block_t<typename  q::signal_t, typename  q::signal_t> B;
	public:
		typedef typename q::signal_t signal_t;
		typedef typename q::long_signal_t long_signal_t;
		typedef typename q::parameter_t parameter_t;
		struct config_s {
			typename B::config_s fb;
			parameter_t	gain;
			unsigned shift;
		};
		struct present_s {
			typename B::present_s fb;
		};
		parameter_t gain1 = (parameter_t)0;
		parameter_t gain2 = (parameter_t)0;

	protected:
		virtual void execute(void) {
			present_s& present = iblock::present_cast<present_s>();
			long_signal_t tmp = (long_signal_t)gain1 * present.fb.output + gain2 * B::input.value();
			present.fb.output = q::round_s(tmp, iblock::config_cast<config_s>().shift);
		}

		virtual bool reconfig(void) {
			const config_s& config = iblock::config_cast<config_s>();
			gain1 = config.gain;
			gain2 = (q::ones - config.gain);
			B::reconfig();
			return true;
		};

	public:
		filter_b(isubsystem& _subsystem, cstr  _name, config_s& _config, present_s& _present)
			: B(_subsystem, _name, _config.fb, _present.fb) {}
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