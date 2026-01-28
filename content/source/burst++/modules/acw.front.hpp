#ifndef burst_modules_acwc_front_hpp
#define burst_modules_acwc_front_hpp

#include "burst++/modules/actuator.front.hpp"

namespace burst {
	namespace front {
		namespace  acw {
			#pragma pack(push, 1)

			template<typename number> struct action_s {
				actuator::action_s<number> ac;
				typename number::signal_t current;
			};

			template<typename number> struct feedback_s {
				actuator::feedback_s<number> ac;
				typename number::signal_t current;
			};

			struct modes {
				enum {
					voltage_cl = actuator::modes::last+1 //5
					, speed_cl = actuator::modes::last + 2 //6
					, position_cl = actuator::modes::last + 3 //7
					, currnet = actuator::modes::last + 4 //8
					, speed = actuator::modes::last + 5 //9
					, position = actuator::modes::last + 6 //10
					, last = position
				};
			};
			namespace panics {
				struct bits {
					enum {
						overpower = burst::front::actuator::panics::bits::last + 1
						, overcurrent = burst::front::actuator::panics::bits::last + 2
						, circuit_break = burst::front::actuator::panics::bits::last + 3
						, last = circuit_break
					};
				};
				struct masks {
					enum {
						overpower = 1 << bits::overpower
						, overcurrent = 1 << bits::overcurrent
						, circuit_break = 1 << bits::circuit_break
					};
				};
			}

			#pragma pack(pop)
		}
	}
}
#endif