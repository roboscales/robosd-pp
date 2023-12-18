#ifndef mexo_math_hpp
#define mexo_math_hpp
#include <mexo++\common.hpp>
#include <stdint.h>
#include <limits>  
namespace mexo{

	template< typename T> static T rshift(T _x, uint8_t _dg) {
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
				_x = _x >> _dg ;
			}
		}
		return _x;
	}
	
	template <typename T> T saturate(T _x, T  _lo, T _hi) {
		if (_x < _lo) {
			_x = _lo;
		}
		else if (_x > _hi) {
			_x = _hi;
		}
		return _x;
	}
	
	template <typename I>  static I prec_round(const I& _src, unsigned int _shift) {
		if (_src == 0) {
			return (I)0;
		}
		else {
			if (_src > 0) {
				if (_shift > 0) {
					int r = (1 << (_shift - 1));
					if (std::numeric_limits<I>::max() - _src < r) {
						return std::numeric_limits<I>::max() >> _shift;
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
					int r = (1 << (_shift - 1));
					if (_src - std::numeric_limits<I>::min() < r) {
						return std::numeric_limits<I>::min() >> _shift;
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

	template <typename I, typename T> static T round_to(const I& _src, unsigned int _shift) {
		if (_src == 0) {
			return (T)0;
		}
		else {
			I tmp = prec_round<I>(_src,_shift);
			
			if (tmp  > std::numeric_limits<T>::max()) {
				return std::numeric_limits<T>::max();
			} else if (tmp  < std::numeric_limits<T>::min()) {
				return std::numeric_limits<T>::min();
			} else {
				return (T)tmp;
			}
		}
	}
	
	template <typename I, typename T> static satstate_t satround_to(const I& _src, const range_s <T>& _range, unsigned int _shift, T& _output) {
		if (_range.hi == _range.lo) {
			_output = _range.hi;
			return satstate_t::both;
		} else {
			I tmp = prec_round(_src,_shift);
			if (tmp > _range.hi) {
				_output = _range.hi;
				return satstate_t::hi;
			}
			else if (tmp < _range.lo) {
				_output = _range.lo;
				return satstate_t::lo;
			}
			else {
				_output = (T)tmp;
				return satstate_t::none;
			}			
		}
	}
	
	template< typename digit > struct math {

		typedef typename  digit::discret_t discret_t;
		typedef typename  digit::precise_discret_t precise_discret_t;
		typedef typename  digit::number_t number_t;
		typedef typename  digit::precise_t precise_t;
		typedef typename  digit::parameter_t parameter_t;
		typedef  typename  digit::precise_u precise_u;

		#if ROBO_APP_MEXO_VAR_ENABLED == 1
		typedef typename  digit::var var;
		#endif
		constexpr static number_t max = digit::max;
		constexpr static number_t min = digit::min;
		constexpr static precise_t precise_max = digit::precise_max;
		constexpr static precise_t precise_min = digit::precise_min;
		constexpr static number_t ones = digit::ones;
		constexpr static number_t pi = digit::max;
	

		struct scaler {
			range_s <number_t> _range;
			number_t signal_lo = 0;
			discret_t discret_lo = 0;
			number_t signal_hi = 0;
			discret_t discret_hi = 0;
			precise_t gain = 0;
			typedef math<digit> types;
			void reconfig(number_t _signal_lo, number_t _signal_hi, discret_t _discret_lo, discret_t _discret_hi) {
				signal_hi = _signal_hi;
				signal_lo = _signal_lo;
				discret_hi = _discret_hi;
				discret_lo = _discret_lo;
				gain = (precise_t)(discret_hi - discret_lo);
				gain <<= (1+ digit::bits);
				gain += (signal_hi - signal_lo) / 2; //округление
				gain /= (signal_hi - signal_lo);
			}
			void run(number_t _signal, discret_t & _discret) {
				if (_signal > signal_hi) _signal = signal_hi;
				if (_signal < signal_lo) _signal = signal_lo;
				precise_t tmp =  gain * (_signal - signal_lo);
				tmp += (1 << digit::bits);
				tmp >>= (1 + digit::bits);
				_discret = (discret_t) ( discret_lo +  tmp );
			}
		};

		static number_t extract(precise_u& v) {
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
							return (((v.second) << 1) - 1);
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


		static number_t mult(number_t x1, number_t x2) {
			precise_u tmp;
			tmp.value = (precise_t)x1 * x2;
			return  extract(tmp);
		}

		static number_t add(precise_u& acc, number_t x1, number_t x2) {
			acc.value += (precise_t)x1 * x2;
			return  extract(acc);
		}
		static number_t add(number_t _x0, number_t _x2, number_t _y2) {
			precise_u tmp;
			tmp.value = (precise_t)(_x0) + (precise_t)(_x2)*_y2;
			return extract(tmp);
		}
		
		static precise_t  prec_add(precise_t _x0, precise_t _x2, number_t _y2) {
			precise_t tmp = 32768L * _x0 + _x2 * _y2;
			return rshift<precise_t>(tmp);
		}

		static number_t dot(number_t _x1, number_t _y1, number_t _x2, number_t _y2) {
			precise_u tmp;
			tmp.value = (precise_t)(_x1) *_y1 + (precise_t)(_x2) *_y2 ;
			return s_extract(tmp);
		}


		static number_t sin(number_t _angle) {
			return digit::sin(_angle);
		}

		static number_t cos(number_t _angle) {
			return digit::cos(_angle);
		}
	
		struct unit_test{
			template<typename T> void static round_test (void){
				T res = std::numeric_limits<int8_t>::min() ;
				range_s <T> range = {std::numeric_limits<int8_t>::min(),std::numeric_limits<int8_t>::max()};
				precise_t in = (precise_t)256*std::numeric_limits<int8_t>::min() - 128;
				ROBO_APP_ASSERT( satround_to( in, range, 8, res)==satstate_t::lo )
				ROBO_APP_ASSERT( res == std::numeric_limits<int8_t>::min())
				
				in = (precise_t)256*std::numeric_limits<int8_t>::min() - 127;
				ROBO_APP_ASSERT( satround_to( in, range, 8, res)==satstate_t::none )
				ROBO_APP_ASSERT( res == std::numeric_limits<int8_t>::min())

				in = (precise_t)256 * std::numeric_limits<int8_t>::max()+128;
				ROBO_APP_ASSERT( satround_to( in, range, 8, res)==satstate_t::hi )
				ROBO_APP_ASSERT( res == std::numeric_limits<int8_t>::max())

				in = (precise_t)256 * std::numeric_limits<int8_t>::max()+127;
				ROBO_APP_ASSERT( satround_to( in, range, 8, res)==satstate_t::none )
				ROBO_APP_ASSERT( res == std::numeric_limits<int8_t>::max())


				
				in = (precise_t)127;
				ROBO_APP_ASSERT( satround_to( in, range, 8, res)==satstate_t::none )
				ROBO_APP_ASSERT( res == 0)

				in = (precise_t)128;
				ROBO_APP_ASSERT( satround_to( in, range, 8, res)==satstate_t::none )
				ROBO_APP_ASSERT( res == 1)

				in = (precise_t)-127;
				ROBO_APP_ASSERT( satround_to( in, range, 8, res)==satstate_t::none )
				ROBO_APP_ASSERT( res == 0)

				in = (precise_t)-128;
				ROBO_APP_ASSERT( satround_to( in, range, 8, res)==satstate_t::none )
				ROBO_APP_ASSERT( res == -1)
				ROBO_APP_ASSERT( (round_to<int32_t,int16_t>(32767*65536, 16) == 32767) ) 
				ROBO_APP_ASSERT( (round_to<int32_t,int16_t>(-32768*65536, 16) == -32768) ) 
			}
			volatile static void run(void){
				round_test<int8_t>();			
				round_test<int16_t>();			
			}
		};
	};

	struct digit15{
		
		typedef int16_t discret_t;		
		typedef int32_t precise_discret_t;

		typedef discret_t number_t;
		typedef precise_discret_t precise_t;
		typedef int16_t parameter_t;
		typedef  union  {
			precise_t value;
			struct {
				uint16_t first;
				number_t second;
			};
		} precise_u;
		constexpr static number_t max = std::numeric_limits<number_t>::max() ;
		constexpr static number_t min = -max;
		constexpr static number_t ones = max;
		constexpr static precise_t precise_max = std::numeric_limits<precise_t>::max();
		constexpr static precise_t precise_min = -precise_max;
		constexpr static int bits = 15;
		constexpr static int precise_bits = 31;
		
		static constexpr number_t round(double _x) {
			if (_x > 0) {
				return (number_t)(_x + 0.5);
			}
			else {
				return (number_t)(_x - 0.5);
			}
		}
		
		static constexpr number_t frac(double _x) {			
				return round(_x * max);
		}
		
		static constexpr number_t prec_round(double _x) {
			if (_x > 0) {
				return (precise_t)(_x + 0.5);
			}
			else {
				return (precise_t)(_x - 0.5);
			}
		}
		
		static constexpr number_t prec_frac(double _x) {			
				return prec_round(_x * precise_max);
		}
		
		static number_t sin(number_t _angle);
		static number_t cos(number_t _angle);
	};
			

	
	template <typename D, typename S > D pack(const S & _x, uint8_t _shift) {
		if (_x == S(0)) {
			return (D)0;
		}
		else {
			S tmp=_x;
			if (_shift > 0) {
				int  r = (1 << (_shift - 1)) - 1;
				if (tmp > S(0)) {
					if ((int)(std::numeric_limits<S>::max() - _x) < r) {
						tmp = std::numeric_limits<S>::max() >> _shift;
					}
					else {
						tmp = (tmp + r) >> _shift;
					}
				}
				else {
					if ((int)(tmp + std::numeric_limits<S>::max()) < r) {
						tmp = -(std::numeric_limits<S>::max() >> _shift);
					}
					else {
						tmp = - ((r - tmp) >> _shift);
					}
				}
			}
			if (std::numeric_limits<D>::digits < std::numeric_limits<S>::digits) {
				if (tmp < -std::numeric_limits<D>::max()) {
					return -std::numeric_limits<D>::max();
				}
				else if (_x > std::numeric_limits<D>::max()) {
					return std::numeric_limits<D>::max();
				}
			}
			return D(tmp);
		}
	}
}

#endif