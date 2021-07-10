#ifndef mexo_ramp_hpp
#define mexo_ramp_hpp
#include "mexo/mexo.hpp" 
namespace mexo {
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
	private:
		actual_t rampStep_ = 0;
		range_s<actual_t> standalone_range_;
		actual_t actual_ = (actual_t)0;
	public:

		iblock::input_t<range_s<actual_t>> range;

	protected:
		iblock::satstate execute(deseired_t _deseired) {
			const range_s<actual_t>& r = range.value();

			if (_deseired >= r.hi) {
				_deseired = r.hi;
			}
			else if (_deseired <= r.lo) {
				_deseired = r.lo;
			}


			signal_t delta = _deseired - actual_;
			if (delta > 0) {
				if (delta < rampStep_) {
					actual_ = _deseired;
				}
				else {
					actual_ += rampStep_;
				}
			}
			else {
				if (delta < 0) {
					if ((-delta) < rampStep_) {
						actual_ = _deseired;
					}
					else {
						actual_ -= rampStep_;
					}
				}
			}

			if (_deseired >= r.hi) {
				return  iblock::satstate::up;
			}
			else if (_deseired <= r.lo) {
				return  iblock::satstate::low;
			}
			else {
				return  iblock::satstate::none;
			}

		}
	public:
		actual_t& actual(void) {
			return actual_;
		}

		ramp(const actual_t & _standalone_desirted)
			: range(standalone_range_)
		{
			ROBO_UNUSED(_standalone_desirted);
			standalone_range_ = {};			
		}

		virtual bool applay(const config_s& _config) {
			rampStep_ = _config.rampGain;
			if ((_config.default > _config.range.lo) && (_config.default < _config.range.hi)) {
				standalone_range_ = _config.range;
				actual_ = _config.default;
				return true;
			}
			else {
				return false;
			}
		}


	};
}
#endif