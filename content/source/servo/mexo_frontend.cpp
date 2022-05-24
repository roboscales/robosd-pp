#include "servo/mexo_frontend.hpp"
namespace mexo {
	namespace frontend {
		servo::servo(robo::cstr _name, robo::app::module& _module)
			: robo::frontend::servo(_name, _module) {
		}
	}
}
