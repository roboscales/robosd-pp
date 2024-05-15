#include "core/robosd_list.hpp"
#include "net/robosd_net_link.hpp"
#include "core/robosd_event.hpp"
#include "core/robosd_ini.hpp"

namespace robo{
    namespace net{
        class ROBO_EXPORT iclient {
        public:
            enum class statuses { connected, disconnected, busy, terminated};
            enum class faults { connection_refuse, shutdown_timeout, connect_timeout, connection_abort, panic, send_refuse, send_timeout };
            enum class events { connect, disconnect, terminate, reconnect };
            enum class faults_reaction { skeep = 0, terminate = 1 };
            struct {
                event_t<faults_reaction, faults> on_fault;
                event_t<void, events> on_event;
            } delegat;
        protected:
            enum class commands { none, disconnect, connect } command = commands::none;
        public:
            void connect(void) { command = commands::connect; }
            void disconnect(void) { command = commands::disconnect; }
            virtual statuses status(void) = 0;
        };

        template <class driver > class ROBO_EXPORT client_t : public link, public iclient, protected driver {
        private:
            struct {
                time_us_t connect = 0;
                time_us_t shutdown = 0;
                time_us_t reconnect = 0;
            } timeouts_;
            time_us_t time_shtamp_ = 0;
            enum class states { none, disconnected, shutdown, connected, connect_pause, connecting, terminated } state_ = states::none;
            time_us_t last_connected_ = 0;
            
            void terminate_(void) {
                state_ = states::terminated;
                driver::close();
                delegat.on_event.raise(events::terminate);
            }
            
            void raise_panic_(void) {
                state_ = states::terminated;
                driver::close();
                faults_reaction r = delegat.on_fault.raise(faults::panic);
                delegat.on_event.raise(events::terminate);
            }

            void raise_fault_(faults _fault) {
                faults_reaction r = delegat.on_fault.raise(_fault);
                if (r == faults_reaction::terminate) {
                    terminate_();
                }
            }

            void do_connect_(void) {
                if (driver::status() == statuses::connected ) {
                    state_ = states::connected;
                    delegat.on_event.raise(events::connect);
                }
                else {
                    if (robo::system::time_ms() - time_shtamp_ > timeouts_.connect) {
                        state_ = states::shutdown;
                        time_shtamp_ = robo::system::time_ms();
                        driver::close();
                        raise_fault_(faults::connect_timeout);
                    }
                }
            }

            void do_shutdown_(void) {
                if (driver::status() == statuses::disconnected) {
                    if(command != commands::connect){
                        state_ = states::disconnected;
                        delegat.on_event.raise(events::disconnect);
                    }
                    else {
                        state_ = states::connect_pause;
                        delegat.on_event.raise(events::reconnect);
                        time_shtamp_ = robo::system::time_ms();
                    }
                } else {
                    if (robo::system::time_ms() - time_shtamp_ > timeouts_.shutdown) {
                        state_ = states::disconnected;
                        raise_fault_(faults::shutdown_timeout);
                    }
                }
            }
            void do_pause_(void) {
                if (robo::system::time_ms() - time_shtamp_ > timeouts_.reconnect) {
                    state_ = states::disconnected;
                }
            }
        protected:
            void poll(void) {
                switch (state_) {

                case states::none:
                if (driver::status() == statuses::connected) raise_panic_();
                break;

                case states::disconnected:
                if (driver::status() != statuses::disconnected) raise_panic_();
                if (command == commands::connect) {
                    state_ = states::connecting;
                    time_shtamp_ = robo::system::time_ms();
                    driver::open();
                }
                break;

                case states::connecting:
                if (command == commands::disconnect) {
                    state_ = states::shutdown;
                    time_shtamp_ = robo::system::time_ms();
                    driver::close();
                }
                else {
                    do_connect_();
                }
                break;

                case states::connect_pause:
                if (driver::status() != statuses::disconnected) raise_panic_();
                do_pause_();
                break;
                case states::connected:
                if (driver::status() != statuses::connected) raise_panic_();
                if (command == commands::disconnect) {
                    state_ = states::shutdown;
                    time_shtamp_ = robo::system::time_ms();

                    driver::close();
                }
                else {
                    driver::poll();
                }
                break;

                case states::shutdown:
                do_shutdown_();
                break;



                case states::terminated:
                break;
                }
            }
                
            virtual void driver_refuse_connect(void) {
                if (state_ == states::connecting) {
                    state_ = states::shutdown;
                    time_shtamp_ = robo::system::time_ms();
                    driver::close();
                    raise_fault_(faults::connection_refuse);
                }
                else {
                    raise_panic_();
                }
            }
            virtual void driver_abort_session(void) {
                if (state_ == states::connected) {
                    state_ = states::shutdown;
                    time_shtamp_ = robo::system::time_ms();
                    driver::close();
                    raise_fault_(faults::connection_refuse);
                }
                else {
                    raise_panic_();
                }
            }

        public:
  

            client_t(void) {};
            virtual ~client_t(void) {};
            virtual bool begin(::robo::cstr _alias) { 
                ROBO_LBREAKN(link::begin(_alias));
                ROBO_LBREAKN(::robo::ini::load(alias(), RT("connect_timeout_ms"), timeouts_.connect));
                ROBO_LBREAKN(::robo::ini::load(alias(), RT("shutown_timeout_ms"), timeouts_.shutdown));
                ROBO_LBREAKN(::robo::ini::load(alias(), RT("reconnect_timeout_ms"), timeouts_.reconnect));
                state_ = states::disconnected;
                return true;
            }
            virtual void finish(void) { 
            }
            virtual iclient::statuses status(void) { 
                switch (state_) {
                case states::connected:
                    return statuses::connected;
                case states::disconnected:
                case states::connect_pause:
                return statuses::disconnected;
                case states::terminated:
                    return statuses::terminated;
                default:
                    return statuses::busy;
                }
            };
            bool connected(void) { return state_ == states:: connected; }
            bool disconnected(void) { return state_ == states::disconnected; }
            virtual void reset(void) { return driver::reset(); }

            virtual size_t space(void) {
                if (connected()) {
                    return driver::space();
                }
                else return 0;

            }
            virtual size_t space_max(void) {
                if (connected()) {
                    return driver::space_max();
                }
                else return 0;
            }
        };


