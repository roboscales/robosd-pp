#ifndef mrobosd_modbus_rtu_hpp
#define mrobosd_modbus_rtu_hpp
#include <stdint.h>
namespace robo{
	namespace net{
		namespace modbus_rtu{
			enum{min_frame_length=5, max_reg_count = 126};
			struct commands{
				enum {
					read_regs = 0x03
					, write_reg = 0x06
					, write_regs = 0x010
				} ;
			};
			struct errors{
				enum class frame{
					none = 0x00
					, command = 0x01
					, length = 0x02
					, crc = 0x03
					, proto = 0x04
					, post = 0x05
					, count = 0x06
				};
				enum class proto{
					success = 0x00
					, code = 0x01 //	Принятый код функции не может быть обработан.
					, address = 0x02	//Адрес данных, указанный в запросе, недоступен.
					, value = 0x03	//Значение, содержащееся в поле данных запроса, является недопустимой величиной.
					, fault = 0x04	//Невосстанавливаемая ошибка имела место, пока ведомое устройство пыталось выполнить затребованное действие.
					, timeout = 0x05	//Ведомое устройство приняло запрос и обрабатывает его, но это требует много времени. Этот ответ предохраняет ведущее устройство от генерации ошибки тайм-аута.
					, busy = 0x06	//Ведомое устройство занято обработкой команды. Ведущее устройство должно повторить сообщение позже, когда ведомое освободится.
					, panic = 0x07	//Ведомое устройство не может выполнить программную функцию, заданную в запросе. Этот код возвращается для неуспешного программного запроса, использующего функции с номерами 13 или 14. Ведущее устройство должно запросить диагностическую информацию или информацию об ошибках от ведомого.
					, hard = 0x08	//Ведомое устройство при чтении расширенной памяти обнаружило ошибку паритета. Ведущее устройство может повторить запрос, но обычно в таких случаях требуется ремонт.
					, getway = 0x0A	//Шлюз неправильно настроен или перегружен запросами.
					, lost = 0x0B	//Slave устройства нет в сети или от него нет ответа.
				};
			};
		
			/*
			// D - класс адаптации к конкретному устройству
			class D{
			public:
				//отправка данных
				void phy_post(uint16_t _length);
				uint8_t * answer();
				
				//запись регистра _index
				errors::proto phy_write(int _index, uint16_t _data);
				//запись _count регистров, начиная с _index
				errors::proto phy_write(int _index, const uint16_t * _data, uint8_t _count);
				//чтение регистра
				errors::proto phy_read(int _index, uint16_t & _data);
				//чтение _count регистров, начиная с _index
				errors::proto phy_read(int _index, uint16_t * _data, uint8_t _count);
			};*/

