#include "net/platform/ip/robosd_udp_linux.hpp"
#include "core/robosd_ini.hpp"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <csignal>
#include <list>
#include <map>
#include <algorithm>

namespace linuxx {	
	/*
	class std_signal {
	public:
		typedef robo::delegat::ref<void,int> handler;
	private:
		typedef  std::list<handler *> handlers;
		std::map<int, handlers *> store_;
		handlers io_;
		static std_signal & instance_() {
			static std_signal instance__;
			return instance__;
		} 
		void io_handler__(int signal) { 			
			for (handlers::iterator it = io_.begin(); it != io_.end(); it++) 
				(*(*it))(signal);
		}
		static void io_handler___(int _signal) {
			instance_().io_handler__(_signal);
		}
		std_signal(void) {
			store_[SIGIO] = &io_;
			signal(SIGIO, io_handler___);
		}				
		bool attach_(int _signal, handler * _h) {
			std::map<int, handlers *>::iterator it = store_.find(_signal);
			if (it != store_.end()) {
				it->second->push_back(_h);
				return true;
			}
			return false;
		}
		void dettach_(handler * _h) {
			for (std::map<int, handlers *>::iterator it = store_.begin(); it != store_.end(); it++) {
				it->second->remove(_h);
			}
		}
	public:
		static bool attach(int _signal, handler * _h) {
			return instance_().attach_(_signal, _h);
		}
		
		static void dettach(handler * _h) {
			instance_().dettach_(_h);
		}
	};
	*/
    udp::udp(void)
        : ::robo::net::udp()
		//, io_handler_(*this, &udp::io_handler__)
    {
	}

    bool udp::try_receive(unsigned int _receive_tm) {
	    struct sockaddr_in remaddr;
	    socklen_t addrlen = sizeof(remaddr);
	    int recvlen = recvfrom(socket_, receive_buf_, receive_buf_size_, 0, (struct sockaddr *)&remaddr, &addrlen);
	    if (recvlen>0) {
		    receive(receive_buf_, recvlen);	        
	    }
	    return true;
	}


	
    bool udp::do_post( const uint8_t * _data, size_t _size){
	    outcom_addr__.sin_port = htons(outcom_port()); 
	    ROBO_LBREAKN_F( sendto(socket_,
	                    _data,
	                    _size,  
	                   0,
	                   (const struct sockaddr *) &outcom_addr__, 
	                   sizeof(outcom_addr__)) > 0, "send error"); 
	    return true;
    }


    bool udp::begin(::robo::cstr _alias)
	{
        ROBO_LBREAKN(::robo::net::udp::begin(_alias));

        //setsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
        receive_buf_size_ = 4096;
        ::robo::ini::try_load(alias(), RT("PAYLOAD_MAX_SIZE"), receive_buf_size_);
        if(receive_buf_size_>0) receive_buf_ = new uint8_t [receive_buf_size_];
        ROBO_LBREAKN(receive_buf_!=nullptr);		
		
		struct sockaddr_in  incom;
		
		incom =  { };
		
		incom.sin_addr.s_addr =  htonl(INADDR_ANY); //inet_addr(outcom_addr_.c_str()); 
		incom.sin_port = htons(incom_port()); 
		incom.sin_family = AF_INET; 
		
		socket_ = socket(AF_INET, SOCK_DGRAM, 0);         
		if (socket_ == -1) {
			ROBO_LBREAK_F("could not create  incom UDP socket for port: %d", incom_port());
		}
		
		if (bind(socket_, (struct sockaddr*)&incom, sizeof(incom)) != 0) {
			close(socket_);
			socket_ = -1;
			ROBO_LBREAK_F("could not bind incom  UDP socket with port:  %d", incom_port());
		}
	

		// first: set up a SIGIO signal handler by use of the signal call()
		//std_signal::attach(SIGIO, &io_handler_);
		


		// second: set the process id or process group id that is to receive
		// notification of pending input to its own process id or process 
		// group id
		
		//if (fcntl(socket_, F_SETOWN, getpid()) < 0) {
			//ROBO_BREAK_F("error fcntl(socket_, F_SETOWN, getpid()) ");
		//}

		// third: allow receipt of asynchronous I/O signals
		//if (fcntl(socket_, F_SETFL, FASYNC) < 0) {
			//ROBO_BREAK_F("error fcntl(socket_, F_SETFL, FASYNC)");
		//}
		
		struct timeval read_timeout;
		read_timeout.tv_sec = 0;
		read_timeout.tv_usec = 10;
		setsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof read_timeout);
		
		
        return true;
	}
	bool udp::begin()
	{
        receive_buf_size_ = 4096;
        if(receive_buf_size_>0) receive_buf_ = new uint8_t [receive_buf_size_];
        ROBO_LBREAKN(receive_buf_!=nullptr);		
		
		struct sockaddr_in  incom;
		
		incom =  { };
		
		incom.sin_addr.s_addr =  htonl(INADDR_ANY); //inet_addr(outcom_addr_.c_str()); 
		incom.sin_port = htons(incom_port()); 
		incom.sin_family = AF_INET; 
		
		socket_ = socket(AF_INET, SOCK_DGRAM, 0);         
		if (socket_ == -1) {
			ROBO_LBREAK_F("could not create  incom UDP socket for port: %d", incom_port());
		}
		
		if (bind(socket_, (struct sockaddr*)&incom, sizeof(incom)) != 0) {
			close(socket_);
			socket_ = -1;
			ROBO_LBREAK_F("could not bind incom  UDP socket with port:  %d", incom_port());
		}
		
		struct timeval read_timeout;
		read_timeout.tv_sec = 0;
		read_timeout.tv_usec = 10;
		setsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof read_timeout);
		
		
        return true;
	}
	
    void udp::finish(void) {
		if (socket_ != -1) {
			close(socket_);
			socket_ = -1;
		}

        if(receive_buf_ != nullptr)  delete [] receive_buf_;
	}
	bool udp::do_applay_outcom_address(void) {
		struct hostent        *he;
		network().ascii([&he](const char* _s) { he = gethostbyname(_s); });

		ROBO_LBREAKN_F(he != NULL, "could not resolve  outcom host:  %s", network().c_str());

		outcom_addr__ = { };      
		// Filling server information 
		outcom_addr__.sin_family = AF_INET; 
		outcom_addr__.sin_port = htons(outcom_port()); 
		std::copy_n((uint8_t *)he->h_addr_list[0], he->h_length, (uint8_t *)&outcom_addr__.sin_addr);
		return true;
	}
	
    udp_thread::udp_thread()
        : udp()
    {
    }


    void udp_thread::stop(void) {
        terminate_ = true;
    }
    void udp_thread::finish(void) {
        udp::finish();
        if(recever_thread_ != nullptr){
            recever_thread_->join();
            delete recever_thread_;
            recever_thread_ = nullptr;
        }
    }
    bool udp_thread::start(void) {
        if(recever_thread_!=nullptr){
            return false;
        } else {
            recever_thread_ = new std::thread ( [this]{  while ( !this->terminate_) this->try_receive(10); } );
            return recever_thread_!=nullptr;
        }

    }


    void udp_poll::poll(void){
        try_receive(0);
    };

    udp_poll::udp_poll(bool _backend): udp(), backend_(_backend){
    }
    void udp_poll::start(void){
        start_poll(backend_);
    }
    void udp_poll::stop(void){
        stop_poll();
    }


}

