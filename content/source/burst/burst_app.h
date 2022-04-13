#ifndef burst_app_h
#define burst_app_h

#if defined(__cplusplus)
extern "C"
{
#endif

#include "burst/burst.h"

#ifndef burst_alarm
#define burst_alarm(x)  if(!(x)) burst_hw_crash(BURST_PROC_FILE,BURST_PROC_NAME,BURST_PROC_LINE);
#endif

void burst_hw_crash(const char * _file, const char * _function, int _line);

void burst_hw_fall(void);
void burst_hw_comeback(void);

void * burst_hw_critical_enter(void);
void burst_hw_critical_leave(void* _context);

void * burst_hw_guard_enter(void);
void burst_hw_guard_leave(void* _context);

void burst_hw_guard_lock(void);
void burst_hw_guard_unlock(void);

void burst_sw_begin(void);
void burst_sw_start(void);
void burst_sw_prioritet_loop(void);
void burst_sw_backend_loop(void);
void burst_sw_frontend_loop(void);

void burst_hw_begin(void);
void burst_hw_start(void);
void burst_hw_prioritet_loop(void);
void burst_hw_backend_loop(void);
void burst_hw_frontend_loop(void);

#ifndef BURST_SLOT_COUNT
#define BURST_SLOT_COUNT 4
#endif

#if BURST_SLOT_COUNT > 0
void burst_sw_slot_0(void);
void burst_hw_slot_0(void);
#endif

#if BURST_SLOT_COUNT > 1
void burst_sw_slot_1(void);
void burst_hw_slot_1(void);
#endif

#if BURST_SLOT_COUNT > 2
void burst_sw_slot_2(void);
void burst_hw_slot_2(void);
#endif

#if BURST_SLOT_COUNT > 3
void burst_sw_slot_3(void);
void burst_hw_slot_3(void);
#endif

#if BURST_SLOT_COUNT > 4
void burst_sw_slot_4(void);
void burst_hw_slot_4(void);
#endif

#if BURST_SLOT_COUNT > 5
void burst_sw_slot_5(void);
void burst_hw_slot_5(void);
#endif

#if BURST_SLOT_COUNT > 6
void burst_sw_slot_6(void);
void burst_hw_slot_6(void);
#endif

#if BURST_SLOT_COUNT > 7
void burst_sw_slot_7(void);
void burst_hw_slot_7(void);
#endif


#if BURST_SLOT_COUNT > 8
void burst_sw_slot_8(void);
void burst_hw_slot_8(void);
#endif

#if BURST_SLOT_COUNT > 9
void burst_sw_slot_9(void);
void burst_hw_slot_9(void);
#endif

#if BURST_SLOT_COUNT > 10
void burst_sw_slot_10(void);
void burst_hw_slot_10(void);
#endif

#if BURST_SLOT_COUNT > 11
void burst_sw_slot_11(void);
void burst_hw_slot_11(void);
#endif

#if BURST_SLOT_COUNT > 12
void burst_sw_slot_12(void);
void burst_hw_slot_12(void);
#endif

#if BURST_SLOT_COUNT > 13
void burst_sw_slot_13(void);
void burst_hw_slot_13(void);
#endif

#if BURST_SLOT_COUNT > 14
void burst_sw_slot_14(void);
void burst_hw_slot_14(void);
#endif

#if BURST_SLOT_COUNT > 15
void burst_sw_slot_15(void);
void burst_hw_slot_15(void);
#endif

#if defined(__cplusplus)
}
#endif

#endif
