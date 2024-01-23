#ifndef burst_net_flow_h
#define burst_net_flow_h
#include <stdint.h>
typedef union fb3_proto_id_s{
	struct{
		uint16_t suba:4;
		uint16_t addr:5;
		uint16_t request:1;
		uint16_t slave:1;
	};
	uint16_t value;
} fb3_proto_id_t;

void burst_net_flow_receive(uint16_t _id, const uint8_t * _data, uint8_t _sz);
void burst_net_flow_prf_sent(uint16_t _id, const uint8_t * _data, uint8_t _sz);
void burst_net_flow_perform(uint8_t _suba,const uint8_t * _data, uint8_t _sz);
void burst_net_flow_sended(uint8_t _suba);
uint8_t burst_net_flow_var_ix(uint32_t _long_id);

uint8_t * burst_net_flow_outcom_get(uint8_t _suba, uint8_t _sz);
void burst_net_flow_outcom_post(uint8_t _suba, uint8_t _sz);


typedef union burst_net_flow_descriptor_u {
	struct {
		uint16_t len : 13;
		uint16_t bsign : 1;
		uint16_t bconst : 1;
		uint16_t real : 1;
		//uint16_t fault : 5;
	};
	uint16_t memo;
	uint8_t bytes[2];
} burst_net_flow_descriptor_t;
#endif
