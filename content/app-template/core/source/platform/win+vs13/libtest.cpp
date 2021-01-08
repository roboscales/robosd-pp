#include "pch.h"
#include "CppUnitTest.h"
#include "core/robosd_list.hpp"
#include <iostream>
#include <sstream>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace libtest
{
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
}
