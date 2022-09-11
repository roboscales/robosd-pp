#include "net/robosd_serial.hpp"
#include "net/relay/robosd_serial_relay.hpp"
#include "core/robosd_system.hpp"
#if  APP_NET_RELEY_ENABLED == 1
namespace robo{
	namespace net{
		namespace serial{
			namespace relay{
				channel::channel(int _id, ienvironment & _environment)
					: robo::net::relay::channel(_id)
					, serial(&robo::net::serial_dummy::instance())
					, environment(_environment)
					, decoder_(*this)
					, encoder_(*this)
				{
				}

				void channel::decoder::reset_(void){
					synco_.size = environment.synchro.size;
					synco_.memo = environment.synchro.memo;
					data_.memo = environment.input_buffer.memo;
					data_.size.need = data_.size.total = 0;
					data_.crc = 0;
					last_alive_us_ = ROBO_TIME_US_INFINITE;
					state_ = IDLE;
				}
				void channel::decoder::refuse_(void){
					reset_();
				}
				void channel::decoder::complete_(void){
					::robo::net::relay::machine::receive(channel_, data_.endpoint, environment.input_buffer.memo, data_.size.total);
					reset_();
				}
				channel::decoder::decoder(channel & _channel)  
					: robo::net::relay::decoder(_channel)
					, environment(_channel.environment)
				{
						reset_();
				}

				void channel::decoder::poll(void){
					bool exists = false;
					while ( ((channel &)channel_).serial->available()){
						perform_(((channel &)channel_).serial->get());
						exists = true;
					}
					if (exists){
						last_alive_us_ = robo::system::time_us();
					}
					else{
						if (state_ != IDLE){
							if (robo::system::time_us() - last_alive_us_ > environment.timeouts.broke_us){
								refuse_();
							}
						}
					}
				}

				void channel::decoder::perform_(uint8_t _byte){
					switch (state_){
					case IDLE:
					case RECEIVE_SYNCHRO:
						if (synco_.size > 0){
							if (_byte == *synco_.memo++){
								synco_.size--;
								if (synco_.size == 0){
									state_ = WAIT_TARGET;
								}
								else{
									state_ = RECEIVE_SYNCHRO;
								}
							}
							else{
								refuse_();
							}
							data_.crc += _byte;
							break;
						}
					case WAIT_TARGET:
						data_.endpoint = ((_byte & 0xF0) >> 4);
						data_.size.hi = ((_byte & 0x0F)<<8);
						data_.crc += _byte;
						state_ = WAIT_SIZE;
						break;
					case WAIT_SIZE:
						{
							packet_size_t sz = data_.size.hi + _byte;
							if (sz > environment.input_buffer.size){
								data_.size.total = 0;
								refuse_();
							}
							else {
								if (sz == 0){
									data_.size.total = 0;
									state_ = WAIT_CRC;
								}
								else {
									data_.size.total = data_.size.need = sz;
									state_ = RECEIVE_DATA;
								}
							}
							data_.crc += _byte;
						}
						break;
					case RECEIVE_DATA:
						*data_.memo++ = _byte;
						if (--data_.size.need == 0){
							state_ = WAIT_CRC;
						}
						data_.crc += _byte;
						break;
					case WAIT_CRC:
						if (data_.crc == _byte){
							complete_();
						}
						else{
							refuse_();
						}
					}
				}

				channel::encoder::encoder(channel & _channel) 
					: robo::net::relay::encoder(_channel)
					, environment(_channel.environment)
					, out_(0)
				{

				}
				channel::encoder::~encoder(void){
				}
				robo_result_t channel::encoder::process(void){
					if (state_ == IDLE){
						return ROBO_SUCCESS;
					}
					else {
						bool alive = false;
						if (robo::system::time_us() - last_alive_us_ > environment.timeouts.broke_us){
							state_ = IDLE;
							out_ = 0;
							return ROBO_ERROR;
						}

						while (((channel &)channel_).serial->space()){
							uint8_t tmp;
							switch (state_){
							case SEND_SYNCHRO:
								tmp = *synco_.memo++;
								synco_.size--;
								if (synco_.size == 0){
									state_ = SEND_TARGET;
								}
								break;
							case SEND_TARGET:
								tmp = ((data_.endpoint & 0xF) << 4) + (( data_.size & 0xF00)>>8);
								state_ = SEND_SIZE;
								break;
							case SEND_SIZE:
								tmp = (data_.size & 0xFFF);
								if (data_.size == 0){
									state_ = SEND_CRC;
								}
								else{
									state_ = SEND_DATA;
								}
								break;
							case SEND_DATA:
								tmp = *data_.memo++;
								data_.size--;
								if (data_.size == 0){
									state_ = SEND_CRC;
								}
								break;
							case SEND_CRC:
								((channel &)channel_).serial->put(data_.crc);
								state_ = IDLE;
								out_ = 0;
								return ROBO_SUCCESS;
							default:
								return ROBO_SUCCESS;								
							}
							data_.crc += tmp;
							((channel &)channel_).serial->put(tmp);
							alive = true;
						}
						if (alive){
							last_alive_us_ = robo::system::time_us();
						}
						return ROBO_CONTINUE;
					}
				}
				void channel::encoder::send(robo::net::relay::out * _out){
					robo_size_t sz = _out->size();
					if (state_ != IDLE || out_ != 0 || sz > max_packet_size){
						_out->refuse();
					}
					else {
						out_ = _out;
						synco_.size = environment.synchro.size;
						synco_.memo = environment.synchro.memo;
						data_.memo = out_->data();
						data_.size = (packet_size_t)sz;
						data_.endpoint = (uint8_t)out_->endpoint_id();
						data_.crc = 0;
						state_ = SEND_SYNCHRO;
						last_alive_us_ = robo::system::time_us();
					}
				}
				void channel::connect(iserial * _serial){
					if (serial){
						serial->reset();
					}
					if (_serial){
						serial = _serial;
					}
					else{
						serial = &robo::net::serial_dummy::instance();
					}
				}
			}
		}
	}
}
#endif