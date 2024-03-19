#include "burst/cliche/ds18b20.h"



typedef struct DS18B20_reset_request_s{
	oneware_machine_request_t request;
	uint8_t command[1];
	uint8_t answ[1];
} DS18B20_reset_request_t;

typedef struct DS18B20_readrom_request_s{
	oneware_machine_request_t request;
	uint8_t command[72];
	uint8_t answ[72];
} DS18B20_readrom_request_t;


typedef struct DS18B20_readScratchpad_request_s{
	oneware_machine_request_t request;
	uint8_t command[80];
	uint8_t answ[80];
} DS18B20_readScratchpad_request_t;

typedef struct DS18B20_skipRom_request_s{
	oneware_machine_request_t request;
	uint8_t command[8];
	uint8_t answ[8];
} DS18B20_skipRom_request_t;

typedef struct DS18B20_convertT_request_s{
	oneware_machine_request_t request;
	uint8_t command[8];
	uint8_t answ[8];
} DS18B20_convertT_request_t;

#define OW_0    0x00
#define OW_1    0xff
enum {
	DS18B20_request_reset =1
	, DS18B20_request_readrom =2
	, DS18B20_request_readScratchpad =3
	, DS18B20_request_skipRom =4
	, DS18B20_request_convertT =5
};
DS18B20_reset_request_t DS18B20_reset_request={
	{DS18B20_request_reset,burst_true}
	,{0xF0}
	,{}
};

DS18B20_readrom_request_t DS18B20_readrom_request={
	{DS18B20_request_readrom,burst_false}
	,{OW_1, OW_1, OW_0, OW_0, OW_1, OW_1, OW_0, OW_0}
	,{}
};

DS18B20_readScratchpad_request_t DS18B20_readScratchpad_request={
	{DS18B20_request_readScratchpad,burst_false}
	,{OW_0, OW_1, OW_1, OW_1, OW_1, OW_1, OW_0, OW_1}
	,{}
};

DS18B20_skipRom_request_t DS18B20_skipRom_request={
	{DS18B20_request_skipRom ,burst_false}
	,{OW_0, OW_0, OW_1, OW_1, OW_0, OW_0, OW_1, OW_1}
	,{}
};

DS18B20_convertT_request_t DS18B20_convertT_request={
	{DS18B20_request_convertT ,burst_false}
	,{OW_0, OW_0, OW_1, OW_0, OW_0, OW_0, OW_1, OW_0}
	,{}
};

oneware_machine_request_p DS18B20_ReadScratchpadSequence_requests[3]={
	&DS18B20_reset_request.request
	, &DS18B20_readrom_request.request
	, &DS18B20_readScratchpad_request.request
};

oneware_machine_request_p DS18B20_readTemp_requests[6]={
	&DS18B20_reset_request.request
	, &DS18B20_skipRom_request.request
	, &DS18B20_convertT_request.request
	, &DS18B20_reset_request.request
	, &DS18B20_skipRom_request.request
	, &DS18B20_readScratchpad_request.request
};



DS18B20_sequence_t DS18B20_ReadScratchpadSequence = {
	&DS18B20_ReadScratchpadSequence_requests[0]
	, 3
	, 0
};

DS18B20_sequence_t DS18B20_readTempSequence = {
	&DS18B20_readTemp_requests[0]
	, 6
	, 0
};
	//DS18B20_InitializationCommand(&temperatureSensor);
  //DS18B20_ReadRom(&temperatureSensor);
 // DS18B20_ReadScratchpad(&temperatureSensor);


//uint8_t DS18B20_reset_request 
 //= {.code = 0xCC, .rxBytesNum = 0, .txBytesNum = 0};


