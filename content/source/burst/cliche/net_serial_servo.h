#include "burst/burst_app.h"
#include "burst/burst_timer.h"
#include "burst/net/burst_serial_servo.h"

#include "burst/cliche/_begin.h"

#ifdef CLCH_HEADER

extern burst_serial_servo_t  CLCH_NAME;

#else

BURST_WEAK burst_bool_t  PREFIX(hw_send_packet_)(uint8_t * _data, uint8_t _sz){
	BURST_UNUSED(_data);
	BURST_UNUSED(_sz);
	return 0; 
}
BURST_WEAK  burst_time_us_t PREFIX(hw_send_packet_timeout_us_)(uint8_t _sz){
	BURST_UNUSED(_sz);
	return 0; 
}


BURST_WEAK void  PREFIX(hw_aborttx_)(void){ }

BURST_WEAK void PREFIX(hw_start_receive_)(void){

}
BURST_WEAK  burst_bool_t PREFIX(hw_ready_)(void){
	return 	burst_false;
}

burst_size_t  PREFIX(serial_available_)(void);
burst_size_t  PREFIX(serial_space_)(void);
burst_size_t PREFIX(serial_getb_)(uint8_t* _buf, burst_size_t _max_sz);
burst_bool_t PREFIX(serial_putb_)(const uint8_t* _buf, burst_size_t _sz);
uint8_t PREFIX(get_)(void);
void PREFIX(serial_put_)(uint8_t _data);
uint8_t PREFIX(serial_get_)(void);
void PREFIX(serial_begin_)(void);
void PREFIX(serial_start_)(void);
void PREFIX(serial_reset_)(void);
void PREFIX(serial_finish_)(void);
void PREFIX(poll_)(void);
void PREFIX(receive_packet_)(const uint8_t * _data, uint8_t _sz);
uint8_t PREFIX(query_packet_)(uint8_t * _data, uint8_t _max_sz);
void PREFIX(refuse_)(void);
void PREFIX(complete_)(void);
void PREFIX(begin_)(void);
void PREFIX(start_)(void);
void PREFIX(reset_)(void);
void PREFIX(finish_)(void);


#define RING_PREFIX_NAME PREFIX(incom)
#define RING_SIZE_BITS PREFIX(INCOM_RING_SIZE_BITS)
#define RING_LOCK() uint32_t context = burst_guard_enter();
#define RING_UNLOCK() burst_guard_leave(context);
#include "burst/burst_ring.inc.h"

#define RING_PREFIX_NAME PREFIX(outcom)
#define RING_SIZE_BITS PREFIX(OUTCOM_RING_SIZE_BITS)
#define RING_LOCK() uint32_t context = burst_guard_enter();
#define RING_UNLOCK() burst_guard_leave(context);
#include "burst/burst_ring.inc.h"

uint8_t PREFIX(tx_buffer_)[PREFIX(outcom_size_)];

//burst_time_us_t (*hw_send_packet_timeout_us)(uint8_t _sz);

burst_serial_servo_t  CLCH_NAME ={
	{
		PREFIX(serial_available_)
		, PREFIX(serial_space_)
		, PREFIX(serial_getb_)
		, PREFIX(serial_putb_)
		, PREFIX(serial_get_)
		, PREFIX(serial_put_)
		, PREFIX(serial_begin_)
		, PREFIX(serial_start_)
		, PREFIX(serial_reset_)
		, PREFIX(serial_finish_)
	}
	, PREFIX(hw_send_packet_timeout_us_)
	, PREFIX(hw_send_packet_)
	, PREFIX(hw_aborttx_)
	, PREFIX(hw_start_receive_)
	, PREFIX(hw_ready_)
	, PREFIX(poll_)
	, PREFIX(receive_packet_)
	, PREFIX(query_packet_)
	, PREFIX(refuse_)
	, PREFIX(complete_)
	, PREFIX(begin_)
	, PREFIX(start_)
	, PREFIX(reset_)
	, PREFIX(finish_)
	, PREFIX(tx_buffer_)
	, PREFIX(outcom_size_)
};



BURST_WEAK  void PREFIX(poll_)(void){
	burst_serial_servo_poll_ (&CLCH_NAME); 
}
BURST_WEAK  void PREFIX(receive_packet_)(const uint8_t * _data, uint8_t _sz){ 
	PREFIX(incom_buf_put)(_data, _sz);
	CLCH_NAME.hw_start_receive();	
}
BURST_WEAK  uint8_t PREFIX(query_packet_)(uint8_t * _data, uint8_t _max_sz){
	return PREFIX(outcom_buf_get)(_data, _max_sz);
}

BURST_WEAK  void PREFIX(refuse_)(void){
	CLCH_NAME.hw_abort_tx();
	burst_serial_servo_refuse_ (&CLCH_NAME); 
	CLCH_NAME.hw_start_receive();
}
BURST_WEAK  void PREFIX(complete_)(void){
	burst_serial_servo_complete_ (&CLCH_NAME);
	CLCH_NAME.hw_start_receive();
}
BURST_WEAK  void PREFIX(begin_)(void){
	CLCH_NAME.serial.begin();
	burst_serial_servo_begin_(&CLCH_NAME);
}
BURST_WEAK  void PREFIX(start_)(void){
	CLCH_NAME.serial.start();
	burst_serial_servo_start_(&CLCH_NAME);
	CLCH_NAME.hw_start_receive();
}
BURST_WEAK  void PREFIX(reset_)(void){
	CLCH_NAME.serial.reset();
	CLCH_NAME.hw_abort_tx();
	burst_serial_servo_reset_(&CLCH_NAME);
	CLCH_NAME.hw_start_receive();
}
BURST_WEAK  void PREFIX(finish_)(void){
	CLCH_NAME.hw_abort_tx();
	CLCH_NAME.serial.finish();
	burst_serial_servo_finish_(&CLCH_NAME);
}

BURST_WEAK burst_size_t  PREFIX(serial_available_)(void){ 
	return PREFIX(incom_available)();
}
BURST_WEAK burst_size_t  PREFIX(serial_space_)(void){ 
	return PREFIX(outcom_space)();
}
BURST_WEAK burst_size_t PREFIX(serial_getb_)(uint8_t* _buf, burst_size_t _max_sz){
	return PREFIX(incom_buf_get)(_buf,_max_sz);
}
BURST_WEAK  burst_bool_t PREFIX(serial_putb_)(const uint8_t* _buf, burst_size_t _sz){
	return PREFIX(outcom_buf_put)(_buf,_sz);
}
BURST_WEAK  uint8_t PREFIX(serial_get_)(void){ 
	return PREFIX(incom_get)();
}
BURST_WEAK  void PREFIX(serial_put_)(uint8_t _data){
	return PREFIX(outcom_put)(_data);
}
BURST_WEAK  void PREFIX(serial_begin_)(void){
}
BURST_WEAK  void PREFIX(serial_start_)(void){
}
BURST_WEAK  void PREFIX(serial_reset_)(void){
	PREFIX(incom_clear)();
	PREFIX(outcom_clear)();
}
BURST_WEAK  void PREFIX(serial_finish_)(void){
}
#endif
#include "burst/cliche/_end.h"

