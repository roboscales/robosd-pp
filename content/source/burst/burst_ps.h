#ifndef mexo_ps_hpp
#define mexo_ps_hpp
#include "burst/burst.h"
#include "burst/burst_signal.h"
typedef enum {
	burst_ps_command_off = 0
	, burst_ps_command_on = 1
} burst_ps_command_t;
typedef enum {
	 burst_ps_status_unknown = 0
	, burst_ps_status_off = 1
	, burst_ps_status_boot = 2
	, burst_ps_status_on = 3
	, burst_ps_status_shutdown = 4
} burst_ps_status_t;

typedef struct  {
} burst_ps_config_t;
typedef burst_ps_config_t * burst_ps_config_p;

typedef struct {
	burst_ps_command_t command;
	burst_ps_status_t status;
	burst_ps_config_p config;
	burst_satstate_t satstate;
	burst_bool_t ( *active)(void);
	void ( *begin)(burst_ps_config_p);
	void ( *run)(void);
	void ( *boot_begin)(void);
	burst_bool_t ( *do_boot)(void);
	void ( *boot_complete)(void);
	burst_satstate_t ( *do_invert)(void);
	void ( *shutdown_begin)(void);
	burst_bool_t ( *do_shutdown)(void);
	void ( *shutdown_complete)(void);
} burst_ps_t;
typedef burst_ps_t * burst_ps_p;

burst_bool_t burst_ps_active_(burst_ps_p);// { return status_ == status::on; }
void burst_ps_begin_(burst_ps_p, burst_ps_config_p);// {}
void burst_ps_run_(burst_ps_p);// {}


#define BURST_PS( S ) BURST_PS_( S )
#define BURST_PS_( S ) \
extern burst_ps_t  S;

#define BURST_PS_CREATE( S ) BURST_PS_CREATE_( S )
#define BURST_PS_CREATE_( S ) \
BURST_WEAK  burst_bool_t S##_active(void){\
	return burst_ps_active_(&S);\
}\
BURST_WEAK  void S##_begin(burst_ps_config_p _config){\
	burst_ps_begin_(&S,_config);\
}\
BURST_WEAK  void S##_run(void){\
	return burst_ps_run_(&S);\
}\
BURST_WEAK  void S##_boot_begin(void){\
}\
BURST_WEAK  burst_bool_t S##_do_boot(void){\
	return burst_true;\
}\
BURST_WEAK  void S##_boot_complete(void){\
}\
BURST_WEAK  burst_satstate_t S##_do_invert(void){\
	return burst_satstate_both; \
}\
BURST_WEAK  void S##_shutdown_begin(void){\
}\
BURST_WEAK  burst_bool_t S##_do_shutdown(void){\
	return burst_true;\
}\
BURST_WEAK  void S##_shutdown_complete(void){\
}\
burst_ps_t S ={ \
	burst_ps_command_off \
	, burst_ps_status_unknown \
	, 0 \
	, burst_satstate_both \
	, S##_active \
	, S##_begin \
	, S##_poll \
	, S##_boot_begin \
	, S##_do_boot \
	, S##_boot_complete \
	, S##_do_invert \
	, S##_shutdown_begin \
	, S##_do_shutdown \
	, S##_shutdown_complete \
};

typedef struct  {
	burst_ps_config_t ps;
	burst_range_t duty;			
	burst_range_t voltage;			
} burst_ps_dc_config_t;
typedef burst_ps_dc_config_t * burst_ps_dc_config_p;


typedef struct {
	burst_ps_t ps;
	burst_scaler_t scaler;
	burst_signal_t * required;
	burst_signal_t standalone;
	burst_signal_t duty;
	void ( *begin)(burst_ps_dc_config_p);
	void ( *set)(burst_signal_t );
} burst_ps_dc_t ;
typedef burst_ps_dc_t * burst_ps_dc_p;


#define BURST_PS_DC( S ) BURST_PS_DC_( S )
#define BURST_PS_DC_( S ) \
extern burst_ps_dc_t  S;

void burst_ps_dc_begin_(burst_ps_dc_p,burst_ps_dc_config_p );
burst_satstate_t burst_ps_dc_do_invert_(burst_ps_dc_p);

#define BURST_PS_DC_CREATE( S ) BURST_PS_DC_CREATE_( S )
#define BURST_PS_DC_CREATE_( S ) \
BURST_WEAK  burst_bool_t S##_active(void){\
	return burst_ps_active_(&S.ps);\
}\
BURST_WEAK  void S##_ps_begin(burst_ps_config_p _config){ \
	burst_ps_begin_(&S.ps,_config); \
}\
BURST_WEAK  void S##_begin(burst_ps_dc_config_p _config){ \
	burst_ps_dc_begin_(&S,_config); \
}\
BURST_WEAK  void S##_run(void){\
	return burst_ps_run_(&S.ps);\
}\
BURST_WEAK  void S##_boot_begin(void){\
}\
BURST_WEAK  burst_bool_t S##_do_boot(void){\
	return burst_true;\
}\
BURST_WEAK  void S##_boot_complete(void){\
}\
BURST_WEAK  burst_satstate_t S##_do_invert(void){\
	return burst_ps_dc_do_invert_(&S);\
}\
BURST_WEAK  void S##_shutdown_begin(void){\
}\
BURST_WEAK  burst_bool_t S##_do_shutdown(void){\
	return burst_true;\
}\
BURST_WEAK  void S##_shutdown_complete(void){\
}\
BURST_WEAK  void S##_set(burst_signal_t _duty){\
}\
burst_ps_dc_t S ={ \
	{ \
		burst_ps_command_off \
		, burst_ps_status_unknown \
		, 0 \
		, burst_satstate_both \
		, S##_active \
		, S##_ps_begin \
		, S##_run \
		, S##_boot_begin \
		, S##_do_boot \
		, S##_boot_complete \
		, S##_do_invert \
		, S##_shutdown_begin \
		, S##_do_shutdown \
		, S##_shutdown_complete \
	} \
	,{ \
	} \
	, 0 \
	, 0 \
	, 0 \
	, S##_begin \
	, S##_set\
};
#endif
