#ifndef can_flow_bus_hpp
#define can_flow_bus_hpp
#include "core/robosd_backend.hpp"
#include "core/robosd_ring_buf.hpp"
#include "net/robosd_net_master.hpp"
#include "net/robosd_flow_id.h"
namespace robo {
	namespace net {
		class can_flow_bus : public robo::backend::bus {
		public:
			enum { master_buf_size = 10, idle_id = 0xFF, packet_size = 8 };
			struct packet: public stack_t<uint8_t,uint8_t, packet_size> {
				flow_msg_can_id_t id;
			};
			typedef  robo::net::imaster_t<packet> driver;
		private:
			packet in_packet_;
			packet out_packet_;
			driver& driver_;
			msg message_;
			uint16_t wait_id_ = idle_id;
			::robo::delegat::smember<can_flow_bus, void, bool > confirm_delegat_;
		protected:
			virtual bool do_load(void);


			void exchange_confirm(bool _result);

			virtual void reset(void);

			virtual bool post(void);

			virtual void cancel(void);

			virtual bool ready(void);
		public:
			can_flow_bus(robo::cstr _name, robo::app::module* _owner, driver& _driver);
		};
	}
}
#endif