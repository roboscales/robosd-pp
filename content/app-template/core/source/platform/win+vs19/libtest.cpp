//#include "pch.h"

#include "CppUnitTest.h"
#include "core/robosd_list.hpp"
#include "core/robosd_log.hpp"
#include "core/robosd_system.hpp"
#include "core/robosd_string.hpp"
#include "core/robosd_delegat.hpp"
#include "core/robosd_app.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>
#include <windows.h>
#include "core/robosd_backend.hpp"
#include "jsonsl.h"
#include "mexo/mexo.hpp"
#include "mexo/ps.hpp"
#include "mexo/math.hpp"
#include "core/robosd_devagent_common.hpp"

using namespace mexo;

#define MODULE_NAME_STR RT("lib.test")
namespace test {
	#if ROBO_APP_MODULE_ENABLED  == 1
	class module : public robo::app::module {
	protected:
		virtual void frontend_loop(void) {};
		virtual void backend_loop(void) {};
		module(void) : robo::app::module(MODULE_NAME_STR) {}
	public:
		static module& instance(void) {
			static module instance_;
			return instance_;
		}
	};
	#endif
};

#define MODULE_NAME test


struct command {
	enum { key_count = 11 };
	enum class format {
		key = 0
		, number = 2
		, string = 3
	};
	const char* keys[key_count] = {
		"\"timestamp\": "
		,"\"command\": "
		,"\"pos\": {"
		,"\"x\": "
		,"\"y\": "
		,"\"z\": "
		,"\"rot\": {"
		,"\"r\": "
		,"\"p\": "
		,"\"y\": "
		,"\"fingers\": "
	};
	enum class kind { stop = 0, shutdown = 1, reset = 2, status = 3 };
	enum class status { idle = 0, error = 1, moving = 2 };
	struct content {
		char timestamp[15];
		char cmd[10];
		int arm = 1;
		struct {
			float x = 0.f;
			float y = 0.f;
			float z = 0.f;
		} pos;
		struct {
			float r = 0.f;
			float p = 0.f;
			float y = 0.f;
		} rot;
		float fingers[5];
		bool error = false;;
		char buf[512];
		void rand(void) {
			sprintf(timestamp, "%d-%d-%d-%d", ::rand(), ::rand(), ::rand(), ::rand());
			pos.x = (float)::rand();
			pos.y = (float)::rand();
			pos.z = (float)::rand();
			rot.r = (float)::rand();
			rot.p = (float)::rand();
			rot.y = (float)::rand();
			for (int i = 0; i < 5; i++) fingers[i] = (float) ::rand();
		}
		void	create_arm(void) {
			const char* drmt =
				"{\"command\": \"%s\",\"timestamp\" : \"%s\",\"data\" : {\"arm\": %d,\"pos\" : {\"x\": %f,\"y\" : %f,\"z\" : %f},\"rot\" : {\"r\": %f,\"p\" : %f,\"y\" : %f},\"fingers\" : [%f, %f, %f, %f, %f]	}}";
			rand();
			sprintf(cmd, "%s", "ARM");
			buf[
				sprintf(buf, drmt, cmd, timestamp, arm, pos.x, pos.y, pos.z, rot.r, rot.p, rot.y, fingers[0], fingers[1], fingers[2], fingers[3], fingers[4])
			] = 0;
		}
	};
	bool gets(const char* _get, char* _buf) {
		return true;
	}

	bool parse(const char* _buf) {

	}
};
command::content content_;
#include "core/robosd_system_module_reg.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

/*
*/
namespace robo {
	void system::env::print(cstr _str) {
		Logger::WriteMessage(_str);
	}

	void system::env::print(robo::log::verb _verb, robo::cstr _format, va_list  _args) {
		robo::char_t buf[255];
		CHAR buf2[255];
		#if ROBO_UNICODE_ENABLED == 1
		buf[vswprintf_s(buf, 255, _format, _args)] = 0;
		#else
		buf[vsprintf(buf, _format, _args)] = 0;
		#endif
		//Logger::WriteMessage(L"и чо? ОЧЕНЬ КРУТОЙ РУССКИЙ ТеКсТ!!! ");
		Logger::WriteMessage(buf);
		Logger::WriteMessage(RT("\n"));
	}
}
namespace libtest {


