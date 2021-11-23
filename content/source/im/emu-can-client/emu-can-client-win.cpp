#include <iostream>
#include <windows.h>

#include "core/robosd_os.h"
#include "core/robosd_log.h"
#include "core/robosd_mem.h"
#include "core/robosd_ini.h"
#include "core/robosd_string.h"

#include "net/can/platform/emulator/emu_can_common.h"
emu_can_port_p ports;

int    port_count=0;
double period = 1.0 / 20000;
//emu_dev_agent_p agents = 0;
BOOL CtrlHandler(DWORD fdwCtrlType);
byte fRun = 1;

bool received = false;
#include "mexo/net/flow/flow_can_msg_id.h"
flow_msg_id_t id;

extern "C"{
	void can_on_receive_(robo_can_p _can, robo_can_msg_id_t _id, uint8_t * _data, robo_size_t _size);
}

void can_on_receive_(robo_can_p _can, robo_can_msg_id_t _id, uint8_t * _data, robo_size_t _size){
	emu_can_port_p _port = (emu_can_port_p)_can;
	id.value = _id;
	printf("%d \t", _port->can.channel);
	printf("0x%x%x%x ", (unsigned int)((_id & 0xF00) >> 8), (unsigned int)((_id & 0xF0) >> 4), (unsigned int)(_id & 0xF));
	printf("\t%d [", _size);
	for (robo_size_t i = 0; i < _size; i++)
		printf("0x%x%x ", (unsigned int)((_data[i] & 0xF0) >> 4), (unsigned int)(_data[i] & 0xF));
	printf("]\n");
	if (id.slave == 1)
		received = true;
}



robo_result_t init_(const robo_string_t _ini){
	int mask; int verb; int tmp;
	robo_char_t section[ROBO_INI_SECTION_MAX_LEN + 1];
	/*int i, ch, type, id=1111;  // int heap_size;
	robo_char_t dll[ROBO_LIB_NAME_MAX_LEN + 1];
	emu_dev_agent_p _dev_agent = 0;*/
    robo_os_init();
	robo_log_init(0,0, robo_os_log_to_consol);
	robo_log(0,0, "load from %s ", _ini);
	ROBO_CHECKRET(robo_ini_init(_ini));
	robo_ini_load_int("OPTION", "DEBUG_VERB", 0, &verb);
	robo_ini_load_int("OPTION", "DEBUG_MASK", 0, &mask);
	robo_log_init(verb, mask, robo_os_log_to_consol); //здесь можно инициализировать логер
	robo_ini_load_double("OPTION", "GLOBAL_RUN_PERIOD", 0.000005, &period);
	robo_ini_load_int("OPTION", "PORT_COUNT", 1, &port_count);
	if (port_count > 0){
		ports = new emu_can_port_t[port_count];
		emu_can_port_p port = ports;
		if (ports){
			for (int i = 0; i < port_count; i++, port++){
				robo_sprintf(section, ROBO_INI_SECTION_MAX_LEN, "PORT_%d", i);
				robo_ini_load_int(section, "CHANNEL_ID", -1, &tmp);
				port->can.channel = tmp;
				robo_ini_load_int(section, "REPEAT_MAX_COUNT", -1, &(port->repeat_max_count));
				port->can.on_receive = can_on_receive_;
				robo_can_open( &(port->can) );
			}
		}
	}


/*	ROBO_CHECKRET(dev_init());
	agents = new emu_dev_agent_t[device_count];
	ZeroMemory(agents, device_count*sizeof(emu_dev_agent_t));
	_dev_agent = agents;
    for (i=0;i<device_count;i++){
		robo_sprintf(section, ROBO_INI_SECTION_MAX_LEN, "DEVICE_%d", i);

		robo_ini_load_int(section, "ID", -1, &id);
		robo_ini_load_int(section, "TYPE", 0, &type);

		robo_sprintf(section, ROBO_INI_SECTION_MAX_LEN, "DEVICE_TYPE_%d", type);
		robo_ini_load_str(section, "LIB", "emumeXo.dll", dll, ROBO_LIB_NAME_MAX_LEN);
		_dev_agent->ref.id = id;
		_dev_agent->type = type;
		_dev_agent->index = i;
		ROBO_CHECKRET(dev_load_p(_dev_agent, dll));
		_dev_agent++;
    }*/

    SetConsoleCtrlHandler( (PHANDLER_ROUTINE) CtrlHandler, TRUE );
    return ROBO_SUCCESS;
}