        template<class driver > class ROBO_EXPORT tcp_client_t: public client_t<driver> {
        private:
             int servo_port_ = -1;
             string  servo_address_;
        public:
            int servo_port() { return servo_port_; }
            robo::cstr  servo_address() { return servo_address_.c_str(); };
            tcp_client_t(void) :client_t<driver>() {};
            virtual ~tcp_client_t(void) {};
            virtual bool begin(::robo::cstr _alias) {
                ROBO_LBREAKN(client_t<driver>::begin(_alias));
                ROBO_LBREAKN(servo_address_.load(alias(), RT("SERVO_ADDRESS")))
                ROBO_LBREAKN(::robo::ini::load(alias(), RT("SERVO_PORT"), servo_port_));
                driver::setup(servo_port_, servo_address_);
                return true;
            }
        };

        
        template<class driver > class ROBO_EXPORT tcp_poll_client_t : public tcp_client_t<driver> {
        public:
            tcp_poll_client_t(void) :tcp_client_t<driver>() {};
            virtual ~tcp_poll_client_t(void) {};
            bool send(const uint8_t* _message, size_t _length, robo::time_ms_t _timeout) {
                ROBO_LBREAKN(connected());
                ROBO_LBREAKN(driver::begin_send(_message, _length));
                robo::time_ms_t tm = robo::system::time_ms();
                while (driver::sended() < _length) {
                    ROBO_LBREAKN(connected());
                    ROBO_LBREAKN((robo::system::time_ms() - tm <= _timeout));
                    poll();
                }
                return true;
            }
            
            bool wait_connect(void) {
                iclient::connect();
                do {
                    poll();
                } while (status() == statuses::busy);
                if (status() == statuses::connected) {
                    return true;
                }
                else {
                    iclient::disconnect();
                    robo_errlog("error conected to %s : %d", servo_address(), servo_port());
                    return false;
                }
            }

            void connect(void) {
                iclient::connect();
            }

            void disconnect(void) {
                iclient::disconnect();
                do{
                    poll();
                }while (status() == statuses::busy);
                ROBO_VBREAKN(status() == statuses::disconnected);
            }
            bool receive(uint8_t* _message, size_t _length, robo::time_ms_t _timeout) {
                ROBO_LBREAKN(connected());
                ROBO_LBREAKN(driver::begin_receive(_message, _length));
                robo::time_ms_t tm = robo::system::time_ms();
                while (driver::received() < _length) {
                    ROBO_LBREAKN(connected());
                    ROBO_LBREAKN((robo::system::time_ms() - tm <= _timeout));
                    poll();
                }
                return true;
            }
            bool receive() {
                ROBO_LBREAKN(connected());
                uint8_t message[1024] = {};

                ROBO_LBREAKN(driver::begin_receive(message, 1024));
                
                if (driver::received()) {
                    robo_infolog(message);
                }
            }
            //bool receive_wait(uint8_t* message, size_t length, robo::time_ms_t timeout) {
            //}
        };

    }
}
