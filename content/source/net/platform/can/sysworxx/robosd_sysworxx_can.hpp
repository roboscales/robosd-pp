#ifndef __robo_net_vscom_can_hpp
#define  __robo_net_vscom_can_hpp
#include "net/robosd_can.hpp"
#include "core/robosd_system.hpp"
#include "core/robosd_string.hpp"
#include "CanItf.h"
namespace robo {
	namespace net {
		namespace can {
			namespace sysworxx {
				class ROBO_EXPORT port : public ican {
					int bitrate_;
					int channel_;
					string name_;
					bool open_(void);
					bool open__(void);
					enum { chan_count = 4 };
					static port* ports_[chan_count];
					static void on_read_(BYTE bChannel_p, tCanMsgStruct* pCanMsg_p);
				public:
					port(void);
					bool open(cstr _name);
					virtual void close(void);
					virtual bool send(uint32_t _id, const uint8_t* _buf, uint8_t  _len);
					virtual bool ready(void);
					virtual void reset(void);
					virtual void poll(void);
				};
			}
		}
	}
}

#endif