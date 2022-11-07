#define MODULE_NAME  pi_spi_can
#define MODULE_NAME_STR RT("pi-spi-can")
#include "core/robosd_app.hpp"
#include "core/robosd_log.hpp"
#include "net/robosd_can_flow_bus.hpp"
#include "net/robosd_can.hpp"
#include "core/robosd_crc.hpp"
#include "core/robosd_ini.hpp"
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include "net/platform/can/spi/spi2can_common.hpp"
namespace MODULE_NAME {
	class phys {
		class port : public robo::net::ican {
			int handle_;
			int channel_;
			int cs_;
			int bitrate_;
		public:
			port(void){				
			}			
			virtual ~port(void)
			{
			}
			bool do_load(robo::cstr  _sect, robo::cstr  _cur)
			{
				ROBO_LBREAKN(robo::ini::load(_sect, _cur, RT("cs"), cs_));
				ROBO_LBREAKN(robo::ini::load(_sect, _cur, RT("bitrate"), bitrate_));
				ROBO_LBREAKN(robo::ini::load(_sect, _cur, RT("channel"), channel_));
				handle_ = wiringPiSPISetupMode(channel_, bitrate_, 0);
				pinMode(cs_, OUTPUT); 
				return true;
			}
			virtual void close(void)
			{
			}
			virtual bool send(uint32_t _id, const uint8_t* _buf, uint8_t  _len)
			{
				digitalWrite(cs_, LOW);			
				digitalWrite(cs_, LOW);
				spi2can::packet packet;
				packet.id = _id;
				packet.len = _len;
				std::copy_n(_buf, _len, packet.data);
				if (_len < spi2can::packet::data_sz)
					std::fill_n(packet.data + _len, spi2can::packet::data_sz - _len, 0);
				packet.crc = robo::crc7_by_table(packet.memo, spi2can::packet::total_sz - 1);												
				wiringPiSPIDataRW(channel_, const_cast<uint8_t *>(_buf), _len);				
				uint8_t crc = robo::crc7_by_table(packet.memo, spi2can::packet::total_sz - 1);
				if (packet.crc != 92) {	
					static volatile int x = 0;
					x++;
				}
				if (packet.crc == crc) {
					if (on_receive)
						(*(on_receive))(*this, packet.id, packet.memo, packet.len);		
				}
				else {
					static int err = 0;
					err++;
				}
				
				digitalWrite(cs_, HIGH);			
				return true;
			}
			virtual bool ready(void) {}
			virtual void reset(void) {}
			virtual void poll(void) {}
		};

		
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
		port can_;
	public:
		bool do_load(robo::cstr  _sect, robo::cstr  _cur) {
			return can_.do_load(_sect, _cur);
		}
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
			wiringPiSetup();
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
