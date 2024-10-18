#include "burst/net/burst_serial_servo.h"

void burst_serial_servo_begin_(burst_serial_servo_p _servo){
	_servo -> transmit_timeout_us = 0;
	_servo -> transmit_time_us = 0;
}
void burst_serial_servo_start_(burst_serial_servo_p _servo){
}

void burst_serial_servo_try_send_(burst_serial_servo_p _servo){
	if(_servo->tx_size == 0){
		_servo->tx_size = _servo->query_packet( _servo->tx_buffer, _servo->tx_buffer_size);
	}
	if(_servo->tx_size){
		burst_time_us_t tm = _servo->hw_send_packet_timeout_us(_servo->tx_size);
		if(tm){
			_servo -> transmit_timeout_us =  tm;
			_servo -> transmit_time_us = burst_time_us();
			if( _servo -> hw_send_packet( _servo->tx_buffer , _servo->tx_size ) == burst_false ){
				_servo -> refuse();
			}				
		}
	} 
}

void burst_serial_servo_receive_packet_(burst_serial_servo_p _servo, const uint8_t * _data, uint8_t _sz){
}

void burst_serial_servo_poll_(burst_serial_servo_p _servo){	
	if( _servo -> transmit_timeout_us == 0 && _servo->hw_ready() ){
		burst_serial_servo_try_send_(_servo);
	}else{
		if(_servo -> transmit_timeout_us){
			burst_time_us_t now = burst_time_us();
			burst_time_us_t d = now - _servo -> transmit_time_us;
			if( d> _servo -> transmit_timeout_us ){
				_servo -> refuse();
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
}
void burst_serial_servo_finish_(burst_serial_servo_p _servo){
	_servo->tx_size = 0;
	_servo -> transmit_timeout_us = 0;
}
