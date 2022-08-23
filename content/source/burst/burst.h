#ifndef burst_h
#define burst_h

#if defined(__cplusplus)
extern "C"
{
#endif

#include "burst_app_tuning.h"
#include "burst_target.h"
#include "burst_front.h"


/**
@brief  Структура - описатель режима работы
*/
struct burst_dev_s;
typedef struct burst_dev_s burst_dev_t;
typedef burst_dev_t * burst_dev_p;

struct burst_dev_mode_s;
typedef struct burst_dev_mode_s burst_dev_mode_t;
typedef burst_dev_mode_t * burst_dev_mode_p;
typedef void ( * burst_dev_mode_event)(burst_dev_p);

struct burst_dev_config_s{
	int tag;
};
typedef struct burst_dev_config_s burst_dev_config_t;
typedef burst_dev_config_t * burst_dev_config_p;

struct burst_dev_present_s{
	int mode;
};
typedef struct burst_dev_present_s burst_dev_present_t;
typedef burst_dev_present_t * burst_dev_present_p;

/**
@brief Структура. Базовое устройство.
*/
struct burst_dev_ref_s;
typedef struct burst_dev_ref_s burst_dev_ref_t;
typedef burst_dev_ref_t * burst_dev_ref_p;

struct burst_dev_s{
	burst_dev_ref_p ref;
};

struct  burst_dev_mode_s{
	burst_dev_mode_event applay_action;
	burst_dev_mode_event start;
	burst_dev_mode_event stop;
	burst_dev_mode_event loopA;
	burst_dev_mode_event loopB;
	burst_dev_mode_event loopC;
	burst_dev_mode_event frontend_loop;
	burst_dev_p dev;
};

typedef void ( * burst_dev_event)(burst_dev_p);

struct burst_dev_ref_s{
	burst_dev_event begin;
	burst_dev_event start;
	burst_dev_event prioritet_loop;
	burst_dev_event frontend_loop;
	burst_dev_config_p config;
	burst_dev_action_p action;
	burst_dev_feedback_p feedback;
	burst_dev_present_p present;
	int mode_count;
	burst_dev_mode_p * modes;	
	burst_dev_mode_p * modes_end;	
	burst_dev_p dev;
	burst_dev_mode_p actual_mode;
};
void burst_dev_idle_mode_event(burst_dev_p _mode);
void burst_dev_runA(burst_dev_ref_p _ref);
void burst_dev_runB(burst_dev_ref_p _ref);
void burst_dev_runC(burst_dev_ref_p _ref);

void burst_begin(void);
void burst_start(void);
void burst_prioritet_loop(void);
void burst_backend_loop(void);
void burst_frontend_loop(void);

typedef enum  { burst_true = 1, burst_false = 0 } burst_bool_t;
typedef enum  { burst_backend = 1, burst_frontend = 0 } burst_thread_t;
typedef enum  { burst_complete = 1, burst_panic = 0, burst_fault = 3, burst_continue =2 } burst_run_t;

burst_thread_t burst_thread(void);

typedef unsigned burst_size_t;

#if defined(__cplusplus)
}
#endif

#endif
