#ifndef __spi2can_common_hpp
#define __spi2can_common_hpp
#include <stdint.h>
namespace spi2can{
	struct packet
	{
		enum 
		{
			data_sz = 8, total_sz = 11
		};

		union
		{
			struct
			{
				union
				{
					struct
					{
						uint16_t id : 11;		
						uint16_t len : 4;
						uint16_t state : 1;
					};
					uint16_t header;
				}; //2 байта
				uint8_t data[data_sz];  //10 байт
				uint8_t crc;  //11 байта
			};
			uint8_t memo[total_sz];					
		} ;
	};
}
#endif
