/*
typedef struct burst_proto_perfomer_s {
	uint8_t target;

} burst_proto_perfomer_t;
typedef burst_proto_perfomer_t * burst_proto_perfomer_p;

typedef struct burst_proto_s {
	uint8_t target;
} burst_proto_t;
typedef burst_proto_perfomer_t * burst_proto_perfomer_p;
*/
/*



void burst_proto_pool(void);
*/

#include "burst/burst_common.h"

#include "burst/cliche/_begin.h"

#define INCOM_PACKET PREFIX(in_packet_t)
#define OUTCOM_PACKET PREFIX(out_packet_t)

#define INCOM_PACKET_P INCOM_PACKET *
#define OUTCOM_PACKET_P OUTCOM_PACKET *

#ifdef CLCH_HEADER
	#include "burst/burst_timer.h"
	uint32_t PREFIX(hw_critical_enter)(void);
	void PREFIX(hw_critical_leave)(uint32_t);
	void PREFIX(begin)(void);
	void PREFIX(poll)(void);
	void PREFIX(reset)(void);
	void PREFIX(post)(OUTCOM_PACKET_P);
	void PREFIX(on_receive)( INCOM_PACKET_P );
	INCOM_PACKET_P PREFIX(incom_packet_get)(void);
	OUTCOM_PACKET_P PREFIX(outcom_packet_get)(void);
	void PREFIX(on_post_refuse)(void);
	void PREFIX(on_post_confirm)(void);
	void PREFIX(hw_post)(OUTCOM_PACKET_P);
	void PREFIX(hw_post_abort)(void);
	burst_bool_t PREFIX(hw_ready)(void);
	void PREFIX(sw_outcom_confirm)(OUTCOM_PACKET_P);
	void PREFIX(sw_incom_perform)(INCOM_PACKET_P);
