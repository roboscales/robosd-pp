#include "burst/burst_app.h"

#if BURST_TIMER_ENABLED == 1
#include "burst/burst_timer.h"
#endif
#if BURST_BUTTON_ENABLED == 1
#include "burst/burst_button.h"
#endif
struct burst_s;
typedef struct burst_s burst_t;
typedef burst_t * burst_p;


#ifndef BURST_DEV_COUNT
#define BURST_DEV_COUNT 0
#endif

struct burst_s{
	#if BURST_DEV_COUNT == 0
	burst_dev_ref_p * devs;
	#else	
	burst_dev_ref_p devs[BURST_DEV_COUNT];
	#endif
	burst_dev_ref_p * devs_end;
	int dev_count;
};

burst_t burst = BURST_EMPTY_STRUCT;

void burst_dev_idle_event(burst_dev_ref_p _dev){
		BURST_UNUSED(_dev);
}

void burst_dev_attach(burst_dev_ref_p _ref){
	burst_alarm(burst.dev_count<BURST_DEV_COUNT);
	burst.devs[ burst.dev_count ] = _ref;
	burst.dev_count ++ ;
}

burst_dev_mode_t burst_idle_mode ={
	&burst_dev_idle_event
	,&burst_dev_idle_event
	,&burst_dev_idle_event
	,&burst_dev_idle_event
	,&burst_dev_idle_event
	,&burst_dev_idle_event
	,&burst_dev_idle_event
};

void burst_dev_runA(burst_dev_ref_p _ref){
	_ref->actual_mode->loopA(_ref);
}
void burst_dev_runB(burst_dev_ref_p _ref){
	_ref->actual_mode->loopB(_ref);
}
void burst_dev_runC(burst_dev_ref_p _ref){
	_ref->actual_mode->loopC(_ref);
}
typedef enum { burst_core_unknown=0,burst_core_backend=1,burst_core_frontend=2} burst_core_status;
burst_core_status burst_core_status_ = burst_core_unknown;

void burst_begin(void){
	burst_sw_begin();
	burst_dev_ref_p * p ;	
	burst.devs_end = burst.devs+BURST_DEV_COUNT;
	
	for( p= burst.devs; p!=burst.devs_end;p++){		
		burst_alarm(*p);
		burst_alarm((*p)->reset);
		burst_alarm((*p)->start);
		burst_alarm((*p)->realtime_loop);
		burst_alarm((*p)->frontend_loop);
		burst_alarm((*p)->config);
		burst_alarm((*p)->action);
		burst_alarm((*p)->feedback);
		burst_alarm((*p)->perform_panic);		
		if((*p)->mode_count>0){
			burst_alarm((*p)->modes);
		}	
		(*p)->actual_mode = &burst_idle_mode;		
		(*p)->modes_end = (*p)->modes+(*p)->mode_count;
		for(	burst_dev_mode_p  * pm = (*p)->modes; pm!=(*p)->modes_end; ++pm) {
			burst_dev_mode_p m = *pm;
			if(m){
				burst_alarm(m->applay_action);
				burst_alarm(m->start);
				burst_alarm(m->stop);
				burst_alarm(m->loopA);
				burst_alarm(m->loopB);
				burst_alarm(m->loopC);
				burst_alarm(m->frontend_loop);
			}
		}
		//сброс в начальное состояние
		(*p)->reset((*p));
	}
	
	burst_hw_begin();
	burst_core_status_ = burst_core_frontend;
}
burst_bool_t burst_started_ = burst_false;
void burst_start(void){
	burst_dev_ref_p * p ;
	for( p= burst.devs; p!=burst.devs_end;p++){
		(*p)->start(*p);
	}
	burst_hw_start();
	burst_sw_start();
	burst_started_ = burst_true;
}
void burst_reset(void){
	burst_dev_ref_p * p ;
	for( p= burst.devs; p!=burst.devs_end;p++){
		(*p)->reset(*p);
	}
}

