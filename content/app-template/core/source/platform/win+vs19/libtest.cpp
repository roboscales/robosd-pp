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
using namespace mexo;

#define MODULE_NAME_STR RT("lib.test")
namespace test {
#if ROBO_MODULE_ENABLED  == 1
	class module : public robo::app::module {
	protected:
		virtual void frontend_loop(void) {};
		virtual void backend_loop(void) {};
		module(void) : robo::app::module(MODULE_NAME_STR ) {}
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
		int arm =1;
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
			for (int i=0;i<5;i++) fingers[i] = (float) ::rand();
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

namespace libtest
{
	static  void print(robo::log::verb _verb, robo::cstr _format, va_list  _args) {
		robo::char_t buf[255];

#if ROBO_UNICODE_ENABLED == 1
		buf[vswprintf_s(buf, 255, _format, _args)] = 0;
#else
		buf[vsprintf(buf, _format, _args)] = 0;
#endif
		Logger::WriteMessage(buf);
		Logger::WriteMessage(RT("\n"));
	}


	TEST_CLASS(list)
	{
		class item{			
		public:
			::robo::list::unsorted<item>::ref ref;
			item(void) : ref(*this){}
			operator ::robo::list::unsorted<item>::ref & (){ return ref; }
		};
		class sitem{			
		public:
			::robo::list::sorted<sitem,int>::ref ref;
			operator ::robo::list::sorted<sitem,int>::ref & (){ return ref; }
			typedef int priority_t;
			sitem(void) : ref(*this,-1){}
		};
		class uitem{			
		public:
			::robo::list::unique<uitem,int>::ref ref;
			uitem(void) : ref(*this,-1){}
		};
	public:
		
		TEST_METHOD(create)
		{
			item it;			
			Assert::IsFalse(it.ref.attached());
		}

		TEST_METHOD(attach)
		{
			::robo::list::unsorted<item> list;
			item it;			
			it.ref.attach_to(list);
			Assert::IsTrue(it.ref.attached());
			Assert::IsTrue(list.count() == 1);
		}

		TEST_METHOD(detach)
		{
			::robo::list::unsorted<item> list;
			item it;			
			it.ref.attach_to(list);
			it.ref.dettach();
			Assert::IsTrue(list.count() == 0);
			Assert::IsFalse(it.ref.attached());
		}

		TEST_METHOD(move)
		{
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

		TEST_METHOD(finish)
		{
			::robo::list::unsorted<item> list;
			{
				item it[50];			
				for(int i=0;i<50;i++){
					it[i].ref.attach_to(list);
				}				
				Assert::IsTrue(list.count() == 50);

			}
	

			Assert::IsTrue(list.count() == 0);
		}
		TEST_METHOD(drop)
		{
			::robo::list::unsorted<item> list;
			{
				item it[50];			
				for(int i=0;i<50;i++){
					it[i].ref.attach_to(list);
				}				
				Assert::IsTrue(list.count() == 50);

				it[25].ref.dettach();
				Assert::IsTrue(list.count() == 49);
				it[0].ref.dettach();
				Assert::IsTrue(list.count() == 48);
				it[49].ref.dettach();
				Assert::IsTrue(list.count() == 47);

				Assert::IsTrue( list.first() == &(it[1].ref) );
				Assert::IsTrue( list.last() == &(it[48].ref) );

				Assert::IsTrue( it[25].ref.prev() == nullptr );
				Assert::IsTrue( it[25].ref.next() == nullptr );
				Assert::IsTrue( it[0].ref.prev() == nullptr );
				Assert::IsTrue( it[0].ref.next() == nullptr );
				Assert::IsTrue( it[49].ref.prev() == nullptr );
				Assert::IsTrue( it[49].ref.next() == nullptr );

				Assert::IsTrue( it[24].ref.next() == &(it[26].ref) );
				Assert::IsTrue( it[26].ref.prev() == &(it[24].ref) );


				Assert::IsTrue( it[48].ref.next() == nullptr );
				Assert::IsTrue( it[1].ref.prev() == nullptr );
			}
	

			Assert::IsTrue(list.count() == 0);
		}

		TEST_METHOD(sorted)
		{
			::robo::list::sorted<sitem,int> list;
			{
				sitem it[3];
				it[0].ref.set_key(0);
				it[1].ref.set_key(2);
				it[2].ref.set_key(1);
				it[0].ref.attach_to(list);
				it[1].ref.attach_to(list);
				it[2].ref.attach_to(list);
				Assert::IsTrue( list.count() == 3);
				Assert::IsTrue( it[0].ref.next() == &(it[2].ref) );
				Assert::IsTrue( it[2].ref.next() == &(it[1].ref) );
				Assert::IsTrue( it[1].ref.prev() == &(it[2].ref) );
				Assert::IsTrue( it[2].ref.prev() == &(it[0].ref) );
			}
			Assert::IsTrue(list.count() == 0);
		}
		TEST_METHOD(unique)
		{
			::robo::list::unique<uitem,int> list;
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
				Assert::IsTrue( list.count() == 3);
				Assert::IsTrue( it[0].ref.next() == &(it[2].ref) );
				Assert::IsTrue( it[2].ref.next() == &(it[1].ref) );
				Assert::IsTrue( it[1].ref.prev() == &(it[2].ref) );
				Assert::IsTrue( it[2].ref.prev() == &(it[0].ref) );
			}
			Assert::IsTrue(list.count() == 0);
		}
		TEST_METHOD(fifo)
		{
			::robo::queue::fifo<item> queue;
			{
				item it[3];
				
				queue.push( &it[0]);
				queue.push( &it[1]);
				queue.push( &it[2]);

				Assert::IsTrue( queue.first() == &(it[0].ref) );
				Assert::IsTrue( queue.last() == &(it[2].ref) );
				Assert::IsTrue( queue.count() == 3);

			}
			Assert::IsTrue(queue.count() == 0);
		}
		TEST_METHOD(priority)
		{
			::robo::queue::priority<sitem> queue;
			{
				sitem it[3];
				
				queue.push( &it[0]);
				queue.push( &it[1]);
				queue.push( &it[2]);

				it[0].ref.set_key(0);
				it[1].ref.set_key(2);
				it[2].ref.set_key(1);

				Assert::IsTrue( queue.first() == &(it[0].ref) );
				Assert::IsTrue( queue.last() == &(it[1].ref) );
				Assert::IsTrue( queue.count() == 3);
				while( queue.pop() != nullptr );
				Assert::IsTrue( queue.count() == 0);

			}
		}
	};
	TEST_CLASS(log)
	{
		bool err_aram_(void) {
			ROBO_ALARM();
			ROBO_ALARMN(0);
			ROBO_ALARMN(1);
			return true;
		}
		bool err_aram2_(void) {
			ROBO_BREAKN(1, false);
			ROBO_BREAKN_F(1, false, "error %d", 1);
			ROBO_LBREAKN_F(0, "test %d error", -1);
			return true;
		}
		void err_aram3_(void) {
			ROBO_VBREAKN_F(1, "test error %d", 1);
			ROBO_VBREAKN_F(0, "test %d error", -1);
		}
	public:
		TEST_METHOD(err_aram)
		{
			robo::log::begin(robo::log::verb::detail_7, 0, print);
			err_aram_(); ;
			err_aram3_();
			Assert::IsFalse(err_aram2_());
		}
	};

