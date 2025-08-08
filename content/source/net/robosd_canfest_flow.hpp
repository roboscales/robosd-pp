#ifndef robosd_net_canfest_flaw_hpp
#define robosd_net_canfest_flaw_hpp
#include "core/robosd_app.hpp"
#include "core/robosd_log.hpp"
#include "net/robosd_can_flow_bus.hpp"

namespace robo {
	namespace net {
		namespace canfest {
			template <typename D,typename S> class ROBO_EXPORT flow_phys_t : public ::robo::net::can::flow_phys_t<D>
			{
				using B = ::robo::net::can::flow_phys_t<D>;
				uint16_t pdo_number;
			protected:
				virtual void send(const robo::net::can_flow_bus::packet* _outcomm) {
					B::can_instance.send(_outcomm);
				}
				virtual void do_can_receive(::robo::net::ican& _ican, uint32_t _id, const uint8_t* _data, uint8_t _len) {
					::robo::net::can::flow_phys_t<D>::do_can_receive(_ican,_id,_data,_len);
					//if()
				}
				bool load(robo::cstr _current, robo::cstr _common) {
					ROBO_LBREAKN(B::load(_current, _common));
					ROBO_LBREAKN(robo::ini::load(_common, _current, RT("pdo_number"), pdo_number));
					return true;
				}
			};
		}
	}
}
#endif
