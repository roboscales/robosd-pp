#ifndef mexo_math_hpp
#define mexo_math_hpp
#include "mexo/mexo.hpp" 
#include <limits>  
#include <math.h>
namespace mexo {

	struct int15  {
		typedef int16_t discret_t;
		typedef int32_t long_discret_t;
		typedef discret_t signal_t;
		typedef discret_t parameter_t;
		typedef int32_t long_signal_t;
		constexpr static signal_t max = std::numeric_limits<signal_t>::max() ;
		constexpr static signal_t min = -max;
		constexpr static signal_t ones = max;
		constexpr static long_signal_t long_max = std::numeric_limits<long_signal_t>::max();
		constexpr static long_signal_t long_min = -long_max;
		constexpr static int bits = 15;
		constexpr static int long_bits = 31;
		constexpr static ::mexo::var::types discret = ::mexo::var::types::int16;

		#if ROBO_APP_MEXO_VAR_ENABLED == 1
		struct var{
			constexpr static ::mexo::var::types discret = ::mexo::var::types::int16;
			constexpr static ::mexo::var::types long_discret = ::mexo::var::types::int32;
			constexpr static ::mexo::var::types signal = ::mexo::var::types::int16;
			constexpr static ::mexo::var::types parameter = ::mexo::var::types::int16;
			constexpr static ::mexo::var::types long_signal = ::mexo::var::types::int32;
			constexpr static ::mexo::var::types const_discret = ::mexo::var::types::const_int16;
			constexpr static ::mexo::var::types const_long_discret = ::mexo::var::types::const_int32;
			constexpr static ::mexo::var::types const_signal = ::mexo::var::types::const_int16;
			constexpr static ::mexo::var::types const_parameter = ::mexo::var::types::const_int16;
			constexpr static ::mexo::var::types const_long_signal = ::mexo::var::types::const_int32;
		};
		#endif
		static constexpr int15::signal_t round(double _x) {
			if (_x > 0) {
				return (int15::signal_t)(_x + 0.5);
			}
			else {
				return (int15::signal_t)(_x - 0.5);
			}
		}
		static signal_t sin(signal_t _angle);
		static signal_t cos(signal_t _angle);
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
		constexpr static int discret_bits = 15;
		#if ROBO_APP_MEXO_VAR_ENABLED == 1
		struct var{
			constexpr static ::mexo::var::types discret = ::mexo::var::types::int16;
			constexpr static ::mexo::var::types long_discret = ::mexo::var::types::int32;
			constexpr static ::mexo::var::types signal = ::mexo::var::types::real;
			constexpr static ::mexo::var::types parameter = ::mexo::var::types::real;
			constexpr static ::mexo::var::types long_signal = ::mexo::var::types::real;
			constexpr static ::mexo::var::types const_discret = ::mexo::var::types::const_int16;
			constexpr static ::mexo::var::types const_long_discret = ::mexo::var::types::const_int32;
			constexpr static ::mexo::var::types const_signal = ::mexo::var::types::const_real;
			constexpr static ::mexo::var::types const_parameter = ::mexo::var::types::const_real;
			constexpr static ::mexo::var::types const_long_signal = ::mexo::var::types::const_real;
		};
		#endif
	};

	template <typename T> T saturate(T _x, T  _lo, T _hi) {
		if (_x < _lo) {
			_x = _lo;
		}
		else if (_x > _hi) {
			_x = _hi;
		}
		return _x;
	}

