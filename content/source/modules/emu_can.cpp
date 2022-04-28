#define MODULE_NAME  emu_can
#define MODULE_NAME_STR RT("emu_can")
#include "core/robosd_app.hpp"

namespace MODULE_NAME{

	class module : public robo::app::module {
		module(void)
			: robo::app::module(MODULE_NAME_STR) {}
	protected:
	public:
		static module& instance(void) {
			static module instance_;
			return instance_;
		}
	};
}

#include "core/robosd_system_module_reg.hpp"
