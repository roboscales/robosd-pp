#include "burst/burst_hall.h"
#include "burst/burst_timer.h"

enum { hall_qubic_rank = 14, hall_regress_rank = 6, hall_fuzzy_rank = 6 };




typedef struct hall_extra_regress_s {
	hall_extra_t ref;
	int64_t A;
	int64_t B;
	burst_long_signal_t speeds_history[hall_regress_rank];
} hall_extra_regress_t;
typedef hall_extra_regress_t  * hall_extra_regress_p;


typedef struct hall_extra_qubic_s {
	hall_extra_t ref;
	burst_long_signal_t speeds_history[hall_qubic_rank];
	burst_long_signal_t A;
	burst_long_signal_t B;
	burst_long_signal_t C;
	burst_long_signal_t D;	
} hall_extra_qubic_t;
typedef hall_extra_qubic_t  * hall_extra_qubic_p;


typedef struct hall_extra_fuzzy_s {
	hall_extra_t ref;
	burst_long_signal_t speeds_history[hall_fuzzy_rank];
} hall_extra_fuzzy_t;
typedef hall_extra_fuzzy_t  * hall_extra_fuzzy_p;

static const int hall_sectors[8] = {
	/*
	-1 //{ 0, 0, 0 }
	, 4//{ 1, 0, 0 }
	, 0//{ 0, 1, 0 }
	, 5 //{ 1, 1, 0 }
	, 2 //{ 0, 0, 1 }
	, 3 //{ 1, 0, 1 }
	, 1 //{ 0, 1, 1 }
	, -1 //{1, 1, 1}
	*/
	// B C A
	-1 //{ 0, 0, 0 }
	, 0//{ 0, 0, 1 }
	, 2//{ 0, 1, 0 }
	, 1 //{ 0, 1, 1 }
	, 4 //{ 1, 0, 0 }
	, 5 //{ 1, 0, 1 }
	, 3 //{ 1, 1, 0 }
	, -1 //{1, 1, 1}
};
static const int hall_sdiffs[6] = {
	0, 1, 2, 3, -2, -1
};			

burst_signal_t  hall_angles[6] = {
	BURST_SIGNAL_T(0)
	, BURST_SIGNAL_T(0.3333333333)
	, BURST_SIGNAL_T(0.6666666667)
	, BURST_SIGNAL_T(1.0)
	, -BURST_SIGNAL_T(0.6666666667)
	, -BURST_SIGNAL_T(0.3333333333)
};

void hall_update(hall_p _hall, const hall_pins_p _pins){
	//int index = _pins->A + (_pins->B <<1) + (_pins->C <<2);
	int index = _pins->index;
	if(index!=_hall->pins.index){
		_hall->counter.total++;
		if (index > 0) {
			int sector = hall_sectors[index];
			if(sector>=0){
				int delta = sector - _hall->sector_prev;
				_hall->sector_prev = sector;
				if (delta >= 0) {
					delta = hall_sdiffs[delta];
				}
				else {
					delta = hall_sdiffs[6 + delta];
				}
				if (delta == 3) {
					if (_hall->true_diff < 0) {
						delta = -3;
					}
				} else {
					_hall->true_diff = delta;
				}
				int raw;
				if (_hall->config->inv) {
					delta = -delta;
					raw = -hall_angles[sector];
				}
				else {
					raw = hall_angles[sector];
				}
				burst_signal_t angle = raw + _hall->config->offset.native;
				/*if (_dir > 0) {
					angle += _hall->config->offset.dynamic;
				}
				else if ( _dir  < 0) {
					angle -= _hall->config->offset.dynamic;
				}*/			
				_hall->delta_acc += delta;
				_hall->delta = delta;
				_hall->angle = angle;
				_hall->raw = raw;
				_hall->pins.index = index;
				_hall->sector = sector;
				return;
			}
		}
		_hall->counter.fault++;
		//to do так делать нельзя, та как накапливается ошибка!
		_hall->delta = 0;
		_hall->delta_acc = 0;
	} else {
		_hall->delta = 0;
		return;
	}
}

