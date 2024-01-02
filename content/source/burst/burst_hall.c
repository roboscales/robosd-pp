#include "burst/burst_hall.h"
#include "burst/burst_timer.h"

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
const int8_t hall_diffs_[6] = {
	0, 1, 2, 3, -2, -1
};
void hall_begin(hall_p _hall, hall_config_p _config){
	_hall->config = _config;
	_hall->true_diff = 0;
	_hall->sector_prev = -1;
	_hall->pins.index = (unsigned)-1;
}


typedef hall_extra_t  * hall_qubic_p;
	const burst_long_signal_t pi_div_6 = ( (burst_long_signal_t)BURST_SIGNAL_T(1./6.) )<<16;
#define S(x) BURST_SIGNAL_T(x)
int hall_update_(hall_qubic_p _qubic){
	if(_qubic->ticks==0) return 0;
	burst_signal_t actual = *(_qubic->hall.pactual);
	if( _qubic->hall.prev !=  actual){
		_qubic->lost_flag = burst_false;
		uint32_t tm = _qubic->ticks;
		_qubic->angle32 = ((burst_long_signal_t)actual)<<16;
		uint32_t period = tm - _qubic->begin;
		_qubic->period = period;
		_qubic->begin = tm;
		if(period==0){
			_qubic->hall.prev = actual;
			return 0;
		}
		burst_signal_t delta = actual - _qubic->hall.prev;
		_qubic->hall.prev = actual;
		if(delta > 0){
			_qubic->angle32 -= pi_div_6;
		} else if(delta < 0){
			_qubic->angle32 += pi_div_6;
		}
		burst_long_signal_t delta32;
		if(delta>0){
			delta32 = (((burst_long_signal_t)delta) <<16)/period;
		} else {
			delta32 = -((((burst_long_signal_t)-delta) <<16)/period);
		}

		burst_long_signal_p d = _qubic->speeds32;
		if( 	_qubic->fuzzy == burst_true && ( (delta32>0 && _qubic->speeds32[hall_qubic_rank-1] <0) || (delta32<0 && _qubic->speeds32[hall_qubic_rank-1] > 0)) ){
			for( int i=0;i<hall_qubic_rank-1; ++i, ++d) *d = delta32;
		} else{
			burst_long_signal_p s = d+1;
			for( int i=0;i<hall_qubic_rank-1; ++i, ++s,++d) *d = *s;
		}
		*d = delta32;
		return 1;
	} else {
		return 0;
	}
}
void hall_qubic_poll(hall_qubic_p _qubic){
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
	_qubic->fuzzy = burst_false;
	if(hall_update_(_qubic)){	
		const burst_signal_p p = pseudo;
		burst_long_signal_t A =  0;
		burst_long_signal_t B =  0;
		burst_long_signal_t C =  0;
		burst_long_signal_t D =  0;
		burst_long_signal_p s = _qubic->speeds32;
		for( int i=0;i<hall_qubic_rank;++i,++p,++s) D += (( (*p) * (*s) )>>15);
		s = _qubic->speeds32;
		for( int i=0;i<hall_qubic_rank;++i,++p,++s) C += (( (*p) * (*s) )>>15);
		s = _qubic->speeds32;
		for( int i=0;i<hall_qubic_rank;++i,++p,++s) B += (( (*p) * (*s) )>>15);
		s = _qubic->speeds32;
		for( int i=0;i<hall_qubic_rank;++i,++p,++s) A += (( (*p) * (*s) )>>15);
		D+=_qubic->speeds32[0];
	 _qubic->A=A;
	 _qubic->B=B;
	 _qubic->C=C;
	 _qubic->D=D;
	 _qubic->speed32 = (D  +  B*15 /*+ C*225 + A*3375*/);
	}
}

void hall_regres_poll(hall_qubic_p _qubic){
	static const burst_signal_t pseudo[hall_regress_rank*2]={
	    S(0.6667),    S(0.4667),    S(0.2667),    S(0.0667),   -S(0.1333),   -S(0.3333),
   -S(0.1429),   -S(0.0857),   -S(0.0286),    S(0.0286),    S(0.0857),    S(0.1429)
	};
	_qubic->fuzzy = burst_false;
	if(hall_update_(_qubic)){	
		const burst_signal_p p = pseudo;
		int64_t A =  0;
		int64_t B =  0;
		burst_long_signal_p s = _qubic->speeds32+8;
		for( int i=0;i<hall_regress_rank;++i,++p,++s) B += (( (int64_t)(*p) * (*s) )>>15);
		s = _qubic->speeds32+8;
		for( int i=0;i<hall_regress_rank;++i,++p,++s) A += (( (int64_t)(*p) * (*s) )>>15);

		_qubic->A=(burst_long_signal_t)A;
		_qubic->B=(burst_long_signal_t)B;
		_qubic->speed32 = (B  +  A*7);
	}
}

