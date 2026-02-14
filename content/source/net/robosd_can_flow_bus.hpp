#ifndef robosd_can_flow_bus_hpp
#define robosd_can_flow_bus_hpp
#include "servo/robosd_backend.hpp"
#include "core/robosd_ring_buf.hpp"
#include "net/robosd_net_master.hpp"
#include "net/robosd_flow_id.h"
#include "net/robosd_can.hpp"
namespace robo {
	namespace net {
		namespace can {
			namespace flow {
				class ROBO_EXPORT bus : public backend::bus {
				public:
					enum { master_buf_size = 10, idle_id = 0xFF, max_packet_size = 8 };
					struct packet : public stack_t<uint8_t, uint8_t, max_packet_size> {
						flow_msg_can_id_t id;
					};
					typedef  imaster_t<packet> driver;
				private:
					packet in_packet_;
					packet out_packet_;
					driver& driver_;
					uint16_t wait_id_ = idle_id;
					::robo::delegat::owned_fabric<void, bool>::member<bus > confirm_delegat_;
				protected:
					
					virtual bool do_load(void);


					void exchange_confirm(bool _result);

					virtual void reset(void);

					virtual void post(void);

					virtual void cancel(void);

					virtual bool ready(void);
				public:
					bus(cstr _name, app::module* _owner, driver& _driver);
					void set_max_packets_size(uint8_t _sz);
				};

				class ROBO_EXPORT xphys {
					robo::string can_name;
					bus::packet* incomm_ = nullptr;
					const bus::packet* outcomm_ = nullptr;
					robo::delegat::owned_fabric<void, ::robo::net::ican&, uint32_t, const uint8_t*, uint8_t   >::member<xphys> on_can_receive_;
					void on_can_receive__(::robo::net::ican& _ican, uint32_t _id, const uint8_t* _data, uint8_t _len);
				protected:
					virtual void confirm(void) = 0;
					virtual void refuse(void) = 0;
					virtual void do_can_receive(ican& _ican, uint32_t _id, const uint8_t* _data, uint8_t _len);
					virtual bool do_send(ican& _ican, uint32_t _id, const uint8_t* _data, uint8_t _len);
					ican* can_ = nullptr;
				public:
					//message_.tran.size_max =
					virtual uint8_t get_packet_max_size(void) { return 8; };
					virtual void send(const bus::packet* _outcomm);
					void receive(bus::packet* _incomm);
					void send_cancel(void);
					void receive_cancel(void);
					bool panic(void);
					time_us_t wd_us(const bus::packet* _packet);

					#if ROBO_APP_MODULE_ENABLED ==1
					virtual bool do_load(cstr _current, cstr _common);
					virtual void do_clean(void);

					virtual bool do_start(void);
					virtual void do_stop(void);
					#endif

					void poll(void);
					bool ready(void);
					xphys(void);
				};


#if 0
				class ROBO_EXPORT xphys : public ican {
				protected:
					ican* can_instance = nullptr;
				public:
					string name;					
					bool load(cstr _section, cstr _common);
					void clean(void);
					bool open(void);

					virtual void close(void);
					virtual bool send(uint32_t _id, const uint8_t* _data, uint8_t _len);
					virtual bool ready(void);
					virtual void reset(void);
					virtual void poll(void);

					virtual void set_on_receive(ican::on_receive_f* _on_receive);

				};
#endif
			}
		}
	}
}
#endif