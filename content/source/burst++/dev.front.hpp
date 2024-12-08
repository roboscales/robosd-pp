#ifndef dev_front_hpp
#define dev_front_hpp

#ifdef ROBO_APP_BURST_SERVO_SIDE
#include "servo/robosd_proto.hpp"
#endif
#ifdef ROBO_APP_BURST_SIDE
#endif

#include <stdint.h>

namespace burst {
	namespace front {
		namespace board {
			namespace panics {
				struct bits {
					enum {
						overvoltage = 0
						, lovoltage = 1
						, overcurrent = 2
						, locurrent = 3
						, overtemp = 4
						, lotemp = 5
						, config = 6
					};
				};
				struct masks {
					enum {
						overvoltage = 1 << bits::overvoltage
						, lovoltage = 1 << bits::overcurrent
						, overcurrent = 1 << bits::overcurrent
						, locurrent = 1 << bits::locurrent
						, overtemp = 1 << bits::overtemp
						, lotemp = 1 << bits::lotemp
						, config = 1 << bits::config
					};
				};
			}
		}
		struct tp_verb {
			enum {
				realtime = 1
				, backend = 2
				, frontend = 3
				, loop = 4
			};
		};
		namespace dev {
			namespace panics {
				struct bits {
					enum {
						master_lost = 0
						, board = 1
						, last = board
					};
				};
				struct masks {
					enum {
						master_lost = 1 << bits::master_lost
						, board = 1 << bits::board
					};
				};
			}
			struct modes { enum { idle = 0 }; };
			struct action_s {
				#ifdef ROBO_APP_BURST_SERVO_SIDE
				robo::common::devagent::action_s agent;
				#endif
				uint32_t mode;
				bool action_actual;
			};
			struct feedback_s {
				#ifdef ROBO_APP_BURST_SERVO_SIDE
				robo::common::devagent::feedback_s agent;
				#endif
				uint32_t mode;
				bool fault;
			};
			/*
			struct flow_command_ix {
				enum {
					echo = 1
					, var = 2
					, serial_1 = 3
					, serial_2 = 4
					, serial_m = 5
					, snapshot = 6
					, goal = 7
				};
			};
			*/
		}
	}
}
#endif
