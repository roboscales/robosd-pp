#ifndef burst_front_h
#define burst_front_h
#if defined(__cplusplus)
extern "C"
{
#endif

struct burst_dev_action_s{
	int mode;
	int actual;
};
typedef struct burst_dev_action_s burst_dev_action_t;
typedef burst_dev_action_t * burst_dev_action_p;

struct burst_dev_feedback_s{
	int mode;
};
typedef struct burst_dev_feedback_s burst_dev_feedback_t;
typedef burst_dev_feedback_t * burst_dev_feedback_p;

enum{ burst_dev_mode_idle = 0 };

enum {
	burst_panic_board_unknown_bits				= 0, burst_panic_board_unknow='U'
	, burst_panic_board_overtemp_bit			= 1, burst_panic_board_overtemp='T'
	, burst_panic_board_lotemp_bit				= 2, burst_panic_board_lotemp='t'
	, burst_panic_board_overvoltage_bit		=	3, burst_panic_board_overvoltage='V'
	, burst_panic_board_lovoltage_bit			=	4, burst_panic_board_lovoltage='v'
	, burst_panic_board_overcurrent_bit		=	5, burst_panic_board_overcurrent='C'
	, burst_panic_board_locurrent_bit		=	6, burst_panic_board_locurrent='c'
	, burst_panic_dev_unknown_bits				=	0, burst_panic_dev_unknown='U'
	, burst_panic_dev_board_bit					=	1, burst_panic_dev_board='B'
	, burst_panic_dev_master_lost_bit			=	2, burst_panic_dev_master_lost='L'
	, burst_panic_dev_bits = 2
	
};
#if defined(__cplusplus)
}
#endif

#endif