void DS18B20_begin_(DS18B20_p _sensor, oneware_machine_p _channel){
	_sensor->channel = _channel;
	DS18B20_reset_request.request.outcom.buffer =DS18B20_reset_request.command;
	DS18B20_reset_request.request.outcom.payload_size_ =1;
	DS18B20_reset_request.request.incom.buffer =DS18B20_reset_request.answ;
	DS18B20_reset_request.request.incom.payload_size_ =0;
	DS18B20_reset_request.request.size = 1;

	DS18B20_readrom_request.request.outcom.buffer =DS18B20_readrom_request.command;
	DS18B20_readrom_request.request.outcom.payload_size_ =8;
	DS18B20_readrom_request.request.incom.buffer =DS18B20_readrom_request.answ;
	DS18B20_readrom_request.request.incom.payload_size_ =64;
	DS18B20_readrom_request.request.size = 72;
	for( int i=DS18B20_readrom_request.request.outcom.payload_size_;i<DS18B20_readrom_request.request.size;i++){
		DS18B20_readrom_request.request.outcom.buffer[i] = 0xFF;
	}

	DS18B20_readScratchpad_request.request.outcom.buffer =DS18B20_readScratchpad_request.command;
	DS18B20_readScratchpad_request.request.outcom.payload_size_ =8;
	DS18B20_readScratchpad_request.request.incom.buffer =DS18B20_readScratchpad_request.answ;
	DS18B20_readScratchpad_request.request.incom.payload_size_ =72;
	DS18B20_readScratchpad_request.request.size = 80;
	for( int i=DS18B20_readScratchpad_request.request.outcom.payload_size_;i<DS18B20_readScratchpad_request.request.size;i++){
		DS18B20_readScratchpad_request.request.outcom.buffer[i] = 0xFF;
	}

	DS18B20_skipRom_request.request.outcom.buffer =DS18B20_skipRom_request.command;
	DS18B20_skipRom_request.request.outcom.payload_size_ = 8;
	DS18B20_skipRom_request.request.incom.buffer =DS18B20_skipRom_request.answ;
	DS18B20_skipRom_request.request.incom.payload_size_ = 0;
	DS18B20_skipRom_request.request.size = 8;

	DS18B20_convertT_request.request.outcom.buffer =DS18B20_convertT_request.command;
	DS18B20_convertT_request.request.outcom.payload_size_ = 8;
	DS18B20_convertT_request.request.incom.buffer =DS18B20_convertT_request.answ;
	DS18B20_convertT_request.request.incom.payload_size_ = 0;
	DS18B20_convertT_request.request.size = 8;
}