void hall_dummy_interp(hall_qubic_p _qubic){
	_qubic->angle =  *(_qubic->hall.pactual);
}

void hall_fuzzy_poll(hall_qubic_p _qubic){
	static const burst_signal_t pseudo[hall_fuzzy_rank]={
		 S(0.0476),    S(0.0952),    S(0.1429),    S(0.1905),    S(0.2381),    S(0.2857)
	};
	_qubic->fuzzy = burst_true;
	if(hall_update_(_qubic)){	
		const burst_signal_p p = pseudo;
		int64_t A =  0;

		burst_long_signal_p s = _qubic->speeds32+8;
		for( int i=0;i<hall_fuzzy_rank;++i,++p,++s) A += (( (int64_t)(*p) * (*s) ));

		_qubic->speed32 = A>>15;
	}
}

void hall_qubic_interp(hall_qubic_p _qubic){
	if(_qubic->ticks==0){
		burst_signal_t actual = *(_qubic->hall.pactual);
		_qubic->hall.prev = actual;
		_qubic->angle32 = ((burst_long_signal_t)actual)<<16;
		_qubic->speed32 = 0;
		_qubic->ticks++;
	} else {
		_qubic->ticks++;
		if(!_qubic->lost_flag){
			_qubic->angle32 += _qubic->speed32;
			_qubic->angle  = _qubic->angle32 >> 16;
			burst_signal_t delta = _qubic->angle - _qubic->hall.prev;
			static const burst_signal_t alarm = BURST_SIGNAL_T(70./180.);
			static const burst_signal_t pi_6 = BURST_SIGNAL_T(30./180.);
			if (delta > alarm){
				_qubic->speed32 = 0;
				_qubic->angle = _qubic->hall.prev + pi_6;
				_qubic->angle32 = _qubic->angle<<16;

				_qubic->lost = _qubic->ticks;
				_qubic->lost_flag = burst_true;
				_qubic->lost = _qubic->ticks;
				_qubic->period = _qubic->ticks - _qubic->begin;
			} else if(delta < -alarm){
				_qubic->speed32 = 0;
				_qubic->angle = _qubic->hall.prev - pi_6;
				_qubic->angle32 = _qubic->angle<<16;

				_qubic->lost = _qubic->ticks;
				_qubic->lost_flag = burst_true;
				_qubic->period = _qubic->ticks - _qubic->begin;
			}
		} else{
			if(_qubic->ticks - _qubic->lost > 1000000/BURST_TIMER_TICK_US){
				burst_long_signal_p d = _qubic->speeds32;
				burst_long_signal_p s = d+1;
				for( int i=0;i<hall_qubic_rank-1; ++i, ++s,++d) *d = *s;
				*d = 0;
				_qubic->lost = _qubic->ticks;
				_qubic->period = _qubic->ticks - _qubic->begin;
				_qubic->angle = *_qubic->hall.pactual;
				_qubic->angle32 = (*_qubic->hall.pactual)<<16;
			}
		}
		/* _qubic->speed32 =  ((int64_t)_qubic->speed32*255)>>8;
		if(_qubic->speed32>=-255){			
			if(_qubic->speed32<0){
				_qubic->speed32++;
			}
		} else if(_qubic->speed32<=255) {
			if(_qubic->speed32>0){
				_qubic->speed32--;
			}
		}*/
	}
}


/*		
%         figure(11); 
%         plot( [periods], [XX*B] ,'.k')
%         hold off;
%         plot( p, p ,'or')
%         hold on;
        ix= ix+1;
        actual = [actual; d];
        predictopn = [predictopn; p];
        speed = p;
%         if( p0>4 && p<0 )
%             plot(predictopn);
%         end
        p0=p;
    else        
        angle0=angle0+speed*T;
        speed= speed*0.991;
    end
    angle(k) = angle0;*/
