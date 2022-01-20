#include "net/robosd_tcp_master.hpp"
#include "core/robosd_ini.hpp"
namespace robo{
    namespace net{
        tcp_master::tcp_master(): link(){
        };

        tcp_master::~tcp_master(void){
        }


        bool tcp_master::begin(cstr _alias)
        {
            ROBO_LBREAKN(link::begin(_alias));
            ROBO_LBREAKN( ini::load(alias(), RT("INCOM_PORT"), listen_port_) );
            return true;
        }
        void tcp_master::finish(void){
        }

    }
}