const burst_long_signal_t pi_div_6 = ( (burst_long_signal_t)BURST_SIGNAL_T(1./6.) )<<16;
#define S(x) BURST_SIGNAL_T(x)
int hall_update_(hall_extra_p _extra){
	if(_extra->ticks==0) return 0;
	burst_signal_t actual = *(_extra->hall.pactual);
	if( _extra->hall.prev !=  actual){
		_extra->lost_flag = burst_false;
		uint32_t tm = _extra->ticks;
		_extra->angle32 = ((burst_long_signal_t)actual)<<16;
		uint32_t period = tm - _extra->begin;
		_extra->period = period;
		_extra->begin = tm;
		if(period==0){
			_extra->hall.prev = actual;
			return 0;
		}
		burst_signal_t delta = actual - _extra->hall.prev;
		_extra->hall.prev = actual;
		if(delta > 0){
			_extra->angle32 -= pi_div_6;
		} else if(delta < 0){
			_extra->angle32 += pi_div_6;
		}
		burst_long_signal_t delta32 = 0;
		if(delta>0){
			delta32 = (((burst_long_signal_t)delta) <<16)/period;
		} else {
			delta32 = -((((burst_long_signal_t)-delta) <<16)/period);
		}
		
		burst_long_signal_p d = _extra->speeds_history;
		int rank1 = _extra->rank-1;
		burst_bool_t on_sign_reset_flag = _extra->on_sign_reset_flag ;
		burst_long_signal_t last = _extra->speeds_history[rank1];
		if( 	on_sign_reset_flag && ( (delta32>0 && last <0) || (delta32<0 && last > 0)) ){
			for( int i=0;i<rank1; ++i, ++d) *d = delta32;
		} else{
			burst_long_signal_p s = d+1;
			for( int i=0;i<rank1; ++i, ++s,++d) *d = *s;
		}
		*d = delta32;
		return 1;
	} else {
		return 0;
	}
}

void hall_select_extra_(hall_p _hall);
void hall_poll(hall_p _hall){
	if(_hall->config->extra_mode != _hall->extra_mode ){
		hall_select_extra_(_hall);
	}
	if(_hall->extra_mode != hall_extra_mode_none){
		if(hall_update_(_hall->extra)){	
			_hall->extra->aproxx(_hall->extra);
		}
	}
}

void hall_extra_interp_(hall_p _hall);
void hall_interp(hall_p _hall){
	if( _hall->extra_mode != hall_extra_mode_none ){
		hall_extra_interp_(_hall);
	} else{
		 _hall->extra_angle =  _hall->angle;
	}
}


void hall_extra_interp_(hall_p _hall) {
	hall_extra_p _extra = _hall->extra;
	if(_extra->ticks==0){
		burst_signal_t actual = *(_extra->hall.pactual);
		_extra->hall.prev = actual;
		_extra->angle32 = ((burst_long_signal_t)actual)<<16;
		_extra->speed32 = 0;
		_extra->ticks++;
	} else {
		_extra->ticks++;
		if(!_extra->lost_flag){
			_extra->interp(_extra);
			burst_signal_t delta = _extra->angle - _extra->hall.prev;
			static const burst_signal_t alarm = BURST_SIGNAL_T(70./180.);
			static const burst_signal_t pi_6 = BURST_SIGNAL_T(30./180.);
			if (delta > alarm){
				_extra->speed32 = 0;
				_extra->angle = _extra->hall.prev + pi_6;
				_extra->angle32 = _extra->angle<<16;

				_extra->lost = _extra->ticks;
				_extra->lost_flag = burst_true;
				_extra->lost = _extra->ticks;
				_extra->period = _extra->ticks - _extra->begin;
			} else if(delta < -alarm){
				_extra->speed32 = 0;
				_extra->angle = _extra->hall.prev - pi_6;
				_extra->angle32 = _extra->angle<<16;
				_extra->lost = _extra->ticks;
				_extra->lost_flag = burst_true;
				_extra->period = _extra->ticks - _extra->begin;
			}
		} else{
			if(_extra->ticks - _extra->lost > 1000000/BURST_TIMER_TICK_US){
				burst_long_signal_p d = _extra->speeds_history;
				burst_long_signal_p s = d+1;
				for( int i=0;i<_extra->rank-1; ++i, ++s,++d) *d = *s;
				*d = 0;

				_extra->lost = _extra->ticks;
				_extra->period = _extra->ticks - _extra->begin;
				_extra->angle = *_extra->hall.pactual;
				_extra->angle32 = (*_extra->hall.pactual)<<16;
			}
		}
	}
	_hall->extra_angle = _extra->angle;
}

