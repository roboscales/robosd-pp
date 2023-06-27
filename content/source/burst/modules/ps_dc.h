#ifndef ps_dc_hpp
#define ps_dc_hpp
#include "burst/burst_ps.h"

typedef struct  ps_dc_config_s{
	burst_ps_config_t machine;
	burst_range_t duty;			
	burst_range_t voltage;			
} ps_dc_config_t;
typedef ps_dc_config_t * ps_dc_config_p;


typedef struct {
	burst_ps_t machine;
	burst_scaler_t scaler;
	burst_signal_p required;
	burst_signal_t duty;
	void ( *setup)(burst_signal_p _required);
	void ( *set)(burst_signal_t );
} ps_dc_t ;
typedef ps_dc_t * ps_dc_p;


#define BURST_PS_DC( S ) BURST_PS_DC_( S )
#define BURST_PS_DC_( S ) \
extern ps_dc_t  S;

void ps_dc_begin(ps_dc_p,ps_dc_config_p );
burst_satstate_t ps_dc_do_invert_(ps_dc_p);
void ps_dc_setup_(ps_dc_p, burst_signal_p _required);

#define ps_dc_impl( S, D ) \
BURST_WEAK  burst_bool_t S##_active(void){\
	return burst_ps_active_(&D.machine);\
}\
BURST_WEAK  void S##_run(void){\
	burst_ps_run_(&D.machine);\
}\
BURST_WEAK  void S##_boot_begin(void){\
}\
BURST_WEAK  burst_bool_t S##_do_boot(void){\
	return burst_true;\
}\
BURST_WEAK  void S##_boot_complete(void){\
}\
BURST_WEAK  burst_satstate_t S##_do_invert(void){\
	return ps_dc_do_invert_(&D);\
}\
BURST_WEAK  void S##_shutdown_begin(void){\
}\
BURST_WEAK  burst_bool_t S##_do_shutdown(void){\
	return burst_true;\
}\
BURST_WEAK  void S##_shutdown_complete(void){\
}\
BURST_WEAK  void S##_setup(burst_signal_p _required){\
	ps_dc_setup_(&D,_required);\
}\
BURST_WEAK  void S##_set(burst_signal_t _duty){\
}

#define ps_dc_setup( S )\
{ \
	{ \
		burst_ps_command_off \
		, burst_ps_status_unknown \
		, 0 \
		, burst_satstate_both \
		, S##_active \
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
	, S##_setup\
	, S##_set\
}

#define PS_DC( S ) BURST_OBJECT(ps_dc,S)

#define PS_DC_CREATE( S ) BURST_OBJECT_CREATE(ps_dc,S)

#define PS_DC_SUBCREATE( S,P ) BURST_OBJECT_SUBCREATE(ps_dc,S,P)

#define PS_DC_SUBSETUP( S,P )  BURST_OBJECT_SUBSETUP(ps_dc,S,P)


#define PS_DC_CONFIG(a) PS_DC_CONFIG_(a)
#define PS_DC_CONFIG_(a)\
{\
	{}\
	, RANGE_CONFIG(a##_DUTY_LIM)\
	, RANGE_CONFIG(a##_VOLTAGE_LIM)\
}

#endif
