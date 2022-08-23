#ifndef __burst_freemaster_h
#define __burst_freemaster_h
#include "burst/burst.h"
#ifndef BURST_APP_FREEMASTER_SERIAL_ENABLED
#define BURST_APP_FREEMASTER_SERIAL_ENABLED 0
#endif

#ifndef BURST_APP_PROTO_SWITCH_ENABLED
#define BURST_APP_PROTO_SWITCH_ENABLED 0
#endif

#if BURST_APP_FREEMASTER_SERIAL_ENABLED ==1
#include "burst_serial.h"
#endif

#if BURST_APP_FREEMASTER_SERIAL_ENABLED ==1
void burst_fm_connect(burst_serial_p _serial);
void burst_fm_recorder(void);
void burst_fm_poll(void);
#else
#define burst_fm_connect(x)
#define burst_fm_recorder()
#define burst_fm_poll()
#endif

#endif
