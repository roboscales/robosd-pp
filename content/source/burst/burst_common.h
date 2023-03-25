#ifndef burst_common_h
#define burst_common_h
#include <stdint.h>
#include "burst_app_tuning.h"
#include "burst_target.h"
#include "burst_front.h"

#ifndef BURST_DATA_SIZE_TYPE
#define BURST_DATA_SIZE_TYPE unsigned int
#endif
typedef  BURST_DATA_SIZE_TYPE  burst_size_t;

#ifndef BURST_DATA_TYPE
#define BURST_DATA_TYPE uint8_t
#endif

typedef BURST_DATA_TYPE burst_data_t;
typedef burst_data_t * burst_data_p;
typedef  burst_size_t *   burst_size_p;

typedef enum  { burst_true = 1, burst_false = 0 } burst_bool_t;
typedef enum  { burst_backend = 1, burst_frontend = 0 } burst_thread_t;
typedef enum  { burst_complete = 1, burst_panic = 0, burst_fault = 3, burst_continue =2 } burst_run_t;
typedef enum{burst_satstate_none = 0,burst_satstate_lo=1,burst_satstate_hi=2,burst_satstate_both=3} burst_satstate_t;

enum{ VERB_REALTIME = 1,VERB_BACKEND = 2, VERB_LOOP = 3, VERB_FRONTEND = 4};

#endif
