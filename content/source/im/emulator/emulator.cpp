#include <thread>
#include <iostream>
#include "im/edev/edev.hpp"
#include "core/robosd_system.hpp"

#include <chrono>
#if ROBO_UNICODE_ENABLED ==1
int wmain(int _argc, robo::cstr _argv[]) 
#else
int main(int _argc, robo::cstr _argv[]) 
#endif
{
	robo::cstr emu_ini;

	if (_argc>1){
		emu_ini = _argv[1];
	}
	else{
		emu_ini = RT("emulator.ini");
	}
	bool terminated = false;
	typedef std::chrono::high_resolution_clock Time;
	typedef std::chrono::duration<double> fsec;
	auto t0 = Time::now();

	ROBO_JAMPN( robo::system::consol::begin([&](robo::system::consol::event /**/) {terminated = true; }), crash);
	ROBO_JAMPN(robo::system::ini::begin(emu_ini), crash);
	ROBO_JAMPN(robo::edev::agent::begin(), crash);
	robo::system::env::begin();

	
	while (!terminated) {
		auto t1 = Time::now();
		fsec fs = t1 - t0;
		double sec = fs.count();
		robo::edev::agent::run(sec);
	}
crash:
	robo::edev::agent::finish();
	robo::system::ini::finish();
	robo::system::consol::finish();

	char c;
	std::cin >> c;
	return 0;
}

