#include "net/robosd_can_flow_bus.hpp"
namespace robo {
	namespace net {
		namespace can {
			namespace flow {
				bool bus::do_load(void) {
					ROBO_LBREAKN(::robo::backend::bus::do_load());
					//message_.tran.size_max =  get_packet_size();
					return true;
				}
				void bus::exchange_confirm(bool _result) {
					if (wait_id_ != idle_id) {
						if (_result) {
							switch (message_.tran.request) {
							case ROBO_TRAN_EXCANGE:
								if (wait_id_ == in_packet_.id.value) {
									message_.tran.size_actual = in_packet_.len;
									message_.tran.data = in_packet_.values;
									bus::confirm(ROBO_TRAN_COMPLETE);
								}
								else {
									bus::confirm(ROBO_TRAN_REFUSE);
								}
								break;
							case ROBO_TRAN_REQUEST_PUT:
								bus::confirm(ROBO_TRAN_COMPLETE);
								break;
							case ROBO_TRAN_REQUEST_GET:
								if ((wait_id_ == in_packet_.id.value) && (message_.tran.size_actual == in_packet_.len)) {
									message_.tran.data = in_packet_.values;
									bus::confirm(ROBO_TRAN_COMPLETE);
								}
								else {
									bus::confirm(ROBO_TRAN_REFUSE);
									robo_errlog("bus %s refuse msg 0x%x", alias(), (int)in_packet_.id.value);
								}
								break;
							}

						}
						else {
							bus::confirm(ROBO_TRAN_REFUSE);
						}
						wait_id_ = idle_id;
					}
				}

				void bus::reset(void) {
					message_.tran.data = out_packet_.values;
				}

				void bus::post(void) {
					uint16_t id = ((message_.address & 0xF) << 4) + ((message_.suba) & 0xF);
					wait_id_ = 0x400 + id;
					message_.tran.status = ROBO_TRAN_EXECUTE_PHY;
					switch (message_.tran.request) {
					case ROBO_TRAN_EXCANGE:
						out_packet_.id.value = 0x200 + id;
						out_packet_.len = (uint8_t)message_.tran.size_actual;
						driver_.exchange(out_packet_, &in_packet_, &confirm_delegat_);
						break;
					case ROBO_TRAN_REQUEST_PUT:
						out_packet_.id.value = 0x200 + id;
						out_packet_.len = (uint8_t)message_.tran.size_actual;
						in_packet_.len = 0;
						driver_.exchange(out_packet_, nullptr, &confirm_delegat_);
						break;
					case ROBO_TRAN_REQUEST_GET:
						out_packet_.id.value = 0x00 + id;
						out_packet_.len = 1;
						out_packet_.values[0] = (uint8_t)message_.tran.size_actual;
						driver_.exchange(out_packet_, &in_packet_, &confirm_delegat_);
						break;
					case ROBO_TRAN_REBOOT_ME:
					default:
						robo_errlog("bus %s refuse request 0x%x", alias(), message_.tran.request);
						bus::confirm(ROBO_TRAN_REFUSE);
					}
				}

				void bus::cancel(void) {
					driver_.cancel();
					if (wait_id_ != idle_id) {
						wait_id_ = idle_id;
						bus::confirm(ROBO_TRAN_REFUSE);
					}
				}

				bool bus::ready(void) {
					return (wait_id_ == idle_id) && driver_.ready();
				}

				bus::bus(cstr _name, app::module* _owner, driver& _driver)
					: robo::backend::bus(_name, _owner)
					, confirm_delegat_(*this, &bus::exchange_confirm)
					, driver_(_driver) {
					message_.tran.size_max = max_packet_size;
				}
				void bus::set_max_packets_size(uint8_t _sz) {
					ROBO_APP_ASSERT(_sz<= max_packet_size)
					message_.tran.size_max = _sz;					
				}

