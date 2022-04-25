#ifndef __emu_can_hpp
#define  __emu_can_hpp
#include "net/robosd_can.hpp"
#include "core/robosd_system.hpp"

#define ROBO_CAN_CHANNEL_SHARED_FILE_NAME "ROBO_CAN_CHANNEL_SHARED_FILE_NAME_%d"
#define ROBO_CAN_CHANNEL_SHARED_FILE_NAME_LEN 52
#define ROBO_CAN_CHANNEL_CONFIRM_EVENT_NAME "ROBO_CAN_CHANNEL_SHARED_CONFIRM_EVENT_NAME_%d"
#define ROBO_CAN_CHANNEL_LOCK_NAME "ROBO_CAN_CHANNEL_SHARED_LOCK_NAME_%d"
#define ROBO_CAN_CHANNEL_BYTE_COUNT 8
#define ROBO_CAN_CONNECTION_COUNT 255

namespace robo {
	namespace net {
		namespace emu_can {

			class agent;
			class ROBO_EXPORT port : public ican {
				friend class agent;
				struct shared;
				struct ether;
				shared* shared_ = nullptr;
				ether* ether_ = nullptr;
			public:
				int repeat_max_count = 1;
				int channel = 0;
//				int bitrate = 0;
				virtual bool open(bool _owned_view = false);
				virtual void close(void);
				virtual bool send(uint32_t _id, const uint8_t* _buf, uint8_t  _len);
				virtual bool ready(void);
				virtual void reset(void);
				virtual void pool(void);
			};
		}
	}
}
#endif