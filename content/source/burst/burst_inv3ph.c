#include "burst/burst_inv3ph.h"





//static const burst_signal_t one_div_2 = BURST_SIGNAL_T(0.5);
static const burst_signal_t sqrt3_div_2 = BURST_SIGNAL_T(0.8660254037844386); //sqrt2(3)/2
static const burst_signal_t scale = BURST_SIGNAL_T(0.414213562373095); //sqrt2(2) -1
static const burst_signal_t sqrt2_div_2 = BURST_SIGNAL_T(0.7071067811865475); //sqrt2(2)/2

BURST_STATIC_INLINE burst_long_signal_t  mult_(burst_long_signal_t x1 , burst_long_signal_t x2){
	return  (x1 * x2) >> 15;
}

BURST_STATIC_INLINE burst_long_signal_t dot_(burst_long_signal_t _x1, burst_long_signal_t _y1, burst_long_signal_t _x2, burst_long_signal_t _y2) {
	return ( _x1 *_y1 + _x2 *_y2 ) >> 15;
}

BURST_STATIC_INLINE burst_long_signal_t sum_x_ya_(burst_long_signal_t x, burst_long_signal_t y, burst_long_signal_t a) {
	burst_long_signal_t tmp = ((burst_long_signal_t)y) * a;
	tmp =  tmp>>15;
	tmp += x;
	return tmp;
}


void inv3ph_begin(inv3ph_p _inverter, inv3ph_config_p _config) {
	_inverter->discret_hi = _config->native.hi;
	_inverter->discret_lo = _config->native.lo;
	_inverter->pwm_force = _config->pwm_force;
	burst_long_signal_t delta = _config->native.hi - _config->native.lo;
	burst_long_signal_t gain = (burst_long_signal_t)(_config->native.hi - _config->native.lo );
	gain <<= 16;
	gain += ((burst_long_signal_t)BURST_SIGNAL_MAX - BURST_SIGNAL_MIN) / 2; //округление
	gain /= ((burst_long_signal_t)BURST_SIGNAL_MAX - BURST_SIGNAL_MIN);
	_inverter->scale_gain = gain;
	_inverter->discret_delta_lo = -_config->native.lo;
	_inverter->discret_delta_hi = delta;
}

burst_signal_t inv3ph_scale_(inv3ph_p _inverter,burst_signal_t _signal) {
	burst_long_signal_t tmp =  _inverter->scale_gain * ((burst_long_signal_t)_signal - BURST_SIGNAL_MIN);
	tmp += (1 << 15);
	tmp >>= 16;	
	
	if(tmp<_inverter->discret_delta_lo){
		 return _inverter->discret_lo;
	} else if(tmp>_inverter->discret_delta_hi){
		return _inverter->discret_hi;
	} else{
		return  _inverter->discret_lo +  tmp;
	}
}

