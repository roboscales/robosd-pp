#ifndef burst_enco_h
#define burst_enco_h
#include "burst/burst_signal.h"

typedef struct  {
} burst_enco_config_t;
typedef burst_enco_config_t * burst_enco_config_p;
#define ENCO_CONFIG(a) ENCO_CONFIG_(a)
#define ENCO_CONFIG_(a)\
{\
}

typedef struct  {
	burst_enco_config_p config;
	struct{
		unsigned fault;
		unsigned total;
	} counter;
	burst_signal_t delta_acc;
	burst_long_signal_t position;
	burst_bool_t ready;
	void (* run)(void);
	void (* reset)(void);
} burst_enco_t;
typedef burst_enco_t * burst_enco_p;

#define BURST_ENCO( S ) BURST_OBJECT(burst_enco,S)

#define BURST_ENCO_CREATE( S ) BURST_OBJECT_CREATE(burst_enco,S)

#define BURST_ENCO_SUBCREATE( S,P ) BURST_OBJECT_SUBCREATE(burst_enco,S,P)

#define BURST_ENCO_SUBSETUP( S,P )  BURST_OBJECT_SUBSETUP(burst_enco,S,P)

#endif
