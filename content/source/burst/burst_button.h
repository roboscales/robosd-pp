#ifndef burst_btn_h
#define burst_btn_h
#include "burst/burst_timer.h"
#ifndef BURST_BUTTON_ENABLED
#define BURST_BUTTON_ENABLED 0
#endif

#if BURST_BUTTON_ENABLED == 1
typedef enum{ BURST_BTN_PRESSED = 1,BURST_BTN_RELEASED = 0} burst_btn_status_t;
typedef enum{ BURST_BTN_FIXED = 1,BURST_BTN_NORMAL = 0} burst_btn_mode_t;
typedef struct  {
	burst_btn_status_t status;
	burst_btn_mode_t mode;
	void (* begin)(burst_btn_mode_t _mode, burst_time_us_t _permit_us, burst_time_us_t _repeate_us);
	void (* on_raise)(void);
	void (* on_release)(void);
	burst_btn_status_t (* phy_get)(void);
	burst_time_us_t last_us;
	burst_time_us_t permit_us;
	burst_time_us_t pressed_us;
	burst_time_us_t pushdown_us;
	burst_time_us_t repeate_us;
	burst_time_us_t last_repeate_us;
} burst_btn_t;
typedef burst_btn_t * burst_btn_p;

void burst_btn_begin(burst_btn_p _btn, burst_btn_mode_t _mode, burst_time_us_t _permit_us, burst_time_us_t _repeate_us );
void burst_btn_poll(void);

#define BURST_BTN( S ) BURST_BTN_( S )
#define BURST_BTN_( S ) \
extern burst_btn_t  S;

#define BURST_BTN_CREATE( S ) BURST_BTN_CREATE_( S )
#define BURST_BTN_CREATE_( S ) \
BURST_WEAK void S##_begin(burst_btn_mode_t _mode, burst_time_us_t _permit_us, burst_time_us_t _repeate_us ){\
	burst_btn_begin(&S,_mode,_permit_us,_repeate_us);\
}\
BURST_WEAK  void S##_on_raise(void){\
}\
BURST_WEAK  void S##_on_release(void){\
}\
BURST_WEAK burst_btn_status_t  S##_phy_get(void){\
	return BURST_BTN_RELEASED;\
}\
burst_btn_t S ={ \
	BURST_BTN_RELEASED\
	, BURST_BTN_NORMAL\
	, S##_begin\
	, S##_on_raise\
	, S##_on_release\
	, S##_phy_get\
}; 

#endif
#endif