	TEST_CLASS(list) {
		class item {
		public:
			::robo::list::unsorted<item>::ref ref;
			item(void) : ref(*this) {}
			operator ::robo::list::unsorted<item>::ref& () { return ref; }
		};
		class sitem {
		public:
			::robo::list::sorted<sitem, int>::ref ref;
			operator ::robo::list::sorted<sitem, int>::ref& () { return ref; }
			typedef int priority_t;
			sitem(void) : ref(*this, -1) {}
		};
		class uitem {
		public:
			::robo::list::unique<uitem, int>::ref ref;
			uitem(void) : ref(*this, -1) {}
		};
public:

	TEST_METHOD(create) {
		item it;
		Assert::IsFalse(it.ref.attached());
	}

	TEST_METHOD(attach) {
		::robo::list::unsorted<item> list;
		item it;
		it.ref.attach_to(list);
		Assert::IsTrue(it.ref.attached());
		Assert::IsTrue(list.count() == 1);
	}

	TEST_METHOD(detach) {
		::robo::list::unsorted<item> list;
		item it;
		it.ref.attach_to(list);
		it.ref.dettach();
		Assert::IsTrue(list.count() == 0);
		Assert::IsFalse(it.ref.attached());
	}

	TEST_METHOD(move) {
		::robo::list::unsorted<item> list;
		::robo::list::unsorted<item> list2;
		{
			item it;
			it.ref.attach_to(list);
			Assert::IsTrue(list.count() == 1);
			it.ref.attach_to(list2);
			Assert::IsTrue(list.count() == 0);
			Assert::IsTrue(list2.count() == 1);
		}

		Assert::IsTrue(list.count() == 0);
		Assert::IsTrue(list2.count() == 0);

	}

	TEST_METHOD(finish) {
		::robo::list::unsorted<item> list;
		{
			item it[50];
			for (int i = 0; i < 50; i++) {
				it[i].ref.attach_to(list);
			}
			Assert::IsTrue(list.count() == 50);

		}


		Assert::IsTrue(list.count() == 0);
	}
	TEST_METHOD(drop) {
		::robo::list::unsorted<item> list;
		{
			item it[50];
			for (int i = 0; i < 50; i++) {
				it[i].ref.attach_to(list);
			}
			Assert::IsTrue(list.count() == 50);

			it[25].ref.dettach();
			Assert::IsTrue(list.count() == 49);
			it[0].ref.dettach();
			Assert::IsTrue(list.count() == 48);
			it[49].ref.dettach();
			Assert::IsTrue(list.count() == 47);

			Assert::IsTrue(list.first() == &(it[1].ref));
			Assert::IsTrue(list.last() == &(it[48].ref));

			Assert::IsTrue(it[25].ref.prev() == nullptr);
			Assert::IsTrue(it[25].ref.next() == nullptr);
			Assert::IsTrue(it[0].ref.prev() == nullptr);
			Assert::IsTrue(it[0].ref.next() == nullptr);
			Assert::IsTrue(it[49].ref.prev() == nullptr);
			Assert::IsTrue(it[49].ref.next() == nullptr);

			Assert::IsTrue(it[24].ref.next() == &(it[26].ref));
			Assert::IsTrue(it[26].ref.prev() == &(it[24].ref));


			Assert::IsTrue(it[48].ref.next() == nullptr);
			Assert::IsTrue(it[1].ref.prev() == nullptr);
		}


		Assert::IsTrue(list.count() == 0);
	}

	TEST_METHOD(sorted) {
		::robo::list::sorted<sitem, int> list;
		{
			sitem it[3];
			it[0].ref.set_key(0);
			it[1].ref.set_key(2);
			it[2].ref.set_key(1);
			it[0].ref.attach_to(list);
			it[1].ref.attach_to(list);
			it[2].ref.attach_to(list);
			Assert::IsTrue(list.count() == 3);
			Assert::IsTrue(it[0].ref.next() == &(it[2].ref));
			Assert::IsTrue(it[2].ref.next() == &(it[1].ref));
			Assert::IsTrue(it[1].ref.prev() == &(it[2].ref));
			Assert::IsTrue(it[2].ref.prev() == &(it[0].ref));
		}
		Assert::IsTrue(list.count() == 0);
	}
	TEST_METHOD(unique) {
		::robo::list::unique<uitem, int> list;
		{
			uitem it[4];
			it[0].ref.set_key(0);
			it[1].ref.set_key(2);
			it[2].ref.set_key(1);
			it[3].ref.set_key(1);
			Assert::IsTrue(it[0].ref.attach_to(list));
			Assert::IsTrue(it[1].ref.attach_to(list));
			Assert::IsTrue(it[2].ref.attach_to(list));
			Assert::IsFalse(it[3].ref.attach_to(list));
			Assert::IsTrue(list.count() == 3);
			Assert::IsTrue(it[0].ref.next() == &(it[2].ref));
			Assert::IsTrue(it[2].ref.next() == &(it[1].ref));
			Assert::IsTrue(it[1].ref.prev() == &(it[2].ref));
			Assert::IsTrue(it[2].ref.prev() == &(it[0].ref));
		}
		Assert::IsTrue(list.count() == 0);
	}
	TEST_METHOD(fifo) {
		::robo::queue::fifo<item> queue;
		{
			item it[3];

			queue.push(&it[0]);
			queue.push(&it[1]);
			queue.push(&it[2]);

			Assert::IsTrue(queue.first() == &(it[0].ref));
			Assert::IsTrue(queue.last() == &(it[2].ref));
			Assert::IsTrue(queue.count() == 3);

		}
		Assert::IsTrue(queue.count() == 0);
	}
	TEST_METHOD(priority) {
		::robo::queue::priority<sitem> queue;
		{
			sitem it[3];

			queue.push(&it[0]);
			queue.push(&it[1]);
			queue.push(&it[2]);

			it[0].ref.set_key(0);
			it[1].ref.set_key(2);
			it[2].ref.set_key(1);

			Assert::IsTrue(queue.first() == &(it[0].ref));
			Assert::IsTrue(queue.last() == &(it[1].ref));
			Assert::IsTrue(queue.count() == 3);
			while (queue.pop() != nullptr);
			Assert::IsTrue(queue.count() == 0);

		}
	}
	};
	TEST_CLASS(log) {
		bool err_aram_(void) {
			ROBO_ALARM();
			ROBO_ALARMN(0);
			ROBO_ALARMN(1);
			return true;
		}
		bool err_aram2_(void) {
			ROBO_BREAKN(1, false);
			ROBO_BREAKN_F(1, false, "error %d", 1);
			ROBO_LBREAKN_F(0, "русский test %d error", -1);
			return true;
		}
		void err_aram3_(void) {
			ROBO_VBREAKN_F(1, "test error %d", 1);
			ROBO_VBREAKN_F(0, "test %d error", -1);
		}
public:
	TEST_METHOD(err_aram) {
		robo::log::begin(robo::log::verb::detail_7, 0);
		err_aram_(); ;
		err_aram3_();
		Assert::IsFalse(err_aram2_());
	}
	};

