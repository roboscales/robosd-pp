#ifndef robosd_modbus_rtu_hpp
#define robosd_modbus_rtu_hpp
#include <stdint.h>
#include "core/robosd_delegat.hpp"
#include <net/robosd_net_master.hpp>
#include <net/robosd_modbus.hpp>

namespace robo{
	namespace net{
		namespace modbus {
			namespace rtu{
				enum{min_frame_length=5, max_reg_count = 124, max_frame_length=max_reg_count*2+7};
				struct commands{
					enum {
						read_regs = 0x03
						,read_outputs = 0x04
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
						, unknown = 0x06
						, count = 0x07
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
				static inline  uint8_t * write_to(uint16_t _value, uint8_t *  buf){
					*buf++ = (uint8_t)(_value>>8);
					*buf++ = (uint8_t)(_value & 0xFF);
					return buf;
				}
				static inline uint16_t read_from(const uint8_t * & buf){
					uint16_t res = (*buf++) << 8; 
					res |= *buf++;
					return res;
				}
				using  guard =  ::robo::system::guard;
				template <typename D, typename T> class slave_t{
				public:
					// вызовется когда придут данные с интерфейса
					static void on_receive(const uint8_t * _frame, uint16_t _length){
						uint32_t txLen = 0;
						/*Check frame length*/
						if(_length < min_frame_length || _length>max_frame_length){
							T::error(errors::frame::length);
							D::start_receive();
							return;
						}
						/*Check frame address*/
						if(T::address() == (*_frame) ||  (*_frame)==0xFE ){	
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
								D::start_receive();
								return;
							}
						}else{
							D::start_receive();
						}						
					}
					static void answer(errors::proto _error, int _command){
						enum {header=3, total=5, marker = 0x80};
						uint8_t * answer = D::answer_frame_get();
						uint8_t * ansfirst= answer;
						*answer++  = T::address();
						if( _error == errors::proto::success){
							*answer++ =  _command;
						} else{
							*answer++ =  _command | marker;
						}
						*answer++ =  (uint8_t)_error;
						uint16_t crc = T::crc(ansfirst, header);
						*(uint16_t*)answer = crc;
						if( D::post(total) != errors::proto::success )
							D::start_receive();
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
						uint16_t addr = read_from(_payload);

						/*Get number of registers to read*/
						uint16_t regNum = read_from(_payload);

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
						*answer ++= payload_len; 
						
						/*Copy data from memory to frame*/
						for(uint16_t i = 0; i < regNum; ++i, ++addr )
						{
							uint16_t reg; 
							{	
								guard g__;
								auto res = T::read(addr,reg);
								if( res != errors::proto::success){
									return res;
								}
							}						
							answer=write_to(reg,answer);
						}
						uint16_t crc_len= 3+ payload_len;
						uint16_t crc = T::crc(ansfirst, crc_len);
						*(uint16_t*)answer = crc;
						return D::post(5+payload_len);					
					}
					static errors::proto write_reg(const uint8_t * _payload, uint32_t _length){
						if(_length<4){
							return errors::proto::fault;
						}

						/*Get start address*/
						uint16_t addr = read_from(_payload);
						
						uint8_t * answer = D::answer_frame_get();
						uint8_t * ansfirst= answer;
						*answer++  = T::address();
						*answer++ =  commands::write_reg;
						answer = write_to(addr,answer);
						uint32_t value = read_from(_payload);
						answer = write_to(value,answer);
						/*Set number of bytes*/
						{
							guard g__;
							auto res = T::write(addr, value );
							if( res != errors::proto::success){
								return res;
							}					
						}

						uint16_t crc_len= 6;
						uint16_t crc = T::crc(ansfirst, crc_len);
						*(uint16_t*)answer = crc;
						return D::post(8);					
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
						for(uint16_t i = 0; i < regNum; ++i, ++addr )
						{
							{
								guard g__;
								auto res = T::write(addr, read_from(_payload) );
								if( res != errors::proto::success){
									return res;
								}					
							}
						}
						uint16_t crc_len= 6;
						uint16_t crc = T::crc(ansfirst, crc_len);
						*(uint16_t*)answer = crc;
						return D::post(8);	
					}
				};
				struct packet{
					uint8_t memo[max_frame_length];
					uint8_t size;
				};
				
