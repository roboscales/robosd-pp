
#include "im/edev/edev.h"
#include "core/robosd_os.h"
#include "core/robosd_log.h"
#include "core/robosd_mem.h"
#include "core/robosd_ini.h"
#include "core/robosd_string.h"
#include <stdio.h>
#include <stdlib.h>
#include "im/emulator/emulator-lib.h"
#include <time.h>
#include <pthread.h>

int    device_count=0;
double period = 1.0 / 20000;
emu_dev_agent_p agents = 0;
robo_time_us_t begin_time_us_;
robo_time_us_t current_time_us_;
robo_time_us_t next_time_us_;
robo_time_us_t period_us_;
robo_time_us_t now_us() {
	static struct timespec  tm;
	clock_gettime(CLOCK_REALTIME, &tm);
	return (robo_time_us_t)((tm.tv_nsec + 500) / 1000) + tm.tv_sec * 1000000 - begin_time_us_;
}

robo_result_t ROBO_DECL emulator_init(const robo_string_t _ini){
	int i, type, id=1111; int verb; int mask;// int heap_size;
	robo_char_t section[ROBO_INI_SECTION_MAX_LEN + 1];
	robo_char_t so[ROBO_LIB_NAME_MAX_LEN + 1];
    robo_char_t type_[ROBO_INI_SECTION_MAX_LEN + 1];

    emu_dev_agent_p _dev_agent = nullptr;
	robo_os_init();
	robo_log_init(0, 0, robo_os_log_to_consol);
    robo_log(0, 0, RS("load from %s "), _ini);
	ROBO_CHECKRET(robo_ini_init(_ini));
    robo_ini_load_int(RS("OPTION"), RS("DEBUG_VERB"), 0, &verb);
    robo_ini_load_int(RS("OPTION"), RS("DEBUG_MASK"), 0, &mask);
	robo_log_init(verb, mask, robo_os_log_to_consol);  //здесь можно инициализировать логер
	//robo_ini_load_int("OPTION", "HEAP_SIZE", 1000000, &heap_size);
	ROBO_CHECKRET(dev_init());
	robo_ini_load_int(RS("OPTION"), RS("DEV_COUNT"), 1, &device_count);
	robo_ini_load_double(RS("OPTION"), RS("GLOBAL_RUN_PERIOD_SEC"), 0.000005, &period);
	period_us_ = period*1000000;
	agents = (emu_dev_agent_p) calloc(device_count, sizeof(emu_dev_agent_t));
	_dev_agent = agents;
    for (i=0;i<device_count;i++){
		robo_sprintf(section, ROBO_INI_SECTION_MAX_LEN, RS("DEVICE_%d"), i);

		robo_ini_load_int(section, RS("ID"), -1, &id);

        robo_ini_load_str(section, "TYPE", "", type_, ROBO_INI_SECTION_MAX_LEN);
        robo_sprintf(section, ROBO_INI_SECTION_MAX_LEN, "DEVICE_TYPE_%s", type_);
	    robo_ini_load_str(section, RS("LIB"), RS("libemumeXo.so"), so, ROBO_LIB_NAME_MAX_LEN);
		_dev_agent->ref.id = id;
        robo_string_create_copy(type_, &(_dev_agent->type));
        _dev_agent->index = i;
	    ROBO_CHECKRET(dev_load_p(_dev_agent, so));
		_dev_agent++;
    }
	next_time_us_ = begin_time_us_ = 0;
	begin_time_us_ = now_us();
	return ROBO_SUCCESS;
}

void ROBO_DECL emulator_deinit(void){
    dev_deinit();
    robo_log_deinit();
	if (agents){
		free( agents);
		agents = 0;
	}
	robo_ini_deinit();
	robo_os_deinit();
}


	
void ROBO_DECL emulator_realtime_run(void)
{

	if (now_us() > next_time_us_) {
		next_time_us_ += period_us_;				
		emulator_loop_run();
	}
	
}
//---------------------------------------------------------------------------

void ROBO_DECL emulator_loop_run(void)
{
	static int n = 0;
	static double t = 0;
	static unsigned long calc_delay;   // For converting tick into real time
	static volatile double mean_calc_delay = 0;   // For converting tick into real time
	current_time_us_ =  now_us();
	dev_run(period, t);
	calc_delay = (unsigned long)(now_us() - current_time_us_);
	mean_calc_delay += calc_delay;
	t += period;
	n++;
    if (n >= 2000000) {
        mean_calc_delay = mean_calc_delay / 2000000;
		robo_detaillog(5, 1, RS("mean_calc_delay:%f\n"), mean_calc_delay);
		n = 0;
		mean_calc_delay = 0;
	}
}
