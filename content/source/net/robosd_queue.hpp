#ifndef __robosd_net_queue_h
#define __robosd_net_queue_h
#include "core/robosd_ring_buf.hpp"
namespace robo{
	namespace net{
		template <class DRIVER, uint8_t BITS, class DATA_T, bool _autoCommit> class  ROBO_EXPORT queue : private ring_t< BITS, DATA_T >{
			typedef typename DRIVER::guard guard;
			typedef ring_t< BITS, DATA_T > buffer;
			DRIVER & driver_;
		protected:
		public:
			size_t size(void) { return buffer::SIZE;  }
			queue(DRIVER & _driver) : ring_t<BITS,DATA_T>(), driver_(_driver){}
			virtual  ~queue(){ };

			bool put(DATA_T n){
				guard __pg(driver_);
				if (!buffer::full()){
					buffer::put(n);
					if (_autoCommit || buffer::full()){
						driver_.raise();
					}
					return true;
				}
				else {
					return false;
				}
			}

			bool put(const DATA_T * _data, size_t _count){
				guard __pg(driver_);
				if (buffer::SIZE - buffer::count() >= _count){
					while (_count--)
						buffer::put(*_data++);
					if (_autoCommit){
						driver_.raise();
					}
					return true;
				}
				else {
					return false;
				}
			}

			size_t get(DATA_T * _data, size_t __max_count){
				guard __pg(driver_);
				size_t _count = 0;
				while (buffer::available() && __max_count--){
					*_data++ = buffer::get();
					_count++;
				}
				return _count;
			}
			DATA_T get(void){
				guard __pg(driver_);
				return buffer::get();
			}
			bool avalable(void){
				guard __pg(driver_);
				return buffer::available();
			}
			bool full(void){
				guard __pg(driver_);
				return buffer::full();
			}
			void clear(void){
				guard __pg(driver_);
				buffer::clear();
			}
			size_t count(void){
				guard __pg(driver_);
				return buffer::count();
			}
			size_t space(){
				guard __pg(driver_);
				return buffer::space();
			}
		};
	}
}

#endif


