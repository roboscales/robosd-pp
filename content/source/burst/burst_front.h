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

#if defined(__cplusplus)
}
#endif

#endif