#else

	#define RING_PREFIX_NAME PREFIX(incom_queue)
	#define RING_LOCK() uint32_t context = PREFIX(hw_critical_enter)()
	#define RING_UNLOCK() PREFIX(hw_critical_leave)(context)
	#define RING_DATA_T intptr_t
	#define RING_SIZE_BITS PREFIX(INCOM_SIZE_BITS)
	#include "burst/burst_ring.inc.h"
	
	#define RING_PREFIX_NAME PREFIX(incom_pool)
	#define RING_LOCK() uint32_t context = PREFIX(hw_critical_enter)()
	#define RING_UNLOCK() PREFIX(hw_critical_leave)(context)
	#define RING_DATA_T intptr_t
	#define RING_SIZE_BITS PREFIX(INCOM_SIZE_BITS)
	#include "burst/burst_ring.inc.h"

	#define RING_PREFIX_NAME PREFIX(outcom_queue)
	#define RING_LOCK() uint32_t context = PREFIX(hw_critical_enter)()
	#define RING_UNLOCK() PREFIX(hw_critical_leave)(context)
	#define RING_DATA_T intptr_t
	#define RING_SIZE_BITS PREFIX(OUTCOM_SIZE_BITS)
	#include "burst/burst_ring.inc.h"

	#define RING_PREFIX_NAME PREFIX(outcom_pool)
	#define RING_LOCK() uint32_t context = PREFIX(hw_critical_enter)()
	#define RING_UNLOCK() PREFIX(hw_critical_leave)(context)
	#define RING_DATA_T intptr_t
	#define RING_SIZE_BITS PREFIX(OUTCOM_SIZE_BITS)
	#include "burst/burst_ring.inc.h"


	#define RING_PREFIX_NAME PREFIX(outcom_answ)
	#define RING_LOCK() uint32_t context = PREFIX(hw_critical_enter)()
	#define RING_UNLOCK() PREFIX(hw_critical_leave)(context)
	#define RING_DATA_T intptr_t
	#define RING_SIZE_BITS PREFIX(OUTCOM_SIZE_BITS)
	#include "burst/burst_ring.inc.h"
	
	#define  POP(L) POP_(PREFIX(L))
	#define  POP_(L) POP__(L)
	#define  POP__(L) L##_pop  
	OUTCOM_PACKET_P POP(outcom_queue)(void);
	OUTCOM_PACKET_P POP(outcom_pool)(void);
	OUTCOM_PACKET_P POP(outcom_answ)(void);
	INCOM_PACKET_P POP(incom_queue)(void);
	INCOM_PACKET_P POP(incom_pool)(void);

	#define  PUSH(L) PUSH_(PREFIX(L))
	#define  PUSH_(L) PUSH__(L)
	#define  PUSH__(L) L##_push
	void PUSH(outcom_queue)(OUTCOM_PACKET_P);
	void PUSH(outcom_pool)(OUTCOM_PACKET_P);
	void PUSH(outcom_answ)(OUTCOM_PACKET_P);
	void PUSH(incom_pool)(INCOM_PACKET_P);
	void PUSH(incom_queue)(INCOM_PACKET_P);
	
	OUTCOM_PACKET PREFIX(outcom_packet_arr_) [1<<PREFIX(OUTCOM_SIZE_BITS)] = {};
	INCOM_PACKET PREFIX(incom_packet_arr_) [1<<PREFIX(INCOM_SIZE_BITS)] = {};
	
	void PREFIX(begin)(void){
		{
			OUTCOM_PACKET_P p = PREFIX(outcom_packet_arr_);
			for(int i=0 ; i < (1<<PREFIX(OUTCOM_SIZE_BITS)) ; ++i,++p){
				PUSH(outcom_pool)(p);
			}
		}
		{
			INCOM_PACKET_P p = PREFIX(incom_packet_arr_);
			for(int i=0 ; i < (1<<PREFIX(INCOM_SIZE_BITS)) ; ++i,++p){
				PUSH(incom_pool)(p);
			}
		}
	}

	typedef enum{ PREFIX(STATUS_IDLE) = 0, PREFIX(STATUS_TRANSPORT) =1, PREFIX(STATUS_REPEAT) =2 } PREFIX(status_t);

	OUTCOM_PACKET_P	PREFIX(outcom_) = 0;
	burst_time_us_t PREFIX(outcom_start_) = 0;
	PREFIX(status_t) 	PREFIX(status) = PREFIX(STATUS_IDLE);
	int PREFIX(outcom_repeat_) = 0;
	

	void PREFIX(poll)(void){
		switch(PREFIX(status)){
			case PREFIX(STATUS_IDLE):
			if(PREFIX(hw_ready)()){
				PREFIX(outcom_) = POP(outcom_queue)();
				if( PREFIX(outcom_) ){
					PREFIX(outcom_start_) = burst_time_us();
					PREFIX(outcom_repeat_) = 0;
					PREFIX(status) =  PREFIX(STATUS_TRANSPORT);
					PREFIX(hw_post)(PREFIX(outcom_));
				}
			}
			break;
			case PREFIX(STATUS_TRANSPORT):
			{
				burst_time_us_t now = burst_time_us();
				if( now - PREFIX(outcom_start_) > ( (burst_packet_p)PREFIX(outcom_))->timeout ){
					PREFIX(hw_post_abort)();
					PREFIX(on_post_refuse)();
				}
			}
			break;				
			case PREFIX(STATUS_REPEAT):
			if(PREFIX(hw_ready)()){
				PREFIX(outcom_start_) = burst_time_us();
				PREFIX(hw_post)(PREFIX(outcom_));
				PREFIX(status) =  PREFIX(STATUS_TRANSPORT);
			}
			break;
		}
		{
			OUTCOM_PACKET_P p = POP(outcom_answ)();
			if(p){
				PREFIX(sw_outcom_confirm)(p);
				PUSH(outcom_pool)(p);
			}
		}
		{
			INCOM_PACKET_P p = POP(incom_queue)();
			if(p){
				PREFIX(sw_incom_perform)(p);
				PUSH(incom_pool)(p);
			}
		}
	}

	void PREFIX(on_post_refuse)(void){
		if(PREFIX(outcom_)){
			PREFIX(outcom_repeat_)++;
			burst_packet_p packet = (burst_packet_p)PREFIX(outcom_);
			if( PREFIX(outcom_repeat_) <= packet->repeat_count  ){
				PREFIX(status) =  PREFIX(STATUS_REPEAT);
			} else{ 
				PREFIX(status) =  PREFIX(STATUS_IDLE);
				packet->result =  burst_false;
				PUSH(outcom_answ)(PREFIX(outcom_));
				PREFIX(outcom_) = 0;
			}
		} else{
			PREFIX(status) =  PREFIX(STATUS_IDLE);
		}
	}
	
	void PREFIX(on_post_confirm)(void){
		PREFIX(status) =  PREFIX(STATUS_IDLE);
		if(PREFIX(outcom_)){
			burst_packet_p packet = (burst_packet_p)PREFIX(outcom_);
			packet->result =  burst_true;
			PUSH(outcom_answ)(PREFIX(outcom_));
			PREFIX(outcom_) = 0;
		} else {
			PREFIX(status) =  PREFIX(STATUS_IDLE);
		}
	}
	
	void PREFIX(reset)(void){
		if(PREFIX(outcom_)){
			PUSH(outcom_pool)(PREFIX(outcom_));
		}
		{
			OUTCOM_PACKET_P p = POP(outcom_answ)();
			while(p != 0 ){
				PUSH(outcom_pool)(p);
				p = POP(outcom_answ)();
			}
		}
		
		{
			OUTCOM_PACKET_P p = POP(outcom_queue)();
			while(p != 0 ){
				PUSH(outcom_pool)(p);
				p = POP(outcom_queue)();
			}
		}
		{
			INCOM_PACKET_P p = POP(incom_queue)();
			while(p != 0 ){
				PUSH(incom_pool)(p);
				p = POP(incom_queue)();
			}
		}

	}
	
	void PREFIX(on_receive)(INCOM_PACKET_P _p){
		PUSH(incom_queue)(_p);		
	}
		
	void PREFIX(post)(OUTCOM_PACKET_P _p ){
		PUSH(outcom_queue)(_p);		
	}
	INCOM_PACKET_P PREFIX(incom_packet_get)(void){
		return POP(incom_pool)();	
	}
	OUTCOM_PACKET_P PREFIX(outcom_packet_get)(void){
		return POP(outcom_pool)();	
	}
	
	BURST_WEAK void PREFIX(hw_post)(OUTCOM_PACKET_P _p){
		BURST_UNUSED(_p);
		PREFIX(on_post_refuse)();
	}

	BURST_WEAK burst_bool_t PREFIX(hw_ready)(void){
		return burst_true;
	}
	BURST_WEAK void PREFIX(hw_post_abort)(void){
	}
	BURST_WEAK uint32_t PREFIX(hw_critical_enter)(void){
		return 0xffffffff;
	}
	BURST_WEAK void PREFIX(hw_critical_leave)(uint32_t _ctx){
		BURST_UNUSED(_ctx);
	}
	
	BURST_WEAK void PREFIX(sw_outcom_confirm)(OUTCOM_PACKET_P _p){
		BURST_UNUSED(_p);
	}

	BURST_WEAK void PREFIX(sw_incom_perform)(INCOM_PACKET_P _p){
		BURST_UNUSED(_p);
	}


	#undef POP
	#undef POP_
	#undef POP__
	#define  POP(L,P) POP_(PREFIX(L),P)
	#define  POP_(L,P) POP__(L,P)
	#define  POP__(L,P) P L##_pop(void) { intptr_t tmp = 0; L##_buf_get(&tmp,1); return (P)tmp; }  
	POP(outcom_queue,OUTCOM_PACKET_P)
	POP(outcom_pool,OUTCOM_PACKET_P)
	POP(outcom_answ,OUTCOM_PACKET_P)
	POP(incom_queue,INCOM_PACKET_P)
	POP(incom_pool,INCOM_PACKET_P)

	#undef PUSH
	#undef PUSH_
	#undef PUSH__
	#define  PUSH(L,P) PUSH_(PREFIX(L),P)
	#define  PUSH_(L,P) PUSH__(L,P)
	#define  PUSH__(L,P) void L##_push( P _p ) { intptr_t tmp  = (intptr_t)_p;L##_buf_put(&tmp,1);}  
	PUSH(outcom_queue,OUTCOM_PACKET_P);
	PUSH(outcom_pool,OUTCOM_PACKET_P)
	PUSH(outcom_answ,OUTCOM_PACKET_P)
	PUSH(incom_pool,INCOM_PACKET_P)
	PUSH(incom_queue,INCOM_PACKET_P)


	#undef POP
	#undef POP_
	#undef POP__
	#undef PUSH
	#undef PUSH_
	#undef PUSH__
	
#endif
#undef OUTCOM_PACKET_P
#undef INCOM_PACKET_P
#include "burst/cliche/_end.h"


