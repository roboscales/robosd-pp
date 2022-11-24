#ifndef __robosd_net_udp_hpp
#define __robosd_net_udp_hpp

#include "core/robosd_list.hpp"
#include "net/robosd_net_link.hpp"

namespace robo{
    namespace net{
        class ROBO_EXPORT udp: public port{
             int outcom_port_ = -1;
             int incom_port_ = -1;
             ::robo::string  network_;
        public:
            inline int outcom_port(void) {return outcom_port_;};
            inline int incom_port(void) {return incom_port_;};
            const ::robo::string &   network(){return network_;};
            //опция- послать на другой порт
            bool postto(int _port, const uint8_t * _data, size_t _len);
            udp(void);
            virtual ~udp(void);
            virtual bool begin(cstr _alis);
            virtual void finish(void);
	        bool set_outcom_address(cstr _outcom_address = nullptr);
        protected:
	        virtual bool do_applay_outcom_address(void) {return true;}
        };

    }
}
#endif