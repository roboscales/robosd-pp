#include "core/robosd_string.h"
#include "core/robosd_ini.hpp"
#include "net/ip/robosd_tcp_master_win.hpp"
namespace win{
    void tcp_master::stop(void){
        if(terminate_event_){
            SetEvent(terminate_event_);
        }
    }
    tcp_master::tcp_master(void): ::robo::net::tcp_master(){
    };

    tcp_master::~tcp_master(void){
    }

    bool tcp_master::do_accept_(void){
        remote_ = accept(local_, (SOCKADDR*)(&remote_addr_), &remote_addr_size_);
        if(remote_ == INVALID_SOCKET){
            robo_errlog("robo net: accept fail with code %d",WSAGetLastError ());
        }else {
            robo_infolog("robo net : remote connection from: %s",inet_ntoa (remote_addr_.sin_addr));
            state_ = state::active;

            receive_event_= WSACreateEvent();

            if(receive_event_ == NULL){
                robo_errlog("robo net: WSACreateEvent fail with code %d", WSAGetLastError());
            } else {
                if (WSAEventSelect(remote_, receive_event_, FD_READ | FD_CLOSE)){
                    robo_errlog("robo net: WSAEventSelect fail with code %d", WSAGetLastError());
                } else {
                    state_ = state::active;
                    return true;
                }
            }
        }
        return false;
    }

    bool tcp_master::accept_run_(unsigned int _receive_tm){
        WSAEVENT events[2] = {terminate_event_, accept_event_ };
        WSANETWORKEVENTS    ws;
        if (WSAWaitForMultipleEvents(2, events, false, (DWORD)  _receive_tm , false) == 1){
            ResetEvent (accept_event_);
            if(!WSAEnumNetworkEvents(local_, accept_event_, &ws)){
                if (ws.iErrorCode[FD_ACCEPT_BIT]){
                    robo_errlog("robo net: FD_ACCEPT fail. Stopping..");
                } else {
                    if( do_accept_() ){
                        return true;
                    } else {
                        remote_close_();
                    }
                }
            } else {
                robo_errlog("robo net: WSAEnumNetworkEvent fail with code %d", WSAGetLastError ());
            }
        }
        return false;
    }

    bool tcp_master::active_run_(unsigned int _receive_tm){
        WSAEVENT events[3] = {terminate_event_, accept_event_, receive_event_};
        WSANETWORKEVENTS    ws;
        switch ( WSAWaitForMultipleEvents(3, events, false, (DWORD)  _receive_tm , false) ){
            case 0:
                break;
            case 1:
                if(!WSAEnumNetworkEvents(local_, accept_event_, &ws)){
                    if (ws.iErrorCode[FD_ACCEPT_BIT]){
                        robo_errlog("robo net: FD_ACCEPT fail. Stopping..");
                    } else {
                        remote_close_();
                        state_ = state::accept;
                        if( do_accept_() ){
                            return true;
                        } else {
                            remote_close_();
                        }
                    }
                } else {
                    robo_errlog("robo net: WSAEnumNetworkEvent fail with code %d", WSAGetLastError ());
                    ResetEvent (accept_event_);
                }
            case 2:
                if(!WSAEnumNetworkEvents(remote_, receive_event_ , &ws)){
                      if(ws.lNetworkEvents & FD_READ){
                          if(ws.iErrorCode[FD_READ_BIT]){
                              robo_errlog("robo net: FD_READ fail");
                              break;
                          } else {
                              int rb;
                              while( (rb= recv(remote_, (char *)receive_buf_, receive_buf_size_, 0)) > 0 ){
                                receive(receive_buf_, rb);                                
                              }
                              return true;
                          }
                      }
                      if(ws.lNetworkEvents & FD_CLOSE){

                          if(ws.iErrorCode[FD_CLOSE_BIT]){
                              robo_errlog("robo  net: FD_CLOSE fail with code %d ", ws.iErrorCode[FD_CLOSE_BIT]);
                          } else {
                              robo_infolog("robo net: Client with from %s  disconnected", inet_ntoa (remote_addr_.sin_addr) );
                          }
                          remote_close_();
                          state_ = state::accept;
                      }

                  } else {
                    robo_errlog("robo net: WSAEnumNetworkEvent fail with code %d", WSAGetLastError ());
                    ResetEvent (accept_event_);
                }
            break;

        }
        return false;
    }

    void tcp_master::remote_close_(void){
        if( receive_event_ != NULL){
            WSAEventSelect(remote_, receive_event_, 0);
            CloseHandle((HANDLE)receive_event_);
        }
        if( remote_ != INVALID_SOCKET ){
            shutdown(remote_, SD_BOTH);
            closesocket (remote_);
            remote_ = INVALID_SOCKET;
        }
    }

