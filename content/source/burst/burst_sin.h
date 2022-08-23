#ifndef __burst_sin_h
#define __burst_sin_h
#include "burst/burst_signal.h"
#define PI BURST_SIGNAL_MAX
#define PI_DIV_2 ((BURST_SIGNAL_MAX+1)>>1)
#define PI_DIV_4 (BURST_SIGNAL_MAX>>2)

burst_signal_t burst_sin(burst_signal_t);
burst_signal_t burst_cos(burst_signal_t);
burst_signal_t burst_atan2(burst_signal_t y, burst_signal_t x);

#endif
