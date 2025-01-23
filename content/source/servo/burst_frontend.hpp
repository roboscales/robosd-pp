#ifndef burst_frontend_hpp
#define burst_frontend_hpp
#include "servo/robosd_frontend.hpp"
#include "servo/mexo_proto.hpp"
namespace burst {
	namespace frontend {

		class servo : public robo::frontend::servo {
		public:
			servo(robo::cstr _name, robo::app::module& _module);
		};

	}
}
#endif

