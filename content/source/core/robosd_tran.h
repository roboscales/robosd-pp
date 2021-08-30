#ifndef __robo_tran_h
#define __robo_tran_h

#include <stdint.h>

typedef enum{
	ROBO_TRAN_NONE = 0
	, ROBO_TRAN_EXECUTE = 0x10
	, ROBO_TRAN_EXECUTE_START = ROBO_TRAN_EXECUTE | 0x1
	, ROBO_TRAN_EXECUTE_PHY = ROBO_TRAN_EXECUTE | 0x2
	, ROBO_TRAN_COMPLETE = 1
	, ROBO_TRAN_REFUSE = 2
} robo_tran_status_t;

typedef enum{
	ROBO_TRAN_REQUEST_GET = 1
	, ROBO_TRAN_REQUEST_PUT = 2
	, ROBO_TRAN_EXCANGE = 3
	, ROBO_TRAN_REBOOT_ME = 4
} robo_tran_request_t;

#ifndef ROBO_TRAN_COMMAND_ID
#define ROBO_TRAN_COMMAND_ID uint8_t
#endif

typedef  ROBO_TRAN_COMMAND_ID robo_tran_command_id_t;

#ifndef ROBO_TRAN_DEV_ID
#define ROBO_TRAN_DEV_ID uint8_t
#endif
typedef  ROBO_TRAN_DEV_ID robo_tran_dev_id_t;

#ifndef ROBO_TRAN_HEADER_IX
#define ROBO_TRAN_HEADER_IX uint16_t
#endif
typedef  ROBO_TRAN_HEADER_IX robo_tran_header_ix_t;

#ifndef ROBO_PHY_CHAN_IX
#define ROBO_PHY_CHAN_IX uint8_t
#endif
typedef  ROBO_PHY_CHAN_IX robo_phy_chan_ix_t;
#define ROBO_PHY_CHAN_AUTO 0xFF

typedef union robo_tran_header_s{
	struct{
		robo_tran_dev_id_t dev_id;
		robo_tran_command_id_t command;
	};
	robo_tran_header_ix_t hash;
} robo_tran_header_t;
typedef  robo_tran_header_t * robo_tran_header_p;


typedef struct robo_tran_s{
	size_t size_actual;
	size_t size_max;
	uint8_t * data;
	robo_tran_status_t status;
	robo_tran_request_t request;
	robo_tran_header_t header;
	robo_phy_chan_ix_t phy_chan;
} robo_tran_t;
typedef  robo_tran_t * robo_tran_p;


#endif
