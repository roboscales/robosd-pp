#ifndef drv8301_h
#define drv8301_h
#include <stdint.h>
#include "drv8301_reg.h"

#ifdef __cplusplus
	extern "C" {
#endif	

typedef struct drv8301_s{
	drv8301_regs_t actual;
	drv8301_regs_t deseired;
	drv8301_poll_status_t poll_status;
	void ( *cs_on)(void);
	void ( *cs_off)(void);
	void ( *put)(uint16_t);
	uint16_t ( *get)(void);
	int ( *complete)(void);
	void ( *on_status_receive)(void);
} drv8301_t;

typedef drv8301_t * drv8301_p;

void drv8301_exchange(drv8301_p _drv, uint16_t _deseired, uint16_t * _actual);

void drv8301_begin(drv8301_p _drv, drv8301_config_p _config);
void drv8301_set_default(drv8301_config_p _config);
void drv8301_status_poll(drv8301_p _drv);
void drv8301_phy_delay_ns(unsigned int _ns);
void drv8301_phy_crash(void);


#define BURST_DRV8301( S ) BURST_DRV8301_( S )
#define BURST_DRV8301_( S ) \
extern drv8301_t  S;

#define BURST_DRV8301_CREATE( S ) BURST_DRV8301_CREATE_( S )
#define BURST_DRV8301_CREATE_( S ) \
BURST_WEAK int					S##_complete(void){ return burst_false; }\
BURST_WEAK void					S##_put(uint16_t _data){ BURST_UNUSED(_data); }\
BURST_WEAK uint16_t			S##_get(void ){ return  0xFFFF;}\
BURST_WEAK void					S##_cs_on(void){}\
BURST_WEAK void					S##_cs_off(void){}\
BURST_WEAK void					S##_on_status_receive(void){}\
drv8301_t  S ={ \
	{}\
	,{}\
	,DRV8301_POLL_STATUS_NONE \
	, S##_cs_on \
	, S##_cs_off \
	, S##_put \
	, S##_get \
	, S##_complete \
	, S##_on_status_receive \
};

#ifdef __cplusplus
}  /* extern "C" */
#endif
	
#endif
