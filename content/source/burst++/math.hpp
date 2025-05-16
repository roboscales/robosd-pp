#ifndef burst_math_hpp
#define burst_math_hpp
#include "burst++/burst.hpp" 
#include "burst++/vartree.hpp"
#include <limits>  
#include <math.h>
namespace burst {

	struct int15  {
		typedef int16_t discret_t;
		typedef int32_t long_discret_t;
		
		typedef discret_t signal_t;
		typedef int64_t extended_signal_t;
		typedef discret_t parameter_t;
		typedef int32_t long_signal_t;
		typedef uint16_t usignal_t;
		typedef uint32_t ulong_signal_t;
		constexpr static signal_t max = std::numeric_limits<signal_t>::max() ;
		constexpr static signal_t min = -max;
		constexpr static signal_t ones = max;
		constexpr static long_signal_t long_max = std::numeric_limits<long_signal_t>::max();
		constexpr static long_signal_t long_min = -long_max;
		constexpr static int bits = 15;
		constexpr static int long_bits = 31;

		#if ROBO_APP_BURST_VARTREE_ENABLED == 1
		struct var{
			constexpr static ::burst::var::types discret = ::burst::var::types::int16;
			constexpr static ::burst::var::types long_discret = ::burst::var::types::int32;
			constexpr static ::burst::var::types udiscret = ::burst::var::types::uint16;
			constexpr static ::burst::var::types ulong_discret = ::burst::var::types::uint32;
			constexpr static ::burst::var::types signal = ::burst::var::types::int16;
			constexpr static ::burst::var::types usignal = ::burst::var::types::uint16;
			constexpr static ::burst::var::types parameter = ::burst::var::types::int16;
			constexpr static ::burst::var::types long_signal = ::burst::var::types::int32;
			constexpr static ::burst::var::types ulong_signal = ::burst::var::types::uint32;
			constexpr static ::burst::var::types const_discret = ::burst::var::types::const_int16;
			constexpr static ::burst::var::types const_long_discret = ::burst::var::types::const_int32;
			constexpr static ::burst::var::types const_signal = ::burst::var::types::const_int16;
			constexpr static ::burst::var::types const_parameter = ::burst::var::types::const_int16;
			constexpr static ::burst::var::types const_long_signal = ::burst::var::types::const_int32;
		};
		#endif
		static int15::signal_t constexpr s_round(double _x) {
			if (_x > 0) {
				return (int15::signal_t)(_x + 0.5);
			}
			else {
				return (int15::signal_t)(_x - 0.5);
			}
		}

		static signal_t constexpr  s_frac(double _x) {
			return s_round(_x* max);
		}

		static long_signal_t constexpr  l_frac(double _x) {
			return l_round(_x* long_max);
		}

		static constexpr long_signal_t l_round(double _x) {
			if (_x > 0) {
				return (long_signal_t)(_x + 0.5);
			}
			else {
				return (long_signal_t)(_x - 0.5);
			}
		}
		static constexpr uint32_t l_rad2ceil(signal_t _x) {
			return ((uint32_t)_x)<<16;
		}
		static constexpr uint16_t s_rad2ceil(signal_t _x) {
			return ((uint16_t)_x);
		}
		static signal_t sin(signal_t _angle);
		static signal_t cos(signal_t _angle);
		static  signal_t sqrt(ulong_signal_t _value);
		static signal_t atan2(signal_t _y,signal_t _x);
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
		#if ROBO_APP_BURST_VARTREE_ENABLED == 1
		struct var{
			constexpr static ::burst::var::types discret = ::burst::var::types::int16;
			constexpr static ::burst::var::types long_discret = ::burst::var::types::int32;
			constexpr static ::burst::var::types signal = ::burst::var::types::real;
			constexpr static ::burst::var::types parameter = ::burst::var::types::real;
			constexpr static ::burst::var::types long_signal = ::burst::var::types::real;
			constexpr static ::burst::var::types const_discret = ::burst::var::types::const_int16;
			constexpr static ::burst::var::types const_long_discret = ::burst::var::types::const_int32;
			constexpr static ::burst::var::types const_signal = ::burst::var::types::const_real;
			constexpr static ::burst::var::types const_parameter = ::burst::var::types::const_real;
			constexpr static ::burst::var::types const_long_signal = ::burst::var::types::const_real;
		};
		#endif
	};

	

	

	