void burst_realtime_loop(void){
	if(burst_started_){
		burst_fall();
		burst_dev_ref_p * p ;
		debug_tp_on(VERB_LOOP);
		debug_tp_on(VERB_REALTIME);
		for( p= burst.devs; p!=burst.devs_end;p++){
			if( (*p) -> panic){
				(*p)->perform_panic(*p);				
			}
			(*p)->realtime_loop(*p);
		}
		burst_sw_realtime_loop();
		burst_hw_realtime_loop();
		debug_tp_off(VERB_REALTIME);
	}
}


void burst_dev_switch_to_idle(burst_dev_ref_p _ref){
	_ref->actual_mode = &burst_idle_mode;
	_ref->mode = burst_dev_mode_idle;
}
void burst_dev_backend_loop(burst_dev_ref_p _ref){
	if(burst_started_){
		//burst_alarm(is_backend__);
		burst_dev_action_p action = _ref->action;
		int action_mode = action->mode;
		burst_dev_mode_p actual_mode = _ref->actual_mode;
		if ( action_mode != _ref->mode) {
			if (actual_mode) {
				actual_mode->stop(_ref);
			}

			if (action_mode == burst_dev_mode_idle) {
				burst_dev_switch_to_idle(_ref);
			}
			else {
				if(action_mode<_ref->mode_count){
					burst_dev_mode_p m = _ref->modes[action_mode];
					if (m == 0) {
						burst_dev_switch_to_idle(_ref);
					}
					else {
						m->start(_ref);
						m->applay_action(_ref);
						_ref->actual_mode = m;
						_ref->mode = action_mode;
					}
				}else{
					burst_dev_switch_to_idle(_ref);
				}
			}
		} else{
			if(action->actual){
				actual_mode->applay_action(_ref);
				action->actual = 0;
			}
		}
	}
}


typedef void ( *burst_slot_f)(void);

#define BURST_SLOT_D(n) \
void burst_slot_##n(void){ \
	burst_sw_slot_##n(); \
	burst_hw_slot_##n(); \
}

#define BURST_SLOT(n) \
&burst_slot_##n

#if BURST_SLOT_COUNT > 0
BURST_SLOT_D(0)
#endif

#if BURST_SLOT_COUNT > 1
BURST_SLOT_D(1)
#endif

#if BURST_SLOT_COUNT > 2
BURST_SLOT_D(2)
#endif

#if BURST_SLOT_COUNT > 3
BURST_SLOT_D(3)
#endif

#if BURST_SLOT_COUNT > 4
BURST_SLOT_D(4)
#endif

#if BURST_SLOT_COUNT > 5
BURST_SLOT_D(5)
#endif

#if BURST_SLOT_COUNT > 6
BURST_SLOT_D(6)
#endif

#if BURST_SLOT_COUNT > 7
BURST_SLOT_D(7)
#endif

#if BURST_SLOT_COUNT > 8
BURST_SLOT_D(8)
#endif

#if BURST_SLOT_COUNT > 9
BURST_SLOT_D(9)
#endif

#if BURST_SLOT_COUNT > 10
BURST_SLOT_D(10)
#endif

#if BURST_SLOT_COUNT > 11
BURST_SLOT_D(11)
#endif

#if BURST_SLOT_COUNT > 12
BURST_SLOT_D(12)
#endif

#if BURST_SLOT_COUNT > 13
BURST_SLOT_D(13)
#endif

#if BURST_SLOT_COUNT > 14
BURST_SLOT_D(14)
#endif

#if BURST_SLOT_COUNT > 15
BURST_SLOT_D(15)
#endif

