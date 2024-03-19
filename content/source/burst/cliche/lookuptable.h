#include "burst/burst_common.h"
#include "burst/burst_signal.h"
#include "burst/cliche/_begin.h"
#ifdef CLCH_HEADER
	burst_signal_t PREFIX(from_pp)(int16_t _x);
#else
#if PREFIX(FROM_PP_COUNT)>1
burst_signal_t PREFIX(from_pp_gain_16)[PREFIX(FROM_PP_COUNT)] = { PREFIX(FROM_PP_GAIN_16) };
burst_signal_t PREFIX(from_pp_offset)[PREFIX(FROM_PP_COUNT)] = { PREFIX(FROM_PP_OFFSET) };
burst_signal_t PREFIX(from_pp)(int16_t _x){
	_x -= PREFIX(FROM_PP_ZERO);
	int ix =  (_x>>PREFIX(FROM_PP_SCALE_BITS)) - 1;
	if(ix<0){
		ix = 0;
	} else {
		if(ix>=PREFIX(FROM_PP_COUNT)){
			ix=PREFIX(FROM_PP_COUNT)-1;
		}
	}
	burst_signal_t g = PREFIX(from_pp_gain_16)[ix];
	burst_signal_t ofs = PREFIX(from_pp_offset)[ix];
	return (burst_signal_t)(((burst_long_signal_t)(_x-ofs)*g + 32767) >> 16);
}
#else
burst_signal_t PREFIX(from_pp)(int16_t _x){
	return (burst_signal_t)( ((burst_long_signal_t)(_x-PREFIX(FROM_PP_OFFSET))*PREFIX(FROM_PP_GAIN_16) + 32767) >> 16);
}
#endif
#endif

#include "burst/cliche/_end.h"
