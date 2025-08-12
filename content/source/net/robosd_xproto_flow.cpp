#include "robosd_xproto_flow.hpp"

namespace robo {
	namespace net {
		namespace can {
			namespace flow {
				namespace xproto {

					instance::map& instance::tables_(void) {
						static map tables__;
						return tables__;
					}

					instance::instance(robo::cstr _instance_name) :ref_(*this, robo::hash(_instance_name)) {
						ROBO_ASSERT(ref_.attach_to(tables_()));
					}

					instance* instance::attach(robo::cstr _instance_name, ::robo::net::ican& _ican) {
						instance* it = tables_().find(robo::hash(_instance_name));
						ROBO_ASSERT(it);
						it->set_handle(_ican);
						return it;
					};
				}
			}
		}
	}
}

