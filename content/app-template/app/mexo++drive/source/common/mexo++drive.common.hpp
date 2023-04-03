#ifndef mexopp_drive_common_hpp
#define mexopp_drive_common_hpp
#include "mexo++drive.names.hpp"

#ifdef MEXO_DRIVE_TUNING
#include MEXO_DRIVE_TUNING
#endif

#ifndef MEXO_DRIVE_NUMBER_TEST_ENABLED
#define MEXO_DRIVE_NUMBER_TEST_ENABLED 0
#endif

#ifndef MPPD_CORE_CLOCK_HZ
#define MPPD_CORE_CLOCK_HZ 72000000
#endif

#define MPPD_CORE_TICK_PER_US (MPPD_CORE_CLOCK_HZ+499999)/1000000


#endif