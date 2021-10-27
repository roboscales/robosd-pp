#include "net/robosd_proto_switch.hpp"
#include <algorithm>
#if ROBO_APP_PROTO_SWITCH_ENABLED == 1
namespace robo{
	namespace net{
    namespace proto{
			namespace switcher{
				key::key(void): len(0){
					std::fill_n(value, ROBO_PROTO_SWITCH_MARKER_MAX_LEN,0);
				}

				bool key::operator==(const marker & _marker){
					if (len == _marker.len){
						return std::equal(value ,value+len, _marker.value);
					}
					else{
						return false;
					}
				}

				void key::buffer::operator = (const key & _key){
					len = _key.len;
					if (len){
						std::copy_n(_key.value, len, value); 
					}
					memo = value;
				}

				port::port(
					type _type
					, iserial & _serial
					, time_us_t _key_reset_us
				)
				: ref_(*this)
				, type_(_type)
				, serial_(_serial)
				, abonent_(nullptr)
				{
					timeouts_.key_reset = _key_reset_us;
					timeouts_.tick = 0;
					ref_.attach_to(core::instance_().ports_.ready);
				}
				port::~port(void){
				}

				void port::disconnect_(void){
					key_.len = 0;
					abonent_ = nullptr;
					ref_.attach_to(core::instance_().ports_.ready);
				}

				void port::poll_(time_us_t _period){

					if (timeouts_.tick > timeouts_.key_reset){
						key_.len = 0;
						timeouts_.tick = 0;
					}
					else{
						timeouts_.tick += _period;
					}

					if (serial_.available()){
						key_.value[key_.len++] = serial_.get();
						if ((!core::instance_().recognize_(this)) && (key_.len == ROBO_PROTO_SWITCH_MARKER_MAX_LEN)){
							serial_.reset();
							key_.len = 0;
						}
					}
				}

				void abonent::disconnect_(void){
					if (port_ != nullptr){
						port_->disconnect_();
						port_ = nullptr;
					}
					stop();
				}
				void abonent::disconnect(void){
					terminate_ = true;
				}
				abonent::abonent(
					const uint8_t * _marker
					, size_t _markerlen
					, time_us_t _lock_us
					, time_us_t _silence_us
				)
				: ref_(*this)
				, marker_(_marker, _markerlen)
				, terminate_(false)
				{
					timeouts_.lock = _lock_us;
					timeouts_.silence = _silence_us;
					timeouts_.tick = 0;
					ref_.attach_to(core::instance_().abonents_);
				}
				abonent::~abonent(void){

				}
				iserial * abonent::serial(void){
					if (port_){
						return &(port_->serial_);
					}
					else {
						return 0;
					}
				}
				void abonent::alive(void){
					timeouts_.tick = 0;
				}
				size_t abonent::available(void){
					if (key_.len > 0) return key_.len;
					if (port_)
						return port_->serial_.available();
					else
						return 0;
				}
				size_t abonent::get(uint8_t* _data, size_t _max_size){
					size_t sz = key_.len;
					if (sz > 0){
						if (sz < _max_size) sz = _max_size;
						std::copy_n(key_.memo, sz, _data); 
						key_.memo += sz;
						key_.len -= sz;
						return sz;						
					}
					else {
						if (port_){
							return port_->serial_.get(_data, _max_size);
						}
						else{
							return 0;
						}
					}
				}
				uint8_t abonent::get(void){
					if (key_.len > 0){
						key_.len--;
						return *(key_.memo)++;
					}
					else {
						if (port_)
							return port_->serial_.get();
						else
							return 0;
					}
				}
				void abonent::reset(void){
					key_.len = 0;
					if (port_)
						port_->serial_.reset();
				}
				size_t abonent::space(void){
					if (port_){
						return port_->serial_.space();
					} else {
						return 0;
					}
				}
				bool  abonent::put(const uint8_t* _data, size_t _size) { 
					alive();
					return port_->serial_.put(_data, _size); 
				};
				bool  abonent::put(uint8_t _ch) { 
					alive();
					return port_->serial_.put(_ch);
				};

				void abonent::poll_(time_us_t _period){
					if (port_){
						timeouts_.tick += _period;
						if ( (timeouts_.tick > timeouts_.silence) || terminate_){
							disconnect_();
						}
					}
				}
				bool core::recognize_(port * _port){
					_port->timeouts_.tick = 0;
					for (abonent::ref * ref = abonents_.first(); ref; ref = ref->next()){
						abonent & a = ref->owner();
						if ( _port->key_ == a.marker_ ){
							if (a.port_ != nullptr) {
								if (a.timeouts_.lock > a.timeouts_.tick){
									break;
								}
								else {
									a.disconnect_();
								}
							}
							_port->ref_.attach_to( core::instance_().ports_.busy);
							_port->abonent_ = &a;
							a.port_ = _port;
							a.timeouts_.tick = 0;
							a.key_ = _port->key_;
							a.start();
							return true;
						}
					}
					return false;
				}
				core::core(void){

				}
				core::~core(void){

				}

				void core::poll_(time_us_t _period){
					for ( abonent::ref * r = abonents_.first(); r; r = r->next() ){
						r->owner().poll_(_period);
					}
					for (port::ref * r = ports_.ready.first(); r; r = r->next()){
						r->owner().poll_(_period);
					}
				}
				core & core::instance_(void){
						static core instance__;
						return instance__;
				}
			}
		}
	}
}
#endif
