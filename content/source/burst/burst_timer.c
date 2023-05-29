#include "burst/burst_timer.h"
#include "burst/burst_app.h"
#if BURST_TIMER_ENABLED == 1

#ifndef BURST_TIMER_BACKEHD_IX_SIZE
#define BURST_TIMER_BACKEHD_IX_SIZE 3
#endif
#ifndef BURST_TIMER_FRONTEND_IX_SIZE
#define BURST_TIMER_FRONTEND_IX_SIZE 3
#endif
burst_timer_p burst_timer_backend_inedex_ [BURST_TIMER_BACKEHD_IX_SIZE] = {};
burst_timer_p burst_timer_frontend_inedex_ [BURST_TIMER_FRONTEND_IX_SIZE] = {};

burst_time_us_t burst_time_us_ = 0;
burst_time_ms_t	burst_time_ms_ = 0;
burst_time_us_t	burst_acc_us_ = 0;
			
burst_bool_t burst_timer_start(burst_timer_p _timer){
	burst_timer_p * ix;
	int sz;
	if( burst_is_backend( ) ){
		ix =  burst_timer_backend_inedex_;
		sz = BURST_TIMER_BACKEHD_IX_SIZE;
	} else{
		ix =  burst_timer_frontend_inedex_;
		sz = BURST_TIMER_FRONTEND_IX_SIZE;
	}
	for(int i = 0; i < sz; ++i,++ix ){
		if( *ix == 0 ){
			*ix = _timer;
			return burst_true;
		}
	}
	burst_crash();
	return burst_false;
}

void burst_timer_stop(burst_timer_p _timer){
	burst_timer_p * ix;
	int sz;
	if( burst_is_backend( ) ){
		ix =  burst_timer_backend_inedex_;
		sz = BURST_TIMER_BACKEHD_IX_SIZE;
	} else{
		ix =  burst_timer_frontend_inedex_;
		sz = BURST_TIMER_FRONTEND_IX_SIZE;
	}
	for(int i = 0; i < sz; ++i,++ix ){
		if( *ix == _timer ){
			*ix = 0;
			return;
		}
	}
	burst_crash();
}

void burst_timer_poll(void){
	burst_timer_p * ix;
	int sz;
	if(burst_is_backend( ) ){
		burst_time_us_ += BURST_TIMER_TICK_US;
		ix =  burst_timer_backend_inedex_;
		sz = BURST_TIMER_BACKEHD_IX_SIZE;
			burst_acc_us_ += BURST_TIMER_TICK_US;
			while (burst_acc_us_ > 1000) {
				burst_acc_us_ -= 1000;
				burst_time_ms_++;
			}
	} else{
		sz = BURST_TIMER_FRONTEND_IX_SIZE;
		ix =  burst_timer_frontend_inedex_;
	}
	for(int i = 0; i < sz; ++i,++ix ){
		burst_timer_p tm = *ix;
		if(tm){
			if( (burst_time_us_ - tm->last) > tm->period ){
				tm->last = burst_time_us_;
				if( tm->on_tick() != burst_continue){
					*ix = 0;
				}					
			}
		}
	}
}
burst_time_us_t burst_time_us(void){
	return burst_time_us_;
}
burst_time_ms_t burst_time_ms(void){
	return burst_time_ms_;
}
#endif
