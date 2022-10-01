#include "burst/burst_sin.h"

burst_signal_t s_extract(burst_long_signal_up v){
	if(v->value==0)
		return 0;
	else if(v->value>0){
		if(v->second<BURST_SIGNAL_MAX/2){
			if( v->first>BURST_USIGNAL_HALF_POS  ){
				return (v->second<<1)+1;
			}else{
				return (v->second<<1);
			}
		}else{
			return BURST_SIGNAL_MAX;
		}
	}else 
		if(v->second>-BURST_SIGNAL_MAX/2){
			if( v->first<BURST_USIGNAL_HALF_NEG  ){
				return -(((-v->second)<<1)+1);
			}else{
				return v->second<<1;
			}
		}else{
			return -BURST_SIGNAL_MAX;
		}
}

burst_signal_t s_mult(burst_signal_t x1 , burst_signal_t x2){
	burst_long_signal_ut tmp;
	tmp.value = (burst_long_signal_t)x1 * x2;
	return  s_extract(&tmp);
}

burst_signal_t s_add(burst_long_signal_up acc, burst_signal_t x1 , burst_signal_t x2){
	acc->value += (burst_long_signal_t)x1 * x2;
	return  s_extract(acc);
}


signal2ph_p abc2ab(signal3ph_p  s3 , signal2ph_p s2){
	burst_long_signal_ut tmp;
	tmp.value = (2L*s3->B + s3->A) * ONE_DIV_SQRT3;
	s2->cross = s_extract(&tmp);
	s2->lateral = s3->A;
	return s2;
}

signal3ph_p ab2abc(signal2ph_p s2, signal3ph_p s3){
	burst_long_signal_ut tmp;
	s3->A = s2->lateral;

	tmp.value = (burst_long_signal_t)(s2->cross)*SQRT3_DIV_2 - (burst_long_signal_t)s2->lateral*BURST_SIGNAL_T(0.5);
	s3->B =  s_extract(&tmp);
	s3->C = -s3->A - s3->B;

	return s3;
}

signal2ph_p rotate(signal2ph_p s2, rot_p rot){
	burst_long_signal_ut lateral;
	burst_long_signal_ut cross;
	lateral.value = (((burst_long_signal_t)(s2->lateral)*rot->cs + (burst_long_signal_t)(s2->cross)*rot->sn));
	cross.value = (((burst_long_signal_t)(-s2->lateral)*rot->sn + (burst_long_signal_t)(s2->cross)*rot->cs));
	s2->lateral = s_extract(&lateral);
	s2->cross = s_extract(&cross);		
	return s2;
}

signal2ph_p transform(signal2ph_p src, signal2ph_p dst, rot_p rot){
	burst_long_signal_ut lateral;
	burst_long_signal_ut cross;
	lateral.value = (burst_long_signal_t)(src->lateral)*rot->cs + (burst_long_signal_t)(src->cross)*rot->sn;
	cross.value = (burst_long_signal_t)(-src->lateral)*rot->sn + (burst_long_signal_t)(src->cross)*rot->cs;
	dst->lateral = s_extract(&lateral);
	dst->cross = s_extract(&cross);		
	return dst;
}
signal2ph_p transform2(burst_signal_t _lat, burst_signal_t _cross, signal2ph_p dst, rot_p rot){
 	burst_long_signal_ut lateral;
	burst_long_signal_ut cross;
	lateral.value = (burst_long_signal_t)(_lat)*rot->cs + (burst_long_signal_t)(_cross)*rot->sn;
	cross.value = (burst_long_signal_t)(-_lat)*rot->sn + (burst_long_signal_t)(_cross)*rot->cs;
	dst->lateral = s_extract(&lateral);
	dst->cross = s_extract(&cross);		
	return dst;
}

rot_p rotcalc( rot_p rot, burst_signal_t phi ){
	rot->sn = burst_sin(phi);
	rot->cs = burst_cos(phi);
	return rot;
}


burst_signal_t s_inc(burst_signal_t val, burst_signal_t x, burst_signal_t _min,burst_signal_t _max){
    burst_long_signal_t val32 = val;
    val32+= x;
    if(val32>_max){        
        val32 = _max;
    }else{
        if(val32<_min){
            val32 = _min;
        }
    }
    return (burst_signal_t) val32;
}

burst_signal_t s_scale_16(burst_signal_t _val, scale_gain_16_t _gain){
	burst_long_signal_ut tmp;
	if (_val > 0){
		tmp.value = (burst_long_signal_t)_val * (_gain.value + 1) * 2048;
		return  s_extract(&tmp);
	}
	else{
		if(_val<0){
			tmp.value = (burst_long_signal_t)(-_val) * (_gain.value +1) * 2048;
				return  -s_extract(&tmp);			
		} else {
			return 0;
		}
	}
}
burst_signal_t s_scale_256(burst_signal_t _val, scale_gain_256_t _gain){
	burst_long_signal_ut tmp;
	if (_val > 0){
		tmp.value = (burst_long_signal_t)_val * (_gain + 1) * 128;
		return  s_extract(&tmp);
	}
	else {
		if(_val<0){
			tmp.value = (burst_long_signal_t)(-_val) * (_gain + 1) * 128;
			return  -s_extract(&tmp);
		}else{
			return 0;
		}
	}
}

void burst_scaler_begin(burst_scaler_p _scaler,burst_range_p _in_range,burst_range_p _out_range ){
	_scaler->in_range.lo = _in_range->lo;
	_scaler->in_range.hi = _in_range->hi;
	_scaler->out_range.lo = _out_range->lo;
	_scaler->out_range.hi = _out_range->hi;
	burst_long_signal_t tmp = (burst_long_signal_t)_scaler->in_range.hi - _scaler->in_range.lo;
	if(tmp==0){
		_scaler->gain = 0;
	} else {
		_scaler->gain = (burst_long_signal_t)(_scaler->out_range.hi - _scaler->out_range.lo);
		_scaler->gain <<= (1+ 15);
		_scaler->gain += tmp / 2; //округление
		_scaler->gain /= tmp;
	}
}

burst_satstate_t burst_scaler_run(burst_scaler_p _scaler, burst_signal_t _in, burst_signal_t * _out){
	burst_satstate_t st;
	if(_scaler->gain == 0){
		return burst_satstate_both;
	}
	if (_in > _scaler->in_range.hi) {
		_in = _scaler->in_range.hi;
		st = burst_satstate_hi;
	} else 	if (_in < _scaler->in_range.lo){
		_in = _scaler->in_range.lo;
		st = burst_satstate_lo;
	} else{
		st = burst_satstate_none;
	}
	
	burst_long_signal_t tmp =  _scaler->gain * (_in - _scaler->in_range.lo);
	tmp += (1 << 15);
	tmp >>= (1 + 15);
	*_out = (burst_signal_t) ( tmp + _scaler->out_range.lo );
	return st;
}

