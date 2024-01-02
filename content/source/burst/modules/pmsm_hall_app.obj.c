#include "burst\modules\pmsm_hall_app.h"

BURST_PS_CREATE(power)

NIKITIN_CREATE(speedse)

ENCO_ABS32_CREATE(enco)

BURST_PI_CREATE(c_lat_pi)

BURST_PI_CREATE(c_cross_pi)

BURST_PI_CREATE(c_hi_pi)

BURST_PI_CREATE(c_lo_pi)

NIKITIN_CREATE(c_lat_flt)

NIKITIN_CREATE(c_cross_flt)

BURST_MOTION_CREATE(motion)

BURST_POSITIONER_CREATE(positioner)

//swt
#define CLCH_NAME swt
#include "burst/cliche/swt.h"
