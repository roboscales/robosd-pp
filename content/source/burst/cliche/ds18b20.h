#ifndef CLCH_NAME
#ifndef ds18b20_h
#define ds18b20_h
#include "burst/burst_common.h"
#include "burst/modules/onewire_machine.h"

#define DS18B20_SERIAL_NUMBER_LEN_BYTES                                     6
#define DS18B20_SERIAL_NUMBER_OFFSET_BYTES                                  1

#define DS18B20_SCRATCHPAD_T_LSB_BYTE_IDX                                   0
#define DS18B20_SCRATCHPAD_T_MSB_BYTE_IDX                                   1
#define DS18B20_SCRATCHPAD_T_LIMIT_H_BYTE_IDX                               2
#define DS18B20_SCRATCHPAD_T_LIMIT_L_BYTE_IDX                               3
#define DS18B20_SCRATCHPAD_CONFIG_BYTE_IDX                                  4

#define DS18B20_9_BITS_CONFIG                                               0x1F
#define DS18B20_10_BITS_CONFIG                                              0x3F
#define DS18B20_11_BITS_CONFIG                                              0x5F
#define DS18B20_12_BITS_CONFIG                                              0x7F

#define DS18B20_9_BITS_DELAY_MS                                             94
#define DS18B20_10_BITS_DELAY_MS                                            188
#define DS18B20_11_BITS_DELAY_MS                                            375
#define DS18B20_12_BITS_DELAY_MS                                            750

#define DS18B20_9_BITS_DATA_MASK                                            0x7F8
#define DS18B20_10_BITS_DATA_MASK                                           0x7FC
#define DS18B20_11_BITS_DATA_MASK                                           0x7FE
#define DS18B20_12_BITS_DATA_MASK                                           0x7FF

#define DS18B20_SIGN_MASK                                                   0xF800

#define DS18B20_T_STEP                                                      0.0625

#define DS18B20_READ_ROM_RX_BYTES_NUM                                       8
#define DS18B20_READ_SCRATCHPAD_RX_BYTES_NUM                                9

typedef struct DS18B20_sequence_s{
	oneware_machine_request_p * requests;
	uint8_t count;
	uint8_t current;
} DS18B20_sequence_t;
typedef DS18B20_sequence_t *  DS18B20_sequence_p;




typedef struct DS18B20_s
{
  burst_bool_t isInitialized;
  burst_bool_t isConnected;
  uint8_t serialNumber[DS18B20_SERIAL_NUMBER_LEN_BYTES];
  uint8_t temperatureLimitLow;
  uint8_t temperatureLimitHigh;
  uint8_t configRegister;
  uint16_t temperature;
	oneware_machine_p channel;
	DS18B20_sequence_p sequence;
	burst_time_us_t begin_pause_us;
	burst_time_us_t seq_us;
	burst_time_us_t delay_us;
	/*void (*begin)(oneware_machine_p channel);
	void (*convertT)(DS18B20_WaitCondition waitCondition);
	void (*readScratchpad)(void);
	void (*writeScratchpad)(uint8_t *data);
	void (*initializationCommand)(void);
	void (*readRom)(void);
	void (*skipRom)(void);*/
} DS18B20_t;
typedef DS18B20_t  * DS18B20_p;


void DS18B20_begin_(DS18B20_p sensor, oneware_machine_p channel);
void DS18B20_pool_(DS18B20_p sensor);
void DS18B20_confirm_(DS18B20_p sensor);
void DS18B20_refuse_(DS18B20_p sensor);
void DS18B20_panic_(DS18B20_p sensor);
void DS18B20_query_(DS18B20_p sensor,DS18B20_sequence_p);

void DS18B20_query_temp_(DS18B20_p sensor);

/*void DS18B20_convertT_(DS18B20_p sensor, DS18B20_WaitCondition waitCondition);
void DS18B20_readScratchpad_(DS18B20_p sensor);
void DS18B20_writeScratchpad_(DS18B20_p sensor, uint8_t *data);
void DS18B20_initializationCommand_(DS18B20_p sensor);
void DS18B20_readRom_(DS18B20_p sensor);
void DS18B20_skipRom_(DS18B20_p sensor);*/

#endif
#else
#include "burst/cliche/_begin.h"
#ifdef CLCH_HEADER
	void PREFIX(on)(int _verb);
	void PREFIX(off)(int _verb);
	burst_bool_t PREFIX(get)(void);
	void PREFIX(verb_set)(int _verb);
	int PREFIX(verb_get)(void);
	void PREFIX(toggle)(int);
	void PREFIX(phy_on)(void);
	void PREFIX(phy_off)(void);
#else

burst_bool_t PREFIX(status);

BURST_WEAK void PREFIX(phy_on)(void){}
BURST_WEAK void PREFIX(phy_off)(void){}
burst_bool_t PREFIX(get)(void){
	return PREFIX(status);
}
void PREFIX(on)(int _verb){
	PREFIX(status) = burst_true;
	PREFIX(phy_on)();
}
void PREFIX(off)(int _verb){
	PREFIX(status) = burst_false;
	PREFIX(phy_off)();
}
void PREFIX(toggle)(int _verb){
	if( PREFIX(status) == burst_false){
		PREFIX(phy_on)();
		PREFIX(status) = burst_true;
	} else {
		PREFIX(status) = burst_false;
		PREFIX(phy_off)();
	}
}



#endif
#include "burst/cliche/_end.h"
#endif
