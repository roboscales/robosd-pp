#ifndef __robosd_net_master_hpp
#define __robosd_net_master_hpp
#include <stdint.h>

#include "core/robosd_delegat.hpp"
#include "core/robosd_system.hpp"
#include "net/robosd_net_trafic.hpp"

namespace robo{
	namespace net{        
		template< typename P > class imaster_t {
		public:
			typedef  ::robo::delegat::ref<void, bool> confirm_delegat;
			virtual void exchange(const P& _outcom_packet, P *  _incom_packet, confirm_delegat* _confirm) = 0;
			virtual void cancel(void) = 0;
			virtual bool ready(void) = 0;
		};
		#if ROBO_APP_MODULE_ENABLED ==1
		template< typename phys, typename P, class guard = ::robo::system::guard > class master_t: public phys, public imaster_t<P>, public app::node {
		#else
		template< typename phys, typename P, class guard= ::robo::system::guard> class master_t : public phys, public imaster_t<P> {
		#endif
		public:
			enum class result { refuse, success, panic };
			private:
			bool wd_enabled = false;
			unsigned int wd_begin_us_ = 0;
			unsigned int wd_delay_us_ = 0;
			
			enum class state { idle, send, receive, stopped, disable, panic};
			enum {incom_size_bits=5};
			
			state state_ = state::disable;
			
			void panic__(void){
				if( phys::panic() ){
					state_ = state::panic;
				} else {
					if(state_!=state::disable && state_ != state::panic ){
						state_ = state::idle;
					}
				}
				reset_();
			}
			void panic_(void){
				refuse();
				panic__();
			}
			
			const P * outcom_packet_ = nullptr;
			P * incom_packet_ = nullptr;
			void reset_(void){
				outcom_packet_ = nullptr;
				incom_packet_ = nullptr;
				wd_enabled = false;
			}
		
		public:
			
			void begin(void){
				guard g__;
				state_ = state::stopped;
			}
			
			void start(void){
				guard g__;
				state_ = state::idle;
			}
			
			void stop(void){
				guard g__;
				switch (state_){
				case state::send:
					phys::send_cancel();
				break;
				case state::receive:
					phys::receive_cancel();
				break;
				default:;
				}
				reset_();
				state_ = state::stopped;
			}
			
			virtual void cancel(void){
				guard g__;
				switch (state_){
				case state::send:
					phys::send_cancel();
				break;
				case state::receive:
					phys::receive_cancel();
				break;
				default:;
				}
				reset_();
				state_ = state::idle;
			}
			typename imaster_t<P>::confirm_delegat * confirm_ = nullptr;
				
			virtual void exchange(const P& _outcom_packet, P* _incom_packet, typename  imaster_t<P>::confirm_delegat * _confirm){
				confirm_ = _confirm;
				guard g__;
				outcom_packet_ = &_outcom_packet;
				incom_packet_ = _incom_packet;
				if (state_ == state::idle){
					state_ = state::send;
					wd_begin_us_ = system::time_us();
					wd_enabled = true;
					wd_delay_us_ = phys::wd_us(outcom_packet_);
					if(incom_packet_){
						wd_delay_us_ += phys::wd_us(incom_packet_);
					}
					phys::send(outcom_packet_);						
				} else {
					panic_();
				}
			}
			
			
			virtual void confirm(void){
				bool cf = false;;
				bool result = false;
				{
					guard g__;
					switch (state_){
					case state::send:
						if(incom_packet_ !=nullptr){
							wd_begin_us_ = system::time_us();
							state_ = state::receive;
							wd_delay_us_ = phys::wd_us(incom_packet_);
							phys::receive(incom_packet_);
							return;
						}
					case state::receive:
						reset_();
						state_ = state::idle;
						cf = true;
						result = true;
						break;
					default:;
						cf = true;
						result = false;
						panic_();					
					}
				}
				if(cf) if(confirm_) (*confirm_)(result);
			}

			void refuse(void){
				if(confirm_) (*confirm_)(false);
				guard g__;
				reset_();
				switch (state_){
				case state::send:
					phys::send_cancel();
					state_ = state::idle;
					break;
				case state::receive:
					phys::receive_cancel();
					state_ = state::idle;
					break;						
					default:
					break;						
				}
				//if (confirm_) (*confirm_)(false);
			}				
			
			void poll(void){
				if(wd_enabled){
					if( system::time_us() - wd_begin_us_ > wd_delay_us_ ){
						refuse();
					}
				}
			}
			
			virtual bool ready(void){
				guard g__;
				return (state_ == state::idle) && (phys::ready());
			}		
			#if ROBO_APP_MODULE_ENABLED ==1
			master_t(cstr _name, app::node* _owner) : app::node(_name, _owner) {
}
			virtual bool do_load(void) {
				ROBO_LBREAKN(app::node::do_load());
				ROBO_LBREAKN(phys::do_load(current_path(), defaults_path()));
				return true;
			}
			virtual void do_clean(void) {
				app::node::do_load();
				phys::do_clean();
			}

			virtual bool do_start(void) {
				ROBO_LBREAKN(app::node::do_start());
				ROBO_LBREAKN(phys::do_open());
				return true;
			}
			virtual void do_stop(void) {
				app::node::do_stop();
				phys::do_close();
			}

			#endif
		};
		

		
		template <typename D, typename E> class dispetcher_adapter_t{
			public:
				robo::time_us_t timeout_us = 0;
				robo::time_us_t last_request_us = 0;
			protected:
				virtual void dispetcher_confirm(void) = 0;
				virtual void dispetcher_refuse(const E & ) = 0;
				virtual bool dispetcher_ready(void) = 0;
				virtual bool dispetcher_request(void) = 0;
			public:
				statistic_s<E> statistic = {};
				void on_confirm(void){
					robo::system::critical g__;
					statistic.confirm++;
					dispetcher_confirm();
				}
				
				void on_refuse(const E & _err){
					robo::system::critical g__;
					statistic.refuse.detail[(int)_err]++;
					dispetcher_refuse(_err);
				}
				

				void	poll(void) {
					bool ready;
					{
						robo::system::critical g__;
						ready = dispetcher_ready();
					}
					auto now = robo::system::time_us();
					if(ready && ( now - last_request_us >= timeout_us) ){
						if( dispetcher_request() ){
							last_request_us = now;
							statistic.request++;
						}						
					}			
				}
					
				uint16_t crc(const uint8_t * _data, size_t _length){						
					//return ::robo::crc16_modbus_by_table(_data,_length);
					return D::crc(_data,_length);
				}
		};

	}
}
#endif