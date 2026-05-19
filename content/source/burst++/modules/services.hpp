#ifndef burst_service_hpp
#define burst_service_hpp
#include "burst++/modules/services.common.hpp"
#include "net/robosd_modbus_rtu.hpp"
#include "net/robosd_serial.hpp"
#include "core/robosd_crc.hpp"

namespace burst{
	namespace service {
		struct dummy_guard{};
		namespace rs485{
			#if SERVICE_RS485_A_TAG != SERVICE_RS485_TAG_NONE
			namespace A{
				namespace prf{				
					#if SERVICE_RS485_A_TAG == SERVICE_RS485_TAG_MODBUS_RTU_SLAVE
					ROBOSD_MODBUS_RTU_SLAVE_DRIVER(modbus_rtu_slave);
					#endif
					#if SERVICE_RS485_A_TAG == SERVICE_RS485_TAG_SERIAL
					#ifndef SERVICE_485_A_BUFFER_SIZE_BITS
					#define SERVICE_485_A_BUFFER_SIZE_BITS 5
					#endif
					#ifndef SERVICE_485_A_WATCHDOG_US
					#define SERVICE_485_A_WATCHDOG_US 1000
					#endif
					ROBOSD_HALF_DUPLEX_SERIAL_DRIVER(serial,SERVICE_485_A_BUFFER_SIZE_BITS,SERVICE_485_A_WATCHDOG_US);
					#endif	
					#if SERVICE_RS485_A_TAG == SERVICE_RS485_TAG_MODBUS_RTU_MASTER
					ROBOSD_MODBUS_RTU_MASTER_DRIVER(modbus_rtu_master);
					#endif
				}
			}
			#endif
			
			#if SERVICE_RS485_B_TAG != SERVICE_RS485_TAG_NONE
			namespace B{
				namespace prf{				
					#if SERVICE_RS485_B_TAG == SERVICE_RS485_TAG_MODBUS_RTU_SLAVE
					ROBOSD_MODBUS_RTU_SLAVE_DRIVER(modbus_rtu_slave);
					#endif
					#if SERVICE_RS485_B_TAG == SERVICE_RS485_TAG_SERIAL
					#ifndef SERVICE_485_B_BUFFER_SIZE_BITS
					#define SERVICE_485_B_BUFFER_SIZE_BITS 5
					#endif
					#ifndef SERVICE_485_B_WATCHDOG_US
					#define SERVICE_485_B_WATCHDOG_US 1000
					#endif
					ROBOSD_HALF_DUPLEX_SERIAL_DRIVER(serial,SERVICE_485_B_BUFFER_SIZE_BITS,SERVICE_485_B_WATCHDOG_US);
					#endif	
					#if SERVICE_RS485_B_TAG == SERVICE_RS485_TAG_MODBUS_RTU_MASTER
					ROBOSD_MODBUS_RTU_MASTER_DRIVER(modbus_rtu_master);
					#endif
				}
			}
			#endif
			#if SERVICE_RS485_C_TAG != SERVICE_RS485_TAG_NONE
			namespace C{
				namespace prf{				
					#if SERVICE_RS485_C_TAG == SERVICE_RS485_TAG_MODBUS_RTU_SLAVE
					ROBOSD_MODBUS_RTU_SLAVE_DRIVER(modbus_rtu_slave);
					#endif
					#if SERVICE_RS485_C_TAG == SERVICE_RS485_TAG_SERIAL
					#ifndef SERVICE_485_C_BUFFER_SIZE_BITS
					#define SERVICE_485_C_BUFFER_SIZE_BITS 5
					#endif
					#ifndef SERVICE_485_C_WATCHDOG_US
					#define SERVICE_485_C_WATCHDOG_US 1000
					#endif
					ROBOSD_HALF_DUPLEX_SERIAL_DRIVER(serial,SERVICE_485_C_BUFFER_SIZE_BITS,SERVICE_485_C_WATCHDOG_US);
					#endif	
					
					#if SERVICE_RS485_C_TAG == SERVICE_RS485_TAG_MODBUS_RTU_MASTER
					ROBOSD_MODBUS_RTU_MASTER_DRIVER(modbus_rtu_master);
					#endif
				}
			}
			#endif

