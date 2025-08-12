#define MODULE_NAME  emu_can
#define MODULE_NAME_STR RT("emu_cans")
#include "core/robosd_app.hpp"
#include "core/robosd_log.hpp"
#include "net/robosd_can_flow_bus.hpp"
#include "net/platform/can/emulator/emu_can.hpp"
#include "net/robosd_can.hpp"

namespace MODULE_NAME{
	class phys : public ::robo::app::node {
	public:
		phys(robo::cstr _name, node* _owner) : node(_name, _owner) {}
		using B = ::robo::net::emu_can::port;
		B instance;
		virtual bool do_load(void) {
			ROBO_LBREAKN(robo::ini::load(current_path(), defaults_path(), RT("channel"), instance.channel));
			ROBO_LBREAKN(robo::ini::load(current_path(), defaults_path(), RT("repeat_max_count"), instance.repeat_max_count));
			ROBO_LBREAKN(instance.reg(current_path()));
			return true;
		}
		virtual bool do_start(void) {
			ROBO_LBREAKN( instance.open(false) );
			return true;
		}
		virtual void poll(void) {
			instance.poll();
		}
		virtual void do_clean(void) {
			instance.close();
			instance.unreg();
		}
	};
	static const inline robo::char_t nm[] = MODULE_NAME_STR;
	using module = robo::net::cans_module_t< phys, nm>;
}

#include "core/robosd_system_module_reg.hpp"
