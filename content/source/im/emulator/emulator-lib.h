#ifndef _emulator_lib_h
#define _emulator_lib_h
#include "core/robosd_common.h"

#include "__robosd_head_begin.h"

ROBO_EXPORT robo_result_t ROBO_DECL emulator_init(const robo_string_t _ini);

ROBO_EXPORT void ROBO_DECL emulator_deinit(void);

ROBO_EXPORT void ROBO_DECL emulator_realtime_run(void);
ROBO_EXPORT void ROBO_DECL emulator_loop_run(void);

#include "__robosd_head_end.h"

#endif
