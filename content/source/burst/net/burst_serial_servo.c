#include "burst/net/burst_serial_servo.h"

void burst_serial_servo_begin_(burst_serial_servo_p _servo){
	_servo -> transmit_timeout_us = 0;
	_servo -> transmit_time_us = 0;
}
void burst_serial_servo_start_(burst_serial_servo_p _servo){
	_servo -> hw_start_receive();
}

void burst_serial_servo_try_send_(burst_serial_servo_p _servo){
	if(_servo->tx_size == 0){
		_servo->tx_size = _servo->serial.getb( _servo->tx_buffer, _servo->tx_buffer_size);
	}
	if(_servo->tx_size){
		_servo -> transmit_timeout_us = _servo->hw_send_packet( _servo->tx_buffer , _servo->tx_size );			
		_servo -> transmit_time_us = burst_time_us();
	} 
}
void burst_serial_servo_receive_packet_(burst_serial_servo_p _servo, const uint8_t * _data, uint8_t _sz){
}

void burst_serial_servo_poll_(burst_serial_servo_p _servo){	
	if( _servo->hw_ready() ){
		burst_serial_servo_try_send_(_servo);
	}else{
		if(_servo -> transmit_timeout_us){
			burst_time_us_t now = burst_time_us();
			burst_time_us_t d = now - _servo -> transmit_time_us;
			if( d> _servo -> transmit_timeout_us ){
				_servo -> transmit_timeout_us = 0;
				_servo->hw_abort_tx();
			}
		}
	}
}


void burst_serial_servo_refuse_(burst_serial_servo_p _servo){
	_servo -> transmit_timeout_us = 0;
}
void burst_serial_servo_complete_ (burst_serial_servo_p _servo){
	_servo -> transmit_timeout_us = 0;
	_servo->tx_size = 0;
}
void burst_serial_servo_reset_(burst_serial_servo_p _servo){
	_servo->tx_size = 0;
	_servo -> transmit_timeout_us = 0;
	_servo->hw_abort_tx();
	_servo->hw_start_receive();
}
void burst_serial_servo_finish_(burst_serial_servo_p _servo){
	_servo->tx_size = 0;
	_servo -> transmit_timeout_us = 0;
	_servo->hw_abort_tx();
}