	template< typename digit > struct fixed_point:public digit {
//		typedef digit digit_s;
		typedef typename  digit::discret_t discret_t;
		typedef typename  digit::long_discret_t long_discret_t;
		typedef typename  digit::signal_t signal_t;
		typedef typename  digit::usignal_t usignal_t;
		typedef typename  digit::ulong_signal_t ulong_signal_t;
		typedef typename  digit::long_signal_t long_signal_t;
		typedef typename  digit::parameter_t parameter_t;
		typedef typename digit::extended_signal_t extended_signal_t;

		#if ROBO_APP_BURST_VARTREE_ENABLED == 1
		typedef typename  digit::var var;
		#endif
		constexpr static signal_t max = digit::max;
		constexpr static signal_t min = digit::min;
		constexpr static long_signal_t long_max = digit::long_max;
		constexpr static long_signal_t long_min = digit::long_min;
		constexpr static signal_t ones = digit::ones;
		constexpr static signal_t pi = digit::max;
		
		static constexpr typename digit::signal_t s_round(double _x) {
			return digit::s_round(_x);
		}
		
		static constexpr typename digit::long_signal_t l_round(double _x) {
			return digit::l_round(_x);
		}


		static  signal_t s_sqrt(ulong_signal_t _value) {
			return digit::sqrt(_value);
		}

		static constexpr signal_t s_sat(long_signal_t _x) {
			if ( _x > digit::max) {
				return digit::max;
			}
			else if (_x < digit::min) {
				return digit::min;
			}
			else {
				return _x;
			}
		}

		static constexpr signal_t s_inc(signal_t val, signal_t x, signal_t _min, signal_t _max) {
			long_signal_t val32 = val;
			val32 += x;
			if (val32 > _max) {
				val32 = _max;
			}
			else {
				if (val32 < _min) {
					val32 = _min;
				}
			}
			return (signal_t)val32;
		}

		static constexpr typename digit::signal_t s_frac(double _x) {
			return s_round(_x* digit::max);
		}

		static constexpr typename digit::long_signal_t l_frac(double _x) {
			return l_round(_x* digit::long_max);
		}
		static constexpr uint32_t l_rad2ceil(signal_t _x) {
			return digit::l_rad2ceil(_x);
		}
		static constexpr uint16_t s_rad2ceil(signal_t _x) {
			return digit::s_rad2ceil(_x);
		}

		/*/constexpr static signal_t one_div_2 = digit::round(0.5 * max);
		constexpr static signal_t one_div_3 = digit::round( (1.0/3.0) * max);
		constexpr static signal_t two_div_3 = digit::round((2.0 / 3.0) * max);
		constexpr static signal_t one_div_sqrt3 = digit::round( max * robo::one_div_sqrt3<double> );
		constexpr static signal_t one_div_sqrt2 = digit::round(max * robo::one_div_sqrt2<double>);
		constexpr static signal_t sqrt3_div_2 = digit::round(max * robo::sqrt3_div_2<double>);
		constexpr static signal_t sqrt2_div_2 = digit::round(max * robo::sqrt2_div_2<double>);
		*/
		template <typename T> static satstates s_round(const long_signal_t& _src, const range_s <T>& _range, unsigned int _shift, T& _output) {
			if (_range.hi == _range.low) {
				_output = _range.hi;
				return satstates::both;
			}
			long_signal_t tmp = robo::digit::round(_src,_shift);
			/*
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
			}*/
			if (tmp > _range.hi) {
				_output = _range.hi;
				return satstates::up;
			}
			else if (tmp < _range.low) {
				_output = _range.low;
				return satstates::low;
			}
			else {
				_output = (T)tmp;
				return satstates::none;
			}
		}
	
		
		
		template <typename T> static T t_round (const long_signal_t& _src, unsigned int _shift) {
			if (_src == 0) {
				return (T)0;
			}
			else {
				long_signal_t tmp = round_l(_src,_shift);
				if (tmp  > std::numeric_limits<T>::max()) {
					return std::numeric_limits<T>::max();
				} else if (tmp  < -std::numeric_limits<T>::max()) {
					return -std::numeric_limits<T>::max();
				} else {
					return (T)tmp;
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
		
		struct discret2signal {
			
			const struct config_s {
				struct{
					range_s <signal_t> signal;
					range_s <discret_t> raw;
				} range;
			} & config;
			
			#define DISCRET2SIGNAL_CONFIG(a) DISCRET2SIGNAL_CONFIG_(a)
			#define DISCRET2SIGNAL_CONFIG_(a)\
			{\
				{\
					BURST_RANGE_CONFIG(a##_SIGNAL)\
					,BURST_RANGE_CONFIG(a##_RAW)\
				}\
			}
			
			long_discret_t gain = 0;
				
			//typedef fixed_point<digit> types;
			void reconfig(void) {
				gain = (long_discret_t)(config.range.signal.hi - config.range.signal.lo);
				gain <<= (1+ digit::bits);
				gain += ((config.range.raw.hi - config.range.raw.lo) / 2); //округление
				gain /= (config.range.raw.hi - config.range.raw.lo);
			}
			discret2signal(const config_s & _config): config(_config){}
			signal_t run( const discret_t & _raw ) {
				if (_raw > config.range.raw.hi) return config.range.signal.hi;
				if (_raw < config.range.raw.lo) return config.range.signal.lo;
				long_discret_t tmp =  gain * (_raw - config.range.raw.lo);
				tmp += (1 << digit::bits);
				tmp >>= (1 + digit::bits);
				return (signal_t) ( config.range.signal.lo +  tmp );
			}
		};
		union long_signal_u {
			long_signal_t value;
			struct {
				uint16_t first;
				signal_t second;
			};
		};

		static signal_t s_extract(long_signal_t _t) {
			return ((long_signal_u*)(&_t))->second;
		}

		static constexpr signal_t s_extract(long_signal_u& v) {
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

		static signal_t s_dot(signal_t _x1, signal_t _y1, signal_t _x2, signal_t _y2) {
			long_signal_u tmp;
			tmp.value = (long_signal_t)(_x1) *_y1 + (long_signal_t)(_x2) *_y2 ;
			return s_extract(tmp);
		}

		static constexpr signal_t   s_add(signal_t _x0, signal_t _x2, signal_t _y2) {
			long_signal_u tmp;
			tmp.value = (long_signal_t)(_x0) + (long_signal_t)(_x2)*_y2;
			return s_extract(tmp);
		}
		static  signal_t constexpr s_fma(signal_t _a, signal_t _b, signal_t _c)
		{
			long_signal_u tmp;
			tmp.value = (long_signal_t)(_a)*_b;
			return _c+s_extract(tmp);
			
		}		
		template<typename T>	 static  long_signal_t  constexpr l_fma(signal_t _a, signal_t _b, long_signal_t _c)
		{
			return _a*_b + _c;			
		}	
		static signal_t sin(signal_t _angle) {
			return digit::sin(_angle);
		}
		static signal_t atan2(signal_t _y,signal_t _x) {
			return digit::atan2(_y,_x);
		}
		static signal_t cos(signal_t _angle) {
			return digit::cos(_angle);
		}
		static long_signal_t  l_add(long_signal_t _x0, long_signal_t _x2, signal_t _y2) {
			long_signal_t tmp = 32768L * _x0 + _x2 * _y2;
			return  robo::digit::round(tmp);
		}
		struct qa{
			parameter_t gain;
			parameter_t qgain;
			uint8_t shift;
			uint8_t qshift;
			long_signal_t offset;
			long_signal_t min;
			long_signal_t max;
			signal_t approxx(signal_t value){
				long_signal_t tmp = value;
				long_signal_t qa = value;
				tmp *= gain;
				tmp -= offset;
				qa = qa*value;
				qa = qa*qgain;
				qa = round_l(qa,qshift);
				tmp+=qa;
				tmp = round_l(tmp,shift);
				tmp = robo::saturate<long_signal_t>(tmp,long_min,long_max);				
				return (signal_t)tmp;
			}
			qa(
				parameter_t _gain
				,parameter_t _qgain
				, uint8_t _shift
				, uint8_t _qshift
				, long_signal_t _offset
				, long_signal_t _min
				,long_signal_t _max
			)
			: gain(_gain)
			, qgain(_qgain)
			, shift(_shift)
			, qshift(_qshift)
			, offset(_offset)
			, min(_min)
			, max(_max)
			{
			}

		};
		
	
	};



	template< typename q > struct float_point {

		typedef typename  q::discret_t discret_t;
		typedef typename  q::long_discret_t long_discret_t;
		typedef typename  q::signal_t signal_t;
		typedef typename  q::long_signal_t long_signal_t;
		typedef typename  q::parameter_t parameter_t;

		constexpr static signal_t pi = robo::pi<signal_t>;
//		constexpr static signal_t one_div_sqrt3 = robo::one_div_sqrt3<typename signal_t>;
//		constexpr static signal_t sqrt3_div_2 = robo::sqrt3_div_2<signal_t>;
		constexpr static signal_t ones = (signal_t)1;
		constexpr static signal_t max = q::max;
		constexpr static signal_t min = q::min;
		constexpr static signal_t long_max = q::long_max;
		constexpr static signal_t long_min = q::long_min;

		template <typename T> static satstates round_s(const long_signal_t& _src, const range_s<T>& _range, unsigned int _shift, T& _output) {
			if (_range.hi == _range.low) {
				_output = _range.hi;
				return satstates::both;
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
				return satstates::up;
			}
			else if (tmp < _range.low) {
				_output = _range.low;
				return satstates::low;
			}
			else {
				_output = tmp;
				return satstates::none;
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

	#if 0
	template<typename q> class to_digit_scale
		: public controller_handler< typename q::signal_t, typename q::discret_t > {
		typedef controller_handler<typename q::signal_t, typename q::discret_t> A;
	public:
		typedef typename q::signal_t signal_t;
		typedef typename q::long_signal_t long_signal_t;
		typedef typename q::parameter_t parameter_t;
		typedef typename q::discret_t discret_t;
		typedef q types;
		struct config_s {
			typename A::config_s cb;
			parameter_t scale;
			uint8_t shift;
		};
		typedef typename A::present_s present_s;
	protected:
		virtual void execute(void) {
			const config_s& config = handler::config<to_digit_scale>();
			present_s& present = handler::present<to_digit_scale>();
			long_signal_t tmp = (long_signal_t)config.scale * *A::deseired;
			present.satstate.local = q::round_s(tmp, A::range, config.shift, *A::output);
			A::update_satstate();
		}
		
		#if ROBO_APP_MEXO_VAR_ENABLED == 1
		virtual void do_handler_create_vars(var::record::list & _vars, int _master_key) {
			A::do_handler_create_vars(_vars,_master_key);
			const config_s& config = handler::config<to_digit_scale>();
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
			const config_s& conf = A::template config<ramp>();
			if (var::machine::actual_mode() >= var::machine::mode::tuning) {
				var::record::create(q::var::signal, conf.rampStep, RT("st"), _master_key, _vars);
				var::record::create(var::uint8, conf.shift, RT("sh"), _master_key, _vars);
			}
		};
		#endif
		virtual void execute(void) {
			satstate_t remote = A::master_satstate;
			const config_s& config =handler::config<ramp>();
			present_s& present = handler::present<ramp>();
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

		void do_handler_adjust(void) {
			//const config_s& config = config_cast<config_s>();
			present_s& present = handler::present<ramp>();
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
			const config_s& cnfg = A::template config<fast_filter>();
			present_s& psnt = A::template present<fast_filter>();
			if (var::machine::actual_mode() >= var::machine::mode::config) {
				var::record::create(var::uint8, cnfg.shift, RT("sh"), _master_key, _vars);
				var::record::create(q::var::parameter, gain, RT("g"), _master_key, _vars);
			}
			if (var::machine::actual_mode() >= var::machine::mode::full) {
				var::record::create(q::var::signal, psnt.fb.output, RT("o"), _master_key, _vars);
			}
		};
		#endif


		void execute(void) {
			const config_s& config = handler::config<fast_filter>();
			present_s& present = handler::present<fast_filter>();
			long_signal_t tmp = (long_signal_t)present.fb.output * gain + A::input;
			present.fb.output = (signal_t)(((int)tmp) >> config.shift);
		}
		bool do_handler_reconfig(void) {
			const config_s& config = handler::config<fast_filter>();
			gain = (1 << config.shift) - 1;
			return true;
		}
		void do_handler_adjust(void) {
			handler::present<fast_filter>().fb.output = 0; 
		}

	public:
		fast_filter(const config_s& _config
					, present_s& _present
					, signal_t& _input
		)
			: A(_config.fb, _present.fb, _input) {}
	};


	template<typename q, bool autoreset = false>  class  filter
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
			present_s& prsnt = B::template present<filter>();
			const config_s& conf = B::template config<filter>();
			if (var::machine::actual_mode() >= var::machine::mode::config) {
				var::record::create(q::var::parameter, conf.gain, RT("g"), _master_key, _vars);
				var::record::create(var::uint8, conf.shift.gain, RT("sh.g"), _master_key, _vars);
				var::record::create(var::uint8, conf.shift.gain, RT("sh.presc"), _master_key, _vars);
				var::record::create(var::uint8, conf.shift.gain, RT("sh.val"), _master_key, _vars);
			}
			if (var::machine::actual_mode() >= var::machine::mode::full) {
				var::record::create(q::var::signal, prsnt.fb.output, RT("o"), _master_key, _vars);
			}
		}
		#endif

		void execute(void) {
			present_s& present = handler::present<filter>();
			const config_s& conf = handler::config<filter>();
			long_signal_t tmp = present.filtered * gain1 + B::input * gain2;
			present.filtered = q::round_l(tmp, conf.shift.gain);
			present.fb.output = q:: template round_t<signal_t>(present.filtered, conf.shift.value+ conf.shift.gain);
			if (autoreset) {
				B::input = (signal_t)0;
			}
		}

		bool do_handler_reconfig(void) {
			const config_s& conf = handler::config<filter>();
			parameter_t ones = (parameter_t)(1 << conf.shift.gain);
			if (conf.gain > ones) {
				gain1 = ones;
			}
			else {
				gain1 = conf.gain;
			}
			gain2 = (ones - conf.gain) * (1 << conf.shift.presc)*(1<< conf.shift.gain);
			return true;
		};
		
		void do_handler_adjust(void) {
			//const config_s& config = template config_cast<config_s>();
			present_s& present = handler::present<filter>();
			present.fb.output = 0;
			present.filtered = 0;
		}
	public:
		filter(const config_s& _config
			   , present_s& _present
			   , signal_t& _input
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
			B = q::dot(_ab.alfa, -one_div_2,_ab.beta, sqrt3_div_2);
			C = -A - B;
		}

	};
	
	#endif


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
	/*
	template  <number, number r> constexpr typename number::signal_t operator"" rad() noexcept {
		return round(r * digit::max);
	}
	
	constexpr signal_t operator""_fraq(double _fraq) noexcept {
		return round(_fraq * digit::max);
	}
	*/
	

	template <typename T> void range_set(range_s<T>& _dst, const T & __src, const range_s<T> &_lim) {

		auto _src = __src < T(0) ? T(0) : __src;
		if (_src > _lim.hi) {
			_dst.hi = _lim.hi;
		}
		else if (_src < _lim.lo) {
			_dst.hi = _lim.lo;
		}
		else {
			_dst.hi = _src;
		}
		_src = -_src;
		if (_src > _lim.hi) {
			_dst.lo = _lim.hi;
		}
		else if (_src < _lim.lo) {
			_dst.lo = _lim.lo;
		}
		else {
			_dst.lo = _src;
		}
	}

	template <typename S, typename T> T  range_apply( const S & _src, const range_s<T> & _lim) {
		if (_src > _lim.hi) {
			return _lim.hi;
		}
		else if (_src < _lim.lo) {
			return _lim.lo;
		}
		else {
			return (T)_src;
		}
	}
	/*
static long_signal_t l_round(const long_signal_t& _src, unsigned int _shift) {
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
		}	*/
}

#endif