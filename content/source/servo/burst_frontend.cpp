#include "servo/burst_frontend.hpp"
namespace burst {
	namespace frontend {
		servo::servo(robo::cstr _name, robo::app::module& _module)
			: robo::frontend::servo(_name, _module) {
		}
	}
}