burst_slot_f burst_slots[BURST_SLOT_COUNT]={
	#if BURST_SLOT_COUNT > 0
	BURST_SLOT(0)
	#endif
	#if BURST_SLOT_COUNT > 1
	,BURST_SLOT(1)
	#endif

	#if BURST_SLOT_COUNT > 2
	,BURST_SLOT(2)
	#endif

	#if BURST_SLOT_COUNT > 3
	,BURST_SLOT(3)
	#endif

	#if BURST_SLOT_COUNT > 4
	,BURST_SLOT(4)
	#endif

	#if BURST_SLOT_COUNT > 5
	,BURST_SLOT(5)
	#endif

	#if BURST_SLOT_COUNT > 6
	,BURST_SLOT(6)
	#endif

	#if BURST_SLOT_COUNT > 7
	,BURST_SLOT(7)
	#endif

	#if BURST_SLOT_COUNT > 8
	,BURST_SLOT(8)
	#endif

	#if BURST_SLOT_COUNT > 9
	,BURST_SLOT(9)
	#endif

	#if BURST_SLOT_COUNT > 10
	,BURST_SLOT(10)
	#endif

	#if BURST_SLOT_COUNT > 11
	,BURST_SLOT(11)
	#endif

	#if BURST_SLOT_COUNT > 12
	,BURST_SLOT(12)
	#endif

	#if BURST_SLOT_COUNT > 13
	,BURST_SLOT(13)
	#endif

	#if BURST_SLOT_COUNT > 14
	,BURST_SLOT(14)
	#endif

	#if BURST_SLOT_COUNT > 15
	,BURST_SLOT(15)
	#endif
};
burst_slot_f * burst_slot = burst_slots;
burst_slot_f * burst_slots_end = burst_slots+BURST_SLOT_COUNT;

void burst_backend_loop(void){
	if(burst_started_){
		debug_tp_on(VERB_BACKEND);
		#if BURST_TIMER_ENABLED == 1
		burst_timer_poll();
		#endif
		#if BURST_SLOT_COUNT > 0
		(*burst_slot)();
		burst_slot++;
		if(burst_slot==burst_slots_end) 
			burst_slot = burst_slots;
		#endif
		{
			burst_dev_ref_p * p ;
			for( p= burst.devs; p!=burst.devs_end;p++){
				burst_dev_backend_loop((*p));
			}
		}
		burst_hw_backend_loop();
		burst_sw_backend_loop();
		debug_tp_off(VERB_BACKEND);
		debug_tp_off(VERB_LOOP);
		burst_comeback();
	}
}

void burst_frontend_loop(void){
	debug_tp_on(VERB_FRONTEND);
	#if BURST_TIMER_ENABLED == 1
	burst_timer_poll();
	#endif
	burst_sw_frontend_loop();
	burst_hw_frontend_loop();
	{
		burst_dev_ref_p * p ;
		for( p= burst.devs; p!=burst.devs_end;p++){
			burst_dev_ref_p s = *p;
			s->frontend_loop(s);
			s->actual_mode->frontend_loop(s);
		}
	}
	debug_tp_off(VERB_FRONTEND);
	
	#if BURST_BUTTON_ENABLED == 1
	burst_btn_poll();
	#endif
}


#ifdef BURST_WEAK

BURST_WEAK void burst_sw_begin(void){}
BURST_WEAK void burst_hw_begin(void){}

BURST_WEAK void burst_sw_start(void){}
BURST_WEAK void burst_hw_start(void){}

BURST_WEAK void burst_sw_backend_loop(void){}
BURST_WEAK void burst_hw_backend_loop(void){}

BURST_WEAK void burst_sw_realtime_loop(void){}
BURST_WEAK void burst_hw_realtime_loop(void){}

BURST_WEAK void burst_sw_frontend_loop(void){}
BURST_WEAK void burst_hw_frontend_loop(void){}

#if BURST_SLOT_COUNT > 0
BURST_WEAK void burst_sw_slot_0(void){}
BURST_WEAK void burst_hw_slot_0(void){}
#endif

#if BURST_SLOT_COUNT > 1
BURST_WEAK  void burst_sw_slot_1(void){}
BURST_WEAK  void burst_hw_slot_1(void){}
#endif

