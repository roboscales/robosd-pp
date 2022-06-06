#include "core/robosd_app.hpp"
#include "core/robosd_system.hpp"
#include "core/robosd_ini.hpp"
#include <thread>
#include <iostream>
#include <chrono>
#include <thread>
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
		ini = RT("flow2serial++.ini");
	}

	std::thread backend([ini] {
		ROBO_JAMPN(robo::system::consol::begin([&](robo::system::consol::event /**/) {robo::app::machine::stop(); }), crash);
		robo_infolog("flow2serial++ begin %s", RT(""));
		

//::robo::string* sv = new ::robo::string(RT("gbplf"));
//*sv = _var;
//delete sv;

		ROBO_JAMPN(robo::app::machine::begin(ini), crash);
		ROBO_JAMPN(robo::app::machine::start(), crash);

		{
			std::thread frontend([] {
				while (!robo::app::machine::terminated()) {
					robo::app::machine::frontend_loop();
				}
			});

			while (!robo::app::machine::terminated()) {
				robo::app::machine::backend_loop();
			}
			frontend.join();
		}
	crash:
		robo::app::machine::finish();
		robo::system::consol::finish();

	});

	backend.join();
	
	robo_infolog("warlock's central was finished%s",RT(""));
	return 0;
}