void deinit_(void){
    //dev_deinit();

	if (ports){
		emu_can_port_p port = ports;
		for (int i = 0; i < port_count; i++, port++){
			robo_can_close( &(port->can));
		}
	}
	delete[] ports;
	ports = 0;
    robo_log_deinit();
	robo_ini_deinit();
	robo_os_deinit();
}

int cnt = 0;


int main(int argc, char* argv[])
{
    int n=0;  
    double t = 0;
    int count=0;
    LARGE_INTEGER ticksPerSecond;
    LARGE_INTEGER tick0;   // A point in time
    LARGE_INTEGER tick1;   // A point in time
    unsigned long tick_delay;   // For converting tick into real time
    LARGE_INTEGER ticb;   // A point in time
    LARGE_INTEGER tice;   // A point in time
    static unsigned long calc_delay;   // For converting tick into real time
    static volatile double mean_calc_delay=0;   // For converting tick into real time
    const char * ini=0;
    if(argc>1){
        ini = argv[1];
    }else{
        ini = "*\\emulator-test.ini";

    }
	if (init_( (const robo_string_t) ini) >= ROBO_SUCCESS){
		QueryPerformanceFrequency(&ticksPerSecond);
		tick_delay = (unsigned long)(period*ticksPerSecond.QuadPart);
		QueryPerformanceCounter(&tick0);
		tick1.QuadPart = tick0.QuadPart + tick_delay;
		while (fRun){
			count++;
			QueryPerformanceCounter(&tick0);
			if (tick0.QuadPart > tick1.QuadPart){
				QueryPerformanceCounter(&ticb);
				QueryPerformanceCounter(&tice);
				cnt++;
				//MEXO_OUT_STREAM_UPDATE = 0, MEXO_OUT_STREAM_GET_PAGE = 0x80, MEXO_OUT_STREAM_GET_NEXT_PAGE
				uint8_t tmp;
				switch (cnt){
					case 1:
						id.addr = 0xA;
						id.request = 1;
						id.slave = 0;
						id.suba = 0x1;
						tmp = 0;
						if (robo_can_send( &(ports->can), id.value, &tmp, 1) < ROBO_SUCCESS){
							cnt--;
						}
						else{
							received = true;
						}
						break;
					case 2:
					case 3:
					case 4:
					case 5:
						id.addr = 0xA;
						id.request = 0;
						id.slave = 0;
						id.suba = 0x01;
						tmp = 8;
						if (!received || robo_can_send(&(ports->can), id.value, &tmp, 1) < ROBO_SUCCESS){
							cnt--;
						}else{
							received = false;
						}

						break;
					case 6:
						id.addr = 0xA;
						id.request = 0;
						id.slave = 0;
						id.suba = 0x01;
						tmp = 3;
						if (!received || robo_can_send(&(ports->can), id.value, &tmp, 1) < ROBO_SUCCESS){
							cnt--;
						}
						else{
							received = false;
						}
						break;
					case 7:
						if (!received) cnt--; else	cnt = 0;
						break;
				}
				
				if (ports){
					emu_can_port_p port = ports;
					for (int i = 0; i < port_count; i++, port++){
						emu_can_port_poll(port);
					}
				}
				
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
	}
	deinit_();
	scanf("%d", &n);
	return 0;
}
//---------------------------------------------------------------------------


BOOL CtrlHandler( DWORD fdwCtrlType )
{
  fRun = 0;
  switch( fdwCtrlType )                                                     
  {
    // Handle the CTRL-C signal.
    case CTRL_C_EVENT:
      return( TRUE );

    // CTRL-CLOSE: confirm that the user wants to exit.
    case CTRL_CLOSE_EVENT:
      return( TRUE );

    // Pass other signals to the next handler.
    case CTRL_BREAK_EVENT:
      return ( TRUE );

    case CTRL_LOGOFF_EVENT:
      return FALSE;

    case CTRL_SHUTDOWN_EVENT:
      return FALSE;

    default:
      return FALSE;
  }
}

