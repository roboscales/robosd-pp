#ifndef clch_ps_h
#define clch_ps_h
#include "burst/burst_common.h"
typedef enum {
	clch_ps_command_off = 0
	, clch_ps_command_on = 1
} clch_ps_command_t;
typedef enum {
	 clch_ps_status_unknown = 0
	, clch_ps_status_off = 1
	, clch_ps_status_boot = 2
	, clch_ps_status_on = 3
	, clch_ps_status_shutdown = 4
} clch_ps_status_t;
#endif
#if defined(CLCH_NAME) | 1
#include "burst/cliche/_begin.h"

#ifdef CLCH_HEADER
	void PREFIX(run)(void);
	void PREFIX(boot_begin)(void);
	burst_bool_t PREFIX(do_boot)(void);
	void PREFIX(boot_complete)(void);
	burst_satstate_t PREFIX(do_invert)(void);
	void PREFIX(shutdown_begin)(void);
	burst_bool_t PREFIX(do_shutdown)(void);
	void PREFIX(shutdown_complete)(void);
	extern int PREFIX(status);
	extern clch_ps_command_t PREFIX(command);
	extern burst_satstate_t	PREFIX(satstate);
#else

	BURST_WEAK void PREFIX(boot_begin)(void){
	}
	BURST_WEAK burst_bool_t PREFIX(do_boot)(void){
		return burst_true;
	}
	BURST_WEAK void PREFIX(boot_complete)(void){
	}
	BURST_WEAK burst_satstate_t PREFIX(do_invert)(void){
		return burst_satstate_both;
	}
	BURST_WEAK void PREFIX(shutdown_begin)(void){
	}
	BURST_WEAK burst_bool_t PREFIX(do_shutdown)(void){
		return burst_true;
	}
	BURST_WEAK void PREFIX(shutdown_complete)(void){
	}


int PREFIX(status);
clch_ps_command_t PREFIX(command);
burst_satstate_t	PREFIX(satstate);

void PREFIX(run)(void){
	switch (PREFIX(status)) {
		case clch_ps_status_unknown:
			break;
		case clch_ps_status_off:
			if (PREFIX(command) == clch_ps_command_on) {
				PREFIX(boot_begin)();
				PREFIX(status) = clch_ps_status_boot;
			}		else {
				break;
			}

		case clch_ps_status_boot:
			if (PREFIX(do_boot())) {
				PREFIX(satstate) = burst_satstate_none;
				PREFIX(boot_complete)();
				PREFIX(satstate) =PREFIX(do_invert());
				PREFIX(status) = clch_ps_status_on;
				return;
			}	else {
				break;
			}
			
		case clch_ps_status_on:
			if (PREFIX(command) == clch_ps_command_on) {
				PREFIX(satstate)=PREFIX(do_invert());
				return;
			}	else {
				PREFIX(satstate) = burst_satstate_both;
				PREFIX(status) = clch_ps_status_shutdown;
				PREFIX(shutdown_begin)();
			}
			
		case clch_ps_status_shutdown:
			if (PREFIX(do_shutdown)()) {
				PREFIX(shutdown_complete)();
				PREFIX(status) = clch_ps_status_off;
			} else {
					break;
			}
	}
}

#endif
#include "burst/cliche/_end.h"
#endif
