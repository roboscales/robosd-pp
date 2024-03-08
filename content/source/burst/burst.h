#ifndef burst_h
#define burst_h

#if defined(__cplusplus)
extern "C"
{
#endif


#include "burst/burst_common.h"
#include "burst/burst_signal.h"

/**
@brief  Структура - описатель режима работы
*/

struct burst_dev_mode_s;
typedef struct burst_dev_mode_s burst_dev_mode_t;
typedef burst_dev_mode_t * burst_dev_mode_p;

typedef unsigned int burst_time_us_t;
typedef unsigned int burst_time_ms_t;

typedef struct  burst_hyst_s{
		burst_signal_t overhi;
		burst_signal_t hi;
		burst_signal_t lo;
		burst_signal_t ultralo;
} burst_hyst_t;

#ifndef BURST_PROTECTION_ENABLED
#define BURST_PROTECTION_ENABLED 0
#endif

typedef struct burst_config_s{
	int vercion;
	struct{
		#if BURST_PROTECTION_ENABLED == 1
		burst_time_us_t reset_timeout_us;
		#if BURST_PANICS_BOARD_TEMPER_ENABLED == 1 
		burst_hyst_t temp_pp;
		#endif
		#if BURST_PANICS_BOARD_VOLTAGE_ENABLED == 1 
		burst_hyst_t voltage_pp;
		#endif
		#endif
		#if BURST_PANICS_BOARD_CURRENT_ENABLED == 1 
		burst_signal_t overcurrent_pp;
		burst_signal_t locurrent_pp;
		#endif
	} panics;
} burst_config_t;

#ifndef BURST_VERCION
#define BURST_VERCION 0
#endif

#if BURST_PANICS_BOARD_TEMPER_ENABLED ==1 && BURST_PROTECTION_ENABLED == 1
#define BURST_PANICS_BOARD_TEMPER_CO()\
,{\
	BURST_PANICS_BOARD_TEMPER_OVERHI_PP\
	, BURST_PANICS_BOARD_TEMPER_HI_PP\
	, BURST_PANICS_BOARD_TEMPER_LO_PP\
	, BURST_PANICS_BOARD_TEMPER_ULTRALO_PP\
}
#else
#define BURST_PANICS_BOARD_TEMPER_CO()
#endif

#if BURST_PANICS_BOARD_VOLTAGE_ENABLED ==1 && BURST_PROTECTION_ENABLED == 1
#define BURST_PANICS_BOARD_VOLTAGE_CO()\
, {\
	BURST_PANICS_BOARD_VOLTAGE_OVERHI_PP\
	, BURST_PANICS_BOARD_VOLTAGE_HI_PP\
	, BURST_PANICS_BOARD_VOLTAGE_LO_PP\
	, BURST_PANICS_BOARD_VOLTAGE_ULTRALO_PP\
}
#else
#define BURST_PANICS_BOARD_VOLTAGE_CO()
#endif



#if BURST_PANICS_BOARD_CURRENT_ENABLED ==1 && BURST_PROTECTION_ENABLED == 1
	#define BURST_PANICS_BOARD_CURRENT_CO()\
		,BURST_PANICS_BOARD_OVERCURRENT_PP\
		,BURST_PANICS_BOARD_LOCURRENT_PP
#else
#define BURST_PANICS_BOARD_CURRENT_CO()
#endif



#define BURST_CONFIG() {\
	BURST_VERCION\
	,{\
		BURST_PANICS_BOARD_RESET_TIMEOUT_PP\
		BURST_PANICS_BOARD_TEMPER_CO()\
		BURST_PANICS_BOARD_VOLTAGE_CO()\
		BURST_PANICS_BOARD_CURRENT_CO()\
	}\
}

struct burst_dev_config_s{
	int tag;
	#if BURST_PANICS_MASTER_LOST_ENABLED == 1
	burst_time_us_t alive_period_us;
	#endif

};

typedef struct burst_dev_config_s burst_dev_config_t;
typedef burst_dev_config_t * burst_dev_config_p;

#if BURST_PANICS_MASTER_LOST_ENABLED == 1
#define 	BURST_PANICS_MASTER_LOST_CO(a) ,a##_ALIVE_PERIOD_US 
#else
#define 	BURST_PANICS_MASTER_LOST_CO(a)
#endif

#define DEV_CONFIG(a) DEV_CONFIG_(a)
#define DEV_CONFIG_(a)\
{\
	a##_TAG\
	BURST_PANICS_MASTER_LOST_CO(a)\
}



/**
@brief Структура. Базовое устройство.
*/
struct burst_dev_ref_s;
typedef struct burst_dev_ref_s burst_dev_ref_t;
typedef burst_dev_ref_t * burst_dev_ref_p;
typedef void ( * burst_dev_mode_event)(burst_dev_ref_p);

/*struct burst_dev_s{
	burst_dev_ref_p ref;
};
*/
struct  burst_dev_mode_s{
	burst_dev_mode_event applay_action;
	burst_dev_mode_event start;
	burst_dev_mode_event stop;
	burst_dev_mode_event loopA;
	burst_dev_mode_event loopB;
	burst_dev_mode_event loopC;
	burst_dev_mode_event frontend_loop;
};

