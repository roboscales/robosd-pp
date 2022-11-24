#ifndef __robosd_net_udp_linux_hpp
#define __robosd_net_udp_linux_hpp


#include "net/robosd_udp.hpp"

#include <thread>
//#include <sys/socket.h>
//#include <arpa/inet.h>
#include <netdb.h>
namespace linux {
    class ROBO_EXPORT  udp :public ::robo::net::udp {
    private:
	    uint8_t *  receive_buf_ = nullptr;
	    uint16_t receive_buf_size_ = 0;
	    int try_counter_ = 0;
	    int socket_ = -1;
	    struct sockaddr_in  incom_;

	    struct sockaddr_in     outcom_addr__;
		

	    //robo::delegat::owned_fabric<void, int>::member<udp> io_handler_;
	    //void io_handler__(int );
	        
	    
		//enum class  receive_state { none, ready,  wait } receive_state_ = receive_state::none;

	    bool open_(void);
        void close_(void);
    protected:
        //inline const SOCKET & remote(void ){ return socket_; }
        //int receive_tm = 0;
        virtual bool  do_post( const uint8_t * _data, size_t _len);

        bool try_receive(unsigned int _receive_tm);
	    virtual bool do_applay_outcom_address(void);
    public:
        virtual void reset(void){};
        virtual size_t space(void){ return 0xFFFF; };
        virtual size_t space_max(void){  return 0xFFFF; };
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