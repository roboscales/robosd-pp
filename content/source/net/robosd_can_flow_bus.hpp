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
				class bus : public backend::bus {
				public:
					enum { master_buf_size = 10, idle_id = 0xFF, packet_size = 8 };
					struct packet : public stack_t<uint8_t, uint8_t, packet_size> {
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
				};

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
			}
		}
	}
}
#endif