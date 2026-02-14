#ifndef dev_front_hpp
#define dev_front_hpp

#include "servo/robosd_proto.hpp"

#include <stdint.h>

namespace burst {
	namespace front {
		namespace board {
			enum class commands {none = 0,configure=1};
			enum class statuses{ unknown = 0, startuped, configure, restart, normal};

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
                        , sto_active = 7
                        , friendly_driver_in_trouble = 8
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
                        , sto_active = 1 << bits::sto_active
                        , friendly_driver_in_trouble = 1 << bits::friendly_driver_in_trouble
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
				robo::common::devagent::action_s command;
				#endif
				#ifdef ROBO_APP_BURST_SIDE
				robo::common::devagent::commands::remote command;
				#endif

				uint32_t mode;
				bool action_actual;
			};
			struct feedback_s {
				#ifdef ROBO_APP_BURST_SERVO_SIDE
				robo::common::devagent::feedback_s status;
				#endif
				#ifdef ROBO_APP_BURST_SIDE
					robo::common::devagent::statuses::remote status;
				#endif
				uint32_t mode;
				bool fault;
			};


		}
	}
}
#endif