	TEST_CLASS(util){
#if ROBO_APP_ALLOC_ENABLED ==1
		TEST_METHOD(memo)
		{
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
		TEST_METHOD(string)
		{
			robo::log::begin(robo::log::verb::detail_7, 0, print);
			robo::string * strings[4];
			robo::string str1(RT("oppa %d %s"), 1974, RT("какая прелесть"));
			robo::string str2; str2.format( RT("oppa %d %s"), 1974,  RT("какая прелесть"));
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

			Assert::IsTrue(	ret	);


			robo::string n(RT("-1.88855"));
			double dn = 0.;
			ROBO_ALARMN( n.to_number(dn) );
			Assert::IsTrue( dn == -1.88855);

			float fn = 0.f;
			ROBO_ALARMN(n.to_number(fn));
			float err = -1.88855f - fn;
			Assert::IsTrue( err<0.0001 && err>-0.0001);

			int in = 0;
			ROBO_ALARMN(n.to_number(in));
			Assert::IsTrue(in == -1);
			unsigned int uin = 100;
			ROBO_ALARMN(n.to_number(uin));
			Assert::IsTrue(uin == 100);

		}

		static void test_simple( robo::cstr _src, robo::string & dst) {
			//dst.format(RT("copy %s"), src.c_str() );
			dst = RT(" copy ");
			dst += _src;
		}

		
		static void test_void(void) {
		}

		static int test_void2(void * _instance) {
			return *(int *)_instance + 1;
		}
		class member_test {
		public:
			int run(int _x) {
				return _x * _x;
			}
		};
		
		TEST_METHOD(delegat)
		{
			
			//robo::delegat::simple< robo::delegat::base< void, robo::cstr, robo::string& >, void, robo::cstr , robo::string& > recorder(test_simple);
			robo::delegat::ssimple< void, robo::cstr, robo::string& > recorder(test_simple);
			
			robo::string tmp;
			robo::string tmp2;
			recorder( RT("aaaa"), tmp);
			test_simple(RT("aaaa"), tmp2);
			Assert::IsTrue(tmp == tmp2);
			Assert::IsTrue(tmp == RT(" copy aaaa"));

			robo::delegat::ssimple<void> test_void_(test_void);
			test_void_();

			int instance = 5;
			robo::delegat::suni<int> test_void_2_(&instance, test_void2);
			Assert::IsTrue(test_void_2_()==6);

			member_test  member_test_;
			robo::delegat::smember< member_test, int,int> member_test__(&member_test_, &member_test::run);
			Assert::IsTrue(member_test_.run(5) == member_test__(5) );

		}
#if ROBO_APP_INI_ENABLED == 1 
		TEST_METHOD(ini) {
			robo::log::begin(robo::log::verb::detail_7, 0, print);
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
			robo_infolog("%s",msg.c_str());

		}
#if ROBO_APP_LIB_ENABLED == 1 
		TEST_METHOD(app) {
			{
				std::ofstream ini(RT("E:\\~temp.ini"));
				ini
					<< "[SETTINGS]\n"
					<< "DEBUG_VERB=7\n"
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
					<< "[тестовый роутер]\n"
					<< "ROUT_TABLE_SIZE=1\n"
					<< "RT_1= 1 2  0xff  3 4 5\n"
					;
			}
#if ROBO_MODULE_ENABLED  == 1
			class ddddd : public robo::frontend::idevagent {
			public:
				enum class icommand { external, service, stopped, fault, reset, none };
				enum class istate { external, independed, service, stopped, fault, unknown, locked };
				struct iaction {
				};
				struct ifeedback {
				};
				struct ideseired : public robo::frontend::idevagent::ideseired {
				};
				struct istatus : public robo::frontend::idevagent::istatus {
				};
				ddddd(void) :robo::frontend::idevagent() {}
			};
			
			typedef robo::backend::devagent< ddddd > devagent;
			
			robo::backend::boardagent boardagent(RT("тестовая барда"), test::module::instance());

			class bus : public  robo::backend::bus {
			public:
				virtual bool post(msg* _msg) { return false; };
				virtual void cancel(void) {};
				virtual bool ready(void) { return false; };
				virtual msg* get_msg(void) { return 0;  };
				virtual void  release_msg(msg*) { };

				bus( robo::cstr _name, robo::app::module* _module): robo::backend::bus( _name, _module){
				}
			};

			bus bus_(RT("тестовая логическая шина"), &test::module::instance());

			devagent agent(RT("тестовый агент"), boardagent );
			robo::backend::router router(RT("тестовый роутер"), test::module::instance());
			
			if (robo::app::machine::begin(RT("E:\\~temp.ini"), print)) {
				robo::app::machine::start();
				std::thread backend_thrd([] {
					while (!robo::app::machine::terminated()) {
						robo::app::machine::backend_loop();
						Sleep(10);
					}
				});
				std::thread stop_thrd([] {
					Sleep(3000);
					robo::app::machine::stop();					
				});
				while ( !robo::app::machine::terminated() ) {
					robo::app::machine::frontend_loop();
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
			const char* buf)
		{
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
				)
			{
				const char* s = buf - (state->pos_cur - state->pos_begin);
					if (strcmp(key, "arm") == 0) {
						content_.arm = atoi(s);
					}
				if (strcmp(key, "x") == 0) {
					if (is_pos_) {
						content_.pos.x = (float) atof(s);
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
			if(!is_finger_)
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
			jsonsl_t parser =  jsonsl_new(100);
			parser->action_callback_POP = pop_callback;
			jsonsl_enable_all_callbacks(parser);
			jsonsl_feed(parser, c.buf, (size_t)strlen(c.buf));
			jsonsl_destroy(parser);
			
		}

	};


	TEST_CLASS(mexo)
	{
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

			static int samples[::mexo::machine::slot_count + 5] = {1, 1, 16, 16, 16
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
				flags.slots[ ::mexo::machine::slot_index() ]++;
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
			slots.attach({0,4,8,12});
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
			for (int i = 0; i < ::mexo::machine::slot_count + 5; ++i, ++src,++dst) {
				if ( *src != *dst) {
					success = false;
					break;
				}
			}
			Assert::IsTrue(success);
		}

		struct float_to_int16 {
			struct config_s {
				int16_t lo;
				int16_t up;
				float scale;
			} config;
			iblock::satstate dirrect(float _deseired, int16_t& _duty) {
				float tmp = _deseired * config.scale;
				if (tmp > 0.) tmp += 0.5f;
				else
					if (tmp < 0.) tmp -= 0.5f;
				
				_duty = (int16_t)tmp;
				
				if (_duty >= config.up) {
					_duty = config.up;
					return iblock::satstate::up;
				}
				else if (tmp <= config.lo) {
					_duty = config.lo;
					return iblock::satstate::low;
				}
				else {
					return iblock::satstate::none;
				}
			}
			void revert(int16_t _duty, float& _actual) {
				_actual = _duty / config.scale;
			}
		};

		class fake_dc_periphery  {
		public:
			typedef signal_t deseired_t;
			typedef int16_t actual_t;
		private:
			float_to_int16 	float_to_int16_;
		public:
			int16_t duty;
			struct config_s {
				::mexo::iblock::config_s block;
				float_to_int16::config_s converter;
			};
		protected:
			static void boot_begin(void) {}
			static bool do_boot(void) { return true; }
			void boot_complete(actual_t _duty) { duty  = _duty;  }

			static void shutdown_begin(void) {  }
			static bool do_shutdown(void) { return true; }
			static void shutdown_complete(void) {  }

			void do_run(actual_t _duty) { duty = _duty;  }

			iblock::satstate dirrect(deseired_t _deseired, actual_t& _duty) {
				return float_to_int16_.dirrect(_deseired, _duty);
			}
			void revert(actual_t _duty, deseired_t & _actual) {
				float_to_int16_.revert(_duty, _actual);
			}
			bool applay(const config_s& _config) {
				if (_config.converter.lo < _config.converter.up && _config.converter.scale > 1.f / 32767) {
					float_to_int16_.config = _config.converter;
					return true;
				}
				else {
					return false;
				}
			}
		};

		typedef controller_block_t< ::mexo::ps::pwm<fake_dc_periphery>  > fake_dc;
		typedef controller_block_t< ramp< signal_t > > voltage;

		/*class power_dc_2 {
		protected:
			void boot_begin(void) {}
			bool do_boot(void) { return true; }
			void boot_complete(void) { }

			void shutdown_begin(void) {  }
			bool do_begin(void) { return true; }
			void shutdown_complete(void) {  }
		};*/

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
		TEST_METHOD(ps) {
			::mexo::prioritet_subsystem hardware_subsystem(RT("hardware"), true);
			fake_dc::config_s dc_config = {
				{0} //block
				,{ //converter
					-4095
					, 4095
					, 4095./12.
				}
			};
			voltage::config_s dcv_config = {
				{0} //block
				, 0.8f
				,{ //
					-12.f
					, 12.f
				}
				, 0.f
			};
			fake_dc dc(hardware_subsystem, RT("dc"), dc_config, 0);
			voltage dcv(hardware_subsystem, RT("dcv"), dcv_config, 0);
			dc.link_to(dcv);
			/*typedef ::mexo::ps::vdc< power_dc_1>  dc_1_t;
			dc_1_t::config_s dc1_cfg = {
				{0} //block
				,0.5f
				,{-12,12}
				,0
			};

			dc_1_t power_supply_1(hardware_subsystem, RT("power_supply-1"), dc1_cfg);

			actuator  a_1(RT("actuator-1"), power_supply_1);

			*/
			::mexo::machine::begin();
			::mexo::machine::start();
			//hardware_subsystem
			//a_1.action_inst.voltage = 1.f;
			//a_1.action_inst.mode = actuator::voltage_id;
			dcv.standalone_deseired = 12.f;
			dc.on();
			
			for (int i = 0; i < 16; ++i) {
				::mexo::machine::priority_loop();
				::mexo::machine::backend_loop();
				::mexo::machine::frontend_loop();
			}

			Assert::IsTrue(dc.actual.value() >4000 && dc.actual.value() < 4096);
			Assert::IsTrue(dcv.actual.value() > 11.f && dcv.actual.value() < 12.1f);
		}
	};

}

