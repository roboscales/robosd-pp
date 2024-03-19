#include "burst/modules/onewire_machine.h"
void oneware_machine_begin_(oneware_machine_p _oneware_machine, oneware_machine_config_p _oneware_config ){
	_oneware_machine->config = _oneware_config;
	
}
void oneware_machine_exchange_(oneware_machine_p _oneware_machine,	oneware_machine_request_p _request ){
	_oneware_machine->request = _request;
	if(_request->is_slow){
		_oneware_machine->request->is_slow = burst_true;
		_oneware_machine->hw_bitrate_slow();
	}
	burst_time_us_t tmp;
	// _oneware_machine->request->size = _oneware_machine->request->incom.payload_size_ + _oneware_machine->request->outcom.payload_size_;
	tmp = _oneware_machine->hw_begin_receive(_oneware_machine->request->incom.buffer, _oneware_machine->request->size);
	tmp += _oneware_machine->hw_begin_send(_request->outcom.buffer, _oneware_machine->request->size);
	if(tmp == 0){
		_oneware_machine->refuse();
	} else{
		_oneware_machine->request_timeout_us = tmp;
		_oneware_machine->request_us = burst_time_us();
	}
}
void   oneware_byte_decode(const uint8_t * src, uint8_t * dst)
{
  uint8_t rxByte = 0x00;

  for (uint8_t i = 0; i < 8; ++i, ++src)
  {
    if (*src == 0xFF)
    {
			rxByte |= (1 << i);
    }

  }

  * dst = rxByte;
}

void   oneware_stream_decode(const uint8_t * _stream, uint8_t * _dst ,uint8_t _length)
{
	uint8_t bytes_count = _length>>3;
	uint8_t * dst = _dst;
  for (uint8_t i = 0; i < bytes_count; ++i, _stream+=8,++dst)
  {
		oneware_byte_decode(_stream,dst);
  }
}

static uint8_t CalculateChecksum(uint8_t *data, uint8_t length)
{
  uint8_t checksum = 0;

  while (length--)
  {
    uint8_t currentByte = *data++;

    for (uint8_t i = 8; i; i--)
    {
      uint8_t temp = (checksum ^ currentByte) & 0x01;
      checksum >>= 1;

      if (temp)
      {
        checksum ^= 0x8C;
      }

      currentByte >>= 1;
    }
  }

  return checksum;
}

void oneware_machine_confirm_(oneware_machine_p _oneware_machine){
	if(_oneware_machine->request){
		/*
		if(_oneware_machine->status == oneware_machine_status_post){
			if(_oneware_machine->request->incom.size>0){
				_oneware_machine->status =  oneware_machine_status_receive;
				burst_time_us_t tmp = _oneware_machine->hw_begin_receive(_oneware_machine->request->incom.buffer,_oneware_machine->request->incom.size);
				if(tmp == 0){
					_oneware_machine->refuse();
				} else{
					_oneware_machine->request_timeout_us = tmp;
					_oneware_machine->request_us = burst_time_us();
				}
				return;
			} else{
				_oneware_machine->on_confirm(_oneware_machine->request);
			}
		} else if(_oneware_machine->status == oneware_machine_status_receive){
				_oneware_machine->on_confirm(_oneware_machine->request);
		} else{
			_oneware_machine->on_refuse(_oneware_machine->request);
		}
		*/
		if( _oneware_machine->request->size > 8){
			oneware_stream_decode(
				_oneware_machine->request->incom.buffer+_oneware_machine->request->outcom.payload_size_
			, _oneware_machine->request->incom.buffer, _oneware_machine->request->size);
			uint8_t n =  (_oneware_machine->request->incom.payload_size_>>3)-1;
			uint8_t crc= CalculateChecksum(_oneware_machine->request->incom.buffer, n);
			if(crc ==  _oneware_machine->request->incom.buffer[n]){
				_oneware_machine->on_confirm(_oneware_machine->request);
			} else{
				_oneware_machine->on_refuse(_oneware_machine->request);
			}
		} else{
			_oneware_machine->on_confirm(_oneware_machine->request);
		}
		if(_oneware_machine->request->is_slow){
			_oneware_machine->hw_bitrate_fast();			
		}

		_oneware_machine->request = 0;
		
	} else{
		_oneware_machine->on_panic();
	}
}

void oneware_machine_refuse_(oneware_machine_p _oneware_machine){
	if(_oneware_machine->request){
		if(_oneware_machine->request->is_slow){
			_oneware_machine->request->is_slow = burst_false;
			_oneware_machine->hw_bitrate_fast();			
		}
		_oneware_machine->on_refuse(_oneware_machine->request);
		_oneware_machine->request = 0;
	} else {
		_oneware_machine->on_panic();
	}
}
void oneware_machine_receive_(oneware_machine_p _oneware_machine,uint8_t _size){
	if(_oneware_machine->request){
		if(_oneware_machine->request->size == _size){
			_oneware_machine->confirm();
		} 
	} else {
		_oneware_machine->on_panic();
	}
}

void oneware_machine_poll_(oneware_machine_p _oneware_machine){
	if(_oneware_machine->request){
		if( burst_time_us() - _oneware_machine->request_us > _oneware_machine->request_timeout_us ){
			_oneware_machine->hw_abort();
			_oneware_machine->on_refuse(_oneware_machine->request);
		}
	}
}