typedef void ( * burst_dev_event)(burst_dev_ref_p);

typedef enum{bust_request_status_none = 0,bust_request_status_query=1,bust_request_status_confirm=2,bust_request_status_panic=-1} bust_request_status_t;
#if BURST_QUEUE_ENABLED == 1
typedef struct bust_request_s{
	bust_request_status_t status;
	void (* query)(struct bust_request_s *);
	void (* confirm)(struct bust_request_s *);
} bust_request_t;

typedef struct burst_dev_request_s{
	bust_request_t ref;
	burst_dev_ref_p owner;
	burst_dev_mode_event on_complete;
} burst_dev_request_t;

void bust_post(bust_request_t * _request);
#endif

struct burst_dev_ref_s{
	burst_dev_event reset;
	burst_dev_event start;
	burst_dev_event realtime_loop;
	burst_dev_event frontend_loop;
	struct{
		burst_dev_mode_event on_run;
		#if BURST_QUEUE_ENABLED == 0
		burst_dev_mode_event on_complete;
		bust_request_status_t status ;
		#else
		burst_dev_request_t request;
		#endif
	} update_feedback;
	burst_dev_event perform_panic;
	burst_dev_config_p config;
	burst_dev_action_p action;
	burst_dev_feedback_p feedback;
	int mode_count;
	burst_dev_mode_p * modes;	
	burst_dev_mode_p * modes_end;	
	burst_dev_mode_p actual_mode;
	#if BURST_PROTECTION_ENABLED == 1
	burst_dev_event realtime_protection;
	burst_dev_event frontend_protection;
	#endif
	int mode;	
	uint32_t panic;
	#if BURST_PANICS_MASTER_LOST_ENABLED == 1
	burst_time_us_t master_alive_tm;
	burst_bool_t master_exists;
	#endif

};

#if BURST_PROTECTION_ENABLED == 1
void burst_dev_realtime_protection(burst_dev_ref_p _ref);
void burst_dev_frontend_protection(burst_dev_ref_p _ref);
#endif

void burst_dev_idle_event(burst_dev_ref_p _ref);
void burst_dev_runA(burst_dev_ref_p _ref);
void burst_dev_runB(burst_dev_ref_p _ref);
void burst_dev_runC(burst_dev_ref_p _ref);

void burst_dev_attach(burst_dev_ref_p _ref);

extern burst_dev_mode_t burst_idle_mode;
void burst_config_set( burst_config_t *);
void burst_begin(void);
void burst_reset(void);
void burst_start(void);
void burst_realtime_loop(void);
void burst_backend_loop(void);
void burst_frontend_loop(void);

void burst_hw_on_crash(void);
void burst_sw_on_crash(const char * _file, const char * _function, int _line);
void burst_hw_reboot(void);
void burst_event_perform_panic(burst_dev_ref_p _dev);
void burst_board_raise_panic(uint32_t flag);
void burst_dev_raise_panic(burst_dev_ref_p _dev, uint32_t flag);
void burst_dev_reset_panic(burst_dev_ref_p _dev, uint32_t flag);
uint32_t burst_core_panics(void);
void burst_core_reset_panics(void);

#if BURST_PANICS_MASTER_LOST_ENABLED == 1
void burst_master_alive(burst_dev_ref_p _ref);
#endif

#if BURST_PANICS_BOARD_TEMPER_ENABLED == 1
int burst_board_temper_get_pp(void);
#endif

#if BURST_PANICS_BOARD_VOLTAGE_ENABLED == 1 
int burst_board_voltage_get_pp(void);
#endif

#if BURST_PANICS_BOARD_CURRENT_ENABLED == 1
int burst_board_current_get_pp(void);
#endif

#ifndef burst_crash
#define burst_crash()  { burst_hw_on_crash(); burst_sw_on_crash(BURST_PROC_FILE,BURST_PROC_NAME,BURST_PROC_LINE); }
#endif

#ifndef burst_alarm
#define burst_alarm(x)  if(!(x)){ burst_crash(); };
#endif

#ifndef BURST_DEBUG_TP_ENABLED
#define BURST_DEBUG_TP_ENABLED 0
#endif

#if BURST_DEBUG_TP_ENABLED == 1

#define CLCH_NAME burst_tp
#define CLCH_HEADER 
#include "burst/cliche/tp.h"

#define debug_tp_on(n)  burst_tp_on(n)
#define debug_tp_off(n)  burst_tp_off(n)
#define debug_set_verb(n)  burst_tp_verb_set(n)

#else

#define debug_tp_on(n)
#define debug_tp_off(n)
#define debug_set_verb(n) 

#endif

#if defined(__cplusplus)
}
#endif



void burst_query_feedback(burst_dev_ref_p _ref, burst_dev_mode_event _on_complete);

#ifndef BURST_STRING_HASH_ENABLED
#define BURST_STRING_HASH_ENABLED 0
#endif
#if BURST_STRING_HASH_ENABLED
int32_t burst_string_hash(const char * _str, int _start);
#endif

#endif