void inv3ph_run(inv3ph_p _inverter, burst_signal_t _cross, burst_signal_t _lateral, burst_signal_t _angle){
	rotcalc( &(_inverter->rot), _angle );
	_inverter->angle = _angle;
	_inverter->dq.cross = _cross;
	_inverter->dq.lateral = _lateral;
	burst_long_signal_t lateral = mult_(_inverter->dq.lateral, sqrt2_div_2);
	burst_long_signal_t cross = mult_(_inverter->dq.cross, sqrt2_div_2);
	_inverter->ab.alfa = dot_(_inverter->rot.cs, lateral, -_inverter->rot.sn, cross);
	_inverter->ab.beta = dot_(_inverter->rot.sn, lateral, _inverter->rot.cs, cross);

	burst_long_signal_t pwmA;
	burst_long_signal_t pwmB;
	burst_long_signal_t pwmC;
	
	_inverter->swm = 1;
	
	burst_long_signal_t x, y, z;
	burst_long_signal_t v2 = _inverter->ab.beta >> 1;
	x = _inverter->ab.beta;
	y = sum_x_ya_(v2,_inverter->ab.alfa, sqrt3_div_2);
	z = sum_x_ya_(v2,_inverter->ab.alfa, -sqrt3_div_2);
	if (y < 0) {
		if (z < 0) {
			pwmA = y - z;
			pwmB = pwmA + 2 * z;
			pwmC = pwmA - 2 * y;
			_inverter->swm = 5;
		}
		else {
			if (x > 0) {
				pwmA = -x + y;
				pwmC = pwmA - 2 * y;
				pwmB = pwmC + 2 * x;
				_inverter->swm = 3;
			}
			else {
				pwmA = x - z;
				pwmB = pwmA + 2 * z;
				pwmC = pwmB - 2 * x;
				_inverter->swm = 4;
			}
		}
	}
	else {
		if (z < 0) {
			if (x > 0) {
				pwmA = x - z;
				pwmB = pwmA + 2 * z;
				pwmC = pwmB - 2 * x;
				_inverter->swm = 1;
			}
			else {
				pwmA = -x + y;
				pwmC = pwmA - 2 * y;
				pwmB = pwmC + 2 * x;
				_inverter->swm = 6;
			}
		}
		else {
			pwmA = y - z;
			pwmB = pwmA + 2 * z;
			pwmC = pwmA - 2 * y;
			_inverter->swm = 2;
		}
	}

	pwmA += mult_(pwmA, scale);
	pwmB += mult_(pwmB, scale);
	pwmC += mult_(pwmC, scale);
	
	burst_long_signal_t pwm_force = _inverter->pwm_force;
	if( pwm_force > 0){
		burst_long_signal_t lo = BURST_SIGNAL_MIN + pwm_force;
		burst_long_signal_t hi = BURST_SIGNAL_MAX - pwm_force;
		
		BURST_SATURATE(pwmA,lo,hi);
		BURST_SATURATE(pwmB,lo,hi);
		BURST_SATURATE(pwmC,lo,hi);
		
		_inverter->pwm.A = (burst_signal_t)pwmA;
		_inverter->pwm.B = (burst_signal_t)pwmB;
		_inverter->pwm.C = (burst_signal_t)pwmC;

		if(pwmA>0){
			pwmA+=pwm_force;
		}
		if(pwmB>0){
			pwmB+=pwm_force;
		}
		if(pwmC>0){
			pwmC+=pwm_force;
		}
		
		if(pwmA<0){
			pwmA -= pwm_force;
		}
		if(pwmB<0){
			pwmB -= pwm_force;
		}
		if(pwmC<0){
			pwmC -= pwm_force;
		}		
	} else{
		BURST_SATURATE(pwmA,BURST_SIGNAL_MIN,BURST_SIGNAL_MAX);
		BURST_SATURATE(pwmB,BURST_SIGNAL_MIN,BURST_SIGNAL_MAX);
		BURST_SATURATE(pwmC,BURST_SIGNAL_MIN,BURST_SIGNAL_MAX);
		
		_inverter->pwm.A = (burst_signal_t)pwmA;
		_inverter->pwm.B = (burst_signal_t)pwmB;
		_inverter->pwm.C = (burst_signal_t)pwmC;
	}
	
	_inverter->duty.A = inv3ph_scale_(_inverter,pwmA);
	_inverter->duty.B = inv3ph_scale_(_inverter,pwmB);
	_inverter->duty.C = inv3ph_scale_(_inverter,pwmC);
}

void current3ph_begin(current3ph_p _sensor, current3ph_config_p _config, inv3ph_p _inverter, burst_signal_t * _raw){
	_sensor->inverter = _inverter;
	_sensor->raw.A = _raw+_config->adc_index[0];
	_sensor->raw.B = _raw+_config->adc_index[1];
	_sensor->raw.C = _raw+_config->adc_index[2];
	if(_config->deform.enable){
		_sensor->deform = _config->deform.matrix;
	} else {
		_sensor->deform = 0;
	}
}
void current3ph_run(current3ph_p _sensor){
	burst_long_signal_t * R  = _sensor->deform;
	burst_long_signal_t a;
	burst_long_signal_t b;
	burst_long_signal_t c;
	if(R){	
		burst_signal_t A = *_sensor->raw.A;
		burst_signal_t B = *_sensor->raw.B;
		burst_signal_t C = *_sensor->raw.C;
		a = (burst_signal_t)(( R[0]*A + R[1]*B + R[2]*C  )>>15);
		b =  (burst_signal_t)(( R[3]*A + R[4]*B + R[5]*C  )>>15);
		c = (burst_signal_t)(( R[6]*A + R[7]*B + R[8]*C  )>>15);
		burst_long_signal_t ofs = ((a+b+c)*BURST_SIGNAL_T(0.33333333))>>15;
		a-=ofs;
		b-=ofs;
		c-=ofs;
	} else {
		a = *_sensor->raw.A;
		b = *_sensor->raw.B;
		c = *_sensor->raw.C;
	}
	const burst_signal_t one_div_sqrt3 = BURST_SIGNAL_T(0.5773502691896258);
	burst_long_signal_t beta = ( ( b*2 + a ) * one_div_sqrt3)>>15;
	burst_signal_t sn = _sensor->inverter->rot.sn;
	burst_signal_t cs = _sensor->inverter->rot.cs;
	_sensor->dq.lateral = dot_(cs, a, sn, beta);
	_sensor->dq.cross = dot_(-sn, a, cs, beta);	
	_sensor->ab.alfa = a;
	_sensor->ab.beta = beta;
	_sensor->abc.A = a;
	_sensor->abc.B = b;
	_sensor->abc.C = c;
	
}