				template <typename D,typename T> class master_t: public ::robo::net::master_t<D, packet>, public T {
					using M = ::robo::net::master_t<D, packet>;				
					::robo::delegat::owned_fabric<void,bool>::member<master_t> write_regs_confirm_;
					::robo::delegat::owned_fabric<void,bool>::member<master_t> read_regs_confirm_;
					packet outcom_ ;
					packet incom_;

					uint8_t address_;
					uint8_t command_;
					uint8_t wait_length_;
					uint16_t reg_adress_;
					uint16_t count_;
					uint16_t * incom_regs_;
					void write_regs_confirm__(bool result_){
						if(result_){
							
							if(incom_.size != wait_length_ ){
								T::on_refuse(errors::frame::length);
								return;							
							}
							uint8_t crc_length=wait_length_-2;
							uint16_t crc = T::crc(incom_.memo,crc_length);
							if( *(uint16_t*)(incom_.memo+crc_length) != crc){
								T::on_refuse(errors::frame::crc);
								return;
							}
							
							const uint8_t * ptr = incom_.memo;
							uint8_t addr = *ptr++;
							if(addr!= address_){
								T::on_refuse(errors::frame::proto);
								return;
							}
							
							uint8_t cmd = *ptr++;
							if(cmd!= command_){
								T::on_refuse(errors::frame::proto);
								return;
							}
							
							uint16_t reg_address = read_from(ptr);
							if(reg_address!=reg_adress_){
								T::on_refuse(errors::frame::proto);
								return;
							}
							
							uint16_t count = read_from(ptr);
							if(count!=count_){
								T::on_refuse(errors::frame::proto);							
								return;
							}
							
							T::on_confirm();
							
						} else {
							T::on_refuse(errors::frame::unknown);
						}
					}
					
					void read_regs_confirm__(bool result_){
						if(result_){
							
							if(incom_.size != wait_length_ ){
								T::on_refuse(errors::frame::length);
								return;							
							}
							uint8_t crc_length=wait_length_-2;
							uint16_t crc = T::crc(incom_.memo,crc_length);
							if( *(uint16_t*)(incom_.memo+crc_length) != crc){
								T::on_refuse(errors::frame::crc);
								return;
							}
							
							const uint8_t * ptr = incom_.memo;
							uint8_t addr = *ptr++;
							if(addr!= address_){
								T::on_refuse(errors::frame::proto);
								return;
							}
							
							uint8_t cmd = *ptr++;
							if(cmd!= command_){
								T::on_refuse(errors::frame::proto);
								return;
							}
							
							uint8_t payload_len =*ptr++;
							if(payload_len!=count_*2){
								T::on_refuse(errors::frame::proto);
								return;
							}
							
							{
								guard g__;
								for(int i=0;i<count_;++i,++incom_regs_){
									*incom_regs_ = read_from(ptr);
								}							
							}
							
							T::on_confirm();
							
						} else {
							T::on_refuse(errors::frame::unknown);
						}
					}
					

				public:
					virtual void poll(void){
						::robo::net::master_t<D, packet>::poll();
						T::poll();
					}
					master_t()
						: write_regs_confirm_(*this,&master_t::write_regs_confirm__)
						, read_regs_confirm_(*this,&master_t::read_regs_confirm__)
						{
						}
				
					void write_regs(uint8_t _address, uint16_t _reg_adress, uint8_t _count, const uint16_t * _data ){
						if( (_count>=1) && (_count<=max_reg_count) ){
							address_ = _address;
							reg_adress_ = _reg_adress;
							command_ = commands::write_regs;
							count_ = _count;
							incom_regs_ = 0;
							wait_length_ = 8;
							uint8_t* ptr = outcom_.memo;
							*ptr++ = address_;
							*ptr++ = command_;
							ptr= write_to(_reg_adress,ptr);
							ptr = write_to(_count,ptr);
							{
								guard g__;
								for(uint8_t i=0; i < _count; ++i,++_reg_adress, ++_data ){
									volatile uint16_t tmp = *_data;
									ptr = write_to(tmp,ptr);
								}
							}
							uint16_t crc_len= 6 + 2*_count;
							uint16_t crc = T::crc(outcom_.memo, crc_len);
							*(uint16_t*)ptr = crc;		
							outcom_.size = crc_len+2;
							incom_.size = wait_length_;
							M::exchange(outcom_, &incom_, &write_regs_confirm_);
						}					
					}
					void write_reg(uint8_t _address, uint16_t _reg_adress, uint16_t _data ){
							address_ = _address;
							reg_adress_ = _reg_adress;
							command_ = commands::write_reg;
							count_ = 1;
							incom_regs_ = 0;
							wait_length_ = 8;
							uint8_t* ptr = outcom_.memo;
							*ptr++ = address_;
							*ptr++ = command_;
							ptr= write_to(_reg_adress,ptr);
							ptr = write_to(_data,ptr);
							uint16_t crc_len= 6 ;
							uint16_t crc = T::crc(outcom_.memo, crc_len);
							*(uint16_t*)ptr = crc;		
							outcom_.size = crc_len+2;
							incom_.size = wait_length_;
							M::exchange(outcom_, &incom_, &write_regs_confirm_);					
					}
					void read_regs__(uint8_t _address, uint16_t _reg_adress, uint8_t _count, uint16_t * _incom_regs, uint8_t _command ){
						if( (_count>=1) && (_count<=max_reg_count) ){
							address_ = _address;
							reg_adress_ = _reg_adress;
							command_ = _command;
							count_ = _count;
							incom_regs_ = _incom_regs;
							wait_length_ = 5 +_count*2;
							uint8_t* ptr = outcom_.memo;
							*ptr++ = address_;
							*ptr++ = command_;
							ptr = write_to(_reg_adress,ptr);
							ptr = write_to(_count,ptr);
							uint16_t crc_len= 6;
							uint16_t crc = T::crc(outcom_.memo, crc_len);
							*(uint16_t*)ptr = crc;		
							outcom_.size = crc_len+2;
							incom_.size = wait_length_;
							M::exchange(outcom_, &incom_, &read_regs_confirm_);
						}					
					}
					void read_regs(uint8_t _address, uint16_t _reg_adress, uint8_t _count, uint16_t * _incom_regs ){
						read_regs__(_address,_reg_adress,_count,_incom_regs,commands::read_regs);
					}
					void read_outputs(uint8_t _address, uint16_t _reg_adress, uint8_t _count, uint16_t * _incom_regs ){
						read_regs__(_address,_reg_adress,_count,_incom_regs, commands::read_outputs);
					}
					
				};

