#include "net/robosd_udp.hpp"
#include "core/robosd_string.hpp"
#include "core/robosd_ini.hpp"
namespace robo{
    namespace net{
        udp::udp(void): port(){
        };

        udp::~udp(void){
        }

        bool udp::postto(int _port, const uint8_t * _data, size_t _len){
            if(outcom_port_!= _port){
                int tmp = outcom_port_;
                outcom_port_ = _port;
                bool ret = port::post(_data, _len);
                outcom_port_ = tmp;
                return ret;
            } else {
                return port::post(_data, _len);
            }
        }

        bool udp::begin(cstr _alis)
        {
            ROBO_LBREAKN(link::begin(_alis))
            ROBO_LBREAKN(network_.load(alias(), RT("OUTCOM_ADDR")))
            ROBO_LBREAKN( ::robo::ini::load(alias(), RT("INCOM_PORT"), incom_port_) );
            ROBO_LBREAKN(::robo::ini::load(alias(), RT("OUTCOM_PORT"), outcom_port_) );
            return true;
        }
        void udp::finish(void){
            link::finish();
        }

    }
}
