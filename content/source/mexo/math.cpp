#include "mexo/math.hpp"
namespace mexo {
	/*
	signal2ph_s& operator >> (const signal3ph_s& _s3, signal2ph_s& _s2) {
		_s2.cross = ( signal_t(2) * _s3.B + _s3.A) * one_div_sqrt3;
		_s2.lateral = _s3.A;
		return _s2;
	}

	signal3ph_s &  operator >> (const signal2ph_s& _s2, signal3ph_s& _s3) {
		_s3.A = _s2.lateral;
		_s3.B = _s2.cross * sqrt3_div_2 - _s2.lateral * signal_t(0.5);
		_s3.C = -_s3.A - _s3.B;
		return _s3;
	}

	void transform::forward(const signal2ph_s& _src, signal2ph_s& _dst) {
		_dst.lateral = _src.lateral * cos_ - _src.cross * sin_;
		_dst.lateral = -_src.lateral * sin_ + _src.cross * cos_;
	}
	void transform::backward(const signal2ph_s& _src, signal2ph_s& _dst) {
		_dst.lateral = _src.lateral * cos_ + _src.cross * sin_;
		_dst.lateral = -_src.lateral * sin_ + _src.cross * cos_;
	}

*/
}
