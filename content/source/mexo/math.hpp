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
			, long_max = (1 << ((int)long_bits - 1)) - 1
			, long_min = -long_max
			, ones = max
		};
	};

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
		constexpr static long_signal_t long_max = std::numeric_limits<float>::max();
		constexpr static long_signal_t long_min = std::numeric_limits<float>::lowest();
	};


	template< typename digit > struct fixed_point {

		typedef typename  digit::discret_t discret_t;
		typedef typename  digit::long_discret_t long_discret_t;
		typedef typename  digit::signal_t signal_t;
		typedef typename  digit::long_signal_t long_signal_t;
		typedef typename  digit::parameter_t parameter_t;
		constexpr static signal_t max = digit::max;
		constexpr static signal_t min = digit::min;
		constexpr static long_signal_t long_max = digit::long_max;
		constexpr static long_signal_t long_min = digit::long_min;
		constexpr static signal_t ones = digit::ones;
		constexpr static signal_t pi = digit::max;
		constexpr static signal_t one_div_sqrt3 = (digit::max * robo::one_div_sqrt3<signal_t>);
		constexpr static signal_t sqrt3_div_2 = (digit::max * robo::sqrt3_div_2<signal_t>);

		template <typename T> static satstate_t round_s(const long_signal_t& _src, const range_s <T>& _range, unsigned int _shift, T& _output) {
			if (_range.hi == _range.low) {
				_output = _range.hi;
				return satstate_t::both;
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
				return satstate_t::up;
			}
			else if (tmp < _range.low) {
				_output = _range.low;
				return satstate_t::low;
			}
			else {
				_output = (T)tmp;
				return satstate_t::none;
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

		template <typename T> static satstate_t round_s(const long_signal_t& _src, const range_s<T>& _range, unsigned int _shift, T& _output) {
			if (_range.hi == _range.low) {
				_output = _range.hi;
				return satstate_t::both;
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
				return satstate_t::up;
			}
			else if (tmp < _range.low) {
				_output = _range.low;
				return satstate_t::low;
			}
			else {
				_output = tmp;
				return satstate_t::none;
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


	template<typename q> class to_digit_scale
		: public controller_handler< typename q::signal_t, typename q::discret_t > {
		typedef controller_handler<typename q::signal_t, typename q::discret_t> A;
	public:
		typedef typename q::signal_t signal_t;
		typedef typename q::long_signal_t long_signal_t;
		typedef typename q::parameter_t parameter_t;
		typedef typename q::discret_t discret_t;
		struct config_s {
			typename A::config_s cb;
			parameter_t scale;
			unsigned shift;
		};
		typedef typename A::present_s present_s;
	protected:
		virtual void execute(void) {
			const config_s& config = handler::config_cast<config_s>();
			present_s& present = handler::present_cast<present_s>();
			long_signal_t tmp = (long_signal_t)config.scale * *A::deseired;
			present.satstate.local = q::round_s(tmp, A::range, config.shift, *A::output);
			A::update_satstate();
		}
		virtual void do_handler_adjust(void) {
			//
		}
	public:
		to_digit_scale(const config_s& _config
					   , present_s& _present
					   , const range_s<signal_t>& _range
					   , const satstate_t& _master_satstate
		)
			: A(_config.cb, _present, _range, _master_satstate) {}
	};

	template<typename q> class ramp
		: public controller_handler< typename q::signal_t, typename q::signal_t > {
		typedef controller_handler<typename q::signal_t, typename  q::signal_t> A;
	public:
		typedef typename q::signal_t signal_t;
		typedef typename q::long_signal_t long_signal_t;
		typedef typename q::parameter_t parameter_t;

		struct config_s {
			typename A::config_s cb;
			signal_t rampStep;
			unsigned shift;
		};
		typedef typename A::present_s present_s;
	protected:

		virtual void execute(void) {
			satstate_t remote = A::master_satstate;
			const config_s& config = handler::config_cast<config_s>();
			present_s& present = handler::present_cast<present_s>();
			if (remote == satstate_t::both) {
				present.satstate.actual = satstate_t::both;
				return;
			}


			satstate_t inp_sut;
			signal_t input = *A::deseired;
			signal_t out = *A::output;
			if (input >= A::range.hi) {
				input = A::range.hi;
				inp_sut = satstate_t::up;
			}
			else if (input <= A::range.low) {
				input = A::range.low;
				inp_sut = satstate_t::low;
			}
			else {
				inp_sut = satstate_t::none;
			}

			long_signal_t delta = input - out;

			if (delta > std::numeric_limits<signal_t>::epsilon()) {
				if (remote != satstate_t::up) {
					if (delta < config.rampStep) {
						*A::output = input;
						present.satstate.local = inp_sut;
					}
					else {
						*A::output = out + config.rampStep;
						present.satstate.local = satstate_t::none;
					}
				}
				else {
					present.satstate.local = satstate_t::none;
				}
			}
			else {
				if (delta < -std::numeric_limits<signal_t>::epsilon()) {
					if (remote != satstate_t::low) {
						if ((-delta) < config.rampStep) {
							*A::output = input;
							present.satstate.local = inp_sut;
						}
						else {
							*A::output = out - config.rampStep;
							present.satstate.local = satstate_t::none;
						}
					}
					else {
						present.satstate.local = satstate_t::none;
					}
				}
				else {
					present.satstate.local = inp_sut;
				}
			}
			A::update_satstate();
		}

		virtual void do_handler_adjust(void) {
			//const config_s& config = handler::config_cast<config_s>();
			present_s& present = handler::present_cast<present_s>();
			*A::output = 0;
			present.satstate.actual = satstate_t::both;
		}
	public:
		ramp(const config_s& _config
			 , present_s& _present
			 , const range_s<signal_t>& _range
			 , const satstate_t& _master_satstate
		)
			: A(_config.cb, _present, _range, _master_satstate) {}
	};

	template<typename q> class fast_filter
		: public function_handler< typename q::signal_t, typename q::signal_t > {
		typedef function_handler<typename q::signal_t, typename  q::signal_t> A;
	public:
		typedef typename q::signal_t signal_t;
		typedef typename q::long_signal_t long_signal_t;
		typedef typename q::parameter_t parameter_t;

		struct config_s {
			typename A::config_s fb;
			unsigned shift;
		};
		struct present_s {
			typename A::present_s fb;
		};
		parameter_t  gain;
	protected:

		void execute(void) {
			const config_s& config = handler::config_cast<config_s>();
			present_s& present = handler::present_cast<present_s>();
			long_signal_t tmp = (long_signal_t)present.fb.output * gain + A::input;
			present.fb.output = (signal_t)(((int)tmp) >> config.shift);
		}
		bool do_handler_reconfig(void) {
			const config_s& config = handler::config_cast<config_s>();
			gain = (1 << config.shift) - 1;
			return true;
		}
		virtual void do_handler_adjust(void) { handler::present_cast<present_s>().fb.output = 0; }

	public:
		fast_filter(const config_s& _config
					, present_s& _present
					, const signal_t& _input
		)
			: A(_config.fb, _present.fb, _input) {}
	};


	template<typename q>  class  filter
		: public function_handler<typename  q::signal_t, typename  q::signal_t> {
		typedef function_handler<typename  q::signal_t, typename  q::signal_t> B;
	public:
		typedef typename q::signal_t signal_t;
		typedef typename q::long_signal_t long_signal_t;
		typedef typename q::parameter_t parameter_t;
		struct config_s {
			typename B::config_s fb;
			parameter_t	gain;
			struct {
				unsigned gain;
				unsigned presc;
				unsigned value;
			} shift;
		};
		struct present_s {
			typename B::present_s fb;
			long_signal_t filtered;
		};

		parameter_t gain1 = (parameter_t)0;
		parameter_t gain2 = (parameter_t)0;
	protected:
		void execute(void) {
			present_s& present = handler::present_cast<present_s>();
			const config_s& config = handler::config_cast<config_s>();
			long_signal_t tmp = present.filtered * gain1 + B::input * gain2;
			present.filtered = q::round_s(tmp, config.shift.gain);
			present.fb.output = (signal_t)q::round_s(present.filtered, config.shift.value);
		}

		bool do_handler_reconfig(void) {
			const config_s& config = handler::config_cast<config_s>();
			parameter_t ones = (parameter_t)(1 << config.shift.gain);
			if (config.gain > ones) {
				gain1 = ones;
			}
			else {
				gain1 = config.gain;
			}
			gain2 = (ones - config.gain) * (1 << config.shift.presc);
			return true;
		};
		virtual void do_handler_adjust(void) {
			//const config_s& config = config_cast<config_s>();
			present_s& present = handler::present_cast<present_s>();
			present.fb.output = 0;
			present.filtered = 0;
		}
	public:
		filter(const config_s& _config
			   , present_s& _present
			   , const signal_t& _input
		)
			: B(_config.fb, _present.fb, _input) {}
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