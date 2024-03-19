#ifndef oneware_machine_h
#define oneware_machine_h
#include "burst/burst_timer.h"

typedef struct  {
} oneware_machine_config_t;

typedef oneware_machine_config_t * oneware_machine_config_p;

#define oneware_machine_CONFIG(a) oneware_machine_CONFIG_(a)
#define oneware_machine_CONFIG_(a)\
{\
}

typedef struct  {
	int tag;
	burst_bool_t is_slow;	
	struct{
		uint8_t payload_size_;
		uint8_t * buffer;
	} outcom;
	struct{
		uint8_t payload_size_;
		uint8_t * buffer;
	} incom;
	uint8_t size;
} oneware_machine_request_t;
typedef oneware_machine_request_t * oneware_machine_request_p;

typedef struct  {
	oneware_machine_config_p config;
	burst_time_us_t (* hw_begin_receive)(uint8_t * _buf, uint8_t _size);
	burst_time_us_t (* hw_begin_send)(const uint8_t * _buf, uint8_t _size);
	void (* hw_bitrate_slow)(void);
	void (* hw_bitrate_fast)(void);
	void (* hw_abort)(void);
	void (* begin)(oneware_machine_config_p _oneware_config);
	void (* exchange)(oneware_machine_request_p request);
	burst_bool_t (* ready)(void);
	void (* confirm)(void);
	void (* refuse)(void);			
	void (* receive)(uint8_t _size);		
	void (* poll)(void);		
	void (* on_confirm)( oneware_machine_request_p _request);
	void (* on_refuse)( oneware_machine_request_p _request);			
	void (* on_panic)( void);			
	oneware_machine_request_p request;
	burst_time_us_t request_us;
	burst_time_us_t request_timeout_us;
} oneware_machine_t;

typedef oneware_machine_t * oneware_machine_p;

void oneware_machine_begin_(oneware_machine_p _oneware_machine, oneware_machine_config_p _oneware_config );
void oneware_machine_exchange_( oneware_machine_p _oneware_machine,	oneware_machine_request_p request);
void oneware_machine_confirm_(oneware_machine_p _oneware_machine);
void oneware_machine_refuse_(oneware_machine_p _oneware_machine);
void oneware_machine_receive_(oneware_machine_p _oneware_machine,uint8_t _size);
void oneware_machine_poll_(oneware_machine_p _oneware_machine);

#define oneware_machine_impl( S, D ) \
BURST_WEAK  burst_time_us_t S##_hw_begin_receive(uint8_t * _buf, uint8_t _size){\
	return 0;\
}\
BURST_WEAK  burst_time_us_t S##_hw_begin_send(const uint8_t * _buf, uint8_t _size){\
	return 0;\
}\
BURST_WEAK  void S##_hw_bitrate_slow(void){\
}\
BURST_WEAK  void S##_hw_bitrate_fast(void){\
}\
BURST_WEAK  void S##_hw_abort(void){ \
}\
BURST_WEAK  void S##_begin(oneware_machine_config_p _oneware_config){\
	return oneware_machine_begin_(&D,_oneware_config);\
}\
BURST_WEAK  void S##_exchange(oneware_machine_request_p request){ \
	oneware_machine_exchange_(&D,request);\
}\
BURST_WEAK  burst_bool_t S##_ready(void){\
	return D.request!=0;\
}\
BURST_WEAK  void S##_confirm(void){\
	return oneware_machine_confirm_(&D);\
}\
BURST_WEAK  void S##_refuse(void){\
	return oneware_machine_refuse_(&D);\
}\
BURST_WEAK  void S##_poll(void){\
	return oneware_machine_poll_(&D);\
}\
BURST_WEAK  void S##_receive(uint8_t _size){ \
	oneware_machine_receive_(&D,_size);\
}\
BURST_WEAK  void S##_on_confirm(oneware_machine_request_p _request){\
}\
BURST_WEAK  void S##_on_refuse(oneware_machine_request_p _request){\
}\
BURST_WEAK  void S##_on_panic(void){\
}

#define oneware_machine_setup( S ) \
{\
	0\
	, S##_hw_begin_receive\
	, S##_hw_begin_send\
	, S##_hw_bitrate_slow\
	, S##_hw_bitrate_fast\
	, S##_hw_abort\
	, S##_begin\
	, S##_exchange\
	, S##_ready\
	, S##_confirm\
	, S##_refuse\
	, S##_receive\
	, S##_poll\
	, S##_on_confirm\
	, S##_on_refuse\
	, S##_on_panic\
	,0\
	,0\
	,0\
} 

#define ONEWARE_MACHINE( S ) BURST_OBJECT(oneware_machine,S)

#define ONEWARE_MACHINE_CREATE( S ) BURST_OBJECT_CREATE(oneware_machine,S)

#define ONEWARE_MACHINE_SUBCREATE( S,P ) BURST_OBJECT_SUBCREATE(oneware_machine,S,P)

#define ONEWARE_MACHINE_SUBSETUP( S,P )  BURST_OBJECT_SUBSETUP(oneware_machine,S,P)

#endif



