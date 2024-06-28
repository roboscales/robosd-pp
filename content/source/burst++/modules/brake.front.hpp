#ifndef burst_modules_brake_front_hpp
#define burst_modules_brake_front_hpp

#include "burst++/dev.front.hpp"

namespace burst {
	namespace front {
		namespace  brake {
			#pragma pack(push, 1)
			struct modes {
				enum {
					dummy = 0
					, pwm = 1
					, current = 2
				};
			};


			#pragma pack(pop)


		}
	}
}

#endif