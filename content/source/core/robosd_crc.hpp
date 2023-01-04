#ifndef robosd_crc_hpp
#define robosd_crc_hpp

#include "core/robosd_common.hpp"
namespace robo {

#ifndef ROBO_APP_CRC8_ETSI_EN_302_307_ENABLED
#define ROBO_APP_CRC8_ETSI_EN_302_307_ENABLED 0
#endif

#if ROBO_APP_CRC8_ETSI_EN_302_307_ENABLED == 1
uint8_t crc8_ETSIEN302307(uint8_t * _pdata, size_t _size);
#endif

#ifndef ROBO_APP_CRC8_BY_TABLE_ENABLED
#define ROBO_APP_CRC8_BY_TABLE_ENABLED 0
#endif

#if ROBO_APP_CRC8_BY_TABLE_ENABLED == 1
uint8_t crc8_by_table(uint8_t * _pdata, size_t _len);
#endif

#ifndef ROBO_APP_CRC7_BY_TABLE_ENABLED
#define ROBO_APP_CRC7_BY_TABLE_ENABLED 0
#endif

#if ROBO_APP_CRC7_BY_TABLE_ENABLED == 1
uint8_t crc7_by_table(uint8_t * _pdata, size_t _len);
#endif

#if ROBO_APP_CRC16_MODBUS_BY_TABLE_ENABLED == 1
uint16_t crc16_modbus_by_table(uint8_t * _pdata, size_t _len );
#endif
	
#endif
}