			template <typename D, typename T> class slave_t{
			public:
				// вызовется когда придут данные с интерфейса
				static void on_receive(const uint8_t * _frame, uint16_t _length){
					uint32_t txLen = 0;
					/*Check frame length*/
					if(_length < min_frame_length){
						T::error(errors::frame::length);
						return;
					}
					/*Check frame address*/
					if(T::address() == (*_frame) ){	
						_length -= 2;
						uint16_t requried_crc = *((uint16_t*)&_frame[_length]);
						uint16_t actual_crc = T::crc(_frame, _length);_frame++;		
						if(requried_crc == actual_crc){
							int command = *_frame; _frame++;
							_length	 -= 2;
							switch(command){
								case commands::read_regs: perform(  read_regs(_frame,_length),command ); break;
								case commands::write_reg: perform(  write_reg(_frame,_length),command );break;
								case commands::write_regs: perform(  write_regs(_frame,_length),command );break;
								default: perform(errors::proto::code,command);break;
							}
						} else{
							T::error(errors::frame::crc);
							return;
						}
					}				
				}
				static void answer(errors::proto _error, int _command){
					enum {header=3, total=5, marker = 0x80};
					uint8_t * answer = D::answer_frame_get();
					uint8_t * ansfirst= answer;
					*answer++  = T::address();
					if( _error ==errors::proto::success){
						*answer++ =  _command;
					} else{
						*answer++ =  _command | marker;
					}
					*answer++ =  (uint8_t)_error;
					uint16_t crc = T::crc(ansfirst, header);
					*(uint16_t*)answer = crc;
					D::post(total);
				}
				static void perform(errors::proto _error, int _command){	
					if(_error != errors::proto::success){
						T::error(errors::frame::proto);
						answer(_error,_command);
					}else{
						T::error(errors::frame::none);
					}
				}
				static errors::proto read_regs(const uint8_t * _payload, uint32_t _length){
					if(_length<4){
						return errors::proto::fault;
					}

					/*Get start address*/
					uint16_t addr = (*_payload) << 8; _payload++;
					addr |= *_payload; _payload++;

					/*Get number of registers to read*/
					uint16_t regNum = (*_payload++) << 8; 
					regNum |= *_payload++;

					/*Check max regs to read*/
					if(regNum > max_reg_count){
						return errors::proto::fault;
					}
					
					uint8_t * answer = D::answer_frame_get();
					uint8_t * ansfirst= answer;
					*answer++  = T::address();
					*answer++ =  commands::read_regs;
					/*Set number of bytes*/
					uint16_t payload_len = regNum << 1;
					//*(uint16_t *)answer = regNum; answer+=2;
					*answer ++= payload_len; 
					
					/*Copy data from memory to frame*/
					for(uint16_t i = 0; i < regNum; ++i, ++addr, answer+=2 )
					{
						auto res = T::read(addr, *(uint16_t*)answer);
						if( res != errors::proto::success){
							return res;
						}					
					}
					uint16_t crc_len= 3+ payload_len;
					uint16_t crc = T::crc(ansfirst, crc_len);
					*(uint16_t*)answer = crc;
					return D::post(5+payload_len);					
				}
				 
				static errors::proto write_reg(const uint8_t * _payload, uint32_t _length){
					return errors::proto::address;
				}
				static errors::proto write_regs(const uint8_t * _payload, uint32_t _length){
					if(_length<4){
						return errors::proto::fault;
					}

					uint8_t * answer = D::answer_frame_get();
					uint8_t * ansfirst= answer;
					*answer++  = T::address();
					*answer++ =  commands::write_regs;

					/*Get start address*/
					uint8_t tmp = *_payload++;
					uint16_t addr = tmp << 8;  
					*answer++ =tmp; 
					
					tmp = *_payload++;
					addr |= tmp;
					*answer++ = tmp; 
					
					/*Get number of registers to read*/
					tmp = *_payload++;
					uint16_t regNum = tmp << 8; 
					*answer++ =tmp; 

					tmp = *_payload++;
					regNum |= tmp;
					*answer++ =tmp; 
					//*answer++ = regNum;
					
					
					/*Check max regs to read*/
					if(regNum > max_reg_count){
						return errors::proto::fault;
					}
					
					/*Set number of bytes*/
					uint16_t payload_len = regNum << 1;
					/*Copy data from memory to frame*/
					for(uint16_t i = 0; i < regNum; ++i, ++addr, _payload+=2 )
					{
						auto res = T::write(addr, *(uint16_t*)_payload);
						if( res != errors::proto::success){
							return res;
						}					
					}
					uint16_t crc_len= 6;
					uint16_t crc = T::crc(ansfirst, crc_len);
					*(uint16_t*)answer = crc;
					return D::post(8);	
				}
			};
		}
	}
}
#endif



/*

Handle Write Register command
static uint32_t MB_WriteRegHandler(uint8_t * frame, uint32_t len)
{
    MB_Error_t error = MB_ERROR_NO;
    uint32_t txLen = 0;
    Get start address to write
    uint16_t startAddr = frame[2] << 8 | frame[3];
    Check address range
    if(startAddr > MB_REGS_NUM)
        error = MB_ERROR_WRONG_ADDRESS;
    if(error == MB_ERROR_NO)
    {
        MB_SetReg(startAddr, (frame[4] << 8 | frame[5]));
        txLen = 6;
         Calculate CRC
        uint16_t crc = MB_GetCRC(frame, txLen);
        frame[txLen++] = crc;
        frame[txLen++] = crc >> 8;
    }
    else
    {
        txLen = MB_ErrorHandler(frame, len, error);
    }
    return txLen;
}

Handle write registers command
static uint32_t MB_WriteRegsHandler(uint8_t * frame, uint32_t len)
{
    MB_Error_t error = MB_ERROR_NO;
    uint32_t txLen = 0;
    Get start address to write
    uint16_t startAddr = frame[2] << 8 | frame[3];
    Get number of registers to write
    uint16_t regNum = frame[4] << 8 | frame[5];
    Check address range
    if((startAddr + regNum) > MB_REGS_NUM)
        error = MB_ERROR_WRONG_ADDRESS;
    Check max regs to read
    if(regNum > 126)
        error = MB_ERROR_WRONG_VALUE;
    if(error == MB_ERROR_NO)
    {
        for(uint16_t i = 0; i < regNum; i++)
            MB_SetReg(startAddr + i, __REV16(*((uint16_t*)&frame[7] + i)));
        txLen = 6;
        Calculate CRC
        uint16_t crc = MB_GetCRC(frame, txLen);
        frame[txLen++] = crc;
        frame[txLen++] = crc >> 8;
    }
    else
    {
        txLen = MB_ErrorHandler(frame, len, error);
    }
    return txLen;
}
}
Error handler
static uint32_t MB_ErrorHandler(uint8_t * frame, uint32_t len, MB_Error_t error)
{
    uint32_t txLen = 3;
    frame[1] |= 0x80;
    frame[2] = error;
    uint16_t crc = MB_GetCRC(frame, txLen);
    frame[txLen++] = crc;
    frame[txLen++] = crc >> 8;
    return txLen;
}
*/