#ifndef __robo_net_serial_reley_h
#define __robo_net_serial_reley_h
#include "net/robosd_serial.hpp"
#include "net/relay/robosd_relay.hpp"

#if  APP_NET_RELEY_ENABLED == 1
namespace robo{
	namespace net{
		namespace serial{
			namespace relay{
				typedef uint16_t packet_size_t;


				class ROBO_EXPORT channel : public robo::net::relay::channel{
				public:
					iserial * serial;
					struct ienvironment{
						struct{
							robo_byte_p memo;
							packet_size_t size;
						} synchro;
						struct{
							robo_byte_p memo;
							packet_size_t size;
						} input_buffer;
						struct{
							robo_time_us_t broke_us;
						} timeouts;
					}	environment;
				private:
					class ROBO_EXPORT decoder : public robo::net::relay::decoder{
					public:
						channel::ienvironment & environment;
					private:
						struct{
							packet_size_t size = 0;
							robo_byte_p memo = 0;
						} synco_;
						struct {
							robo_byte_p memo = 0;
							struct{
								packet_size_t need;
								packet_size_t total;
								packet_size_t hi;
							} size;
							uint8_t crc;
							uint8_t endpoint;
						} data_;
						robo_time_us_t last_alive_us_;
						enum  { IDLE, RECEIVE_SYNCHRO, WAIT_TARGET, WAIT_SIZE, RECEIVE_DATA, WAIT_CRC } state_ = IDLE;
						void reset_(void);
						void refuse_(void);
						void complete_(void);
						void perform_(uint8_t _byte);
					public:
						decoder(channel & _channel);
						virtual void poll(void);
					};
					class ROBO_EXPORT encoder : public robo::net::relay::encoder{
					public:
						channel::ienvironment & environment;
					private:
						robo::net::relay::out * out_;
						struct{
							packet_size_t size = 0;
							robo_byte_p memo = 0;
						} synco_;
						struct {
							robo_byte_p memo = 0;
							packet_size_t size = 0;
							uint8_t crc;
							uint8_t endpoint;
						} data_;
						robo_time_us_t last_alive_us_;
						enum  { IDLE, SEND_SYNCHRO, SEND_TARGET, SEND_SIZE, SEND_DATA, SEND_CRC } state_ = IDLE;
					protected:
						virtual robo_result_t process(void);
						virtual void send(robo::net::relay::out * _out);
					public:
						encoder(channel & _channel);
						virtual ~encoder(void);
					};
					decoder decoder_;
					encoder encoder_;
				public:
					enum { max_packet_size = 4095 };
					channel(int _id, ienvironment & _environment);
					/*				virtual void receive(robo_byte_p _data, packet_size_t _size);
									virtual void complete(packet * _packet);
									virtual void refuse(packet * _packet);*/
					void connect(iserial * _serial);
				};

			}
		}
	}
} 


#endif

#endif
