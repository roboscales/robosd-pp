#ifndef dev_front_hpp
#define dev_front_hpp
#ifdef MEXO_SERVO_SIDE
#include "core/robosd_proto.hpp"
#define mexo_proto_signal_t float
#define mexo_proto_long_signal_t double
#endif
#ifdef MEXO_DEVICE_SIDE
#define mexo_proto_signal_t typename types::signal_t
#define mexo_proto_long_signal_t typename types::long_signal_t
#endif
#include <stdint.h>
namespace mexo {
	namespace front {
		namespace dev {
			struct mode { enum { idle = 0, fault = 0x255 }; };
			struct action_s 
			{
				#ifdef MEXO_SERVO_SIDE
				robo::common::devagent::action_s agent;
				#endif
				uint8_t mode;
			};
			struct feedback_s {
				#ifdef MEXO_SERVO_SIDE
				robo::common::devagent::feedback_s agent;
				#endif
				uint8_t mode;
			};
		}
	}
}
#endif

/*
				#ifdef MEXO_SERVO_SIDE
				: public robo::common::devagent::feedback_s
				#endif
			{
*/