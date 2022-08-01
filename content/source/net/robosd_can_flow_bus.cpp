#include "net/robosd_can_flow_bus.hpp"
namespace robo {
	namespace net {
		bool can_flow_bus::do_load(void) {
			ROBO_LBREAKN(::robo::backend::bus::do_load());
			return true;
		}
		void can_flow_bus::exchange_confirm(bool _result) {
			if (wait_id_ != idle_id) {
				if (_result ) {
					switch (message_.tran.request) {
					case ROBO_TRAN_EXCANGE:
						if (wait_id_ == in_packet_.id.value) {
							message_.tran.size_actual = in_packet_.len;
							message_.tran.data = in_packet_.values;
							bus::confirm(ROBO_TRAN_COMPLETE);
						}
						else {
							bus::confirm(ROBO_TRAN_REFUSE);
						}
						break;
					case ROBO_TRAN_REQUEST_PUT:
						bus::confirm(ROBO_TRAN_COMPLETE);
						break;
					case ROBO_TRAN_REQUEST_GET:
						if ( (wait_id_ == in_packet_.id.value) && (message_.tran.size_actual == in_packet_.len) ) {
							message_.tran.data = in_packet_.values;
							bus::confirm(ROBO_TRAN_COMPLETE);
						}
						else {
							bus::confirm(ROBO_TRAN_REFUSE);
							robo_errlog("bus %s refuse msg 0x%x", alias(), (int)in_packet_.id.value);
						}
						break;
					}

				}
				else {
					bus::confirm(ROBO_TRAN_REFUSE);
				}
				wait_id_ = idle_id;
			}
		}

		void can_flow_bus::reset(void) {
			message_.tran.data = out_packet_.values;
		}

		void can_flow_bus::post(void) {
			uint16_t id = ((message_.address & 0xF) << 4) + ((message_.suba) & 0xF);
			wait_id_ = 0x400 + id;
			message_.tran.status = ROBO_TRAN_EXECUTE_PHY;
			switch (message_.tran.request) {
			case ROBO_TRAN_EXCANGE:
				out_packet_.id.value = 0x200 + id;
				out_packet_.len = (uint8_t)message_.tran.size_actual;
				driver_.exchange(out_packet_, &in_packet_, &confirm_delegat_);
				break;
			case ROBO_TRAN_REQUEST_PUT:
				out_packet_.id.value = 0x200 + id;
				out_packet_.len = (uint8_t)message_.tran.size_actual;
				in_packet_.len = 0;
				driver_.exchange(out_packet_, nullptr, &confirm_delegat_);
				break;
			case ROBO_TRAN_REQUEST_GET:
				out_packet_.id.value = 0x00 + id;
				out_packet_.len = 1;
				out_packet_.values[0] = (uint8_t)message_.tran.size_actual;
				driver_.exchange(out_packet_, &in_packet_, &confirm_delegat_);
				break;
			case ROBO_TRAN_REBOOT_ME:
			default:
			robo_errlog("bus %s refuse request 0x%x", alias(), message_.tran.request);
			bus::confirm(ROBO_TRAN_REFUSE);
			}
		}

		void can_flow_bus::cancel(void) {
			driver_.cancel();
			if (wait_id_ != idle_id) {
				wait_id_ = idle_id;
				bus::confirm(ROBO_TRAN_REFUSE);
			}
		}
			
		bool can_flow_bus::ready(void) {
			return (wait_id_ == idle_id)  && driver_.ready();
		}

		can_flow_bus::can_flow_bus(robo::cstr _name, robo::app::module* _owner, driver & _driver)
			: robo::backend::bus(_name, _owner)
			, confirm_delegat_(*this, &can_flow_bus::exchange_confirm)
			, driver_(_driver) {
				message_.tran.size_max = packet_size;
		}
	}
}