	TEST_CLASS(util) {
		#if ROBO_APP_ALLOC_ENABLED ==1
		TEST_METHOD(memo) {
			robo::system::mem::stat memstat0 = robo::system::get_mem_statistic();
			enum { cnt = 100 };
			void* ptrs[cnt] = {};
			for (int i = 0; i < 1000000; i++) {
				int no = rand() % cnt;
				void* p = ptrs[no];
				if (p == nullptr) {
					if (rand() % 100 > 98) {
						robo::system::fall  f__;
						ptrs[no] = new uint32_t[rand() % 100 + 1];
					}
					else {
						ptrs[no] = new uint32_t[rand() % 100 + 1];
					}
				}
				else {
					delete[] p;
					ptrs[no] = nullptr;
				}
			}

			for (int i = 0; i < cnt; i++) {
				void* p = ptrs[i];
				if (p != nullptr) {
					delete[] p;
				}
			}

			robo::system::mem::stat memstat1 = robo::system::get_mem_statistic();
			Assert::IsTrue(memstat0.used.size == memstat1.used.size);
		}
		#endif
		TEST_METHOD(string) {
			robo::log::begin(robo::log::verb::detail_7, 0);
			robo::string* strings[4];
			robo::string str1(RT("oppa %d %s"), 1974, RT("какая прелесть"));
			robo::string str2; str2.format(RT("oppa %d %s"), 1974, RT("какая прелесть"));
			robo::string str3(str2);
			robo::string str4; str4 = str1;
			strings[0] = &str1;
			strings[1] = &str2;
			strings[2] = &str3;
			strings[3] = &str4;
			bool ret = true;
			for (int i = 0; i < 4; i++)
				for (int j = 0; j < 4; j++) {
					ret &= (*strings[i] == *strings[j]);
				};
			robo_infolog("%s", str4.c_str());

			Assert::IsTrue(ret);


			robo::string n(RT("-1.88855"));
			double dn = 0.;
			ROBO_ALARMN(n.to_number(dn));
			Assert::IsTrue(dn == -1.88855);

			float fn = 0.f;
			ROBO_ALARMN(n.to_number(fn));
			float err = -1.88855f - fn;
			Assert::IsTrue(err<0.0001 && err>-0.0001);

			int in = 0;
			ROBO_ALARMN(n.to_number(in));
			Assert::IsTrue(in == -1);
			unsigned int uin = 100;
			ROBO_ALARMN(n.to_number(uin));
			Assert::IsTrue(uin == 100);

		}

		static void test_simple(robo::cstr _src, robo::string& dst) {
			//dst.format(RT("copy %s"), src.c_str() );
			dst = RT(" copy ");
			dst += _src;
		}


		static void test_void(void) {}

		static int test_void2(void* _instance) {
			return *(int*)_instance + 1;
		}
		class member_test {
		public:
			int run(int _x) {
				return _x * _x;
			}
		};

		TEST_METHOD(delegat) {

			//robo::delegat::simple< robo::delegat::base< void, robo::cstr, robo::string& >, void, robo::cstr , robo::string& > recorder(test_simple);
			robo::delegat::ssimple< void, robo::cstr, robo::string& > recorder(test_simple);

			robo::string tmp;
			robo::string tmp2;
			recorder(RT("aaaa"), tmp);
			test_simple(RT("aaaa"), tmp2);
			Assert::IsTrue(tmp == tmp2);
			Assert::IsTrue(tmp == RT(" copy aaaa"));

			robo::delegat::ssimple<void> test_void_(test_void);
			test_void_();

			int instance = 5;
			robo::delegat::suni<int> test_void_2_(&instance, test_void2);
			Assert::IsTrue(test_void_2_() == 6);

			member_test  member_test_;
			robo::delegat::smember< member_test, int, int> member_test__(&member_test_, &member_test::run);
			Assert::IsTrue(member_test_.run(5) == member_test__(5));

		}
		#if ROBO_APP_INI_ENABLED == 1 
		TEST_METHOD(ini) {
			robo::log::begin(robo::log::verb::detail_7, 0);
			{
				std::ofstream ini(RT("E:\\~temp.ini"));
				ini
					<< "[SETTINGS]\n"
					<< "PATAM1=1\n"
					<< "PATAM1=2.0007\n"
					<< "PATAM3=\"Юсупов - красавчик!\"\n";
			}
			robo::system::ini::begin(RT("E:\\~temp.ini"));
			robo::string msg;
			Assert::IsTrue(msg.load(RT("SETTINGS"), RT("PATAM3")));
			robo_infolog("%s", msg.c_str());

		}
		#if ROBO_APP_LIB_ENABLED == 1 
		TEST_METHOD(app) {
			{
				std::ofstream ini(RT("E:\\~temp.ini"));
				ini
					<< "[SETTINGS]\n"
					<< "DEBUG_VERB=7\n"
					<< "TIMER_PERIOD_US=500\n"
					<< "TIMER_SHOW_PERIOD_MS=10000\n"
					<< "DEBUG_MASK_BITS=0 1 2 3\n"
					<< "[MODULES]\n"
					<< "COUNT=1\n"
					<< "M_1=\"lib.test\"\n"
					<< "[тестовая логическая шина]\n"
					<< "BUS_ID=1\n"
					<< "DEFAULT_TIMEOUT_US=200\n"
					<< "[тестовая барда]\n"
					<< "REQUEST_PAUSE_US=2000]\n"
					<< "[тестовый агент]\n"
					<< "ALIAS=\"охренеть на сколько тестовый агент\"\n"
					<< "BUS_NAME=\"lib.test/тестовая логическая шина\"\n"
					<< "ROUTER_NAME=\"lib.test/тестовый роутер\"\n"
					<< "BOARD_DEV_ID=0\n"
					<< "BOARD_ADDRESS=0x0A\n"
					<< "ENABLED=1\n"
					<< "[тестовый роутер]\n"
					<< "ROUT_TABLE_SIZE=1\n"
					<< "RT_1= 1 2  0xff  3 4 5\n"
					;
			}
			#if ROBO_APP_MODULE_ENABLED  == 1
			class ddddd : public robo::common::idevagent {
			protected:
				void upplay_action(void) {};
				void uppdate_feedback(void) {};
			public:
				struct iaction {
					robo::common::idevagent::icommand command;
				};
				struct ifeedback {
					iaction goal;
					struct ipresent {
						robo::common::idevagent::istatus status;
					} present;
				};

				ddddd(void) :robo::common::idevagent() {}
			};

			typedef robo::backend::devagent< ddddd > devagent;
			devagent::iaction front_action = {};
			devagent::ifeedback front_feedback = {};
			robo::backend::boardagent boardagent(RT("тестовая барда"), test::module::instance());

			class bus : public  robo::backend::bus {
			public:
				virtual bool post(msg* _msg) { return false; };
				virtual void cancel(void) {};
				virtual bool ready(void) { return false; };
				virtual msg* get_msg(void) { return 0; };
				virtual void  release_msg(msg*) {};

				bus(robo::cstr _name, robo::app::module* _module) : robo::backend::bus(_name, _module) {}
			};

			bus bus_(RT("тестовая логическая шина"), &test::module::instance());

			devagent agent(RT("тестовый агент"), boardagent, front_action, front_feedback);
			robo::backend::router router(RT("тестовый роутер"), test::module::instance());

			if (robo::app::machine::begin(RT("E:\\~temp.ini")) && robo::app::machine::start()) {
				robo::frontend::pulse p(
					new robo::signal::temporary(
						[] {
							robo::system::printf(RT("\t%d\tPULSE FRONTEND! is bakend? - %s\n"), system::env::realtime_us(), robo::system::env::is_backend() ? RT("ДА!") : RT("НЕТ!"));
						}
					),
					new robo::signal::temporary(
						[] {
							robo::system::printf(RT("\t%2.2f\tPULSE BACKEND! is bakend? - %s\n"), 1.0 * system::env::realtime_us() / 1000000, robo::system::env::is_backend() ? RT("ДА!") : RT("НЕТ!"));
						}
					)
							);
				p.start(2000000);

				signal::simple tmd([] {
					robo::system::printf(RT("\t%2.2f\t TIMER BACKEND! is bakend? - %s\n"), 1.0 * system::env::realtime_us() / 1000000, robo::system::env::is_backend() ? RT("ДА!") : RT("НЕТ!"));
								   });
				backend::timer::core::start(&tmd, 500000);

				signal::simple tmd2([] {
					robo::system::printf(RT("\t%2.2f\tTIMER FRONTEND! is bakend? - %s\n"), 1.0 * system::env::realtime_us() / 1000000, robo::system::env::is_backend() ? RT("ДА!") : RT("НЕТ!"));
									});

				frontend::timer tm2(tmd2);
				tm2.start(1000000);

				signal::simple tmd3([] {
					robo::system::printf(RT("\t%2.2f\t FLOOD BACKEND! is bakend? - %s\n"), 1.0 * system::env::realtime_us() / 1000000, robo::system::env::is_backend() ? RT("ДА!") : RT("НЕТ!"));
									});
				backend::timer::core::start(&tmd3, 100000);

				signal::simple tmd4([] {
					robo::system::printf(RT("\t%2.2f\tFLOOD FRONTEND! is bakend? - %s\n"), 1.0 * system::env::realtime_us() / 1000000, robo::system::env::is_backend() ? RT("ДА!") : RT("НЕТ!"));
									});

				frontend::timer tm4(tmd4);
				tm2.start(50000);

				robo::delegat::simple< backend::repeater, void > r([] {
					robo::system::printf(RT("\t%2.2f\tREPEATER FLOOD BACKEND! is bakend? - %s\n"), 1.0 * system::env::realtime_us() / 1000000, robo::system::env::is_backend() ? RT("ДА!") : RT("НЕТ!"));
																   });
				r.start(200000);


				robo::delegat::simple< frontend::repeater, void > r2([] {
					robo::system::printf(RT("\t%2.2f\tREPEATER FLOOD FRONTEND! is bakend? - %s\n"), 1.0 * system::env::realtime_us() / 1000000, robo::system::env::is_backend() ? RT("ДА!") : RT("НЕТ!"));
																	 });
				r2.start(200000);

				std::thread backend_thrd([] {
					while (!robo::app::machine::terminated()) {
						robo::app::machine::backend_loop();
					}
										 });

				devagent::iaction* ptr = &front_action;
				robo::signal::simple s([] {
					robo_errlog("zbs");
									   });



				std::thread stop_thrd([ptr, &s] {
					robo::frontend::shared::exchange(*ptr, &s);

					robo::backend::queue::post(new robo::signal::temporary(
						[] {
							robo::system::printf(RT("is bakend? - %s"), robo::system::env::is_backend() ? RT("ДА!") : RT("НЕТ!"));
						}
					), robo::signal::performer::priority::lo);
					Sleep(1000);
					robo::frontend::shared::exchange(*ptr, &s);
					Sleep(10000);
					robo::app::machine::stop();
									  });

				while (!robo::app::machine::terminated()) {
					robo::app::machine::frontend_loop();
					frontend::queue::poll();
					Sleep(10);
				}
				stop_thrd.join();
				backend_thrd.join();
			}

			robo::app::machine::finish();
			#endif
		}
		#endif
		#endif


		static void pop_callback(jsonsl_t jsn,
								 jsonsl_action_t action,
								 struct jsonsl_state_st* state,
								 const char* buf) {
			static char key[20];
			static bool is_rot_ = false;
			static bool is_pos_ = false;
			static bool is_finger_ = false;
			if (
				((state->special_flags & JSONSL_SPECIALf_UNSIGNED) == JSONSL_SPECIALf_UNSIGNED)
				||
				((state->special_flags & JSONSL_SPECIALf_UNSIGNED) == JSONSL_SPECIALf_UNSIGNED)
				||
				((state->special_flags & JSONSL_SPECIALf_FLOAT) == JSONSL_SPECIALf_FLOAT)
				||
				((state->special_flags & JSONSL_SPECIALf_EXPONENT) == JSONSL_SPECIALf_EXPONENT)
				) {
				const char* s = buf - (state->pos_cur - state->pos_begin);
				if (strcmp(key, "arm") == 0) {
					content_.arm = atoi(s);
				}
				if (strcmp(key, "x") == 0) {
					if (is_pos_) {
						content_.pos.x = (float)atof(s);
					}
					else {
						content_.error = true;
					}

				}
				if (strcmp(key, "y") == 0) {
					if (is_pos_) {
						content_.pos.y = (float)atof(s);
					}
					else
						if (is_rot_) {
							content_.rot.y = (float)atof(s);
						}
						else {
							content_.error = true;
						}
				}
				if (strcmp(key, "z") == 0) {
					if (is_pos_) {
						content_.pos.z = (float)atof(s);
					}
					else {
						content_.error = true;
					}
				}
				if (strcmp(key, "r") == 0) {
					if (is_rot_) {
						content_.rot.r = (float)atof(s);
					}
					else {
						content_.error = true;
					}
				}
				if (strcmp(key, "p") == 0) {
					if (is_rot_) {
						content_.rot.p = (float)atof(s);
					}
					else {
						content_.error = true;
					}
				}
				if (strcmp(key, "fingers") == 0) {
					static int counter = 0;
					if (counter < 5) {
						is_finger_ = true;
					}
					else {
						is_finger_ = false;
					}
					content_.fingers[counter++] = (float)atof(s);
				}
			}
			if (!is_finger_)
				key[0] = 0;
			switch (state->type) {
			case JSONSL_T_HKEY:
			case JSONSL_T_LIST:
			{
				const char* s = buf - (state->pos_cur - state->pos_begin) + 1;
				char* d = key;
				for (size_t i = state->pos_begin + 1; i < state->pos_cur; ++i, ++s, ++d)	*d = *s;	*d = 0;
			}
			if (strcmp(key, "pos") == 0) {
				is_pos_ = true;
				is_rot_ = false;
			}
			if (strcmp(key, "rot") == 0) {
				is_rot_ = true;
				is_pos_ = false;
			}
			break;
			case JSONSL_T_STRING:
			{
				const char* s = buf - (state->pos_cur - state->pos_begin) + 1;
				char* d = nullptr;
				if (strcmp(key, "timestamp") == 0) {
					d = content_.timestamp;
				}
				else if (key, "command") {
					d = content_.cmd;
				}
				if (d) {
					for (size_t i = state->pos_begin + 1; i < state->pos_cur; ++i, ++s, ++d) *d = *s;		*d = 0;
				}
			}
			break;
			}
		}
		TEST_METHOD(json) {
			command::content c;
			c.create_arm();
			//const char* arm = "{ \"a\": \"arma aram\" }";
			jsonsl_t parser = jsonsl_new(100);
			parser->action_callback_POP = pop_callback;
			jsonsl_enable_all_callbacks(parser);
			jsonsl_feed(parser, c.buf, (size_t)strlen(c.buf));
			jsonsl_destroy(parser);

		}

	};

