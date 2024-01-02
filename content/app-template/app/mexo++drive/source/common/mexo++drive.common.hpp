#ifndef mexopp_drive_common_hpp
#define mexopp_drive_common_hpp

#define ENV_TYPE_NONE 0

#include "mexo++drive.names.hpp"

#ifdef MPPD_BOARD_TUNING
#include MPPD_BOARD_TUNING
#endif

#ifdef MPPD_DRIVE_TUNING
#include MPPD_DRIVE_TUNING
#endif

#ifndef MPPD_DRIVE_NUMBER_TEST_ENABLED
#define MPPD_DRIVE_NUMBER_TEST_ENABLED 0
#endif

#ifndef MPPD_MEXO_TP1_ENABLED
#define MPPD_MEXO_TP1_ENABLED 0
#endif

#ifndef MPPD_MEXO_SLOT_COUNT
#define MPPD_MEXO_SLOT_COUNT 4
#endif

#ifndef MPPD_CORE_CLOCK_HZ
#define MPPD_CORE_CLOCK_HZ 72000000
#endif

#define MPPD_CORE_TICK_PER_US (MPPD_CORE_CLOCK_HZ+499999)/1000000

#ifndef MPPD_PWM_PERIOD_US
#define MPPD_PWM_PERIOD_US 50L
#endif

#ifndef MPPD_PWM_TIMER_PRESC
#define MPPD_PWM_TIMER_PRESC 1
#endif

#define MPPD_TIMER_TICK_US (MPPD_PWM_PERIOD_US*MPPD_PWM_TIMER_PRESC)
#define MPPD_PWM_MODULO ( MPPD_CPU_FREQ_HZ* MPPD_PWM_PERIOD_US / 2000000L  - 1L )

#endif