void hall_extra_regress_approxx_(hall_extra_p _extra) {
	static const burst_signal_t pseudo[hall_regress_rank*2]={
	    S(0.6667),    S(0.4667),    S(0.2667),    S(0.0667),   -S(0.1333),   -S(0.3333),
   -S(0.1429),   -S(0.0857),   -S(0.0286),    S(0.0286),    S(0.0857),    S(0.1429)
	};
	hall_extra_regress_p _regres = (hall_extra_regress_p)_extra;
	const burst_signal_p p = pseudo;
	
		
	int64_t A =  0;
	int64_t B =  0;

	burst_long_signal_p s = _regres->speeds_history;
	for( int i=0;i<hall_regress_rank;++i,++p,++s) B += (( (int64_t)(*p) * (*s) )>>15);
	
	s = _regres->speeds_history;
	for( int i=0;i<hall_regress_rank;++i,++p,++s) A += (( (int64_t)(*p) * (*s) )>>15);
	
	_regres->A=(burst_long_signal_t)A;
	_regres->B=(burst_long_signal_t)B;
	_extra->speed32 = (B  +  A*7);		
}

void hall_extra_regress_interp_(hall_extra_p _extra){
	_extra->angle32 += _extra->speed32;
	_extra->angle  = _extra->angle32 >> 16;
}


void hall_extra_qubic_approxx_(hall_extra_p _extra) {
	static const burst_signal_t pseudo[hall_qubic_rank*4]={
		S(0.1429),	S(0.4835),	S(0.0440),	-S(0.2108),	-S(0.3157),	-S(0.3057),	-S(0.2158),
			-S(0.0809),	S(0.0639),	S(0.1838),	S(0.2438),	S(0.2088),	S(0.0440),	-S(0.2857),
		-S(0.5049),	-S(0.1226),	S(0.1176),	S(0.2399),	S(0.2680),	S(0.2259),	S(0.1377),
			S(0.0271),	-S(0.0817),	-S(0.1650),	-S(0.1987),	-S(0.1589),	-S(0.0216),	S(0.2370),
		S(0.0641),	S(0.0090),	-S(0.0241),	-S(0.0392),	-S(0.0401),	-S(0.0306),	-S(0.0147),
			S(0.0038),	S(0.0210),	S(0.0332),	S(0.0364),	S(0.0268),	S(0.0006),	-S(0.0462),
		-S(0.0025),	-S(0.0002),	S(0.0011),	S(0.0017),	S(0.0016),	S(0.0011),	S(0.0004),
			-S(0.0004),	-S(0.0011),	-S(0.0016),	-S(0.0017),	-S(0.0011),	S(0.0002),	S(0.0025)
		};
	hall_extra_qubic_p _qubic = (hall_extra_qubic_p)_extra;

	const burst_signal_p p = pseudo;
	burst_long_signal_t A =  0;
	burst_long_signal_t B =  0;
	burst_long_signal_t C =  0;
	burst_long_signal_t D =  0;
	burst_long_signal_p s = _qubic->speeds_history;
	for( int i=0;i<hall_qubic_rank;++i,++p,++s) D += (( (*p) * (*s) )>>15);
	s = _qubic->speeds_history;
	for( int i=0;i<hall_qubic_rank;++i,++p,++s) C += (( (*p) * (*s) )>>15);
	s = _qubic->speeds_history;
	for( int i=0;i<hall_qubic_rank;++i,++p,++s) B += (( (*p) * (*s) )>>15);
	s = _qubic->speeds_history;
	for( int i=0;i<hall_qubic_rank;++i,++p,++s) A += (( (*p) * (*s) )>>15);
	D+=_qubic->speeds_history[0];
 _qubic->A=A;
 _qubic->B=B;
 _qubic->C=C;
 _qubic->D=D;
 _extra->speed32 = (D  +  B*15 /*+ C*225 + A*3375*/);	
}