	enum { joint_count = 8, coord_count = 6 };
	enum { Z1 = 0, YAW2 = 1, YAW3 = 2, ROLL4 = 3, PITHCH5 = 4, ROLL6 = 5, PITHC7 = 6, PITCH8 = 7 };
	static  const robo::cstr actuator_names[joint_count] = { RT("Z1"), RT("YAW2"), RT("YAW3"), RT("ROLL4"), RT("PITHCH5"), RT("ROLL6"), RT("PITHC7"), RT("PITCH8") };

	struct frontend_content {

		struct joint {

			struct data {
				int mode;
				float voltage;
				float current;
				float speed;
				float position;
			};

			struct iaction : public data {
				robo::common::idevagent::icommand command;
				int mode;
			} action;



			struct ifeedback {
				iaction goal;
				struct ipresent : public data {
					robo::common::idevagent::istatus status;
					int mode;
				} present;
			} feedback;

		};

		joint joints[joint_count];

		union ihuman {
			struct {
				union {
					struct {
						float x;
						float y;
						float z;
					};
					float position[3];
				};
				union {
					struct {
						float yaw;
						float pitch;
						float roll;
					};
					float angle[3];
				};
			};
			float values[coord_count];
		};

		struct iaction {
			int process;
			struct {
				struct {
					ihuman speed;
					ihuman position;
				}desired;

