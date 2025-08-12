#ifndef robosd_flow_id_h
#define  robosd_flow_id_h
#include <stdint.h>
typedef union{
	struct {
		uint16_t suba : 4;
		uint16_t addr : 4;
		uint16_t user : 1;
		uint16_t request : 1;
		uint16_t slave : 1;
	};
	uint16_t value;
} flow_msg_can_id_t;
typedef struct{ 
	union{
		struct {
			uint8_t suba : 4;
			uint8_t user : 1;
			uint8_t request : 1;
			uint8_t slave : 1;
		};
		uint8_t value;
	};
	uint8_t addr;
} flow_msg_header_t;

#endif