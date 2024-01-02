#include "burst/burst_common.h"

#include "burst/cliche/_begin.h"
#ifdef CLCH_HEADER
	void PREFIX(on)(int _verb);
	void PREFIX(off)(int _verb);
	burst_bool_t PREFIX(get)(void);
	void PREFIX(verb_set)(int _verb);
	int PREFIX(verb_get)(void);
	void PREFIX(toggle)(int);
	void PREFIX(phy_on)(void);
	void PREFIX(phy_off)(void);
#else

burst_bool_t PREFIX(status);

BURST_WEAK void PREFIX(phy_on)(void){}
BURST_WEAK void PREFIX(phy_off)(void){}
burst_bool_t PREFIX(get)(void){
	return PREFIX(status);
}
void PREFIX(on)(int _verb){
	PREFIX(status) = burst_true;
	PREFIX(phy_on)();
}
void PREFIX(off)(int _verb){
	PREFIX(status) = burst_false;
	PREFIX(phy_off)();
}
void PREFIX(toggle)(int _verb){
	if( PREFIX(status) == burst_false){
		PREFIX(phy_on)();
		PREFIX(status) = burst_true;
	} else {
		PREFIX(status) = burst_false;
		PREFIX(phy_off)();
	}
}



#endif
#include "burst/cliche/_end.h"
