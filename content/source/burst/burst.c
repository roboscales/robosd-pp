#include "burst/burst_app.h"
#if BURST_TIMER_ENABLED == 1
#include "burst/burst_timer.h"
#endif
struct burst_s;
typedef struct burst_s burst_t;
typedef burst_t * burst_p;


#ifndef BURST_DEV_COUNT
#define BURST_DEV_COUNT 1
#endif

struct burst_s{
	#if BURST_DEV_COUNT == 0
	burst_dev_ref_p devs;
	#else	
	burst_dev_ref_t devs[BURST_DEV_COUNT];
	#endif
	burst_dev_ref_p devs_end;
};

burst_t burst = BURST_EMPTY_STRUCT;

void burst_dev_idle_mode_event(burst_dev_p _mode){
		BURST_UNUSED(_mode);
}
burst_dev_mode_t burst_idle_mode ={
	&burst_dev_idle_mode_event
	,&burst_dev_idle_mode_event
	,&burst_dev_idle_mode_event
	,&burst_dev_idle_mode_event
	,&burst_dev_idle_mode_event
	,&burst_dev_idle_mode_event
	,&burst_dev_idle_mode_event
	,0
};

void burst_dev_runA(burst_dev_ref_p _ref){
	_ref->actual_mode->loopA(_ref->dev);
}
void burst_dev_runB(burst_dev_ref_p _ref){
	_ref->actual_mode->loopB(_ref->dev);
}
void burst_dev_runC(burst_dev_ref_p _ref){
	_ref->actual_mode->loopC(_ref->dev);
}

void burst_begin(void){
	burst_dev_ref_p p ;
	burst_dev_mode_p * m ;
	burst.devs_end = burst.devs+BURST_DEV_COUNT;
	
	for( p= burst.devs; p!=burst.devs_end;p++){
		burst_alarm(p->dev);
		burst_alarm(p->prioritet_loop);
		p->actual_mode = &burst_idle_mode;		
		p->dev->ref=p;
		if(p->begin)
			p->begin(p->dev);
		p->modes_end = p->modes+p->mode_count;
		for( m= p->modes; m!=p->modes_end;m++){
			if(m){
				(*m)->dev = p->dev;
			}
		}
	}
	
	burst_sw_begin();
	burst_hw_begin();
}

void burst_start(void){
	burst_dev_ref_p p ;
	for( p= burst.devs; p!=burst.devs_end;p++){
		if(p->start)
			p->start(p->dev);
	}
	burst_hw_start();
	burst_sw_start();
}

void burst_prioritet_loop(void){
	burst_dev_ref_p p ;
	for( p= burst.devs; p!=burst.devs_end;p++){
		p->prioritet_loop(p->dev);
	}
	burst_sw_prioritet_loop();
	burst_hw_prioritet_loop();
}


void burst_dev_switch_to_idle(burst_dev_ref_p _ref){
	_ref->actual_mode = &burst_idle_mode;
	_ref->present->mode = burst_dev_mode_idle;
}
void burst_dev_backend_loop(burst_dev_ref_p _ref){
		//burst_alarm(is_backend__);
		burst_dev_action_p action = _ref->action;
		int action_mode = action->mode;
		burst_dev_mode_p actual_mode = _ref->actual_mode;
		if ( action_mode != _ref->present->mode) {
			if (actual_mode) {
				actual_mode->stop(_ref->dev);
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
						m->start(_ref->dev);
						m->applay_action(_ref->dev);
						_ref->actual_mode = m;
						_ref->present->mode = action_mode;
					}
				}else{
					burst_dev_switch_to_idle(_ref);
				}
			}
		} else{
			if(action->actual){
				actual_mode->applay_action(_ref->dev);
				action->actual = 0;
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
	BURST_SLOT(0)
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

burst_thread_t  burst_thread_ = burst_frontend;
burst_thread_t burst_thread(void){
	return burst_thread_;
}
void burst_backend_loop(void){
	burst_alarm(burst_thread_ != burst_frontend)
	burst_thread_ = burst_backend;
	#if BURST_TIMER_ENABLED == 1
	burst_timer_poll();
	#endif
	burst_sw_backend_loop();
	burst_hw_backend_loop();
	(*burst_slot)();
	burst_slot++;
	if(burst_slot==burst_slots_end) burst_slot = burst_slots;
	{
		burst_dev_ref_p p ;
		for( p= burst.devs; p!=burst.devs_end;p++){
			burst_dev_backend_loop(p);
		}
	}
	burst_thread_ = burst_frontend;
}

void burst_frontend_loop(void){
	#if BURST_TIMER_ENABLED == 1
	burst_timer_poll();
	#endif
	burst_sw_frontend_loop();
	burst_hw_frontend_loop();
	{
		burst_dev_ref_p p ;
		for( p= burst.devs; p!=burst.devs_end;p++){
			if(p->frontend_loop)
				p->frontend_loop(p->dev);
		}
	}
}

#ifdef BURST_WEAK

BURST_WEAK void burst_sw_begin(void){}
BURST_WEAK void burst_hw_begin(void){}

BURST_WEAK void burst_sw_start(void){}
BURST_WEAK void burst_hw_start(void){}

BURST_WEAK void burst_sw_backend_loop(void){}
BURST_WEAK void burst_hw_backend_loop(void){}

BURST_WEAK void burst_sw_prioritet_loop(void){}
BURST_WEAK void burst_hw_prioritet_loop(void){}

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

BURST_WEAK  void burst_hw_crash(const char * _file, const char * _function, int _line){
	BURST_UNUSED(_file);
	BURST_UNUSED(_function);
	BURST_UNUSED(_line);
}
BURST_WEAK  void burst_hw_fall(void){}
BURST_WEAK  void burst_hw_comeback(void){}

BURST_WEAK  void * burst_hw_critical_enter(void){
	return 0;
}
BURST_WEAK  void burst_hw_critical_leave(void* _context){
	BURST_UNUSED(_context);
}

BURST_WEAK  void * burst_hw_guard_enter(void){
	return 0;
}
BURST_WEAK  void burst_hw_guard_leave(void* _context){
	BURST_UNUSED(_context);
}

BURST_WEAK  void burst_hw_guard_lock(void){}
BURST_WEAK  void burst_hw_guard_unlock(void){}

#endif
