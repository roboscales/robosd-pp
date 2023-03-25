#ifndef __burst_signal_h
#define __burst_signal_h

#include "burst/burst_common.h"
#include <stdint.h>
#ifndef burst_signal_t
#define burst_signal_t int16_t
#define BURST_SIGNAL_MAX 32767
#define BURST_SIGNAL_MIN (-32767)


typedef  uint16_t burst_usignal_t;
#define BURST_USIGNAL_HALF_POS 0x3FFF 
#define BURST_USIGNAL_HALF_NEG 0xC001 
#endif

#ifndef burst_long_signal_t
#define burst_long_signal_t  int32_t
typedef  uint32_t burst_ulong_signal_t;
#define BURST_LONG_SIGNAL_MAX 2147483647L
#define BURST_LONG_SIGNAL_MIN (-2147483647L)
#define BURST_ULONG_SIGNAL_HALF_POS 0x3FFFFFFFL
#define BURST_ULONG_SIGNAL_HALF_NEG 0xC0000001L 
#endif


#ifndef burst_frac4_t
#define burst_frac4_t uint8_t
#endif
#ifndef burst_frac8_t
#define burst_frac8_t uint8_t
#endif


#define burst_signal_p burst_signal_t *
#define burst_long_signal_p burst_long_signal_t *
#define burst_ulong_signal_p burst_ulong_signal_t *

#ifndef burst_parametr_t
#define burst_parametr_t int16_t
#endif
#define burst_parametr_p burst_parametr_t *

#define BURST_ROUND_POS(f) ((f<BURST_SIGNAL_MAX)?(f+0.5):BURST_SIGNAL_MAX)
#define BURST_ROUND_NEG(f) ((f>BURST_SIGNAL_MIN)?(f-0.5):BURST_SIGNAL_MIN)
#define BURST_ROUND_POS32(f) ((f<BURST_LONG_SIGNAL_MAX)?(f+0.5):BURST_LONG_SIGNAL_MAX)
#define BURST_ROUND_NEG32(f) ((f>BURST_LONG_SIGNAL_MIN)?(f-0.5):BURST_LONG_SIGNAL_MIN)

#ifndef BURST_SIGNAL_T
#define BURST_SCALE_POS16(x) (((x)<1.0) ? (x*BURST_SIGNAL_MAX):BURST_SIGNAL_MAX)
#define BURST_SCALE_NEG16(x) (((x)>-1.0) ? ((-(x) )*(BURST_SIGNAL_MIN)) :(BURST_SIGNAL_MIN))
#define BURST_SIGNAL_T(x)  (burst_signal_t)( (x>0)? BURST_ROUND_POS(BURST_SCALE_POS16(x)) : BURST_ROUND_NEG(BURST_SCALE_NEG16(x)) )
#endif

#ifndef BURST_LONG_SIGNAL_T
#define BURST_SCALE_POS32(x) ((x<1.0) ? (x*BURST_LONG_SIGNAL_MAX):BURST_LONG_SIGNAL_MAX)
#define BURST_SCALE_NEG32(x) ((x>-1.0) ? ((-x)*(BURST_SIGNAL_MIN)) :(BURST_SIGNAL_MIN))
#define BURST_LONG_SIGNAL_T(x)  (burst_long_signal_t)( (x>0)? BURST_ROUND_POS32(BURST_SCALE_POS32(x)) : BURST_ROUND_NEG32(BURST_SCALE_NEG32(x)) )
#endif

#ifndef BURST_OFFSET_OF
#define BURST_OFFSET_OF(t, memb) ((size_t)(&(((t *)0)->memb)))
#endif

#ifndef BURST_SATURATE
#define BURST_SATURATE(x,Lo,Hi) if(x<Lo){x=Lo;}else if (x>Hi) {x=Hi;}
#endif

#ifndef BURST_LONG_TO_SIGNAL
#define BURST_LONG_TO_SIGNAL(x) ((burst_signal_t)(x))
#endif

#ifndef BURST_SIGNAL_TO_LONG
#define BURST_SIGNAL_TO_LONG(x) ((burst_long_signal_t)(x))
#endif

#ifndef BURST_RIGHT_SHIFT
#define BURST_RIGHT_SHIFT(x,n) ( (x)>0 ? ((x)>>(n)) :  (-( (-(x))>>(n))) ) 
#endif
#define BURST_RSH BURST_RIGHT_SHIFT

