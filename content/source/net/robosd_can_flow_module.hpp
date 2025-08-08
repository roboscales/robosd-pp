#ifndef robosd_net_can_flow_module_hpp
#define robosd_net_can_flow_module_hpp
#include "core/robosd_app.hpp"
#include "core/robosd_log.hpp"
#include "net/robosd_can_flow_bus.hpp"
#include "net/robosd_can.hpp"
namespace robo {
	namespace net {
		namespace can {
			template <typename D> class ROBO_EXPORT flow_phys_t {
				robo::net::can_flow_bus::packet* incomm_ = nullptr;
				const robo::net::can_flow_bus::packet* outcomm_ = nullptr;
				robo::delegat::owned_fabric<void, ::robo::net::ican&, uint32_t, const uint8_t*, uint8_t   >::member<flow_phys_t> on_can_receive_;
				void on_can_receive__(::robo::net::ican& _ican, uint32_t _id, const uint8_t* _data, uint8_t _len) {
					do_can_receive(_ican, _id, _data, _len);
				}
			protected:
				virtual void confirm(void) = 0;
				virtual void refuse(void) = 0;
				virtual void do_can_receive(::robo::net::ican& _ican, uint32_t _id, const uint8_t* _data, uint8_t _len) {
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
				D /*robo::net::can::sysworxx::port*/ can_instance;
			public:
				virtual void send(const robo::net::can_flow_bus::packet* _outcomm) {
					if (can_instance.ready()) {
						if (can_instance.send(_outcomm->id.value, _outcomm->values, _outcomm->len)) {
							outcomm_ = _outcomm;
							//confirm();
						}
						else {
							refuse();
						}
					}
					else {
						refuse();
					}
				}
				void receive(robo::net::can_flow_bus::packet* _incomm) {
					incomm_ = _incomm;
				}
				void send_cancel(void) {
					outcomm_ = nullptr;
					can_instance.reset();
				}
				void receive_cancel(void) {
					incomm_ = nullptr;
					can_instance.reset();
				}
				bool panic(void) {
					return false;
				}
				robo::time_us_t wd_us(const robo::net::can_flow_bus::packet* _packet) {
					//todo!!
					robo::time_us_t tm = _packet->len * 100 + 200;
					if (tm < 2000) tm = 2000;
					return tm;
				}
				bool do_load(robo::cstr _current, robo::cstr _common) {
					ROBO_LBREAKN(can_instance.load( _current,  _common));
					//ROBO_LBREAKN(robo::ini::load(_common, _current, RT("CHANNEL"), can_instance.channel));
					//ROBO_LBREAKN(robo::ini::load(_common, _current, RT("REPEAT_MAX_COUNT"), can_instance.repeat_max_count));
					ROBO_LBREAKN(can_instance.open(_current));
					return true;
				}

				void do_clean(void) {
					can_instance.close();
				}

				void poll(void) {
					can_instance.poll();
				}
				bool ready(void) {
					return can_instance.ready();
				}
				flow_phys_t(void)
					: on_can_receive_(*this, &flow_phys_t::on_can_receive__)
					//, on_can_instanceevent_(*this, &phys::on_can_instanceevent__)
				{
					can_instance.set_on_receive(&on_can_receive_);
					//can_instance.set_on_event(&on_can_instanceevent_);
				}
			};
			template<typename P, robo::cstr N> class ROBO_EXPORT flow_module_t
				: public robo::app::module {
				flow_module_t(void)
					: robo::app::module(N) {}
				typedef robo::net::master_t<P, robo::net::can_flow_bus::packet> driver;
				robo::net::can_flow_bus** buses_ = nullptr;
				driver** drivers_ = nullptr;
				int bus_count_ = 0;

			protected:
				/*
				robo::net::can_instanceflow_bus::packet pk;
				robo::net::can_instanceflow_bus::packet res;
				*/
				virtual void backend_loop(void) {
					driver** d = drivers_;
					for (int i = 0; i < bus_count_; ++i, ++d) {
						(*d)->P::poll();
						(*d)->poll();
					}
				}
				virtual void frontend_loop(void) {
				}
				virtual bool do_load(void) {
					ROBO_LBREAKN(robo::app::module::do_load());
					ROBO_LBREAKN(robo::ini::load(current_path(), RT("BUS_COUNT"), bus_count_));
					if (bus_count_ > 0) {
						buses_ = new robo::net::can_flow_bus * [bus_count_];
						drivers_ = new driver * [bus_count_];
						robo::net::can_flow_bus** b = buses_;
						driver** d = drivers_;
						for (int i = 0; i < bus_count_; ++i, ++b, ++d) {
							(*b) = nullptr;
							(*d) = nullptr;
						}

						b = buses_;
						d = drivers_;
						for (int i = 0; i < bus_count_; ++i, ++b, ++d) {
							robo::string name(RT("channel-%d"), i + 1);
							(*d) = new driver(name, this);
							ROBO_LBREAKN((*d) != nullptr)
								name.format(RT("bus-%d"), i + 1);
							(*b) = new robo::net::can_flow_bus(name, this, **d);
							ROBO_LBREAKN((*b) != nullptr);
						}
					}
					return true;
				}
				virtual bool do_start(void) {
					driver** d = drivers_;
					for (int i = 0; i < bus_count_; ++i, ++d) {
						(*d)->start();
					}
					return true;
				}
				virtual void do_stop(void) {
					driver** d = drivers_;
					for (int i = 0; i < bus_count_; ++i, ++d) {
						(*d)->stop();
					}
				}
				virtual void do_clean(void) {

					if (buses_ != nullptr) {
						robo::net::can_flow_bus** b = buses_;
						for (int i = 0; i < bus_count_; ++i, ++b) {
							if ((*b) != nullptr) delete (*b);
						}
						delete[] buses_;
						buses_ = nullptr;
					}
					if (drivers_ != nullptr) {
						driver** d = drivers_;
						for (int i = 0; i < bus_count_; ++i, ++d) {
							if ((*d) != nullptr) delete (*d);
						}
						delete[] drivers_;
						drivers_ = nullptr;
					}
				}
			public:
				static flow_module_t& instance(void) {
					static flow_module_t instance_;
					return instance_;
				}
			};
		}
	}
}
#endif
