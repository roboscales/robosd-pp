#define MODULE_NAME  vscom_can
#define MODULE_NAME_STR RT("vscom_can")
#include "core/robosd_app.hpp"
#include "core/robosd_log.hpp"
#include "net/robosd_can_flow_bus.hpp"
#include "net/platform/can/vscom/robosd_vscom_can.hpp"
namespace MODULE_NAME{
	class phys {
		robo::net::can_flow_bus::packet* incomm_= nullptr;
		robo::delegat::srmember<phys, void, ::robo::net::ican&, uint32_t, const uint8_t*, uint8_t   > on_can_receive_;
		virtual void confirm(void) = 0;
		virtual void refuse(void) = 0;
		void on_can_receive__(::robo::net::ican& _ican, uint32_t _id, const uint8_t* _data, uint8_t _len) {			
			if (incomm_ != nullptr) {
				incomm_->id.value = _id;
				incomm_->len = _len;
				std::copy_n(_data, _len, incomm_->values);
				incomm_ = nullptr;
				confirm();
			}			
		}
		robo::net::vscom_can::port can_;
	public:
		void send(const robo::net::can_flow_bus::packet* _outcomm) {
			if (can_.ready()) {
				if (can_.send(_outcomm->id.value, _outcomm->values, _outcomm->len)) {
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
		void receive(robo::net::can_flow_bus::packet* _incomm) {
			incomm_ = _incomm;
		}
		void send_cancel(void) {
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
			return _packet->len * 1000+200;
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
			/*
			static robo::time_us_t last_us = 0;
			robo::time_us_t now_us = robo::system::env::time_us();
			static bool odd = false;
			static uint8_t counter = 0;
			if (now_us - last_us > 1000000) {
				last_us = now_us;
				if (odd) {
					pk.id.value = 0x0A1;
					pk.len = 1;
					pk.values[0] = 1;
					drivers_[0]->exchange(pk, &res, nullptr);
					//drivers_[1]->exchange(pk, &res, nullptr);
				}
				else {
					pk.id.value = 0x2A1;
					pk.len = 1;
					pk.values[0] = counter++;
					drivers_[0]->exchange(pk, nullptr, nullptr);
					//pk.values[0]++;
					//drivers_[1]->exchange(pk, nullptr, nullptr);
				}
				odd = !odd;
			}*/
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
}

#include "core/robosd_system_module_reg.hpp"
