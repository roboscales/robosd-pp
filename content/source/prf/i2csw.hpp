#ifndef i2scw_hpp
#define i2scw_hpp
#include "stdint.h"
namespace robo{
	namespace prf{
		template <typename D> class i2csw_t{
			public:
			//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// ПРИМЕР ИСПОЛЬЗОВАНИЯ
//=========================================================================================
//   Запись uint16_t во внешнюю еепром (FRAM FM24CL64) или любой другой 24LC памяти, 
//	 две ячейки,  указывается адрес первой ячейки, следующая идет adr++
//=========================================================================================
//
//void FRAM_W_INT(uint16_t adr, uint16_t dat){
//start_cond ();
//send_byte (0xA2); //адрес чипа + что будем делать (записывать)
//send_byte    ((adr & 0xFF00) >> 8);  
//send_byte    (adr & 0x00FF);
//send_byte    ((dat & 0xFF00) >> 8);  
//send_byte    (dat & 0x00FF);
//istop_cond();
//}

//=========================================================================================
//   Считывание uint16_t из внешней еепром (FRAM FM24CL64) или любой другой 24LC памяти, 
//	 две ячейки,  указывается адрес первой ячейки, следующая идет adr++
//=========================================================================================
//uint16_t FRAM_R_INT(uint16_t adr){
//uint16_t dat;
//start_cond ();
//send_byte (0xA2);
//send_byte    ((adr & 0xFF00) >> 8);  
//send_byte    (adr & 0x00FF);
//restart_cond ();
//send_byte (0xA3);
//dat =  get_byte(0);	
//dat <<= 8; 
//dat |= get_byte(1);
//stop_cond();
//return dat;
//}

			uint32_t frame_error=0; 
		
			void stop_cond (void)  // функция генерации условия стоп 
			{
				uint16_t SCL, SDA;
				D::scl_out(); // притянуть SCL (лог.0)
				D::delay();
				D::sda_out(); // притянуть SDA (лог.0)
				D::delay();

				D::scl_in(); // отпустить SCL (лог.1)
				D::delay();
				D::sda_in(); // отпустить SDA (лог.1)
				D::delay();
						
				// проверка фрейм-ошибки
				frame_error=0;		// сброс счётчика фрейм-ошибок
				SCL=D::scl_get();
				SDA=D::sda_get();
				if (SCL == 0) frame_error++;   // проберяем, чтобы на ноге SDA была лог.1, иначе выдаём ошибку фрейма
				if (SDA == 0) frame_error++;   // проберяем, чтобы на ноге SCL была лог.1, иначе выдаём ошибку фрейма
				D::delay();
			}

			void start (void) // функция инициализации шины
			{
					stop_cond();   // стоп шины
					stop_cond();   // стоп шины
			}
			//----------------------------------------------------
			void start_cond (void)  // функция генерации условия старт
			{
					D::sda_out(); // притянуть SDA (лог.0)
					D::delay();
					D::scl_out(); // притянуть SCL (лог.0)
					D::delay();
			}
			//----------------------------------------------------
			void restart_cond (void)   // функция генерации условия рестарт
			{
					D::sda_in(); // отпустить SDA (лог.1)
					D::delay();
					D::scl_in(); // отпустить SCL (лог.1)
					D::delay();
					D::sda_out(); // притянуть SDA (лог.0)
					D::delay();
					D::scl_out(); // притянуть SCL (лог.0)
					D::delay();
			}

				//----------------------------------------------------
			uint8_t send_byte (uint8_t data)  // функция  отправки байта  
			{   
				uint8_t i;
				uint8_t ack=1;           //АСК, если АСК=1 – произошла ошибка
				uint16_t SDA;   
				for (i=0;i<8;i++)
					{
							if (data & 0x80) 
							{
								D::sda_in(); // лог.1
							}
							else 
							{
								D::sda_out(); // Выставить бит на SDA (лог.0
							}
							D::delay();
							D::scl_in();   // Записать его импульсом на SCL       // отпустить SCL (лог.1)
							D::delay();
							D::scl_out(); // притянуть SCL (лог.0)
							data<<=1; // сдвигаем на 1 бит влево								
						}
						D::sda_in(); // отпустить SDA (лог.1), чтобы ведомое устройство смогло сгенерировать ACK
						D::delay();
						D::scl_in(); // отпустить SCL (лог.1), чтобы ведомое устройство передало ACK
						D::delay();
						SDA=D::sda_get();
						if (SDA==0x00) ack=1; else ack=0;    // Считать ACK

						D::scl_out(); // притянуть SCL (лог.0)  // приём ACK завершён

						return ack; // вернуть ACK (0) или NACK (1)   
			}
			//----------------------------------------------------
			uint8_t get_byte (uint8_t last_byte) // функция принятия байта
			{
				uint8_t i, res=0;
				uint16_t SDA;
				D::sda_in(); // отпустить SDA (лог.1)

				for (i=0;i<8;i++)
				{
						res<<=1;
						D::scl_in(); // отпустить SCL (лог.1)      //Импульс на SCL
						D::delay();
						D::sda_in();
						SDA=D::sda_get();
						if (SDA==1) res=res|0x01; // Чтение SDA в переменную  Если SDA=1 то записываем 1
						D::scl_out(); // притянуть SCL (лог.0)
						D::delay();
				}

				if (last_byte==0){ D::sda_out();} // притянуть SDA (лог.0)     // Подтверждение, ACK, будем считывать ещё один байт
				else {D::sda_in();} // отпустить SDA (лог.1)                 // Без подтверждения, NACK, это последний считанный байт
				D::delay();
				D::scl_in(); // отпустить SCL (лог.1)
				D::delay();
				D::scl_out(); // притянуть SCL (лог.0)
				D::delay();
				D::sda_in(); // отпустить SDA (лог.1)

				return res; // вернуть считанное значение
			}
			
			bool read(uint8_t _dev, uint8_t _addr, uint8_t * _memo, uint8_t _len){
				if( _len > 0){
					start_cond ();
					if(!send_byte ( (_dev<<1))) { stop_cond(); return false;}
					if(!send_byte    ( _addr )) { stop_cond(); return false;} 
					restart_cond ();
					if(!send_byte ((_dev<<1) | 1)) { stop_cond(); return false;}
					for(int i = 0;i< _len; ++i){
						*_memo++ = get_byte(i);	
					}
					stop_cond();
					return (frame_error == 0);
				} else {
					return false;
				}
			}
			
			bool write(uint8_t _dev, uint8_t _addr, const uint8_t * _memo, uint8_t _len){
				if( _len > 0){
					start_cond ();
					if(!send_byte ( (_dev<<1))) return false;
					if(!send_byte    ( _addr )) return false; 
					for(int i = 0;i< _len; ++i){
						send_byte(*_memo++);	
					}
					stop_cond();
					return (frame_error == 0);
				} else {
					return false;
				}
			}
		};
	}
}
#endif