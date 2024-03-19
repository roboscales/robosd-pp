#ifndef burst_net_flow_serial_h
#define burst_net_flow_serial_h
#include "burst/burst_serial.h"

uint8_t burst_net_flow_serial_execute(const uint8_t * _buf_in, uint8_t _in_sz, uint8_t * _buf_out, uint8_t _max_sz);
extern burst_serial_t  burst_net_flow_serial;
#endif