#if BURST_SLOT_COUNT > 2
BURST_WEAK  void burst_sw_slot_2(void){}
BURST_WEAK  void burst_hw_slot_2(void){}
#endif

#if BURST_SLOT_COUNT > 3
BURST_WEAK  void burst_sw_slot_3(void){}
BURST_WEAK  void burst_hw_slot_3(void){}
#endif

#if BURST_SLOT_COUNT > 4
BURST_WEAK  void burst_sw_slot_4(void){}
BURST_WEAK  void burst_hw_slot_4(void){}
#endif

#if BURST_SLOT_COUNT > 5
BURST_WEAK  void burst_sw_slot_5(void){}
BURST_WEAK  void burst_hw_slot_5(void){}
#endif

#if BURST_SLOT_COUNT > 6
BURST_WEAK  void burst_sw_slot_6(void){}
BURST_WEAK  void burst_hw_slot_6(void){}
#endif

#if BURST_SLOT_COUNT > 7
BURST_WEAK  void burst_sw_slot_7(void){}
BURST_WEAK  void burst_hw_slot_7(void){}
#endif


#if BURST_SLOT_COUNT > 8
BURST_WEAK  void burst_sw_slot_8(void){}
BURST_WEAK  void burst_hw_slot_8(void){}
#endif

#if BURST_SLOT_COUNT > 9
BURST_WEAK  void burst_sw_slot_9(void){}
BURST_WEAK  void burst_hw_slot_9(void){}
#endif

#if BURST_SLOT_COUNT > 10
BURST_WEAK  void burst_sw_slot_10(void){}
BURST_WEAK  void burst_hw_slot_10(void){}
#endif

#if BURST_SLOT_COUNT > 11
BURST_WEAK  void burst_sw_slot_11(void){}
BURST_WEAK  void burst_hw_slot_11(void){}
#endif

#if BURST_SLOT_COUNT > 12
BURST_WEAK  void burst_sw_slot_12(void){}
BURST_WEAK  void burst_hw_slot_12(void){}
#endif

#if BURST_SLOT_COUNT > 13
BURST_WEAK  void burst_sw_slot_13(void){}
BURST_WEAK  void burst_hw_slot_13(void){}
#endif

#if BURST_SLOT_COUNT > 14
BURST_WEAK  void burst_sw_slot_14(void){}
BURST_WEAK  void burst_hw_slot_14(void){}
#endif

#if BURST_SLOT_COUNT > 15
BURST_WEAK  void burst_sw_slot_15(void){}
BURST_WEAK  void burst_hw_slot_15(void){}
#endif

BURST_WEAK  void burst_sw_on_crash(const char * _file, const char * _function, int _line){
	BURST_UNUSED(_file);
	BURST_UNUSED(_function);
	BURST_UNUSED(_line);
}
BURST_WEAK  void burst_hw_on_crash(void){
}
BURST_WEAK  void burst_hw_fall(void){}
BURST_WEAK  void burst_hw_comeback(void){}

BURST_WEAK  burst_guard_op_t burst_hw_critical_enter(void){
	return burst_guard_op_skip;
}
BURST_WEAK  void burst_hw_critical_leave(void){
}

BURST_WEAK  burst_guard_op_t burst_hw_guard_enter(void){
	return burst_guard_op_skip;
}
BURST_WEAK  void burst_hw_guard_leave(void){
}

BURST_WEAK  burst_guard_op_t burst_hw_guard_lock(void){ 
return burst_guard_op_skip;
}
BURST_WEAK  void burst_hw_guard_unlock(void){ 
}
#endif

BURST_WEAK  void burst_hw_reboot(void){}

	burst_bool_t burst_is_frontend(void){
	return (burst_bool_t) (burst_core_status_== burst_core_frontend );
}
burst_bool_t burst_is_backend(void){
	return (burst_bool_t) (burst_core_status_== burst_core_backend);
}
#ifndef BURST_CORE_DEBUG
#define BURST_CORE_DEBUG 1
#endif

