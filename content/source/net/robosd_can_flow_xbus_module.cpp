#include "net/robosd_can_flow_module.hpp"
#include "net/robosd_can.hpp"

#define MODULE_NAME  can_flow_xbus
#define MODULE_NAME_STR RT("can_flow_xbus")

namespace MODULE_NAME {
	

	static const inline robo::char_t nm[] = MODULE_NAME_STR;
	using module = robo::net::can::flow::module_t< robo::net::can::flow::phys_t< robo::net::can::flow::xphys >, nm>;

}
#include "core/robosd_system_module_reg.hpp"