//#include "pch.h"
#include "CppUnitTest.h"
#include "core/robosd_list.hpp"
#include "core/robosd_log.hpp"
#include "core/robosd_system.hpp"
#include "core/robosd_string.hpp"
#include "core/robosd_delegat.hpp"
#include <fstream>
#include <iostream>
#include <sstream>


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

		TEST_METHOD(memo)
		{
			robo::system::memstat memstat0 = robo::system::get_mem_statistic();
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

			robo::system::memstat memstat1 = robo::system::get_mem_statistic();
			Assert::IsTrue(memstat0.used.size == memstat1.used.size);
		}
		TEST_METHOD(string)
		{
			robo::log::begin(robo::log::verb::detail_7, 0, print);
			robo::string * strings[4];
			robo::string str1(RT("oppa %d %s"), 1974, RT("кака€ прелесть"));
			robo::string str2; str2.format( RT("oppa %d %s"), 1974,  RT("кака€ прелесть"));
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

			robo::delegat::simple< void, robo::cstr , robo::string& > recorder(test_simple);
			robo::string tmp;
			robo::string tmp2;
			recorder( RT("aaaa"), tmp);
			test_simple(RT("aaaa"), tmp2);
			Assert::IsTrue(tmp == tmp2);
			Assert::IsTrue(tmp == RT(" copy aaaa"));

			robo::delegat::simple<void> test_void_(test_void);
			test_void_();

			int instance = 5;
			robo::delegat::uni<int> test_void_2_(&instance, test_void2);
			Assert::IsTrue(test_void_2_()==6);

			member_test  member_test_;
			robo::delegat::member< member_test, int,int> member_test__(member_test_, &member_test::run);
			Assert::IsTrue(member_test_.run(5) == member_test__(5) );

		}
		TEST_METHOD(ini) {
			robo::log::begin(robo::log::verb::detail_7, 0, print);
			{
				std::ofstream ini(RT("E:\\~temp.ini"));
				ini
					<< "[SETTINGS]\n"
					<< "PATAM1=1\n"
					<< "PATAM1=2.0007\n"
					<< "PATAM3=\"ёсупов - красавчик!\"\n";
			}
			robo::system::os::ini_init(RT("E:\\~temp.ini"));
			robo::string msg;
			Assert::IsTrue(msg.load(RT("SETTINGS"), RT("PATAM3")));
			robo_infolog("%s",msg.c_str());

		}

	};
}
