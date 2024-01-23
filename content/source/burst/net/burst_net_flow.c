#include "burst/net/burst_net_flow.h"
typedef struct burst_net_flow_buf_s{
	uint8_t sz;
	uint8_t data[8];
} burst_net_flow_buf_t;
burst_net_flow_buf_t burst_net_flow_buffers[16];

void burst_net_flow_receive(uint16_t _id, const uint8_t * _data, uint8_t _sz){
	fb3_proto_id_t id;
	id.value = _id;
	if(id.slave) return;
	if(id.request){
			burst_net_flow_perform(id.suba,_data,_sz);
	} else {
		uint8_t suba = id.suba;
		uint8_t sz = _data[0];
		if( suba<=15 ){
			burst_net_flow_buf_t * buf = &(burst_net_flow_buffers[suba]);
			if( buf->sz == sz ){
				id.slave = 1;
				burst_net_flow_prf_sent(id.value,buf->data,buf->sz);				
				buf->sz = 0xff;
				burst_net_flow_sended(suba);
			}
		}
	}
}

uint8_t * burst_net_flow_outcom_get(uint8_t _suba, uint8_t _sz){
	if( _suba<=15 &&  _sz<=8 ){
		return burst_net_flow_buffers[_suba].data;
	} else {
		return 0;
	}
}

void burst_net_flow_outcom_post(uint8_t _suba, uint8_t _sz){
	if( _suba<=15 && _sz<=8 ){
		burst_net_flow_buf_t * buf = &(burst_net_flow_buffers[_suba]);
		buf->sz = _sz;
	}	
}




