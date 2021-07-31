#ifndef mexo_math_hpp
#define mexo_math_hpp
#include "mexo/mexo.hpp" 
#include <limits>  

namespace mexo {
	constexpr signal_t pi = robo::pi<signal_t>;
	constexpr signal_t one_div_sqrt3 = robo::one_div_sqrt3<signal_t>;
	constexpr signal_t sqrt3_div_2 = robo::sqrt3_div_2<signal_t>;
	
		

	template <typename A> struct range_s {
		A lo;
		A hi;
	};

	template< typename A>  class  ramp {
	public:
		typedef typename A deseired_t;
		typedef typename A actual_t;

		struct config_s {
			iblock::config_s	block;
			actual_t			rampGain;
			range_s<actual_t>	range;
			actual_t			default;
		};

		class math {
		public:
			actual_t rampStep = 0;
			actual_t actual = (actual_t)0;

			void perform( const deseired_t& _deseired, const range_s<actual_t>& _range ) {
				deseired_t deseired = _deseired;
				if (deseired >= _range.hi) {
					deseired = _range.hi;
				}
				else if (deseired <= _range.lo) {
					deseired = _range.lo;
				}

				signal_t delta = deseired - actual;
				if (delta > 0) {
					if (delta < rampStep) {
						actual = deseired;
					}
					else {
						actual += rampStep;
					}
				}
				else {
					if (delta < 0) {
						if ((-delta) < rampStep) {
							actual = deseired;
						}
						else {
							actual -= rampStep;
						}
					}
				}
			}
		};

		range_s<actual_t> standalone_range;
		iblock::input_t<range_s<actual_t>> range;
	private:
		math math_;
	protected:
		iblock::satstate execute(const deseired_t & _deseired ) {
			const range_s<actual_t> & r = range.value() ;
			math_.perform(_deseired, r );
			if (math_.actual >= r.hi) {
				return  iblock::satstate::up;
			}
			else if (math_.actual <= r.lo) {
				return  iblock::satstate::low;
			}
			else {
				return  iblock::satstate::none;
			}

		}
	public:
		const actual_t& actual(void) {
			return math_.actual;
		}

		ramp(void)
			: range(standalone_range)
		{
			standalone_range = {};			
		}

		bool applay(const config_s& _config) {
			math_.rampStep = _config.rampGain;
			if ((_config.default > _config.range.lo) && (_config.default < _config.range.hi)) {
				standalone_range = _config.range;
				math_.actual = _config.default;
				return true;
			}
			else {
				return false;
			}
		}

	};


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
		void forward (const signal2ph_s& _src, signal2ph_s& _dst);
		void backward(const signal2ph_s& _src, signal2ph_s& _dst);
	};


	signal2ph_s & operator >> (const signal3ph_s& _s3, signal2ph_s& _s2);
	signal3ph_s & operator >> (const signal2ph_s& _s2, signal3ph_s& _s3);

}
#endif