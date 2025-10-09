#include <iostream>
using namespace std;

#include <thread>
#include <iostream>
#include <chrono>
#include "core/robosd_system.hpp"
#include "im/edev/edev.hpp"
#include <fstream> 

#if ROBO_UNICODE_ENABLED ==1
int wmain(int _argc, robo::cstr _argv[]) 
#else
int main(int _argc, const char * _argv[]) 
#endif
{
	typedef std::chrono::high_resolution_clock Time;
	typedef std::chrono::duration<double> fsec;

	robo::cstr emu_ini;

	if (_argc>1){
		emu_ini = _argv[1];
	}
	else{
		emu_ini = RT("/home/anyusupov/sourcetree/damper/content/reference/settings-ref/im/dap-robot-lx.ini");
	}
	
	bool terminated = false;
	
	ROBO_JAMPN( robo::system::consol::begin([&](robo::system::consol::event _ev) {terminated = true; }), crash);
	
	ROBO_JAMPN(robo::edev::agent::begin(emu_ini), crash);
	//robo::system::env::begin();
	{
		double next_time_ = 0;
		double begin_time_ = 0.;


		std::thread th(
			[&]() {
				auto t0 = Time::now();
				while (!terminated) {
					auto t1 = Time::now();
					fsec fs = t1 - t0;
					auto sec = fs.count();
					try {
						robo::edev::agent::backgrounf_run(sec - begin_time_);
					}
					catch (const std::exception& /*err*/)
					{
						terminated = true;
					}
				}
			}
		);

		
		auto t0 = Time::now();
		double sec_stat_acc = 0.;
		int cnt =0;
		while (!terminated) {
			auto t1 = Time::now();
			fsec fs = t1 - t0;
			auto sec = fs.count();
			if (begin_time_ == 0.) { 
				begin_time_ = sec; 
				next_time_ = sec;
			}
			
			robo::edev::agent::run(sec - begin_time_);
			auto t2 = Time::now();
			fsec dfc = t2 - t1;
			auto dsec = dfc.count();
			if (dsec > 0.000010) {
				sec_stat_acc += dsec;
				cnt++;
				if (cnt == 10000) {
					auto dt = 1000000. * sec_stat_acc / cnt;
					if (dt > 0.1) {
						robo_infolog("run period us: %f", dt);
					}
					cnt = 0;
					sec_stat_acc = 0;
				}
			}
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