	template< typename digit > struct fixed_point {

		typedef typename  digit::discret_t discret_t;
		typedef typename  digit::long_discret_t long_discret_t;
		typedef typename  digit::signal_t signal_t;
		typedef typename  digit::long_signal_t long_signal_t;
		typedef typename  digit::parameter_t parameter_t;

		#if ROBO_APP_MEXO_VAR_ENABLED == 1
		typedef typename  digit::var var;
		#endif
		constexpr static signal_t max = digit::max;
		constexpr static signal_t min = digit::min;
		constexpr static long_signal_t long_max = digit::long_max;
		constexpr static long_signal_t long_min = digit::long_min;
		constexpr static signal_t ones = digit::ones;
		constexpr static signal_t pi = digit::max;
		static constexpr int15::signal_t round(double _x) {
			return digit::round(_x);
		}
		/*/constexpr static signal_t one_div_2 = digit::round(0.5 * max);
		constexpr static signal_t one_div_3 = digit::round( (1.0/3.0) * max);
		constexpr static signal_t two_div_3 = digit::round((2.0 / 3.0) * max);
		constexpr static signal_t one_div_sqrt3 = digit::round( max * robo::one_div_sqrt3<double> );
		constexpr static signal_t one_div_sqrt2 = digit::round(max * robo::one_div_sqrt2<double>);
		constexpr static signal_t sqrt3_div_2 = digit::round(max * robo::sqrt3_div_2<double>);
		constexpr static signal_t sqrt2_div_2 = digit::round(max * robo::sqrt2_div_2<double>);
		*/
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
		static long_signal_t round_l(const long_signal_t& _src, unsigned int _shift) {
			if (_src == 0) {
				return (long_signal_t)0;
			}
			else {
				if (_src > 0) {
					if (_shift > 0) {
						int r = (1 << (_shift - 1))-1;
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
						int r = (1 << (_shift - 1))-1;
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
		


		struct scaler {
			range_s <signal_t> _range;
			signal_t signal_lo = 0;
			discret_t discret_lo = 0;
			signal_t signal_hi = 0;
			discret_t discret_hi = 0;
			long_signal_t gain = 0;
			typedef fixed_point<digit> types;
			void reconfig(signal_t _signal_lo, signal_t _signal_hi, discret_t _discret_lo, discret_t _discret_hi) {
				signal_hi = _signal_hi;
				signal_lo = _signal_lo;
				discret_hi = _discret_hi;
				discret_lo = _discret_lo;
				gain = (long_signal_t)(discret_hi - discret_lo);
				gain <<= (1+ digit::bits);
				gain += (signal_hi - signal_lo) / 2; //округление
				gain /= (signal_hi - signal_lo);
			}
			void run(signal_t _signal, discret_t & _discret) {
				if (_signal > signal_hi) _signal = signal_hi;
				if (_signal < signal_lo) _signal = signal_lo;
				long_signal_t tmp =  gain * (_signal - signal_lo);
				tmp += (1 << digit::bits);
				tmp >>= (1 + digit::bits);
				_discret = (discret_t) ( discret_lo +  tmp );
			}
		};

		union long_signal_u {
			long_signal_t value;
			struct {
				uint16_t first;
				signal_t second;
			};
		};

		static signal_t scale_l(long_signal_t _t) {
			return ((long_signal_u*)(&_t))->second;
		}

		static signal_t s_extract(long_signal_u& v) {
			if (v.value == 0) {
				return 0;
			}
			else {
				if (v.value > 0) {
					if (v.second < max / 2) {
						if (v.first > max) {
							return (v.second << 1) + 1;
						}
						else {
							return (v.second << 1);
						}
					}
					else {
						return max;
					}
				}
				else {
					if (v.second > min / 2) {
						if (v.first < min) {
							return -(((-v.second) << 1) + 1);
						}
						else {
							return v.second << 1;
						}
					}
					else {
						return min;
					}
				}
			}
		}


		static signal_t s_mult(signal_t x1, signal_t x2) {
			long_signal_u tmp;
			tmp.value = (long_signal_t)x1 * x2;
			return  s_extract(tmp);
		}

		static signal_t s_add(long_signal_u& acc, signal_t x1, signal_t x2) {
			acc.value += (long_signal_t)x1 * x2;
			return  s_extract(acc);
		}

		static signal_t dot(signal_t _x1, signal_t _y1, signal_t _x2, signal_t _y2) {
			long_signal_u tmp;
			tmp.value = (long_signal_t)(_x1) *_y1 + (long_signal_t)(_x2) *_y2 ;
			return s_extract(tmp);
		}

		static signal_t s_add(signal_t _x0, signal_t _x2, signal_t _y2) {
			long_signal_u tmp;
			tmp.value = (long_signal_t)(_x0) + (long_signal_t)(_x2)*_y2;
			return s_extract(tmp);
		}
		template< typename T> static T s_rshift(T _x, uint8_t _dg) {
			if (_x  == T(0) || (_dg==0)) {
				return _x;
			}
			else {
				if (_x > 0) {
					_x += (1 << (_dg-1) );
					_x >>= _dg;
				}
				else {
					_x -= (1 << (_dg-1));
					// Две инверсии. Выяснить надоли так делать!
					_x = -( (-_x) >> _dg );
				}
			}
			return _x;
		}

		static signal_t sin(signal_t _angle) {
			return digit::sin(_angle);
		}

		static signal_t cos(signal_t _angle) {
			return digit::cos(_angle);
		}
		static long_signal_t  l_add(long_signal_t _x0, long_signal_t _x2, signal_t _y2) {
			long_signal_t tmp = 32768L * _x0 + _x2 * _y2;
			return s_rshift<long_signal_t>(tmp)
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
		typedef typename q types;
		struct config_s {
			typename A::config_s cb;
			parameter_t scale;
			uint8_t shift;
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
		#if ROBO_APP_MEXO_VAR_ENABLED == 1
		virtual void do_handler_create_vars(var::record::list & _vars, int _master_key) {
			A::do_handler_create_vars(_vars,_master_key);
			const config_s& config = handler::config_cast<config_s>();
			if (var::machine::actual_mode() >= var::machine::mode::tuning) {
				var::record::create(q::var::parameter, config.scale, RT("g"), _master_key, _vars );
				var::record::create(var::uint8,  config.shift , RT("sh"), _master_key, _vars);
			}
		};
		#endif
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
			uint8_t shift;
		};
		typedef typename A::present_s present_s;
	protected:
		#if ROBO_APP_MEXO_VAR_ENABLED == 1
		virtual void do_handler_create_vars(var::record::list& _vars, int _master_key) {
			A::do_handler_create_vars(_vars, _master_key);
			const config_s& config = handler::config_cast<config_s>();
			if (var::machine::actual_mode() >= var::machine::mode::tuning) {
				var::record::create(q::var::signal, config.rampStep, RT("st"), _master_key, _vars);
				var::record::create(var::uint8, config.shift, RT("sh"), _master_key, _vars);
			}
		};
		#endif
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

		#if ROBO_APP_MEXO_VAR_ENABLED == 1
		virtual void do_handler_create_vars(var::record::list& _vars, int _master_key) {
			A::do_handler_create_vars(_vars, _master_key);
			const config_s& config = handler::config_cast<config_s>();
			if (var::machine::actual_mode() >= var::machine::mode::config) {
				var::record::create(var::uint8, config.shift, RT("sh"), _master_key, _vars);
			}
		};
		#endif


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
				uint8_t gain;
				uint8_t presc;
				uint8_t value;
			} shift;
		};
		struct present_s {
			typename B::present_s fb;
			long_signal_t filtered;
		};

		parameter_t gain1 = (parameter_t)0;
		parameter_t gain2 = (parameter_t)0;
	protected:
		#if ROBO_APP_MEXO_VAR_ENABLED == 1
		virtual void do_handler_create_vars(var::record::list& _vars, int _master_key) {
			B::do_handler_create_vars(_vars, _master_key);
			const config_s& config = handler::config_cast<config_s>();
			if (var::machine::actual_mode() >= var::machine::mode::config) {
				var::record::create(q::var::parameter, config.gain, RT("g"), _master_key, _vars);
				var::record::create(var::uint8, config.shift.gain, RT("sh.g"), _master_key, _vars);
				var::record::create(var::uint8, config.shift.gain, RT("sh.presc"), _master_key, _vars);
				var::record::create(var::uint8, config.shift.gain, RT("sh.val"), _master_key, _vars);
			}
		}
		#endif

		void execute(void) {
			present_s& present = handler::present_cast<present_s>();
			const config_s& config = handler::config_cast<config_s>();
			long_signal_t tmp = present.filtered * gain1 + B::input * gain2;
			present.filtered = q::round_l(tmp, config.shift.gain);
			present.fb.output = (signal_t)q::round_l(present.filtered, config.shift.value+ config.shift.gain);
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
			gain2 = (ones - config.gain) * (1 << config.shift.presc)*(1<< config.shift.gain);
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

	template<typename q> struct cs_t {
		typedef typename  q::signal_t signal_t;
		signal_t si;
		signal_t co;
		signal_t angle;
		cs_t(void) { si = co = (signal_t)0; }
		cs_t(signal_t _value) { co = si = _value; }
		void rotate(signal_t _angle) {
			angle = _angle;
			si = q::sin(angle);
			co = q::cos(angle);
		}
	};
	template<typename q> struct ab_t;
	template<typename q> struct abc_t;
	template<typename q> struct dq_t {
		typedef typename  q::signal_t signal_t;
		typedef typename  q::long_signal_t long_signal_t;
		signal_t lateral;
		signal_t cross;
		dq_t(void) {
			cross = lateral = (signal_t)0;
		}
		dq_t(signal_t _value) { lateral = cross = _value; }
	};

	template<typename q> struct ab_t {
		typedef typename  q::signal_t signal_t;
		signal_t alfa;
		signal_t beta;
		ab_t(void) { alfa = beta = (signal_t)0; }
		ab_t(signal_t _value) { alfa = beta = _value; }

		void scale_inverce(const dq_t<q>& _dq, const cs_t<q> & _cs) {
			constexpr static signal_t sqrt2_div_2 = q::round(robo::csqrt<double>(2.0) / 2 * q::max);
			signal_t lateral = q::s_mult(_dq.lateral, sqrt2_div_2);
			signal_t cross = q::s_mult(_dq.cross, sqrt2_div_2);

			alfa = q::dot(_cs.co, lateral, -_cs.si, cross);
			beta = q::dot(_cs.si, lateral, _cs.co, cross);
		}

		void inverce(const dq_t<q>& _dq, const cs_t<q>& _cs) {
			
			alfa = q::dot(_cs.co, _dq.lateral, -_cs.si, _dq.cross);
			beta = q::dot(_cs.si, _dq.lateral, _cs.co, _dq.cross);
		}

		void transform(dq_t<q>& _dq, const cs_t<q>& _cs) {
			constexpr static signal_t one_div_sqrt3 = q::round( robo::csqrt<double>(2.0/3.0)/2 * q::max);
			_dq.lateral = q::dot(_cs.co, alfa, _cs.si, beta);
			_dq.cross = q::dot(-_cs.si, alfa, _cs.co, beta);
		}
	};

	template<typename q> struct abc_t {
		typedef typename  q::signal_t signal_t;
		typedef typename  q::long_signal_t long_signal_t;
		signal_t A;
		signal_t B;
		signal_t C;
		abc_t(void) { A = B = C = (signal_t)0; }
		abc_t(signal_t _value) { A = B = C = _value; }
		void transform(ab_t<q>& _ab) {
			constexpr static signal_t one_div_sqrt3 = q::round( 1.0 / robo::csqrt<double>(3.0) * q::max);
			_ab.alfa = A;
			typename q::long_signal_u tmp;
			tmp.value = (long_signal_t(2) * B + A) * one_div_sqrt3;
			_ab.beta = q::s_extract(tmp);
		}
		void inverce(const ab_t<q>& _ab) {
			constexpr static signal_t one_div_2 = q::round(0.5 * q::max);
			constexpr static signal_t sqrt3_div_2 = q::round(robo::csqrt<double>(3.0) / 2 * q::max);
			A = _ab.alfa;
			B = q::dot(_ab.alfa, -one_div_2,_ab.beta, sqrt3_div_2)
			C = -A - B;
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