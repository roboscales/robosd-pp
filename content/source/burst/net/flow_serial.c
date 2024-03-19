#include "flow_serial.h"
#include "burst/burst_app.h"
#define RING_PREFIX_NAME fsincom
#define RING_ONE_DIRRECTION_PUT_GET 1
#define RING_SIZE_BITS fsincom_RING_SIZE_BITS
#define RING_LOCK() uint32_t context = burst_guard_enter();
#define RING_UNLOCK() burst_guard_leave(context);
#include "burst/burst_ring.inc.h"

#define RING_PREFIX_NAME fsoutcom
#define RING_ONE_DIRRECTION_PUT_GET 1
#define RING_SIZE_BITS fsoutcom_RING_SIZE_BITS
#define RING_LOCK()  uint32_t context = burst_guard_enter();
#define RING_UNLOCK() burst_guard_leave(context);
#include "burst/burst_ring.inc.h"

burst_size_t  burst_net_flow_serial_available(void){
	return fsincom_count();
}

burst_size_t  burst_net_flow_serial_space(void){
	return fsoutcom_size_ - fsoutcom_count();
}

uint8_t burst_net_flow_serial_get(void){ 
	return fsincom_get();
}


void burst_net_flow_serial_put(uint8_t _data){
	fsoutcom_put(_data);
}

burst_size_t burst_net_flow_serial_getb(uint8_t* _buf, burst_size_t _max_sz){
	return fsincom_buf_get(_buf,_max_sz);
}

burst_bool_t burst_net_flow_serial_putb(const uint8_t* _buf, burst_size_t _sz){
	return  fsoutcom_buf_put(_buf, _sz);
}
void burst_net_flow_serial_begin(void){
}
void burst_net_flow_serial_start(void){
}
void burst_net_flow_serial_finish(void){
}
void burst_net_flow_serial_reset(void){
	fsincom_clear();
	fsoutcom_clear();
}

burst_serial_t  burst_net_flow_serial = {
	burst_net_flow_serial_available
	, burst_net_flow_serial_space
	, burst_net_flow_serial_getb
	, burst_net_flow_serial_putb
	, burst_net_flow_serial_get
	, burst_net_flow_serial_put
	, burst_net_flow_serial_begin
	, burst_net_flow_serial_start
	, burst_net_flow_serial_reset
	, burst_net_flow_serial_finish
};

uint8_t burst_net_flow_declared_count_ = 0;

uint8_t burst_net_flow_serial_execute(const uint8_t * _buf_in, uint8_t _in_sz, uint8_t * _buf_out, uint8_t _max_sz) {
	uint8_t out_available;
	uint8_t out_total = 0;
	uint8_t in_size = _in_sz;
	uint8_t out_max_size = _max_sz;
	uint8_t in_space = fsincom_size_ - fsincom_count();
	uint8_t outcom_size = 0;
	if (in_size) {
		const uint8_t* in_data = _buf_in;
		uint8_t in_data_size = in_size - 1;
		uint8_t cmd = *in_data; in_data++;
		if (cmd == 0xBB) {
			if (in_space >= in_data_size) {
				if (in_data_size) {
					fsincom_buf_put(in_data,in_data_size);
				}
				return 0;
			}
		}	else if (cmd == 0xAA) {
			//remote_.reset();
			//local_.reset();
			//burst_net_flow_serial_reset();
			burst_net_flow_declared_count_ = 0;
		}
	}

	out_available = fsoutcom_count();
	if (out_available >= burst_net_flow_declared_count_) {
		out_total = burst_net_flow_declared_count_;
		if (out_total > 0) {
			outcom_size = out_total + 1;
		}	else {
			outcom_size = 1;
		}
		
		burst_net_flow_declared_count_ = out_available - burst_net_flow_declared_count_;
		if (in_space > 0xF)
			in_space = 0xF;
		if (burst_net_flow_declared_count_ > 0xF)
			burst_net_flow_declared_count_ = 0xF;
		if (burst_net_flow_declared_count_ > out_max_size - 1)
			burst_net_flow_declared_count_ = out_max_size - 1;
		uint8_t header = (uint8_t)((in_space << 4) + burst_net_flow_declared_count_);
		*_buf_out = header;
		fsoutcom_buf_get(_buf_out+1,out_total);
		return outcom_size;
	}
	fsincom_clear();
	fsoutcom_clear();
	burst_net_flow_declared_count_ = 0;
	return 0;
}