			namespace A{
				#if SERVICE_RS485_A_TAG == SERVICE_RS485_TAG_MODBUS_RTU_SLAVE
				ROBOSD_MODBUS_RTU_SLAVE_SERVO(servo);
				typedef ::robo::net::modbus::rtu::slave_t<prf::modbus_rtu_slave,  servo > modbus_rtu_servo_t;				
				extern modbus_rtu_servo_t modbus_rtu_servo;
				#endif
				
				#if SERVICE_RS485_A_TAG == SERVICE_RS485_TAG_MODBUS_RTU_MASTER
				#ifndef SERVICE_RS485_TAG_MODBUS_A_GUARD 
				#define SERVICE_RS485_TAG_MODBUS_A_GUARD dummy_guard
				#endif
				typedef  robo::net::modbus::rtu::dispetcher_t< prf::modbus_rtu_master , SERVICE_RS485_TAG_MODBUS_A_GUARD> dispetcher_t;
				extern dispetcher_t dispetcher;
				#endif
				
				#if SERVICE_RS485_A_TAG == SERVICE_RS485_TAG_SERIAL
				typedef robo::net::half_duplex_t<prf::serial,SERVICE_485_A_BUFFER_SIZE_BITS,SERVICE_485_A_BUFFER_SIZE_BITS,::robo::system::critical> serial_t;
				//extern robo::net::iserial & serial;
				#endif

			}
			namespace B{
				#if SERVICE_RS485_B_TAG == SERVICE_RS485_TAG_MODBUS_RTU_SLAVE
				ROBOSD_MODBUS_RTU_SLAVE_SERVO(servo);
				typedef ::robo::net::modbus::rtu::slave_t<prf::modbus_rtu_slave,  servo > modbus_rtu_servo_t;				
				extern modbus_rtu_servo_t modbus_rtu_servo;
				#endif

				#if SERVICE_RS485_B_TAG == SERVICE_RS485_TAG_MODBUS_RTU_MASTER
				#ifndef SERVICE_RS485_TAG_MODBUS_B_GUARD 
				#define SERVICE_RS485_TAG_MODBUS_B_GUARD dummy_guard
				#endif
				typedef  robo::net::modbus::rtu::dispetcher_t< prf::modbus_rtu_master,SERVICE_RS485_TAG_MODBUS_B_GUARD > dispetcher_t;
				extern dispetcher_t dispetcher;
				#endif
				
				#if SERVICE_RS485_B_TAG == SERVICE_RS485_TAG_SERIAL
				typedef robo::net::half_duplex_t<prf::serial,SERVICE_485_B_BUFFER_SIZE_BITS,SERVICE_485_B_BUFFER_SIZE_BITS,::robo::system::critical> serial_t;
				//extern robo::net::iserial & serial;
				#endif
			}
			namespace C{
				#if SERVICE_RS485_C_TAG == SERVICE_RS485_TAG_MODBUS_RTU_SLAVE
				ROBOSD_MODBUS_RTU_SLAVE_SERVO(servo);
				typedef ::robo::net::modbus::rtu::slave_t<prf::modbus_rtu_slave,  servo > modbus_rtu_servo_t;				
				extern modbus_rtu_servo_t modbus_rtu_servo;
				#endif
				
				#if SERVICE_RS485_C_TAG == SERVICE_RS485_TAG_MODBUS_RTU_MASTER
				#ifndef SERVICE_RS485_TAG_MODBUS_C_GUARD 
				#define SERVICE_RS485_TAG_MODBUS_C_GUARD dummy_guard
				#endif
				typedef  robo::net::modbus::rtu::dispetcher_t< prf::modbus_rtu_master, SERVICE_RS485_TAG_MODBUS_C_GUARD  > dispetcher_t;
				extern dispetcher_t dispetcher;
				#endif

				#if SERVICE_RS485_C_TAG == SERVICE_RS485_TAG_SERIAL
				typedef robo::net::half_duplex_t<prf::serial,SERVICE_485_C_BUFFER_SIZE_BITS,SERVICE_485_C_BUFFER_SIZE_BITS,::robo::system::critical> serial_t;
				//extern robo::net::iserial & serial;
				#endif
			}
		}
		#if ROBO_APP_BURST_VARTREE_ENABLED == 1
		void varlist(void (*_do_print)(const char* _buf));
		#endif
	}
}

#endif