void hall_extra_qubic_interp_(hall_extra_p _extra){
	_extra->angle32 += _extra->speed32;
	_extra->angle  = _extra->angle32 >> 16;
}


void hall_extra_fuzzy_approxx_(hall_extra_p _extra){
	static const burst_signal_t pseudo[hall_fuzzy_rank]={
		 S(0.0476),    S(0.0952),    S(0.1429),    S(0.1905),    S(0.2381),    S(0.2857)
	};
	hall_extra_fuzzy_p _qubic = (hall_extra_fuzzy_p)_extra;
	const burst_signal_p p = pseudo;
	int64_t A =  0;
	burst_long_signal_p s = _qubic->speeds_history;
	for( int i=0;i<hall_fuzzy_rank;++i,++p,++s) A += (( (int64_t)(*p) * (*s) ));
	_extra->speed32 = A>>15;
}

void hall_extra_fuzzy_interp_(hall_extra_p _extra){
	_extra->angle32 += _extra->speed32;
	_extra->angle  = _extra->angle32 >> 16;
}


hall_extra_fuzzy_t hall_extra_fuzzy = BURST_EMPTY_STRUCT;
hall_extra_qubic_t hall_extra_qubic = BURST_EMPTY_STRUCT;
hall_extra_regress_t hall_extra_regress = BURST_EMPTY_STRUCT;
hall_extra_t hall_extra_dummy = BURST_EMPTY_STRUCT;

void hall_select_extra_(hall_p _hall){
		switch ( _hall->config->extra_mode){
		case hall_extra_mode_qubic:
			_hall->extra = &hall_extra_qubic.ref;			
			_hall->extra_mode = hall_extra_mode_qubic;
		break;
		case hall_extra_mode_regress:
			_hall->extra = &hall_extra_regress.ref;
			_hall->extra_mode = hall_extra_mode_regress;
		break;
		case hall_extra_mode_fuzzy:
			_hall->extra = &hall_extra_fuzzy.ref;
			_hall->extra_mode = hall_extra_mode_fuzzy;
		break;
		default:
			_hall->extra = &hall_extra_dummy;
			_hall->extra_mode = hall_extra_mode_none;
		break;
	}
}

void hall_begin(hall_p _hall, hall_config_p _config){
	_hall->config = _config;
	_hall->true_diff = 0;
	_hall->sector_prev = -1;
	_hall->pins.index = (unsigned)-1;

	hall_extra_fuzzy.ref.aproxx = hall_extra_fuzzy_approxx_;
	hall_extra_fuzzy.ref.interp = hall_extra_fuzzy_interp_;
	hall_extra_fuzzy.ref.rank = hall_fuzzy_rank;
	hall_extra_fuzzy.ref.speeds_history = hall_extra_fuzzy.speeds_history;
	hall_extra_fuzzy.ref.on_sign_reset_flag = burst_true;
	hall_extra_fuzzy.ref.hall.pactual = &(_hall->angle);
	hall_extra_fuzzy.ref.lost = burst_true;

	hall_extra_qubic.ref.aproxx = hall_extra_qubic_approxx_;
	hall_extra_qubic.ref.interp = hall_extra_qubic_interp_;
	hall_extra_qubic.ref.rank = hall_qubic_rank;
	hall_extra_qubic.ref.speeds_history = hall_extra_qubic.speeds_history;
	hall_extra_qubic.ref.hall.pactual = &(_hall->angle);
	hall_extra_qubic.ref.lost = burst_true;

	hall_extra_regress.ref.aproxx = hall_extra_regress_approxx_;
	hall_extra_regress.ref.interp = hall_extra_regress_interp_;
	hall_extra_regress.ref.rank = hall_regress_rank;
	hall_extra_regress.ref.speeds_history = hall_extra_regress.speeds_history;
	hall_extra_regress.ref.hall.pactual = &(_hall->angle);
	hall_extra_regress.ref.lost = burst_true;

	hall_select_extra_(_hall);
}

