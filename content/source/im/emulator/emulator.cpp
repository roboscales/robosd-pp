#include <thread>
#include <iostream>
#include <chrono>
#include "core/robosd_system.hpp"
#include "im/edev/edev.hpp"
#include <fstream> 
#include <windows.h> 
#if ROBO_UNICODE_ENABLED ==1
int wmain(int _argc, robo::cstr _argv[]) 
#else
int main(int _argc, const char * _argv[]) 
#endif
{

	typedef std::chrono::high_resolution_clock Time;
	typedef std::chrono::duration<double> fsec;
	#if 0
	static struct monitor {
		enum { count = 10, length = 100 };
		struct meandr_s {
			struct point {
				double tm;
				int flag;
			};
			Time::time_point t0 = Time::now() ;

			point points[length] = {};
			point* current = points;
			const point* last = points + (length - 1);
			int flag = 0;
			void tick(void) {
				auto t1 = Time::now();
				fsec fs = t1 - t0;
				current->tm = fs.count();
				flag = 1 - flag;
				current->flag = flag;
				if (last == current) {
					current = points;
				}
				else {
					current++;
				}
			}
		};
		meandr_s meandr[count];
	} monitor;
	Time::time_point tk = Time::now();
	std::thread* th[monitor::count];
	for (int i = 0; i < monitor::count; ++i) {
		th[i] = new std::thread([i] {
			SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
			for (int j = 0; j < monitor::length; ++j) {
				monitor.meandr[i].tick();
				using namespace std::chrono_literals;
				std::this_thread::sleep_for(std::chrono::microseconds(500));
			}
		});
	}
	for (int i = 0; i < monitor::count; ++i) {
		std::thread* tmp = th[i];
		tmp->join();
		delete tmp;
	}
	Time::time_point to = Time::now();
	fsec fs = to - tk;
	std::cout << fs.count() << " " << fs.count() / monitor::length / monitor::count*1000000 ;

	std::ofstream  lg;
	lg.open("log.txt");
	for (int j = 0; j < monitor::length; ++j) {
		lg << monitor.meandr[0].points[j].tm << " " << monitor.meandr[0].points[j].flag << std::endl;
	}

	#endif

	#if 1
	robo::cstr emu_ini;

	if (_argc>1){
		emu_ini = _argv[1];
	}
	else{
		emu_ini = RT("emulator.ini");
	}
	bool terminated = false;
	
	uint8_t* a = new  uint8_t[10];
	delete[] a;
	
	ROBO_JAMPN( robo::system::consol::begin([&](robo::system::consol::event _ev) {terminated = true; }), crash);
	ROBO_JAMPN(robo::edev::agent::begin(emu_ini), crash);
	//robo::system::env::begin();
	{
		auto thread_count = robo::edev::agent::threads().count();
		std::thread** threads = nullptr;
		if (thread_count) {
			threads = new std::thread * [thread_count];
			std::thread** pthread = threads;
			for (auto* it = robo::edev::agent::threads().first(); it; it = it->next(),++pthread) {
				robo::edev::agent::thread* th = &(it->owner());
				(*pthread) = new std::thread([ th, &terminated] {
					auto t0 = Time::now();
					while (!terminated) {
						auto t1 = Time::now();
						fsec fs = t1 - t0;
						auto sec = fs.count();
						robo::edev::agent::run(sec);
					}
				});
			}
		}
		
		std::thread th(
			[&]() {
				auto t0 = Time::now();
				while (!terminated) {
					auto t1 = Time::now();
					fsec fs = t1 - t0;
					auto sec = fs.count();
					robo::edev::agent::backgrounf_run(sec);
				}
			}
		);
		
		/*
		auto t0 = Time::now();
		while (!terminated) {
			auto t1 = Time::now();
			fsec fs = t1 - t0;
			sec = fs.count();
			robo::edev::agent::run(sec);
		}
		*/
		th.join();
		if (thread_count) {
			for (int i = 0; i < thread_count; ++i) {
				std::thread* tmp = threads[i];
				tmp->join();
				delete tmp;
			}
		}
	}
crash:
	robo::edev::agent::finish();
	robo::system::ini::finish();
	robo::system::consol::finish();

	char c;
	std::cin >> c;
#endif
	return 0;
	
}

