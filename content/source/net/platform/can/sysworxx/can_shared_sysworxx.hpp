#ifndef can_shared_sysworxx_hpp
#define  can_shared_sysworxx_hpp
#include "net/robosd_can.hpp"
#include "core/robosd_system.hpp"
#include "core/robosd_string.hpp"
#include "core/robosd_list.hpp"
#include "CanItf.h"
namespace robo {
	namespace net {
		namespace can {
			namespace sysworxx {
				class ROBO_EXPORT port : public ican {
				public:
					typedef robo::list::unsorted<port> list;
					typedef list::ref ref;
				private:
					ref ref_;
				public:
					port(void) :ref_(*this) {}
					void receive(uint32_t, const uint8_t*, uint8_t);
					virtual bool open(bool _owned_view = false);
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