void DS18B20_pool_(DS18B20_p _sensor){
	if(_sensor->sequence == 0){
		if(!_sensor->isInitialized){
			if( burst_time_us() - _sensor->seq_us >1000000){
				DS18B20_query_(_sensor,&DS18B20_ReadScratchpadSequence);
				_sensor->seq_us = burst_time_us();
			}
		}
	}else{
		if(_sensor->begin_pause_us){
			if(burst_time_us()-_sensor->begin_pause_us>_sensor->delay_us){
				_sensor->begin_pause_us = 0;
				_sensor->sequence->current++;
				_sensor->channel->exchange(_sensor->sequence->requests[_sensor->sequence->current]);			
			}
		}
	}
}
void DS18B20_confirm_(DS18B20_p _sensor){
	if(_sensor->sequence){
		oneware_machine_request_p request = _sensor->sequence->requests[_sensor->sequence->current];
		switch (request->tag){
			case DS18B20_request_reset:
				_sensor->delay_us = 100;
				break;
			case DS18B20_request_readrom:
				_sensor->delay_us = 100;
			  for (uint8_t i = 0; i < DS18B20_SERIAL_NUMBER_LEN_BYTES; ++i)
				{
					_sensor->serialNumber[i] = request->incom.buffer[i+DS18B20_SERIAL_NUMBER_OFFSET_BYTES];
				}
				break;
			case DS18B20_request_readScratchpad:
				_sensor->delay_us = 100;
				_sensor->temperatureLimitHigh = request->incom.buffer[DS18B20_SCRATCHPAD_T_LIMIT_H_BYTE_IDX];
				_sensor->temperatureLimitLow = request->incom.buffer[DS18B20_SCRATCHPAD_T_LIMIT_L_BYTE_IDX];
				_sensor->configRegister = request->incom.buffer[DS18B20_SCRATCHPAD_CONFIG_BYTE_IDX];
				uint16_t tRegValue = (request->incom.buffer[DS18B20_SCRATCHPAD_T_MSB_BYTE_IDX] << 8) | request->incom.buffer[DS18B20_SCRATCHPAD_T_LSB_BYTE_IDX];
				uint16_t sign = tRegValue & DS18B20_SIGN_MASK;

				if (sign != 0)
				{
					tRegValue = (0xFFFF - tRegValue + 1);
				}
				switch (_sensor->configRegister)
					{
						case DS18B20_9_BITS_CONFIG:
							tRegValue &= DS18B20_9_BITS_DATA_MASK;
							break;

						case DS18B20_10_BITS_CONFIG:
							tRegValue &= DS18B20_10_BITS_DATA_MASK;
							break;

						case DS18B20_11_BITS_CONFIG:
							tRegValue &= DS18B20_11_BITS_DATA_MASK;
							break;

						case DS18B20_12_BITS_CONFIG:
							tRegValue &= DS18B20_12_BITS_DATA_MASK;
							break;

						default:
							tRegValue &= DS18B20_12_BITS_DATA_MASK;
							break;
					}

					_sensor->temperature = ( ((int)(65536.*DS18B20_T_STEP)) *tRegValue)>>16 ;
				break;
			case DS18B20_request_skipRom:
				_sensor->delay_us = 100;
				break;
			case DS18B20_request_convertT:
				 switch (_sensor->configRegister)
				{
					case DS18B20_9_BITS_CONFIG:
						_sensor->delay_us = DS18B20_9_BITS_DELAY_MS*1000;
						break;

					case DS18B20_10_BITS_CONFIG:
						_sensor->delay_us = DS18B20_10_BITS_DELAY_MS*1000;
						break;

					case DS18B20_11_BITS_CONFIG:
						_sensor->delay_us = DS18B20_11_BITS_DELAY_MS*1000;
						break;

					case DS18B20_12_BITS_CONFIG:
						_sensor->delay_us = DS18B20_12_BITS_DELAY_MS*1000;
						break;

					default:
						break;
				}
				break;
					default:
						_sensor->delay_us = 100;
		}
		if(_sensor->sequence->count - 1== _sensor->sequence->current){
			if(_sensor->sequence == &DS18B20_ReadScratchpadSequence){
				_sensor->isInitialized = burst_true;
			}
			_sensor->sequence = 0;
		} else{
			_sensor->begin_pause_us = burst_time_us();
		}
	}
}
void DS18B20_refuse_(DS18B20_p _sensor){
	_sensor->sequence = 0;
}
void DS18B20_panic_(DS18B20_p _sensor){
	_sensor->sequence = 0;
}
void DS18B20_query_(DS18B20_p _sensor,DS18B20_sequence_p _seq){
	_seq->current = 0;
	_sensor->sequence = _seq;
	_sensor->begin_pause_us = 0;
	_seq->current = 0;
	_sensor->channel->exchange(_seq->requests[0]);
}

void DS18B20_query_temp_(DS18B20_p _sensor){
	if(_sensor->isInitialized  ){
		if(_sensor->sequence == 0){
			DS18B20_query_(_sensor, &DS18B20_readTempSequence);
		}
	}
}

/*
void DS18B20_convertT_(DS18B20_p _sensor, DS18B20_WaitCondition waitCondition){
}
void DS18B20_readScratchpad_(DS18B20_p _sensor){
}
void DS18B20_writeScratchpad_(DS18B20_p _sensor, uint8_t *data){
}
void DS18B20_initializationCommand_(DS18B20_p _sensor){
}
void DS18B20_readRom_(DS18B20_p _sensor){
}
void DS18B20_skipRom_(DS18B20_p _sensor){
}
*/
