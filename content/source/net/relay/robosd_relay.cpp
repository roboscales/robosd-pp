#include "net/relay/robosd_relay.hpp"
#include "core/robosd_log.hpp"
#if  ROBO_APP_NET_RELEY_ENABLED == 1
namespace robo{
	namespace net{
		namespace relay{
			packet::~packet(void){
				robo_mem_free(data_);
				size_=  0;
			}
			packet::packet(channel & _channel, endpoint & _endpoint, robo_byte_p _data, robo_size_t _size)
				:data_(0)
				, size_(_size)
				, channel_(_channel)
				, endpoint_(_endpoint)
			{
				robo_mem_alloc(&data_, size_);
				robo_mem_set(data_, 0, size_, _data);
				//ref_.attach_to(endpoint_.incom_);
			}
			packet::packet(channel & _channel, endpoint & _endpoint, robo_size_t _size)
				: data_(0)
				, size_(_size)
				, channel_(_channel)
				, endpoint_(_endpoint)
			{
				robo_mem_alloc(&data_, size_);
			}
			packet::packet(channel & _channel, endpoint & _endpoint, uint8_t _msg, robo_byte_p _data, robo_size_t _size)
				:data_(0)
				, size_(_size+1)
				, channel_(_channel)
				, endpoint_(_endpoint)
			{
				robo_mem_alloc(&data_, size_);
				data_[0] = _msg;
				robo_mem_set(data_, 1, _size, _data + 1);
			}

			int  packet::endpoint_id(void){
				return endpoint_.ref_.id();
			}
			int  packet::channel_id(void){
				return channel_.ref_.id();
			}
			void in::confirm(void){
				channel_.confirm(this);
			}
			void in::refuse(void){ 
				channel_.refuse(this);
			}

			in::in(channel & _channel, endpoint & _endpoint, robo_byte_p _data, robo_size_t _size) :
				packet(_channel, _endpoint, _data, _size), ref_(this){
				ref_.attach_to(endpoint_.incom_);
			}

			void out::confirm(void){ 
				endpoint_.confirm(this); 
			}
			void out::refuse(void){ 
				endpoint_.refuse(this); 
			}
			out::out(channel & _channel, endpoint & _endpoint, robo_byte_p _data, robo_size_t _size) :
				packet(_channel, _endpoint, _data, _size), ref_(this){
			}
			out::out(channel & _channel, endpoint & _endpoint, robo_size_t _size) :
				packet(_channel, _endpoint, _size), ref_(this){
			}

			out::out(channel & _channel, endpoint & _endpoint, uint8_t _msg, robo_byte_p _data, robo_size_t _size)
				: packet(_channel, _endpoint, _msg,  _data, _size), ref_(this){

			}


			channel::channel(int _id) 
				: ref_(this, _id)
				, encoder_(0)
				, decoder_(0)
				, sended_(0)
			{
				ref_.attach_to(machine::instance().channels_);
			}
			void channel::poll(void){
				if (decoder_) decoder_->poll();
				if (encoder_){					
					if (sended_){
						switch (encoder_->process()){
						case ROBO_SUCCESS:
							sended_->confirm();
							machine::release((packet * &)sended_);
							sended_ = 0;
						case ROBO_CONTINUE:
							return;
						default:
							robo_errlog("relay channle refuse packet");
							sended_->refuse();
							machine::release((packet * &)sended_);
							sended_ = 0;
						}
					}
					if (sended_ == 0){
						if (outcom_.count() > 0){
							sended_ = outcom_.pop();
							if (sended_)
								encoder_->send(sended_);
						}
					}					
				}
				else {
					while (outcom_.count() > 0){
						sended_ = outcom_.pop();
						sended_->refuse();
						machine::release((packet * &)sended_);
					}
				}

			}
			decoder::decoder(channel & _channel) : channel_(_channel){
				channel_.decoder_ = this;
			}
			decoder::~decoder(void){
				if (channel_.decoder_ == this) channel_.decoder_ = 0;
			}
			encoder::encoder(channel & _channel) 
				: channel_(_channel)
			{
				channel_.encoder_ = this;
			}
			encoder::~encoder(void){
				if (channel_.encoder_ == this) channel_.encoder_ = 0;
			}

			endpoint::endpoint(void) : ref_(this) {

			}
			bool endpoint::attach(int _id) {
				ROBO_RETL(ref_.attach_to(machine::instance().endpoints_) == ROBO_SUCCESS);
			}
			void endpoint::detach(void) {
				ref_.dettach();
			}

			endpoint::endpoint(int _id) : ref_(this, _id){
				ref_.attach_to(machine::instance().endpoints_);
			}
			endpoint::~endpoint(void){

			}
			void endpoint::poll(void){
				in::ref * r = incom_.first();
				while (r){
					in * p = r->owner();
					r = r->next();
					switch ( process(p)){
					case ROBO_CONTINUE:
						continue;
					case ROBO_SUCCESS:
						p->confirm();
						machine::release((packet * &)p);
						break;
					default:
						p->refuse();
						machine::release((packet * &)p);
					}
				}
			}

			void endpoint::reset(void){
				in::ref * r = incom_.first();
				while (r){
					in * p = r->owner();
					r = r->next();
					p->refuse();
					machine::release((packet * &)p);
				}
			}
			bool machine::receive_(channel & _channel, int _endpoint, robo_byte_p _data, robo_size_t _size){
				endpoint * tg = endpoints_.find(_endpoint);
				if (tg){
					in * pk = new in(_channel, *tg, _data, _size);
					if (pk){
						return true;
					}
				}
				return false;
			}

			bool machine::post_(int _channel, endpoint & _endpoint, robo_byte_p _data, robo_size_t _size){
				channel * ch = channels_.find(_channel);
				if (ch){
					out * pk = new out(*ch, _endpoint, _data, _size);
					if (pk){
						ch->outcom_.push(pk);
						return true;
					}
				}
				return false;
			}
			bool machine::post_(out * _out){				
				if (_out){
					_out->channel_.outcom_.push(_out);
					return true;
				}
				return false;
			}
			bool machine::post_(int _channel, endpoint & _endpoint, uint8_t _msg, robo_byte_p _data, robo_size_t _size){
				channel * ch = channels_.find(_channel);
				if (ch){
					out * pk = new out(*ch, _endpoint, _msg, _data, _size);
					if (pk){
						ch->outcom_.push(pk);
						return true;
					}
				}
				return false;
			}
			machine & machine::instance(void){
				static machine instance__;
				return instance__;
			}
			void machine::poll_(void){
				for (channel::ref * r = channels_.first(); r; r = r->next())
					r->owner()->poll();
				for (endpoint::ref * r = endpoints_.first(); r; r = r->next())
					r->owner()->poll();
			}
			void machine::release_(packet * &  _packet){
				if (_packet){
					delete _packet;
					_packet = 0;
				}
			}
			out * machine::query_(int _channel, endpoint & _endpoint, robo_size_t _size){
				channel * ch = channels_.find(_channel);
				if (ch){
					return new  out(*ch, _endpoint, _size);
				}
				return 0;
			}
            bool machine::ready_(int _channel){
                channel * ch = channels_.find(_channel);
                if (ch){
                    return ch->ready();
                } else {
                    return false;
                }

            }

		}
	}
}
#endif