#include "burst/burst_app.h"
#include "burst/burst_timer.h"

#ifndef clch_net_serial_servo_h
#define clch_net_serial_servo_h

#include "burst/burst_serial.h"

typedef struct burst_serial_servo_s {
	burst_serial_t serial;
	burst_time_us_t (*hw_send_packet_timeout_us)(uint8_t _sz);
	burst_bool_t (*hw_send_packet)(uint8_t * _data, uint8_t _sz);
	void (*hw_abort_tx)(void);
	void (*hw_start_receive)(void);
	burst_bool_t (*hw_ready)(void);
	void (*pool)(void);
	void (*receive_packet)(const uint8_t * _data, uint8_t _sz);
	uint8_t (*query_packet)( uint8_t * _data, uint8_t _max_sz);
	void (*refuse)(void);
	void (*complete)(void);
	void (*begin)(void);
	void (*start)(void);
	void (*reset)(void);
	void (*finish)(void);
	uint8_t * tx_buffer;
	uint8_t tx_buffer_size;
	uint8_t tx_size;
	burst_time_us_t transmit_timeout_us;
	burst_time_us_t transmit_time_us;
} burst_serial_servo_t;

typedef burst_serial_servo_t * burst_serial_servo_p;

void burst_serial_servo_begin_(burst_serial_servo_p _servo);
void burst_serial_servo_start_(burst_serial_servo_p _servo);
void burst_serial_servo_poll_(burst_serial_servo_p _servo);
void burst_serial_servo_refuse_(burst_serial_servo_p _servo);
void burst_serial_servo_complete_(burst_serial_servo_p _servo);
void burst_serial_servo_reset_(burst_serial_servo_p _servo);
void burst_serial_servo_finish_(burst_serial_servo_p _servo);

#endif

