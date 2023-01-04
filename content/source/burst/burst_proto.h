#ifndef burst_proto_h
#define burst_proto_h
#ifdef BURST_CLIENT_SIZE
#endif
#include <stdint.h>

typedef struct burst_proto_perfomer_s {
	uint8_t target;

} burst_proto_perfomer_t;
typedef burst_proto_perfomer_t * burst_proto_perfomer_p;

void burst_proto_encode(uint8_t _target, uint8_t* _payload, uint8_t _pauload_len, uint8_t* _packet);
void burst_proto_receive(uint8_t* _packet, uint8_t _packet_len);
void burst_proto_pool(void);
void burst_proto_env_lock(void);
void burst_proto_env_unlock(void);

#endif