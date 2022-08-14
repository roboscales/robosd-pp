#include <thread>
#include <iostream>
#include <chrono>
#include "im/edev/edev.hpp"
#include "core/robosd_system.hpp"

#if ROBO_UNICODE_ENABLED ==1
int wmain(int _argc, robo::cstr _argv[]) 
#else
int main(int _argc, robo::cstr _argv[]) 
#endif
{
	//uint8_t* a = new  uint8_t[10];
	//delete[] a;

	
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
	uint8_t* a = new  uint8_t[10];
	delete[] a;
	
	ROBO_JAMPN( robo::system::consol::begin([&](robo::system::consol::event _ev) {terminated = true; }), crash);
	ROBO_JAMPN(robo::edev::agent::begin(emu_ini), crash);
	//robo::system::env::begin();
	{
		static double sec = 0;
		std::thread th(
			[&]() {
				while (!terminated) {
					robo::edev::agent::backgrounf_run(sec);
				}
			}
		);
		while (!terminated) {
			auto t1 = Time::now();
			fsec fs = t1 - t0;
			sec = fs.count();
			robo::edev::agent::run(sec);
		}
		th.join();
	}
crash:
	robo::edev::agent::finish();
	robo::system::ini::finish();
	robo::system::consol::finish();

	char c;
	std::cin >> c;
	return 0;
}

