#include "net/platform/ip/robosd_udp_win.hpp"
#include "core/robosd_ini.hpp"

namespace win {
	
		
    udp::udp(void)
        : ::robo::net::udp()
    {
	}

    void udp::close_(void) {
        closesocket(socket_);
        socket_ = INVALID_SOCKET;
        receive_state_ = receive_state::none ;

    }

    bool udp::open_(void) {
        socket_ = socket(AF_INET, SOCK_DGRAM, 0);

        if (socket_ == INVALID_SOCKET)
        {
            ROBO_LBREAK_F("could not create  incom UDP socket for port: %d", incom_port());
        }

        if (bind(socket_, (struct sockaddr*)&incom_, sizeof(incom_)) != 0)
        {
            close_();
            ROBO_LBREAK_F("could not bind incom  UDP socket with port:  %d", incom_port());
        }

        receive_state_ = receive_state::ready;
        return true;
    }

    bool udp::try_receive(unsigned int _receive_tm) {
        int rc;
        switch(receive_state_){
        case receive_state::none:
            ROBO_LBREAKN( open_() );

        case receive_state::ready:

            data_buf_.len = receive_buf_size_;
            data_buf_.buf = (CHAR *)receive_buf_;

            // Make sure the Overlapped struct is zeroed out
            SecureZeroMemory((PVOID) &overlapped_, sizeof(WSAOVERLAPPED) );
            overlapped_.hEvent = event_;

            rc = WSARecvFrom(socket_,
                              &data_buf_,
                              1,
                              &bytes_recv_,
                              &flags_,
                              (SOCKADDR *) & sender_addr_,
                              &sender_addr_size_
                              , &overlapped_
                              , NULL);

            if (rc != 0) {
                int err = WSAGetLastError();
                if(err != WSA_IO_PENDING){
                    close_();
                    return false;
                } else {
                    receive_state_ = receive_state::wait ;
                }
            } else {
                if(bytes_recv_>0){
                    receive(receive_buf_,bytes_recv_);
                    return true;
                } else {
                    return false;
                }
            }
        case receive_state::wait:
            try_counter_++;
            rc = WSAWaitForMultipleEvents(1, &overlapped_.hEvent, TRUE, _receive_tm, TRUE);
            switch(rc){
            case WSA_WAIT_EVENT_0:
                receive_state_ = receive_state::ready ;
                break;
            case WSA_WAIT_TIMEOUT:
                if(try_counter_>100){
                    try_counter_ = 0;
                    receive_state_ = receive_state::ready ;
                }
                return false;
            default:
                close_();
                ROBO_LBREAK();
            }

            rc = WSAGetOverlappedResult(socket_, &overlapped_, &bytes_recv_,
                                    FALSE, &flags_);
            if(rc == TRUE){
                if(bytes_recv_>0){
                    receive(receive_buf_,bytes_recv_);
                    return true;
                } else {
                    return false;
                }
            }else{
                close_();
                return false;
            }
        }
        return false;
	}


	
    bool udp::do_post( const uint8_t * _data, size_t _size){
        outcom_addr__.sin_port = htons(outcom_port());

        int r = sendto(socket_,
            (char *)_data,
            _size,
            0,
            (const struct sockaddr *) &outcom_addr__,
            sizeof(outcom_addr__));
        if (r == SOCKET_ERROR) {
            robo_errlog("socket %s send error", alias().c_str() );
            return false;
        } else {
            return true;
        }

    }


    bool udp::begin(::robo::cstr _alias)
	{
        ROBO_LBREAKN(::robo::net::udp::begin(_alias));

        //setsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
        receive_buf_size_ = 4096;
        ::robo::ini::try_load(alias(), RT("PAYLOAD_MAX_SIZE"), receive_buf_size_);
        if(receive_buf_size_>0) receive_buf_ = new uint8_t [receive_buf_size_];
        ROBO_LBREAKN(receive_buf_!=nullptr);		
		
        std::fill_n((uint8_t *)&incom_, sizeof(incom_) ,0);
		
        incom_.sin_addr.s_addr = htonl(INADDR_ANY);//inet_addr(outcom_addr_.c_str());
        incom_.sin_port = htons(incom_port());
        incom_.sin_family = AF_INET;
		
		struct hostent        *he;
        if ((he = gethostbyname(network().c_str())) == NULL) {
            ROBO_LBREAK_F("could not resolve  outcom host:  %s", network().c_str());
		}

		memset(&outcom_addr__, 0, sizeof(outcom_addr__)); 
      
		// Filling server information 
		outcom_addr__.sin_family = AF_INET; 
        outcom_addr__.sin_port = htons(outcom_port());
		memcpy(&outcom_addr__.sin_addr, he->h_addr_list[0], he->h_length);

        // Create an event handle and setup the overlapped structure.
        event_ = WSACreateEvent();
        ROBO_LBREAKN(event_ != NULL);

        return true;
	}
		
    void udp::finish(void) {
		if (socket_ != INVALID_SOCKET) {
			closesocket(socket_);
			socket_ = INVALID_SOCKET;
		}

        if(receive_buf_ != nullptr)  delete [] receive_buf_;
        WSACloseEvent(event_);
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