				struct {
					ihuman speed;
					ihuman position;
				}preview;

			} human;
		} action;

		struct ifeedback {
			int status;
			struct {
				struct {
					ihuman speed;
					ihuman position;
				} actual;
				struct {
					ihuman speed;
					ihuman position;
				} model;
				struct {
					ihuman speed;
					ihuman position;
				} preview;
			} human;
			struct {
				struct {
					float speed[joint_count];
					float position[joint_count];
				} model;
				struct {
					float speed[joint_count];
					float position[joint_count];
				} preview;
			} actuator;
		}feedback;

	};


	TEST_CLASS(front) {

		//common

		//backend
		/*template < typename T > class shared {
		public:
			typedef typename T::iaction iaction;
			typedef typename T::ifeedback ifeedback;
		public:
			struct ifront {
				iaction& action;
				ifeedback& feedback;
				ifront(iaction& _action, ifeedback& _feedback)
					: action(_action)
					, feedback(_feedback) {
				}
			} front;
			iaction action;
			ifeedback feedback;
			shared(
				iaction& _action
				, ifeedback& _feedback
			) : front(_action, _feedback){
				std::memset(&ifront.action, 0, sizeof (iaction) );
				std::memset(&ifront.feedback, 0, sizeof(ifeedback));
				std::memset(&oper, 0, sizeof(ioper));
			}
		};*/
		class backend;

		class frontend {
			friend class backend;
			frontend_content content_;
		public:
			frontend(void) {};
		};


		class backend : public  robo::app::module {
			class joint : public robo::backend::boardagent {
				friend class backend;
				robo::backend::devagent<frontend_content::joint> instance_;
				joint(int _index, robo::app::module& _owner, frontend_content& _content)
					: robo::backend::boardagent(actuator_names[_index], _owner), instance_(
						actuator_names[_index]
						, *this
						, _content.joints[_index].action
						, _content.joints[_index].feedback
					) {}
			};
			joint z1_;
			joint yaw2_;
			joint yaw3_;
			joint roll4_;
			joint pitch5_;
			joint roll6_;
		public:
			backend(frontend& _frontend)
				: robo::app::module(RT("backend"))
				, z1_(0, *this, _frontend.content_)
				, yaw2_(1, *this, _frontend.content_)
				, yaw3_(1, *this, _frontend.content_)
				, roll4_(1, *this, _frontend.content_)
				, pitch5_(1, *this, _frontend.content_)
				, roll6_(1, *this, _frontend.content_) {}
		};

		TEST_METHOD(test_machine) {};
	};

	TEST_CLASS(mexo) {
		bool result = false;
		TEST_METHOD(test_machine) {
			union {
				struct {
					int begin;
					int start;
					int priority;
					int frontend;
					int backend;
					int slots[::mexo::machine::slot_count];
				};
				int arr[::mexo::machine::slot_count + 5];
			} flags;

			static int samples[::mexo::machine::slot_count + 5] = { 1, 1, 16, 16, 16
				, 4  //0
				, 2  //1
				, 2  //2
				, 2  //3
				, 2  //4
				, 1  //5
				, 1  //6
				, 1  //7
				, 2  //8
				, 1  //9
				, 1  //10
				, 1  //11
				, 2  //12
				, 1  //13
				, 1  //14
				, 1  //15
			};
			::mexo::machine::slot::lambda begin([&flags] {
				flags.begin++;
												});
			::mexo::machine::slot::lambda begin2([this] {
				this->result = true;
												 });
			::mexo::machine::slot::lambda start([&flags] {
				flags.start++;
												});
			::mexo::machine::slot::lambda priority([&flags] {
				flags.priority++;
												   });
			::mexo::machine::slot::lambda frontend([&flags] {
				flags.frontend++;
												   });
			::mexo::machine::slot::lambda backend([&flags] {
				flags.backend++;
												  });
			::mexo::machine::slot::lambda slots([&flags] {
				flags.slots[::mexo::machine::slot_index()]++;
												});


			begin.attach(::mexo::machine::slot::kind::begin);
			begin2.attach(::mexo::machine::slot::kind::begin);
			start.attach(::mexo::machine::slot::kind::start);
			priority.attach(::mexo::machine::slot::kind::priority);
			frontend.attach(::mexo::machine::slot::kind::frontend);
			backend.attach(::mexo::machine::slot::kind::backend);
			slots.attach(0);
			int a[] = { 0,1,2,3 };
			slots.attach(a);
			slots.attach({ 0,4,8,12 });
			for (int i = 0; i < ::mexo::machine::slot_count; i++) {
				slots.attach(i);
			}

			for (int i = 0; i < ::mexo::machine::slot_count + 5; ++i) {
				flags.arr[i] = 0;
			}
			::mexo::machine::begin();
			::mexo::machine::start();
			for (int i = 0; i < 16; ++i) {
				::mexo::machine::priority_loop();
				::mexo::machine::backend_loop();
				::mexo::machine::frontend_loop();
			}
			bool success = true;
			const int* src = samples;
			int* dst = flags.arr;
			for (int i = 0; i < ::mexo::machine::slot_count + 5; ++i, ++src, ++dst) {
				if (*src != *dst) {
					success = false;
					break;
				}
			}
			Assert::IsTrue(success);
		}
	};

