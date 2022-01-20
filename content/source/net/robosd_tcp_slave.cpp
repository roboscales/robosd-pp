#include "net/robosd_tcp_slave.hpp"
#include "core/robosd_string.hpp"
#include "core/robosd_ini.hpp"
namespace robo{
    namespace net{
        tcp_slave::tcp_slave(void): link(){
        };

        tcp_slave::~tcp_slave(void){
        }

        bool tcp_slave::begin(void)
        {
            ROBO_LBREAKN( network_.load( alias(), RT("NETWORK") ) )
            ROBO_LBREAKN(::robo::ini::load(alias(), RT("SERVO_PORT"), servo_port_));
            return true;
        }
        void tcp_slave::finish(void){
        }

    }
}
