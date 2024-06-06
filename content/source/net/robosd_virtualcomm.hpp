#ifndef robosd_virtualcomm_hpp
#define robosd_virtualcomm_hpp
#include "net/robosd_serial.hpp"
#include "core/robosd_event.hpp"
#include <stdint.h>
//виртуальные порты временами ведут себя странно и непредсказуемо в силу самых различных причин
// данная реализация использует только базовые функции - блокирующее чтение и блокирующую запись
namespace robo {
	namespace net {
		namespace virtualcomm {
			typedef events::on_receive::performer on_receive_f;
			typedef events::on_panic::performer on_panic_f;

			namespace driver {
				enum class result { ok, terminate, panic };

				template <typename D> class ROBO_EXPORT receiver_t: public D {
				public:
					struct {
						on_receive_f * on_receive = nullptr;
						on_panic_f * on_panic = nullptr;
					} events;
					
					void receive_thread(void) {
						while (true) {
							switch (D::receive(*events.on_receive)) {
							case result::ok:
							break;
							case result::panic:
							if (events.on_panic) {
								(*(events.on_panic))();
							}
							return;
							break;
							default:
							return;
							};
						}
					}
				};
				#if 0
				template <typename D> class ROBO_EXPORT sender_t {
					iserial* source_ = nullptr;
				public:
					using sender = ::robo::event_t<void>::performer;
					using panicer = ::robo::event_t<void>::performer;

					void send_thread(sender& _sender, panicer& _panicer) {
						while (D::send_active()) {
							switch (D::send(*source_)) {
							case result::ok:
							source_ = nullptr;
							_sender();
							break;
							case result::panic:
							source_ = nullptr;
							_panicer();
							break;
							default:
							source_ = nullptr;
							return;
							}
						}
					}

					void try_send(iserial& _serial) {
						if (source_ == nullptr) {
							source_ = &_serial;
							D::send_activate();
						}

					}
				};

				template <typename D> class ROBO_EXPORT fullduplex_t: public receiver_t<D>, public sender_t<D> {
				};
				#endif
			}
			template <typename D, unsigned S, typename G  > class ROBO_EXPORT receiver_t
				: public ::robo::net::receiver_t<driver::receiver_t<D>, S, G> {
				using B = ::robo::net::receiver_t<driver::receiver_t<D>, S, G>;
			public:

				bool connect(robo::cstr  _port, uint32_t _boudrate) {
					return D::connect(_port, _boudrate);
				}

				void attach(on_receive_f * _receivcer, on_panic_f * _panicer) {
					B::events.on_receive = _receivcer;
					B::events.on_panic = _panicer;
				}

				
		
				receiver_t(void) {}

			};
			
			#if 0
			template <typename D, unsigned SA, unsigned SB, typename G  > class ROBO_EXPORT machine_t
				: public hardware_bridge_t<driver_t<D>, SA, SB, G> {
				using panicer = ::robo::event_t<void>::performer;
				using B = hardware_bridge_t<driver_t<D>, SA, SB, G>;
			private:
				panicer& panicer_;
			public:
				typedef ::robo::delegat::owned::fabric < panicer, void> panicer_fabric;

				bool connect(robo::cstr  _port, uint32_t _boudrate) {
					return D::connect(_port, _boudrate);
				}
				//sender& sender, receiver& _receiver
				machine_t(panicer& _panicer) : panicer_(_panicer) {}

				//void send_thread(sender& _sender, panicer& _panicer) {
					//D::send_thread(::robo::delegat::owned::fabric < ::robo::event_t< void, const uint8_t*, size_t>::performer, void>::member(*this, B::on_confirm), panicer_);
				//}

				void receive_thread(void) {
					D::receive_thread(::robo::delegat::owned::fabric < ::robo::event_t<void>::performer, void>::member(*this, B::on_confirm), panicer_);
				}
			};
			#endif
		}
	}
}
#endif