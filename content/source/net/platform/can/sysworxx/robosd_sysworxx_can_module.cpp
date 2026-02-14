#define MODULE_NAME  sysworxx_can
#define MODULE_NAME_STR RT("sysworxx_can")
#include "core/robosd_app.hpp"
#include "core/robosd_log.hpp"
#include "net/robosd_can_flow_bus.hpp"
#include "net/platform/can/sysworxx/robosd_sysworxx_can.hpp"
namespace MODULE_NAME{
#if 0
	class phys {
		robo::net::can_flow_bus::packet* incomm_= nullptr;
		const robo::net::can_flow_bus::packet* outcomm_ = nullptr;
		robo::delegat::owned_fabric<void, ::robo::net::ican&, uint32_t, const uint8_t*, uint8_t   >::member<phys> on_can_receive_;
		virtual void confirm(void) = 0;
		virtual void refuse(void) = 0;
		void on_can_receive__(::robo::net::ican& _ican, uint32_t _id, const uint8_t* _data, uint8_t _len) {	
			if (outcomm_ && (outcomm_->id.value == _id) ) {
				outcomm_ = nullptr;
				confirm();
			}
			else {
				if (incomm_ != nullptr) {					
					incomm_->id.value = _id;
					if (!( incomm_->id.user )) {
						incomm_->len = _len;
						std::copy_n(_data, _len, incomm_->values);
						incomm_ = nullptr;
						confirm();
					}
				}
			}
		}
		robo::net::can::sysworxx::port can_;
	public:
		void send(const robo::net::can_flow_bus::packet* _outcomm) {
			if (can_.ready()) {
				if (can_.send(_outcomm->id.value, _outcomm->values, _outcomm->len)) {
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
			can_.reset();
		}
		void receive_cancel(void) {
			incomm_ = nullptr;
			can_.reset();
		}
		bool panic(void) {  
			return false; 
		}
		robo::time_us_t wd_us(const robo::net::can_flow_bus::packet* _packet) {
			//todo!!
			robo::time_us_t tm = _packet->len * 100 + 200;
			if (tm < 2000) tm = 2000;
			return tm ;
		}
		bool do_load( robo::cstr _current, robo::cstr _common) {
			//ROBO_LBREAKN(robo::ini::load(_common, _current, RT("CHANNEL"), can_.channel));
			//ROBO_LBREAKN(robo::ini::load(_common, _current, RT("REPEAT_MAX_COUNT"), can_.repeat_max_count));
			ROBO_LBREAKN(can_.open(_current));
			return true;
		}

		void do_clean(void) {
			can_.close();
		}

		void poll(void) {
			can_.poll();
		}
		bool ready(void) {
			return can_.ready();
		}
		phys(void) 
			: on_can_receive_(*this, &phys::on_can_receive__) 
			//, on_can_event_(*this, &phys::on_can_event__)
		{
			can_.set_on_receive(&on_can_receive_);
			//can_.set_on_event(&on_can_event_);
		}
	};
	class module : public robo::app::module {
		module(void)
			: robo::app::module(MODULE_NAME_STR) {}
		typedef robo::net::master_t<phys, robo::net::can_flow_bus::packet> driver;
		robo::net:: can_flow_bus** buses_ = nullptr;
		driver** drivers_ = nullptr;
		int bus_count_ = 0;

	protected:
		virtual void backend_loop(void) {
			driver** d = drivers_;
			for (int i = 0; i < bus_count_; ++i, ++d) {
				(*d)->phys::poll();
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
					ROBO_LBREAKN((*d)!=nullptr)
					name.format(RT("bus-%d"), i+1);
					(*b) = new robo::net::can_flow_bus(name,this,**d);
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
				for (int i = 0; i < bus_count_; ++i,++b) {
					if ((*b)!=nullptr) delete (*b);
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
		static module& instance(void) {
			static module instance_;
			return instance_;
		}
	};
#endif
	
	class phys : public ::robo::app::node{
	public:
		phys(robo::cstr _name, node * _owner) : node(_name, _owner) {}
		using B = ::robo::net::can::sysworxx::port;
		B instance;
		virtual bool do_load(void) {
			ROBO_LBREAKN(instance.open(current_path()));
			ROBO_LBREAKN(instance.reg(current_path()));
			return true;
		}
		virtual bool do_start(void) {
			return true;
		}
		virtual void poll(void) {
			instance.poll();
		}
		virtual void do_clean(void) {
			instance.close();
			instance.unreg();
		}
	};
	static const inline robo::char_t nm[] = MODULE_NAME_STR;
	using module = robo::net::cans_module_t< phys, nm>;
}

#include "core/robosd_system_module_reg.hpp"
