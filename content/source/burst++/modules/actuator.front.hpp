#ifndef burst_modules_actuator_front_hpp
#define burst_modules_actuator_front_hpp
#include <stdint.h>

#include "burst++/dev.front.hpp"

namespace burst {
	namespace front {
		namespace  actuator {
			#pragma pack(push, 1)

			template<typename number> struct action_s {
				dev::action_s dev;
				typename number::signal_t voltage;
				typename number::signal_t speed;
				typename number::long_signal_t position;
			};

			template<typename number> struct feedback_s {
				dev::feedback_s dev;
				typename number::signal_t voltage;
				typename number::signal_t speed;
				typename number::long_signal_t position;
			};

			struct modes{ 
				enum {
					idle = dev::modes::idle
					, fault = 1
					, voltage = 2
					, speed = 3
					, position = 4
					, last = position
				};
			};
			namespace panics {
				struct bits {
					enum {
						possence = burst::front::dev::panics::bits::last + 1
						, overtemp = burst::front::dev::panics::bits::last + 2
						, lotemp = burst::front::dev::panics::bits::last + 3
						, last = lotemp
					};
				};
				struct masks {
					enum {
						possence = 1<<bits::possence
						, overtemp = 1<<bits::overtemp
						, lotemp = 1<<bits::lotemp
					};
				};
			}

			#pragma pack(pop)


		}
	}
}
#endif
