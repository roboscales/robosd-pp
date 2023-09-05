#include "burst/burst_app.h"
#include "burst/burst_timer.h"

#if defined(CLCH_NAME) 
#include "burst/cliche/_begin.h"

#ifdef CLCH_HEADER
#ifndef clch_net_master_h
#define clch_net_master_h


typedef struct PREFIX(packet_s){
	uint8_t * data;
	uint16_t size;
} PREFIX(packet_t);


typedef PREFIX(packet_t) * PREFIX(packet_p);			
void PREFIX(poll)(void);
void PREFIX(refuse)(void);
void PREFIX(confirm)(void);
void PREFIX(begin)(void);
void PREFIX(start)(void);
void PREFIX(exchange)(const PREFIX(packet_p) _outcom_packet, PREFIX(packet_p) _incom_packet);
burst_bool_t PREFIX(ready)(void);
#endif

#else

PREFIX(packet_p) PREFIX(outcom_packet_) = 0;
PREFIX(packet_p) PREFIX(incom_packet_) = 0;

BURST_WEAK burst_bool_t PREFIX(prf_panic)(void){
	return burst_false;
}
BURST_WEAK void PREFIX(prf_begin)(void){
}
BURST_WEAK void PREFIX(prf_start)(void){
}
BURST_WEAK void PREFIX(prf_send_cancel)(void){
}
BURST_WEAK void PREFIX(prf_receive_cancel)(void){
}
BURST_WEAK void PREFIX(prf_start_send)(PREFIX(packet_p) _p){
	BURST_UNUSED(_p);
}
BURST_WEAK void PREFIX(prf_start_receive)(PREFIX(packet_p) _p){
	BURST_UNUSED(_p);
}

BURST_WEAK burst_time_us_t PREFIX(prf_wd_us)(PREFIX(packet_p) _p){
	BURST_UNUSED(_p);
	return 0xFFFF;
}

BURST_WEAK void PREFIX(confirm_callback)(burst_bool_t _r){
	BURST_UNUSED(_r);
}

BURST_WEAK burst_bool_t PREFIX(prf_ready)(void){
	return burst_true;
}

void PREFIX(reset_)(void);
	
typedef enum  { PREFIX(result_refuse), PREFIX(result_success), PREFIX(result_panic) } PREFIX(result);

burst_bool_t PREFIX(wd_enabled) = burst_false;
burst_time_us_t PREFIX(wd_begin_us_) = 0;
burst_time_us_t PREFIX(wd_delay_us_) = 0;
			
typedef enum { PREFIX(state_idle), PREFIX(state_send), PREFIX(state_receive), PREFIX(state_stopped), PREFIX(state_disable), PREFIX(state_panic)} PREFIX(state);
			
PREFIX(state) PREFIX(state_ )= PREFIX(state_disable);
			
void PREFIX(panic_)(void){
	PREFIX(refuse());
	if( PREFIX(prf_panic)() ){
		PREFIX(state_) = PREFIX(state_panic);
	} else {
		PREFIX(state_) = PREFIX(state_idle);
	}
	PREFIX(reset_)();
}

void PREFIX(reset_)(void){
	PREFIX(outcom_packet_) = 0;
	PREFIX(incom_packet_) = 0;
	PREFIX(wd_enabled) = burst_false;
}

void PREFIX(begin)(void){
	PREFIX(prf_begin)();
	PREFIX(state_) = PREFIX(state_stopped);
}
			
void PREFIX(start)(void){
	PREFIX(prf_start)();
	PREFIX(state_) = PREFIX(state_idle);
}
void PREFIX(cancel_)(void){
	switch (PREFIX(state_)){
	case PREFIX(state_send):
		PREFIX(prf_send_cancel)();
	break;
	case PREFIX(state_receive):
		PREFIX(prf_receive_cancel)();
	break;
	default:;
	}
	PREFIX(reset_)();
}			
void PREFIX(stop)(void){
	uint32_t ctx = burst_guard_enter();
	PREFIX(cancel_)();
	PREFIX(state_) = PREFIX(state_stopped);
	burst_guard_leave(ctx);
}
			
void PREFIX(cancel)(void){
	uint32_t ctx = burst_guard_enter();
	PREFIX(cancel_)();
	PREFIX(state_) = PREFIX(state_idle);
	burst_guard_leave(ctx);
}

void PREFIX(exchange)(const PREFIX(packet_p) _outcom_packet, PREFIX(packet_p) _incom_packet){
	uint32_t ctx = burst_guard_enter();
	PREFIX(outcom_packet_) = (_outcom_packet);
	PREFIX(incom_packet_) = (_incom_packet);
	if ( PREFIX(state_) == PREFIX(state_idle) ){
		PREFIX(state_) = PREFIX(state_send);
		PREFIX(wd_begin_us_) = burst_time_us();
		PREFIX(wd_enabled) = burst_true;
		PREFIX(wd_delay_us_) = PREFIX(prf_wd_us)(PREFIX(outcom_packet_));
		if( PREFIX(incom_packet_) ){
			PREFIX(wd_delay_us_) += PREFIX(prf_wd_us)(PREFIX(incom_packet_));
		}
		PREFIX(prf_start_send)(PREFIX(outcom_packet_));						
	} else {
		PREFIX(panic_());
	}
	burst_guard_leave(ctx);
}
			
			
void PREFIX(confirm)(void){
	burst_bool_t cf = burst_false;
	burst_bool_t result = burst_false;
	{
		uint32_t ctx = burst_guard_enter();
		switch ( PREFIX(state_) ){
		case PREFIX(state_send):
			if( PREFIX(incom_packet_) != 0 ){
				PREFIX(wd_begin_us_) = burst_time_us();
				PREFIX(state_)= PREFIX(state_receive);
				PREFIX(wd_delay_us_) = PREFIX(prf_wd_us)(PREFIX(incom_packet_));
				PREFIX(prf_start_receive)(PREFIX(incom_packet_));
				goto end_;
			}
		case PREFIX(state_receive):
			PREFIX(reset_)();
			PREFIX(state_) = PREFIX(state_idle);
			cf = burst_true;
			result = burst_true;
			break;
		default:;
			cf = burst_true;
			result = burst_false;
			PREFIX(panic_)();					
		}
		end_:
			burst_guard_leave(ctx);
	}
	if(cf) PREFIX(confirm_callback)(result);
}

void PREFIX(refuse)(void){
	uint32_t ctx = burst_guard_enter();
	PREFIX(reset_)();
	switch (PREFIX(state_)){
	case PREFIX(state_idle):
		break;
	case PREFIX(state_send):
		PREFIX(prf_send_cancel)();
		break;
	case PREFIX(state_receive):
		PREFIX(prf_receive_cancel)();
		break;						
	default:
		PREFIX(panic_)();			
	}
	PREFIX(state_) = PREFIX(state_idle);
	burst_guard_leave(ctx);
	PREFIX(confirm_callback)(burst_false);
}				
			
void PREFIX(poll)(void){
	if(PREFIX(wd_enabled)){
		if( burst_time_us() - PREFIX(wd_begin_us_) > PREFIX(wd_delay_us_) ){
			PREFIX(refuse)();
		}
	}
}
			
burst_bool_t PREFIX(ready)(void){
	burst_bool_t res;
	uint32_t ctx = burst_guard_enter();
	res =  ( PREFIX(state_) == PREFIX(state_idle) ) && PREFIX(prf_ready)() ? burst_true:burst_false;
	burst_guard_leave(ctx);
	return res;
}		

#endif
#include "burst/cliche/_end.h"
#endif

