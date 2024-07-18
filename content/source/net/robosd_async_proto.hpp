#ifndef robosd_async_proto
#define robosd_async_proto
#include "core/robosd_system.hpp"
#include "net/robosd_serial.hpp"
namespace robo{
	namespace net{
		template <typename D,unsigned SA, unsigned SB, typename G >  class async_serial:  
			public  hardware_bridge_t<D,SA,SB,G> {
			public:
			typedef hardware_bridge_t<D,SA,SB,G> B;
			size_t exchange(
				uint8_t * _in_data
				, size_t _in_size
				, uint8_t * _out_data
				, size_t _out_max_size
			){
				size_t out_available;
				size_t in_space;
				size_t out_size = 0;
				

				if (_in_size){
					in_space = B::bridge::A.space();
				
					size_t in_data_size = _in_size - 1;
					uint8_t cmd = _in_data[0];
					if(cmd == 0xBF || in_space < in_data_size){
						B::reset();
						return 0;
					}
					
					if (in_data_size){
						B::on_receive(_in_data + 1, in_data_size);
					}
					in_space -= in_data_size;
				
					out_available = B::bridge::A.available();
					if(out_available+1 > _out_max_size){
						out_size = _out_max_size-1;
					}else{
						out_size = out_available;
					}
					if (out_size > 0){
						B::bridge::A.get(_out_data + 1, out_size);
						out_available -=out_size;
					}

					if (in_space > 0xF)
						in_space = 0xF;

					if (out_available > 0xF)
						out_available = 0xF;
					*_out_data = (uint8_t)((in_space << 4) + out_available);
					return out_size + 1;
				}else{
					B::reset();
					return 0;
				}
			}	
			
    };
	}
}
#endif
