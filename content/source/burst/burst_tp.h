#ifndef burst_tp_h
#define burst_tp_h
//#include "burst/burst_signal.h"

typedef enum{ BURST_TP_ON = 1,BURST_TP_OFF = 0} burst_status_t;
typedef struct  {
	int verb;
	burst_status_t status;
	void (* on)(int);
	void (* off)(int);
	void (* toggle)(int);
	void (* phy_on)(void);
	void (* phy_off)(void);
} burst_tp_t;
typedef burst_tp_t * burst_tp_p;

void burst_tp_on_(burst_tp_p _tp, int _verb);
void burst_tp_off_(burst_tp_p _tp, int _verb);
void burst_tp_toggle_(burst_tp_p _tp, int _verb);

#define BURST_TP( S ) BURST_TP_( S )
#define BURST_TP_( S ) \
extern burst_tp_t  S;

#define BURST_TP_CREATE( S ) BURST_TP_CREATE_( S )
#define BURST_TP_CREATE_( S ) \
BURST_WEAK  void S##_on(int _verb){\
	burst_tp_on_(&S, _verb);\
}\
BURST_WEAK  void S##_off(int _verb){\
	burst_tp_off_(&S, _verb);\
}\
BURST_WEAK  void S##_toggle(int _verb){\
	burst_tp_toggle_(&S, _verb);\
}\
BURST_WEAK  void S##_phy_on(void){\
}\
BURST_WEAK  void S##_phy_off(void){\
}\
burst_tp_t S ={ \
	0\
	, BURST_TP_OFF\
	, S##_on\
	, S##_off\
	, S##_toggle\
	, S##_phy_on\
	, S##_phy_off\
}; 

#endif