	TEST_CLASS(mexo_digit13) {
		/*
			class actuator: public dev {
			public:
				struct action : public dev::action {
					signal_t voltage = (signal_t)0;
					signal_t current = (signal_t)0;
					signal_t speed = (signal_t)0;
					long_signal_t position = (long_signal_t)0;
					long_signal_t force = (long_signal_t)0;
					struct {
						signal_t voltage = (signal_t)0;
						signal_t current = (signal_t)0;
						signal_t speed = (signal_t)0;
						struct {
							long_signal_t lo = (long_signal_t)0;
							long_signal_t hi = (long_signal_t)0;
						} position;
						struct {
							long_signal_t lo = (long_signal_t)0;
							long_signal_t hi = (long_signal_t)0;
						} forsces;
					} lim;
				} action_inst;
				struct snapshot : public dev::snapshot {
					signal_t voltage = (signal_t)0;
					signal_t current = (signal_t)0;
					signal_t speed = (signal_t)0;
					long_signal_t position = (long_signal_t)0;
					long_signal_t force = (long_signal_t)0;
				} snapshot_inst;

				enum {
					voltage_id = 1
					, current_id = 2
					, speed_id = 3
					, position_id =4
				};

				ps::voltage & psv;

				class mode : ::mexo::dev::mode {

				public:
					virtual void do_start(void) { owner().psv.on(); };
					virtual void do_stop(void) { owner().psv.off();  };

					actuator& owner() { return (actuator&) ::mexo::dev::mode::owner(); };
					mode(int _id, cstr _name, dev& _dev) : ::mexo::dev::mode(_id, _name, _dev) {};
				};

				class voltage_mode : mode {
				private:
					signal_t deseired__ =(signal_t)0;
					::mexo::iblock::output_t<signal_t>  deseired_;
				protected:
					virtual void do_start(void) {
						owner().psv.deseired.link_to(&deseired_);
						deseired__ = owner().action_inst.voltage;
						mode::do_start();
					};
					virtual void applay_action(void) {
						deseired__ = owner().action_inst.voltage;
					}
				public:
					voltage_mode(dev& _dev) : mode(voltage_id, RT("voltage"), _dev), deseired_(deseired__){};
				} voltage_mode_;

	//					: voltage(voltage_id, RT("voltage"), _owner)
	//					, current(current_id, RT("current"), _owner)
	//					, speed(speed_id, RT("speed"), _owner)
	//					, position(position_id, RT("position"), _owner)
	//
				actuator(cstr  _name, ::mexo::ps::voltage& _psv)
					: dev(_name,  action_inst, snapshot_inst),  psv(_psv), voltage_mode_(*this){
				}

			};
			*/
		typedef  fixed_point<int15> types;

		class fake_dc_periphery {
		public:
			types::discret_t duty_;
		protected:
			static void boot_begin(void) {}
			static bool do_boot(void) { return true; }
			void boot_complete(types::discret_t _duty) { duty_ = _duty; }

			static void shutdown_begin(void) {}
			static bool do_shutdown(void) { return true; }
			static void shutdown_complete(void) {}

			void do_run(types::discret_t _duty) {
				duty_ = _duty;
			}
		};
		typedef ::mexo::ps::pwm_b<types, fake_dc_periphery> fake_dc;
		typedef ::mexo::ramp_b<types> voltage_regulator_b;
		class fake_adc {
		public:
			typedef uint16_t  native_t;
			typedef uint32_t  acc_t;
			native_t sence[3];
			void query(void) {};
		};
		typedef adc_diff_b<fake_adc, types	> adcd;

		class prioritet_subsystem {
		public:
			::mexo::prioritet_subsystem hardware_subsystem;

			typedef single_adc_b<fake_adc, types	> adcs;
			adcd::config_s adc_conf;
			adcd::present_s adc_present;
			adcd adc;

			typedef single_adc_b<fake_adc, types	> adcs;
			adcs::config_s ads_conf;
			adcs::present_s ads_present;
			adcs ads;

			fake_dc::config_s dc_config;
			fake_dc::present_s dc_present;
			voltage_regulator_b::config_s dcv_config;

			voltage_regulator_b::present_s dcv_present;
			fake_dc dc;

			voltage_regulator_b dcv;

		public:
			::mexo::iblock::output_t< types::signal_t >& current(void) { return adc.output; }
			::mexo::iblock::output_t< types::signal_t >& current_diff(void) { return adc.output; }
			void reconfig(void) {
				hardware_subsystem.reconfig();
			}
			fake_dc& pwm() { return dc; }
			prioritet_subsystem()
				: hardware_subsystem(RT("hardware"), true)
				, adc(hardware_subsystem, RT("adc"), adc_conf, adc_present)
				, ads(hardware_subsystem, RT("ads"), ads_conf, ads_present)
				, dc(hardware_subsystem, RT("dc"), dc_config, dc_present)
				, dcv(hardware_subsystem, RT("dcv"), dcv_config, dcv_present) {
				adc_conf = { {22} };
				adc_present = { {23} };
				ads_conf = { {24} };
				ads_present = { {25} };

				dc_config = {
					{
						{26} //ref
						,{
							{ //range
								-4095
								, 4095
							}
							, 0
						}
						, 0
					}
					, 128
					,10
				};
				dc_present = {
					{{27}}
				};
				dcv_config = {
					{
						{28} //ref
						,{
							{ //range
								-32767
								, 32767
							}
							, 0
						}
						, 0
					}
					, 100
				};
				dcv_present = {
					{
						{19} //ref
						, 0 //output
					}
				};
				dc.link_to(dcv);
			}
		};
		prioritet_subsystem prioritet_subsystem_;

		TEST_METHOD(ps) {
			/*			::mexo::machine::slot::lambda::mexo::machine::slot::lambda(
							::mexo::machine::slot::kind::start
							, [&] {
								prioritet_subsystem_.dcv_config.cb.standalone.input = 32767;
								prioritet_subsystem_.dc.on();
							}
						);
						*/

			::mexo::machine::begin();
			::mexo::machine::start();
			prioritet_subsystem_.dcv_config.cb.standalone.input = 32767;
			prioritet_subsystem_.dc.on();

			for (int i = 0; i < 330; ++i) {
				::mexo::machine::priority_loop();
				::mexo::machine::backend_loop();
				::mexo::machine::frontend_loop();
			}

			Assert::IsTrue(prioritet_subsystem_.dc_present.cb.output > 4000 && prioritet_subsystem_.dc_present.cb.output < 4096);
			Assert::IsTrue(prioritet_subsystem_.dcv_present.cb.output > 31767 && prioritet_subsystem_.dcv_present.cb.output < 32768);

			prioritet_subsystem_.hardware_subsystem.reconfig();
			Assert::IsTrue(prioritet_subsystem_.dc_present.cb.output == 0);
			Assert::IsTrue(prioritet_subsystem_.dcv_present.cb.output == 0);

			prioritet_subsystem_.dc_config.scale = 256;
			prioritet_subsystem_.dcv_config.rampStep = 100;
			prioritet_subsystem_.dcv_config.cb.standalone.input = -32767;
			for (int i = 0; i < 330; ++i) {
				::mexo::machine::priority_loop();
				::mexo::machine::backend_loop();
				::mexo::machine::frontend_loop();
			}

			Assert::IsTrue(prioritet_subsystem_.dc_present.cb.output == -4095);
			Assert::IsTrue(prioritet_subsystem_.dcv_present.cb.output == -16400);
		}

		#define PS_TEMPLATE_NAME psdev
		#define  psdev_VOLTAGE_REGULATOR_ENABLED 1
		#define  psdev_CURRENT_REGULATOR_ENABLED 1
		#define  psdev_CURRENT_MEASSURY_ENABLED 1
		#define  psdev_CURRENT_DIFF_ENABLED 1
		#define  psdev_CURRENT_FILTER_ENABLED 1
		#define  psdev_CURRENT_DIFF_FILTER_ENABLED 1
		#define psdev_VOLTAGE_MIN_LIM -32767
		#define psdev_VOLTAGE_MAX_LIM 32767
		#define psdev_VOLTAGE_RAMP_GAIN 1
		#define psdev_CURRENT_FILTER_GAIN 200
		#define psdev_CURRENT_FILTER_SHIFT_GAIN 8
		#define psdev_CURRENT_FILTER_SHIFT_PRESC 0
		#define psdev_CURRENT_FILTER_SHIFT_VALUE 0

		#define psdev_CURRENT_DIFF_FILTER_GAIN 200
		#define psdev_CURRENT_DIFF_FILTER_SHIFT_GAIN 8
		#define psdev_CURRENT_DIFF_FILTER_SHIFT_PRESC 0
		#define psdev_CURRENT_DIFF_FILTER_SHIFT_VALUE 0

		#define psdev_CURRENT_PROP_GAIN 10
		#define psdev_CURRENT_MODEL_GAIN 10
		#define psdev_CURRENT_DIFF_GAIN 10
		#define psdev_CURRENT_CONTROL_SHIFT 7
		#define psdev_CURRENT_MODEL_SHIFT 10
		#include "mexo/ps.inc.hpp"
		typedef psdev<types, prioritet_subsystem> psdev_t;

		#define ACTUATOR_TEMPLATE_NAME a1
		#define  a1_ps_POWER_SUPPLY_VOLTAGE_REGULATOR_ENABLED 1
		#define  a1_ps_POWER_SUPPLY_CURRENT_REGULATOR_ENABLED 1
		#define  a1_ps_POWER_SUPPLY_CURRENT_MEASSURY_ENABLED 1
		#define  a1_ps_POWER_SUPPLY_CURRENT_DIFF_ENABLED 1
		#define  a1_ps_POWER_SUPPLY_CURRENT_FILTER_ENABLED 1
		#define  a1_ps_POWER_SUPPLY_CURRENT_DIFF_FILTER_ENABLED 1
		#define a1_ps_VOLTAGE_MIN_LIM -32767
		#define a1_ps_VOLTAGE_MAX_LIM 32767
		#define a1_ps_VOLTAGE_RAMP_GAIN 1
		#define a1_ps_CURRENT_FILTER_GAIN 200
		#define a1_ps_CURRENT_FILTER_SHIFT_GAIN 8
		#define a1_ps_CURRENT_FILTER_SHIFT_PRESC 0
		#define a1_ps_CURRENT_FILTER_SHIFT_VALUE 0

		#define a1_ps_CURRENT_DIFF_FILTER_GAIN 200
		#define a1_ps_CURRENT_DIFF_FILTER_SHIFT_GAIN 8
		#define a1_ps_CURRENT_DIFF_FILTER_SHIFT_PRESC 0
		#define a1_ps_CURRENT_DIFF_FILTER_SHIFT_VALUE 0

		#define a1_ps_CURRENT_PROP_GAIN 10
		#define a1_ps_CURRENT_MODEL_GAIN 10
		#define a1_ps_CURRENT_DIFF_GAIN 10
		#define a1_ps_CURRENT_CONTROL_SHIFT 7
		#define a1_ps_CURRENT_MODEL_SHIFT 10
		#include "mexo/actuator.inc.hpp"
		typedef a1<types, prioritet_subsystem> a1_t;


		TEST_METHOD(ps_dev) {
			class actuator : public ::mexo::ps::dev {
			public:
				struct action_s {
					::mexo::ps::dev::action_s dev;
					bool invers;
					types::signal_t voltage;
					types::signal_t current;
					types::signal_t speed;
					types::long_signal_t position;
					types::long_signal_t force;
				} action;
				typedef ::mexo::quazzy_adapt_b<types> current_regulaor_b;
				typedef ::mexo::filter_b<types> filter_b;
				struct config_s {
					voltage_regulator_b::config_s voltage_regulator;
					filter_b::config_s current_filter;
					current_regulaor_b::config_s current_regulaor;
				}config;

				struct present_s {
					::mexo::ps::dev::present_s dev;
					voltage_regulator_b::present_s voltage_regulator;
					filter_b::present_s current_filter;
					current_regulaor_b::present_s current_regulaor;
				}present;

				prioritet_subsystem& prioritet_subsystem_;
				::mexo::backend_subsystem voltage_control;
				::mexo::backend_subsystem current_control;

				voltage_regulator_b voltage_regulator;
				current_regulaor_b current_regulator;
				filter_b current_filter;

				class voltage_mode :public ::mexo::ps::dev::mode {
				protected:
					actuator& owner(void) { return owner_cast<actuator>(); }
					virtual void applay_action(void) {
						if (owner().action.invers) {
							owner().voltage_regulator.set_input(-owner().action.voltage);
						}
						else {
							owner().voltage_regulator.set_input(owner().action.voltage);
						}
					}
					virtual void do_start(void) {
						owner().prioritet_subsystem_.hardware_subsystem.reconfig();
						owner().voltage_control.reconfig();
						owner().voltage_control.start();
						owner().prioritet_subsystem_.dc.link_to(owner().voltage_regulator);
						owner().on();
					}
					virtual void do_stop(void) {
						owner().voltage_control.stop();
						owner().off();
					}
				public:
					voltage_mode(int _index, actuator& _actuator) :
						::mexo::ps::dev::mode(_index, RT("@mo_@v"), _actuator) {}
				} voltage_mode_;

				class current_mode :public ::mexo::ps::dev::mode {
				protected:
					actuator& owner(void) { return owner_cast<actuator>(); }
					virtual void applay_action(void) {
						if (owner().action.invers) {
							owner().current_regulator.set_input(-owner().action.current);
						}
						else {
							owner().current_regulator.set_input(owner().action.current);
						}
						owner().current_regulator.set_min(-owner().action.voltage);
						owner().current_regulator.set_max(owner().action.voltage);
					}
					virtual void do_start(void) {
						owner().prioritet_subsystem_.dc.link_to(owner().current_regulator);
						owner().prioritet_subsystem_.hardware_subsystem.reconfig();
						owner().current_control.reconfig();
						owner().current_control.start();
						owner().on();
					}
					virtual void do_stop(void) {
						owner().current_control.stop();
						owner().off();
					}
				public:
					current_mode(int _index, actuator& _actuator) :
						::mexo::ps::dev::mode(_index, RT("@mo_@c"), _actuator) {}
				} current_mode_;

				actuator(prioritet_subsystem& _prioritet_subsystem)
					: ::mexo::ps::dev(RT("actuator"), action.dev, present.dev, _prioritet_subsystem.dc)
					, prioritet_subsystem_(_prioritet_subsystem)
					, voltage_control(RT("@v_@co"), false, this)
					, current_control(RT("@c_@co"), false, this)
					, voltage_regulator(voltage_control, RT("@r"), config.voltage_regulator, present.voltage_regulator)
					, current_filter(current_control, RT("@f"), config.current_filter, present.current_filter)
					, current_regulator(current_control, RT("@r"), config.current_regulaor, present.current_regulaor)
					, voltage_mode_(1, *this)
					, current_mode_(2, *this) {
					prioritet_subsystem_.dc.link_to(current_regulator);
					current_regulator.actual.link_to(&current_filter.output);
					current_filter.input.link_to(&prioritet_subsystem_.adc.output);
					present = {};
					action = {};
					config = {
						{
							{
								{28} //ref
								,{
									{ //range
										-32767
										, 32767
									}
									, 0
								}
								, 0
							}
							, 100
						}
						,{
							{}
							, 12
							, {
								8
								, 0
								, 0
							}
						}
						,{
							{}
							,{
								10	//parameter_t propGain;
								,60 //parameter_t modelGain;
								,0	//parameter_t diffGain;
								,7	//uint8_t control_shift;
								,10	//uint8_t model_shift;
							}
						}
					};

				}
			} actuator_(prioritet_subsystem_);

			::mexo::machine::slot::lambda start(
				::mexo::machine::slot::kind::start
				, [&] {
					actuator_.action.voltage = 32767;
					actuator_.action.dev.mode = 1;
				}
			);


			::mexo::machine::begin();
			::mexo::machine::start();

			for (int i = 0; i < 330; ++i) {
				::mexo::machine::priority_loop();
				::mexo::machine::backend_loop();
				::mexo::machine::frontend_loop();
			}

			Assert::IsTrue(prioritet_subsystem_.dc_present.cb.output > 4000 && prioritet_subsystem_.dc_present.cb.output < 4096);
			Assert::IsTrue(actuator_.present.voltage_regulator.cb.output > 31767 && actuator_.present.voltage_regulator.cb.output < 32768);

			actuator_.action.dev.mode = 0;
			for (int i = 0; i < 3; ++i) {
				::mexo::machine::priority_loop();
				::mexo::machine::backend_loop();
				::mexo::machine::frontend_loop();
			}

			actuator_.action.dev.mode = 2;
			for (int i = 0; i < 3; ++i) {
				::mexo::machine::priority_loop();
				::mexo::machine::backend_loop();
				::mexo::machine::frontend_loop();
			}
			Assert::IsTrue(prioritet_subsystem_.dc_present.cb.output == 0);
			actuator_.action.voltage = 10000;
			actuator_.action.current = 10000;
			actuator_.action.dev.actual = true;
			for (int i = 0; i < 300; ++i) {
				::mexo::machine::priority_loop();
				::mexo::machine::backend_loop();
				::mexo::machine::frontend_loop();
			}
			::mexo::machine::priority_loop();
			::mexo::machine::backend_loop();
			::mexo::machine::frontend_loop();
			Assert::IsTrue(prioritet_subsystem_.dc_present.cb.output > 0);

			{
				psdev_t::action_s action;
				psdev_t::present_s present;
				psdev_t::config_s config;
				psdev_t psdev_(prioritet_subsystem_, RT("ps"), action, config, present);
				a1_t::action_s action2;
				a1_t::present_s present2;
				a1_t::config_s config2;
				a1_t a1_(prioritet_subsystem_, RT("a1"), action2, config2, present2);
			}
		}
	};

}
