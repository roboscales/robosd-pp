#include "net/ip/robosd_tcp_master.hpp"
#include <Ws2tcpip.h>
#include <thread>

namespace win{

        class ROBO_EXPORT tcp_master: public ::robo::net::tcp_master{
            SOCKET local_ = INVALID_SOCKET;
            SOCKET remote_ = INVALID_SOCKET;
            enum class state{ none, accept, active} state_ = state::none;

            bool accept_run_(unsigned int _receive_tm);
            bool active_run_(unsigned int _receive_tm);

            void remote_close_(void);
            bool do_accept_(void);
            WSAEVENT terminate_event_ = NULL;
            WSAEVENT accept_event_ = NULL;
            WSAEVENT receive_event_ = NULL;
            uint8_t *  receive_buf_ = nullptr;
            uint16_t receive_buf_size_ = 0;

            SOCKADDR_IN remote_addr_;
            int remote_addr_size_ = sizeof(remote_addr_);
        protected:
            inline const SOCKET & remote(void ){ return remote_; }
            inline const SOCKET & local(void ){ return local_; }

            virtual bool  do_post( const uint8_t * _data, robo_size_t _len);

            bool try_receive(unsigned int _receive_tm);

        public:
            virtual void reset(void) {/*todo отменть отправку*/}
            virtual robo_size_t space(void) { return 0xFFFF;  };
            virtual robo_size_t max_space(void) { return 0xFFFF; };
            tcp_master(void);
            virtual ~tcp_master(void);
            virtual bool begin(const robo_string_t _alis);
            virtual void finish(void);
            virtual void stop(void);
        };

        class ROBO_EXPORT  tcp_master_thread :public tcp_master {
         private:

             bool terminate_ = false;
             std::thread * recever_thread_ = nullptr;
          public:
             tcp_master_thread(void);
             bool start(void);
             virtual void stop(void);
             virtual void finish(void);
         };

        class ROBO_EXPORT  tcp_master_poll :public tcp_master {
            bool backend_;
         protected:
            virtual void poll(void);
         public:
             tcp_master_poll( bool _backend);
             void start(void);
             virtual void stop(void);
         };

}
