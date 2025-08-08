#define MODULE_NAME  emu_can
#define MODULE_NAME_STR RT("emu_can")
#include "core/robosd_app.hpp"
#include "core/robosd_log.hpp"
#include "net/robosd_can_flow_bus.hpp"
#include "net/platform/can/emulator/emu_can.hpp"
#include "net/robosd_can_flow_module.hpp"
namespace MODULE_NAME{
#if 0
	class phys {
		robo::net::can_flow_bus::packet* incomm_= nullptr;
		//const robo::net::can_flow_bus::packet* outcomm_ = nullptr;
		robo::delegat::owned_fabric<void, ::robo::net::ican&, uint32_t, const uint8_t*, uint8_t>::member<phys> on_can_receive_;
		robo::delegat::owned_fabric<void, ::robo::net::ican&, ::robo::net::ican::event  >::member<phys> on_can_event_;
		virtual void confirm(void) = 0;
		virtual void refuse(void) = 0;		
		void on_can_receive__(::robo::net::ican& _ican, uint32_t _id, const uint8_t* _data, uint8_t _len) {			
			if (incomm_ != nullptr) {
				incomm_->id.value = _id;
				incomm_->len = _len;
				std::copy_n(_data, _len, incomm_->values);
				//robo_detaillog(4, ::robo::log::mask::disabled, ">>0x%2x [%d]", (int)incomm_->id.value, (int)incomm_->values[0]);
				incomm_ = nullptr;
				confirm();
			}			
		}
		void on_can_event__(::robo::net::ican& _ican, ::robo::net::ican::event _event) {
			if (/*(outcomm_ != nullptr) || */(incomm_ != nullptr)) {
			//	outcomm_ = nullptr;
				incomm_ = nullptr;
				refuse();				
			}
		}
		robo::net::emu_can::port can_;
	protected:
		bool ready(void) { return can_.ready(); }
	public:
		void send(const robo::net::can_flow_bus::packet* _outcomm) {
			if (can_.ready()) {
				//outcomm_ = _outcomm;
				if (can_.send(_outcomm->id.value, _outcomm->values, _outcomm->len)) {
					confirm();
				}
				else {
					refuse();
				}
				//robo_detaillog(3, ::robo::log::mask::disabled, "<<0x%2x [%d]", (int)_outcomm->id.value, (int)_outcomm->values[0]);
			}
			else {
				refuse();
			}
		}
		void receive(robo::net::can_flow_bus::packet* _incomm) {
			incomm_ = _incomm;
		}
		void send_cancel(void) {
			//outcomm_ = nullptr;
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
			return 8 * 1000+200;
		}
		bool do_load( robo::cstr _common, robo::cstr _current) {
			ROBO_LBREAKN(robo::ini::load(_common, _current, RT("CHANNEL"), can_.channel));
			ROBO_LBREAKN(robo::ini::load(_common, _current, RT("REPEAT_MAX_COUNT"), can_.repeat_max_count));
			ROBO_LBREAKN(can_.open());
			return true;
		}
		void poll(void) {
			can_.poll();
		}
		void do_clean(void) {
			can_.close();
		}
		phys(void) 
			: on_can_receive_(*this, &phys::on_can_receive__) 
			, on_can_event_(*this, &phys::on_can_event__)
		{
			can_.set_on_receive(&on_can_receive_);
			can_.set_on_event(&on_can_event_);
		}
	};
#endif
	class phys: public robo::net::emu_can::port {
	public:
		using B = robo::net::emu_can::port;
		bool load(robo::cstr _current, robo::cstr _common) {
			ROBO_LBREAKN(robo::ini::load(_common, _current, RT("CHANNEL"), channel));
			ROBO_LBREAKN(robo::ini::load(_common, _current, RT("REPEAT_MAX_COUNT"), repeat_max_count));
			return true;
		}
	};
	static const inline robo::char_t nm[] = MODULE_NAME_STR;
	using module = robo::net::can::flow_module_t< robo::net::can::flow_phys_t<phys >, nm>;
}

#include "core/robosd_system_module_reg.hpp"
