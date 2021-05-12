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

#define MODULE_NAME_STR RT("lib.test")
namespace test {
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
};

#define MODULE_NAME test


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

			class ddddd : public robo::frontend::idevagent {
			public:
				enum class icommand { external, service, stopped, fault, reset, none };
				enum class istate { external, independed, service, stopped, fault, unknown, locked };
				struct iaction: public robo::frontend::idevagent::iaction {
				};
				struct ifeedback : public robo::frontend::idevagent::ifeedback {
				};
				struct irequired : public robo::frontend::idevagent::irequired {
				};
				struct istatus : public robo::frontend::idevagent::istatus {
				};
				ddddd(void) :robo::frontend::idevagent() {}
			};
			
			typedef robo::backend::devagent< ddddd > devagent;
			
			robo::backend::boardagent boardagent(RT("тестовая барда"), &test::module::instance());

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
			robo::backend::router router(RT("тестовый роутер"), &test::module::instance());
			
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
		}
#endif
#endif

	};
}

