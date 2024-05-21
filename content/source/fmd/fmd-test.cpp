#include "core/robosd_system.hpp"
#include "core/robosd_ini.hpp"
#include <thread>
#include <iostream>
#include <chrono>
#include <thread>
#include "fmd/fmd.hpp"

#if ROBO_UNICODE_ENABLED == 1
int wmain(int _argc, wchar_t* _argv[]) {
	const wchar_t* ini = 0;
	#else
int main(int _argc, char* _argv[]) {
	const char* ini = 0;
	#endif

	if (_argc > 1) {
		ini = _argv[1];
	}
	else {
		ini = RT("fmd-test.ini");
	}


	bool stop = false;
	fmd fmd_;

	ROBO_JAMPN(robo::system::ini::begin(ini), crash);
	ROBO_JAMPN(robo::system::consol::begin([&](robo::system::consol::event /**/) {  stop = true;  }), crash);
	robo::system::start(50000);
	ROBO_JAMPN(fmd_.begin(RT("fmd")), crash);
	ROBO_JAMPN(fmd_.start(), crash);
	//fmd_.events.
	
	/*robo::system::begin
		setup(time_us_t _period_us) {
		period_us = _period_us;
		time_us = us_acc = last_time_us = 0;
		time_ms = 0;
		terminated = false;
	}*/
	robo_infolog("fmd-test was started with ini %s", ini);
	{
		using namespace std::chrono_literals;
		std::thread th(
			[&]() {
				while (!stop) {
					std::this_thread::sleep_for(50ms);
					robo::system::backend_loop();
				}
			}
		);

		while (!stop && fmd_.poll()) {
			
		}

		th.join();
		robo_infolog("fmd-test was finished%s", RT(""));

	}
	return 0;
crash:
	robo_infolog("fmd-test terminated%s", RT(""));
	return -1;

}