#include "core/robosd_list.hpp"
#include "net/robosd_net_link.hpp"

namespace robo{
    namespace net{
        class ROBO_EXPORT tcp_master: public link{
            int listen_port_ = -1;
        public:
            inline int listen_port(void) {return listen_port_;};
            //опция- послать на другой порт
            tcp_master(void);
            virtual ~tcp_master(void);
            virtual bool begin(cstr _alias);
            virtual void finish(void);
        };

    }
}