#if BURST_CORE_DEBUG ==1
#define  BURST_CORE_ALARM( x ) burst_alarm( (x) )
#else
#define  BURST_CORE_ALARM(x)
#endif

void burst_fall(void){
	BURST_CORE_ALARM( burst_is_frontend() );
	burst_hw_fall();
	burst_core_status_= burst_core_backend;
}
void burst_comeback(void){
	BURST_CORE_ALARM(burst_is_backend());
	burst_hw_comeback();
	burst_core_status_= burst_core_frontend;
}
int32_t guest_count_ = 0;
int32_t lock_count_ = 0;
typedef union {
	uint32_t value;
	struct{
		uint8_t critical_op;
		uint8_t guard_op;
		#if BURST_CORE_DEBUG == 1
		uint8_t guad_context;
		#endif
	};
} burst_guard_context_t;

uint32_t burst_guard_enter(void){
	burst_guard_context_t ctx;
	if (burst_is_frontend()) {
		ctx.critical_op = (uint8_t)burst_hw_critical_enter();
		#if BURST_CORE_DEBUG == 1
		guest_count_++;			
		ctx.guad_context = (uint8_t)burst_core_frontend;
		#endif
		ctx.guard_op = (uint8_t)burst_hw_guard_enter();
	}	else {
		#if BURST_CORE_DEBUG == 1
		lock_count_++;
		ctx.guad_context = (uint8_t)burst_core_backend;
		#endif
		ctx.guard_op = (uint8_t)burst_hw_guard_lock();
	}
	return ctx.value;
}

void burst_guard_leave(uint32_t _context){
	burst_guard_context_t ctx;
	ctx.value	= _context; 
	if (burst_is_frontend()) {
		#if BURST_CORE_DEBUG == 1
		guest_count_--;
		burst_alarm(ctx.guad_context == burst_core_frontend )
		burst_alarm(guest_count_ >= 0);
		#endif

		if ( (burst_guard_op_t)ctx.guard_op == burst_guard_op_run) {
			burst_hw_guard_leave();
		}
		if ( (burst_guard_op_t)ctx.critical_op == burst_guard_op_run) 
			burst_hw_critical_leave();
	}
	else {
		#if BURST_CORE_DEBUG == 1
		lock_count_--;
		burst_alarm(lock_count_ >= 0);
		burst_alarm(ctx.guad_context == burst_core_backend);
		#endif
		if ((burst_guard_op_t)ctx.guard_op == burst_guard_op_run)
			burst_hw_guard_unlock();
	}
}
	
uint32_t burst_critical_enter(void){
	//быстрым процессам сдесь делать нечего - это разборки между потоками "фронткнд"
	#if BURST_CORE_DEBUG == 1
	burst_alarm(burst_is_frontend());
	#endif
	return (uint32_t) burst_hw_critical_enter();
}
void burst_critical_leave(uint32_t _context){
	#if BURST_CORE_DEBUG == 1
	burst_alarm(burst_is_frontend());
	#endif
	if ((burst_guard_op_t)_context == burst_guard_op_run)
		burst_hw_critical_leave();
}

#if BURST_DEBUG_TP_ENABLED == 1
#define CLCH_NAME burst_tp
#include "burst/cliche/tp.h"
#endif

void burst_event_perform_panic(burst_dev_ref_p _dev){	
	if(_dev->mode!=burst_dev_mode_idle){
		burst_dev_mode_p actual_mode = _dev->actual_mode;
		if (actual_mode) {
			actual_mode->stop(_dev);
		}
		
		burst_dev_switch_to_idle(_dev);
	}
	_dev->action->mode = burst_dev_mode_idle;
}

void burst_raise_panic(burst_dev_ref_p _dev, uint32_t flag){
	_dev->panic |= flag;
	burst_event_perform_panic(_dev);
}