				template < class D > class machine_t 
					: public ::robo::net::modbus::rtu::master_t<D,  ::robo::net::dispetcher_adapter_t<D, ::robo::net::modbus::rtu::errors::frame> >{
						public:
						typedef ::robo::net::modbus::rtu::errors::frame errors;
				};	
					
				template < class D > class  dispetcher_t 
					: public  robo::net::modbus::dispetcher_t<machine_t<D>> {
						public:
							dispetcher_t(robo::time_us_t _timeout_us) : robo::net::modbus::dispetcher_t<machine_t<D>> (_timeout_us){
							}
				};
					
				class dummy_servo{
				public:
					static modbus::rtu::errors::proto write(uint16_t _index, uint16_t _data){
						return  modbus::rtu::errors::proto::address;
					}	

					static modbus::rtu::errors::proto read(uint16_t _index, uint16_t & _data){
						return  modbus::rtu::errors::proto::address;
					}

					
					static void error(modbus::rtu::errors::frame _err){
					}
					
					static uint8_t address(void){						
						return 0x00;
					}
					
					static uint16_t crc(const uint8_t * _data, size_t _length){						
						return 0;
					}
				};
			}			
		}
	}
}

#define ROBOSD_MODBUS_RTU_SLAVE_DRIVER(N)\
class  N {\
	public:\
		static robo::net::modbus::rtu::errors::proto post(uint16_t _length);\
		static uint8_t * answer_frame_get(void);\
		static void start_receive(void);\
		static void on_receive(const uint8_t * data, uint16_t _length);\
		static uint16_t crc(const uint8_t * _data, size_t _length){\
			return ::robo::crc16_modbus_by_table(_data,_length);\
		}\
}

#define ROBOSD_MODBUS_RTU_SLAVE_SERVO(N)\
class N{\
public:\
	static robo::net::modbus::rtu::errors::proto write(uint16_t _index, uint16_t _data);\
	static robo::net::modbus::rtu::errors::proto read(uint16_t _index, uint16_t & _data);\
	static void error(robo::net::modbus::rtu::errors::frame _err);\
	static uint8_t address(void);\
	static uint16_t crc(const uint8_t * _data, size_t _length){\
		return ::robo::crc16_modbus_by_table(_data,_length);\
	}\
}
				
#define ROBOSD_MODBUS_RTU_MASTER_DRIVER(N)\
class  N {\
	public:\
		using packet = ::robo::net::modbus::rtu::packet;\
		static robo::time_us_t wd_us( const packet * _pk);\
		static void receive(packet * _pk);\
		static void send(const packet * _pk);\
		static bool panic(void);\
		static void send_cancel(void);\
		static void receive_cancel(void);\
		static bool ready(void);\
		static void confirm(void);\
		static void refuse(void);\
		static uint16_t crc(const uint8_t * _data, size_t _length){\
			return ::robo::crc16_modbus_by_table(_data,_length);\
		}\
}

#endif