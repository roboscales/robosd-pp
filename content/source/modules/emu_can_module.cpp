#define MODULE_NAME  emu_can
#define MODULE_NAME_STR RT("emu_can")
#include "core/robosd_app.hpp"
#include "modules/can_flow_bus.hpp"
#include "net/platform/can/emulator/emu_can.hpp"
namespace MODULE_NAME{
	struct phys {
		robo::net::emu_can::port can;
		//robo::delegat::srmember<phys, void, ::robo::net::ican&, uint32_t, const uint8_t*, uint8_t   > on_can_receive_;
		//void on_can_receive__(::robo::net::ican& _ican, uint32_t _id, const uint8_t* _data, uint8_t _len) {

		//}

		void send(const robo::net::can_flow_bus::packet* _outcomm) {

		}
		void rceive(robo::net::can_flow_bus::packet* _incomm) {

		}
		void send_cancel(void) {

		}
		void receive_cancel(void) {}
		bool panic(void) { return false; }
		robo::time_us_t wd_us(const robo::net::can_flow_bus::packet* _incomm) {
			return 0;
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
		virtual void backend_loop(void) {}
		virtual void frontend_loop(void) {}
		virtual bool do_load(void) { 
			ROBO_LBREAKN(robo::ini::load(alias(), RT("BUS_COUNT"), bus_count_));
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
					(*d) = new driver;
					ROBO_LBREAKN((*d)!=nullptr)
					robo::string name(RT("%s_%d"),alias(),i+1);
					(*b) = new robo::net::can_flow_bus(name,this,**d);
					ROBO_LBREAKN((*b) != nullptr);
					ROBO_LBREAKN((*b)->load());
					ROBO_LBREAKN(robo::ini::load(name, RT("CHANNEL"), (*d)->can.channel));
					ROBO_LBREAKN(robo::ini::load(name, RT("REPEAT_MAX_COUNT"), (*d)->can.repeat_max_count));

					ROBO_LBREAKN((*d)->can.open());

					//(*d)->can_.set_on_receive(&on_can_receive_);
				}
			}
			return true; 
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
