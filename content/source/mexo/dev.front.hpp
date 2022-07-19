#ifndef dev_front_hpp
#define dev_front_hpp
#ifdef ROBO_APP_MEXO_SERVO_SIDE
#include "servo/robosd_proto.hpp"
#define mexo_proto_signal_t float
#define mexo_proto_long_signal_t double
#endif
#ifdef ROBO_APP_MEXO_SIDE
#define mexo_proto_signal_t typename types::signal_t
#define mexo_proto_long_signal_t typename types::long_signal_t
#endif
#include <stdint.h>
namespace mexo {
	namespace front {
		namespace dev {
			struct mode { enum { idle = 0/*, fault = 0x255*/ }; };
			struct action_s 
			{
				#ifdef ROBO_APP_MEXO_SERVO_SIDE
				robo::common::devagent::action_s agent;
				#endif
				uint8_t mode;
			};
			struct feedback_s {
				#ifdef ROBO_APP_MEXO_SERVO_SIDE
				robo::common::devagent::feedback_s agent;
				#endif
				uint8_t mode;
				bool fault;
			};
			struct flow_command_ix {
				enum {
					echo = 1
					, var = 2
					, serial_1 = 3
					, serial_2 = 4
					, serial_m = 5
					, snapshot = 6
				};
			};

		}
	}
}
#endif

/*
				#ifdef ROBO_APP_MEXO_SERVO_SIDE
				: public robo::common::devagent::feedback_s
				#endif
			{
*/