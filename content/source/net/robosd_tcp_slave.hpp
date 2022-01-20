#include "core/robosd_list.hpp"
#include "net/robosd_net_link.hpp"

namespace robo{
    namespace net{
        class ROBO_EXPORT tcp_slave: public link{
             int servo_port_ = -1;
             string  network_;
        public:
            inline int servo_port(void) {return servo_port_;};
            const string &   network(){return network_;};
            tcp_slave(void);
            virtual ~tcp_slave(void);
            virtual bool begin(void);
            virtual void finish(void);
        };

    }
}