				void xphys::on_can_receive__(::robo::net::ican& _ican, uint32_t _id, const uint8_t* _data, uint8_t _len) {
					do_can_receive(_ican, _id, _data, _len);
				}
				void xphys::do_can_receive(ican& _ican, uint32_t _id, const uint8_t* _data, uint8_t _len) {
					//robo_infolog("\t M >> S %x [%x] %x %x %x %x", _id, _len, _data[0], _data[1], _data[2], _data[3]);
					if (outcomm_ && (outcomm_->id.value == _id)) {
						outcomm_ = nullptr;
						confirm();
					}
					else {
						if (incomm_ != nullptr) {
							incomm_->id.value = _id;
							if (!(incomm_->id.user)) {
								incomm_->len = _len;
								std::copy_n(_data, _len, incomm_->values);
								incomm_ = nullptr;
								confirm();
							}
						}
					}
				}
				bool xphys::do_send(ican& _ican, uint32_t _id, const uint8_t* _data, uint8_t _len) {
					//robo_infolog("M << S %x [%x] %x %x %x %x", _id,_len, _data[0], _data[1], _data[2], _data[3]);
					ROBO_LRET(_ican.send(_id, _data, _len));
				}
				void xphys::send(const bus::packet* _outcomm) {
					if (can_->ready()) {
						if (do_send(*can_, _outcomm->id.value, _outcomm->values, _outcomm->len)) {
							outcomm_ = _outcomm;
							confirm();
						}
						else {
							refuse();
						}
					}
					else {
						refuse();
					}
				}
				void xphys::receive(bus::packet* _incomm) {
					incomm_ = _incomm;
				}
				void xphys::send_cancel(void) {
					outcomm_ = nullptr;
					can_->reset();
				}
				void xphys::receive_cancel(void) {
					incomm_ = nullptr;
					can_->reset();
				}
				bool xphys::panic(void) {
					return false;
				}
				time_us_t xphys::wd_us(const bus::packet* _packet) {
					
					//todo!!
					//robo::time_us_t tm = can_->wd_us(_packet->len);
					//robo::time_us_t tm = _packet->len * 100 + 200;
					//if (tm < 2000) tm = 2000;
					return default_timeout_us;
				}
				/*
				решил избаить интерфейс ican от функций, которые должны нести на себе объекты- оболочки или модули  отвечающие за 
				загрузку параметров одного или нескольких реализаций  ican 
				*/
				#if ROBO_APP_MODULE_ENABLED ==1
				bool xphys::do_load(cstr _current, cstr _common) {
					ROBO_LBREAKN(can_name.load(_current, _common, RT("name")));
					ini::try_load( _current, _common, RT("default_timeout_us"), default_timeout_us);
					//ROBO_LBREAKN(can_->load());
					return true;
				}

				void xphys::do_clean(void) {
					can_name.clear();
				}

				bool  xphys::do_start(void) {

					can_ = ican::query(can_name);
					ROBO_LBREAKN(can_ != nullptr);
					can_->set_on_receive(&on_can_receive_);
					return true;
				}
				void xphys::do_stop(void) {
					if (can_) {
						can_->set_on_receive((ican::on_receive_f*)nullptr);
					}
				}
				#endif

				void xphys::poll(void) {
					can_->poll();
				}
				bool xphys::ready(void) {
					return can_->ready();
				}
				xphys::xphys(void)
				: on_can_receive_(*this, &xphys::on_can_receive__)
					//, on_can_instanceevent_(*this, &phys::on_can_instanceevent__)
					{
						//can_instance.set_on_event(&on_can_instanceevent_);
				}

#if 0
				bool xphys::send(uint32_t _id, const uint8_t* _data, uint8_t _len) {
					ROBO_LRET(can_instance->send(_id, _data, _len))
				}

				bool xphys::load(cstr _section, cstr _common) {
					ROBO_LRET(name.load(_section, _common, RT("name")))
				}

				bool xphys::open(void) {
					guard__;
					can_instance = ican::query(name);
					ROBO_LRET(can_instance != nullptr);
				}

				void xphys::close(void) {
					guard__;
					if (can_instance) {
						can_instance->close();
						can_instance->release();
						can_instance = nullptr;
					}
				}
				void xphys::clean(void) {
				}

				void xphys::reset(void) {
					if (can_instance) {
						can_instance->reset();
					}
				}
				void xphys::set_on_receive(ican::on_receive_f* _on_receive) {
					if (can_instance) {
						can_instance->set_on_receive(_on_receive);
					}
				}
				void xphys::poll(void) {
					if (can_instance) {
						can_instance->poll();
					}
				}

				bool xphys::ready(void) {
					if (can_instance) {
						return can_instance->ready();
					}
					else return false;
				}
#endif
			}
		}
	}
}
