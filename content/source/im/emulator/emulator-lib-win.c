
#include <windows.h>

#include "im/edev/edev.h"
#include "core/robosd_os.h"
#include "core/robosd_log.h"
#include "core/robosd_mem.h"
#include "core/robosd_ini.h"
#include "core/robosd_string.h"
#include <stdio.h>

#include "im/emulator/emulator-lib.h"


int    device_count=0;
double period = 1.0 / 20000;
emu_dev_agent_p agents = 0;
LARGE_INTEGER ticksPerSecond;
LARGE_INTEGER tick0;   // A point in time
LARGE_INTEGER tick1;   // A point in time
unsigned long tick_delay;   // For converting tick into real time
LARGE_INTEGER ticb;   // A point in time
LARGE_INTEGER tice;   // A point in time


robo_result_t ROBO_DECL emulator_init(const robo_string_t _ini){
	int i, id=1111; int verb; int mask;// int heap_size;
	robo_char_t section[ROBO_INI_SECTION_MAX_LEN + 1];
	robo_char_t dll[ROBO_LIB_NAME_MAX_LEN + 1];
	robo_char_t type_[ROBO_LIB_NAME_MAX_LEN + 1];
	emu_dev_agent_p _dev_agent = 0;
    robo_os_init();
	robo_log_init(0,0, robo_os_log_to_consol);
	robo_log(0,0, "load from %s ", _ini);
	ROBO_CHECKRET(robo_ini_init(_ini));
	robo_ini_load_int("OPTION", "DEBUG_VERB", 0, &verb);
	robo_ini_load_int("OPTION", "DEBUG_MASK", 0, &mask);
	robo_log_init(verb, mask, robo_os_log_to_consol); //здесь можно инициализировать логер
	//robo_ini_load_int("OPTION", "HEAP_SIZE", 1000000, &heap_size);
	//robo_mem_set_size(heap_size);
	ROBO_CHECKRET(dev_init());
    robo_ini_load_int("OPTION","DEV_COUNT",1,&device_count);
	robo_ini_load_double("OPTION", "GLOBAL_RUN_PERIOD", 0.000005, &period);
	agents = malloc( sizeof(emu_dev_agent_t) * device_count );
	ZeroMemory(agents, device_count*sizeof(emu_dev_agent_t));
	_dev_agent = agents;
    for (i=0;i<device_count;i++){
		robo_sprintf(section, ROBO_INI_SECTION_MAX_LEN, "DEVICE_%d", i);

		robo_ini_load_int(section, "ID", -1, &id);
		robo_ini_load_str(section, "TYPE", "", type_, ROBO_LIB_NAME_MAX_LEN);
		robo_sprintf(section, ROBO_INI_SECTION_MAX_LEN, "DEVICE_TYPE_%s", type_);
		robo_ini_load_str(section, "LIB", "emumeXo.dll", dll, ROBO_LIB_NAME_MAX_LEN);
		_dev_agent->ref.id = id;
		robo_string_create_copy(type_, &(_dev_agent->type));
		_dev_agent->index = i;
		ROBO_CHECKRET(dev_load_p(_dev_agent, dll));
		_dev_agent++;
    }

	QueryPerformanceFrequency(&ticksPerSecond);
	tick_delay = (unsigned long)(period*ticksPerSecond.QuadPart);
	QueryPerformanceCounter(&tick0);
	QueryPerformanceCounter(&tick1);

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
	static int n = 0;
	static double t = 0;
	static int count = 0;
    static unsigned long calc_delay;   // For converting tick into real time
    static volatile double mean_calc_delay=0;   // For converting tick into real time
	count++;
	QueryPerformanceCounter(&tick0);
	if (tick0.QuadPart > tick1.QuadPart){
		QueryPerformanceCounter(&ticb);
		dev_run(period, t);
		QueryPerformanceCounter(&tice);
		calc_delay = (unsigned long)(tice.QuadPart - ticb.QuadPart);
		mean_calc_delay += calc_delay;
		t += period;
		n++;
		if (n >= 2000000){
			mean_calc_delay = (1000000.0*mean_calc_delay / 2000000) / ticksPerSecond.QuadPart;
			printf("mean_calc_delay:%f\n", mean_calc_delay);
			n = 0;
			mean_calc_delay = 0;
		}
		tick1.QuadPart = tick1.QuadPart + tick_delay;
	}
}
//---------------------------------------------------------------------------

void ROBO_DECL emulator_loop_run(void)
{
	static int n = 0;
	static double t = 0;
	static unsigned long calc_delay;   // For converting tick into real time
	static volatile double mean_calc_delay = 0;   // For converting tick into real time
	
	QueryPerformanceCounter(&ticb);
	dev_run(period, t);
	QueryPerformanceCounter(&tice);
	calc_delay = (unsigned long)(tice.QuadPart - ticb.QuadPart);
	mean_calc_delay += calc_delay;
	t += period;
	n++;
	if (n >= 200000){
		mean_calc_delay = (1000000.0*mean_calc_delay / 200000) / ticksPerSecond.QuadPart;
		printf("mean_calc_delay:%f\n", mean_calc_delay);
		n = 0;
		mean_calc_delay = 0;
	}
}
