#ifndef __burst_serial_h
#define __burst_serial_h
#include "burst/burst.h"
#include <stdint.h>
typedef struct burst_serial_s {
	burst_size_t  (*available)(void);
	burst_size_t  (*space)(void);
	burst_size_t (*getb)(uint8_t* , burst_size_t);
	burst_bool_t (*putb)(const uint8_t*, burst_size_t);
	uint8_t (*get)(void);
	void (*put)(uint8_t _data);
	burst_bool_t (*begin)(void);
	burst_bool_t (*start)(void);
	void (*reset)(void);
	void (*finish)(void);
} burst_serial_t;

typedef burst_serial_t * burst_serial_p;

#define BURST_SERIAL( S ) BURST_SERIAL_( S )
#define BURST_SERIAL_( S ) \
extern burst_serial_t  S;

#define BURST_SERIAL_CREATE( S ) BURST_SERIAL_CREATE_( S )
#define BURST_SERIAL_CREATE_( S ) \
BURST_WEAK burst_size_t  S##_available(void){ return 0; }\
BURST_WEAK burst_size_t  S##_space(void){ return 0; }\
BURST_WEAK burst_size_t S##_getb(uint8_t* _buf, burst_size_t _max_sz){\
	BURST_UNUSED(_buf);\
	BURST_UNUSED(_max_sz);\
	return 0;\
}\
BURST_WEAK  burst_bool_t S##_putb(const uint8_t* _buf, burst_size_t _sz){\
	BURST_UNUSED(_buf);\
	BURST_UNUSED(_sz);\
	return 	burst_false;\
}\
BURST_WEAK  uint8_t S##_get(void){ return 0; }\
BURST_WEAK  void S##_put(uint8_t _data){\
	BURST_UNUSED(_data);\
}\
BURST_WEAK  burst_bool_t S##_begin(void){\
	return 	burst_true;\
}\
BURST_WEAK  burst_bool_t S##_start(void){\
	return 	burst_true;\
}\
BURST_WEAK  void S##_reset(void){}\
BURST_WEAK  void S##_finish(void){}\
burst_serial_t  S ={ \
	S##_available \
	, S##_space \
	, S##_getb \
	, S##_putb \
	, S##_get \
	, S##_put \
	, S##_begin \
	, S##_start \
	, S##_reset \
	, S##_finish \
};

BURST_SERIAL(burst_serial_dummy)
#endif