#ifndef BURST_LEFT_SHIFT
#define BURST_LEFT_SHIFT(x,n) ( (x)>0 ? ((x)<<(n)) :  (-( (-(x))<<(n))) ) 
#endif
#define BURST_LSH(x) BURST_LEFT_SHIFT(x)


#ifndef BURST_SIGN
#define BURST_SIGN(x) (((x)==0)? 0 : ((x)<0?-1:1))
#endif

#ifndef BURST_RELE
#define BURST_RELE(x,n)   (((x)<(-(n))) ? -1 : ( ((x)>(n)) ? 1 : 0 ))
#endif

#ifndef BURST_ABS
#define BURST_ABS(x) (((x)==0)? 0 : ((x)<0?-(x):(x)))
#endif

#ifndef BURST_SAT
#define BURST_SAT(x,Lo,Hi) ( ((x)<(Lo))? { (Lo)} : (((x)>(Hi))? (Hi} : (x) ) ) )
#endif

#define signal2ph_t struct signal2ph_s
#define signal2ph_p signal2ph_t *
struct signal2ph_s{
	burst_signal_t cross;
	burst_signal_t lateral;
};

#define rot_t struct rot_s
#define rot_p rot_t *
struct rot_s{
	burst_signal_t cs;
	burst_signal_t sn;
};

#define signal3ph_t struct signal3ph_s
#define signal3ph_p signal3ph_t *
struct signal3ph_s{
	burst_signal_t A;
	burst_signal_t B;
	burst_signal_t C;
};


#define burst_long_signal_ut union burst_long_signal_u
#define burst_long_signal_up burst_long_signal_ut *
union burst_long_signal_u{
	burst_long_signal_t value;
	struct{
		uint16_t first; 
		burst_signal_t second; 
	};
}; 

typedef union  scale_gain_16_s{
	struct{
		burst_frac4_t value : 4;
		burst_frac4_t unused : 4;
	};
	burst_data_t byte;
} scale_gain_16_t;

typedef burst_frac8_t scale_gain_256_t;


burst_signal_t s_mult(burst_signal_t x1 , burst_signal_t x2);
burst_signal_t s_add(burst_long_signal_up acc, burst_signal_t x1 , burst_signal_t x2);

signal2ph_p abc2ab(signal3ph_p , signal2ph_p );
signal3ph_p ab2abc(signal2ph_p , signal3ph_p );
signal2ph_p rotate(signal2ph_p, rot_p );
rot_p rotcalc( rot_p rot, burst_signal_t phi );
burst_signal_t s_extract(burst_long_signal_up v);
signal2ph_p transform(signal2ph_p src, signal2ph_p dst, rot_p rot);
signal2ph_p transform2(burst_signal_t _lat, burst_signal_t _cross, signal2ph_p dst, rot_p rot);

burst_signal_t s_inc(burst_signal_t val, burst_signal_t x, burst_signal_t _min,burst_signal_t _max);

burst_signal_t s_scale_16(burst_signal_t _val, scale_gain_16_t _gain);
burst_signal_t s_scale_256(burst_signal_t _val, scale_gain_256_t _gain);

BURST_STATIC_INLINE burst_signal_t s_mult_floor(burst_signal_t x1, burst_signal_t x2){
	burst_long_signal_ut tmp;
	tmp.value = BURST_LEFT_SHIFT((burst_long_signal_t)x1,1) * x2;
	return tmp.second;
}

#define EXTENDED_SQRT3                  1.7320508075688772935274463415059
#define SQRT3           				BURST_SIGNAL_T(EXTENDED_SQRT3)
#define ONE_DIV_SQRT3           BURST_SIGNAL_T(1.0/EXTENDED_SQRT3)
#define SQRT3_DIV_2 						BURST_SIGNAL_T(EXTENDED_SQRT3/2)


typedef struct {
	burst_signal_t lo;
	burst_signal_t hi;
} burst_range_t;
typedef burst_range_t * burst_range_p;


typedef struct {
	burst_range_t in_range;
	burst_range_t out_range;
	burst_long_signal_t gain;
} burst_scaler_t;
typedef burst_scaler_t * burst_scaler_p;

void burst_scaler_begin(burst_scaler_p,burst_range_p _in_range,burst_range_p _out_range);
burst_satstate_t burst_scaler_run(burst_scaler_p, burst_signal_t _in, burst_signal_t * _out);

#endif
