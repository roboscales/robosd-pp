#include "servo/burst_frontend.hpp"
#include "servo/burst_backend.hpp"
namespace burst {
	namespace frontend {
		robo::quest* devagent::configure_expansion_create(quest* _quest) {
			((burst::backend::devagent *)backend())->post_startup_vars_quest(
				startup_section
				,
				_quest);
			return _quest;
		}
		bool devagent::do_load(void) {
			if (status() != statuses::locals::disabled) {
				ROBO_LBREAKN_F(startup_section.load(current_path(), defaults_path(), RT("startup")), "var's section %s for %s isn't found", RT("startup"), display_alias());
			}
			ROBO_LBREAKN(robo::frontend::devagent::do_load());
			return true;
		}
		servo::servo(robo::cstr _name, robo::app::module& _module)
			: robo::frontend::servo(_name, _module) {
		}		
	}
}
