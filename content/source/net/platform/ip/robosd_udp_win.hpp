#ifndef __robosd_net_udp_win_hpp
#define __robosd_net_udp_win_hpp


#include "net/robosd_udp.hpp"

#include <Ws2tcpip.h>

#include <thread>
namespace win {
    class ROBO_EXPORT  udp :public ::robo::net::udp {
    private:
        SOCKET socket_ = INVALID_SOCKET;
        uint8_t *  receive_buf_ = nullptr;
        uint16_t receive_buf_size_ = 0;
        sockaddr_in     outcom_addr__;
        WSAOVERLAPPED overlapped_;
        WSABUF data_buf_;
        DWORD bytes_recv_ = 0;
        DWORD flags_ = 0;
        int try_counter_ = 0;
        struct sockaddr_in sender_addr_;
        int sender_addr_size_ = sizeof (sender_addr_);
        HANDLE event_ = NULL;
        enum class  receive_state { none, ready,  wait } receive_state_ = receive_state::none;
        struct sockaddr_in  incom_;
        bool open_(void);
        void close_(void);
    protected:
        inline const SOCKET & remote(void ){ return socket_; }
        //int receive_tm = 0;
        virtual bool  do_post( const uint8_t * _data, size_t _len);

        bool try_receive(unsigned int _receive_tm);
    public:
        virtual void reset(void){};
        virtual size_t space(void){ return 0xFFFF; };
        virtual size_t max_space(void){  return 0xFFFF; };
        udp(void);
        virtual bool begin(::robo::cstr _alias);
        virtual void finish(void);
    };


   class ROBO_EXPORT  udp_thread :public udp {
	private:
		
        bool terminate_ = false;
        std::thread * recever_thread_ = nullptr;
     protected:
        void recever_thread_performer_(void);
     public:
        udp_thread(void);
        bool start(void);
		void stop(void);
        virtual void finish(void);
	};

   class ROBO_EXPORT  udp_poll :public udp {
       bool backend_;
    protected:
       virtual void poll(void);
    public:
        udp_poll(bool _backend);
        void start(void);
        void stop(void);
    };
}

#endif