#ifndef __emu_can_hpp
#define  __emu_can_hpp
#include "net/robosd_can.hpp"

#define ROBO_CAN_CHANNEL_SHARED_FILE_NAME "ROBO_CAN_CHANNEL_SHARED_FILE_NAME_%d"
#define ROBO_CAN_CHANNEL_SHARED_FILE_NAME_LEN 52
#define ROBO_CAN_CHANNEL_CONFIRM_EVENT_NAME "ROBO_CAN_CHANNEL_SHARED_CONFIRM_EVENT_NAME_%d"
#define ROBO_CAN_CHANNEL_LOCK_NAME "ROBO_CAN_CHANNEL_SHARED_LOCK_NAME_%d"
#define ROBO_CAN_CHANNEL_BYTE_COUNT 8
#define ROBO_CAN_CONNECTION_COUNT 255

namespace robo {
	namespace net {
		namespace emu_can {
			class port : public ican {
				int repeat_max_count = 1;
				int channel = 0;
				int bitrate = 0;
				virtual bool open(void);
				virtual void close(void);
				virtual bool send(uint16_t _id, uint8_t* _buf, uint8_t  _len);
				virtual bool ready(void);
				virtual void reset(void);
				virtual void pool(void);
			};
		}
	}
}
#endif