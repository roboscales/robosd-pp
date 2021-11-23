#include <thread>
#include <iostream>
#include "im/emulator/emulator-lib.h"
#include "core/robosd_os.h"



bool running = true;

void robo_consol_break(){
	running = false;
}



int main(int _argc, const char * _argv[]){

	const char * emu_ini;

	if (_argc>1){
		emu_ini = _argv[1];
	}
	else{
		emu_ini = "emulator.ini";
	}

	robo_consol_begin(robo_consol_break);

	if (emulator_init((robo_string_t)emu_ini) == ROBO_SUCCESS){
		while (running){
			emulator_realtime_run();
			std::this_thread::sleep_for(std::chrono::seconds(0));
		}
		emulator_deinit();
		
	}
	char c;
	std::cin >> c;
	return 0;
}

