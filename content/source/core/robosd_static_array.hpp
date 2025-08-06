#ifndef robosd_static_array_hpp
#define robosd_static_array_hpp
#include "core/robosd_log.hpp"
namespace robo{
	template<class C, unsigned int N > class array_t	{
			inline static unsigned int count_ = 0;
		public: 
			enum { size = N};
			class item: public C{
			public: 
				item(void) {
					if(count_ < size){
						index_[count_] = this;
						count_++;
					} else {
						//todo
						ROBO_APP_CRASH();
					}
				}
			};
		private:
			static inline item * index_[ size ] ={};
		public:
			unsigned int count(void){
				return count_;
			}			
			C & operator [] (unsigned int _no){
				if(_no < count_){
					return *index_[_no];
				} else{
					ROBO_APP_CRASH();					
				//todo
					return *index_[0];
				}
			}
	};
}
#endif