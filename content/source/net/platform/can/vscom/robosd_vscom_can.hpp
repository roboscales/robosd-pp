#ifndef __robo_net_vscom_can_hpp
#define  __robo_net_vscom_can_hpp
#include "net/robosd_can.hpp"
#include "core/robosd_system.hpp"
#include "core/robosd_string.hpp"
#include "vs_can_api.h"
namespace robo {
	namespace net {
		namespace vscom_can {
			class ROBO_EXPORT port : public ican {
				VSCAN_HANDLE handle_ = -1;
				void * bitrate_ = (void*)0xFFFFFFFF;
				string name_;
				bool open_(void);
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

#endif