    bool  tcp_master::do_post( const uint8_t * _data, robo_size_t _len){
        if(remote_ != INVALID_SOCKET){
            send(remote_,(char *) _data, _len, 0);
            return true;
        }else{
            return false;
        }/*
        ROBO_BREAKN(remote_ != INVALID_SOCKET)
        if( send(remote_,(char *) _data, _len, 0) != 0){
            robo_errlog("robo net: send fail with code %d ",  WSAGetLastError () );
        }
        return true;*/
    }


    bool tcp_master::begin(const robo_string_t _alias)
    {
        ROBO_BREAKN(::robo::net::tcp_master::begin(_alias));

        int tmp;
        robo_ini_load_int(alias() ,RS("PAYLOAD_MAX_SIZE") ,4096, &tmp);
        receive_buf_size_ = (robo_size_t)tmp;
        if(receive_buf_size_>0) receive_buf_ = new uint8_t [receive_buf_size_];
        ROBO_BREAKN(receive_buf_!=nullptr);


        terminate_event_ =  (WSAEVENT)CreateEvent(NULL, true, false, TEXT("ROBO_MASTER_NET_TERMINATE_EVENT") );
        if(terminate_event_ == NULL) {
            robo_errlog("robo net: Create terminate event fail with code %d", GetLastError ());
            goto err;
        }
        local_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(local_ == INVALID_SOCKET){
            robo_errlog("robo net: socket fail with code %d", WSAGetLastError ());
             goto err;
        }
        SOCKADDR_IN           saddr;
        ZeroMemory(&saddr, sizeof (SOCKADDR_IN));
        saddr.sin_family = AF_INET;
        saddr.sin_port = htons (incom_port());
        saddr.sin_addr.s_addr = INADDR_ANY;
        //    saddr.sin_addr.s_addr = inet_addr(host_name);
        if(bind(local_, (SOCKADDR*)(&saddr),sizeof(SOCKADDR_IN)))
        {
          robo_errlog("robo net bind failed with code  %d", WSAGetLastError ());
          goto err;
        }

        if(::listen(local_, 100)){
             robo_errlog("robo net listen failed  with cod  %d", WSAGetLastError ());
             goto err;
         }

         accept_event_ =  WSACreateEvent();
         if (accept_event_ == NULL){
             robo_errlog("robo net: Create acept event event fail with code %d", GetLastError ());
            goto err;
         }

         if(WSAEventSelect(local_, accept_event_, FD_ACCEPT)){
            robo_errlog("robo net :WSAEventSelect (accept event) fail with code %d", WSAGetLastError ());
            goto err;
         }

         robo_infolog("robo net : listen port %d on %s", incom_port(), inet_ntoa (saddr.sin_addr));
         state_ = state::accept;
        return true;
    err:
       finish();
       return false;
    }

    void tcp_master::finish(void){
        if (accept_event_ != NULL){
            if(local_!=INVALID_SOCKET){
                WSAEventSelect(local_, accept_event_, 0);
            }
            WSACloseEvent(accept_event_);
            accept_event_ = NULL;
        }
        if (local_!=INVALID_SOCKET){
            closesocket(local_);
            local_=INVALID_SOCKET;
        }
        if(terminate_event_!=NULL) {
            CloseHandle((HANDLE)terminate_event_);
            terminate_event_ = NULL;
        }
        if(receive_buf_ != nullptr)  delete [] receive_buf_;
    }

    bool tcp_master::try_receive(unsigned int _receive_tm) {
        switch(state_){
        case state::none:
            return false;
        case state::accept:
            return accept_run_(_receive_tm);
        case state::active:
            return active_run_(_receive_tm);

        }
        return false;
    }

    tcp_master_thread::tcp_master_thread(void)
        : tcp_master()
    {
    }


    void tcp_master_thread::stop(void) {
        tcp_master::stop();
        terminate_ = true;
    }
    void tcp_master_thread::finish(void) {
        tcp_master::finish();
        if(recever_thread_ != nullptr){
            recever_thread_->join();
            delete recever_thread_;
            recever_thread_ = nullptr;
        }
    }
    bool tcp_master_thread::start(void) {
        if(recever_thread_!=nullptr){
            return false;
        } else {
            recever_thread_ = new std::thread ( [this]{  while ( !this->terminate_ ) this->try_receive(10) ; } );
            return recever_thread_!=nullptr;
        }

    }


    void tcp_master_poll::poll(void){
        try_receive(0);
    };

    tcp_master_poll::tcp_master_poll(bool _backend): tcp_master(), backend_(_backend){
    }
    void tcp_master_poll::start(void){
        start_poll(backend_);
    }
    void tcp_master_poll::stop(void){
        tcp_master::stop();
        stop_poll();
    }

}
