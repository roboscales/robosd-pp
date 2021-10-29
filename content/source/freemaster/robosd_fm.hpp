#ifndef __freemaster_hpp
#define __freemaster_hpp
#include "core/robosd_common.hpp"

#ifndef ROBO_APP_FREEMASTER_SERIAL_ENABLED
#define ROBO_APP_FREEMASTER_SERIAL_ENABLED 0
#endif

#ifndef ROBO_APP_PROTO_SWITCH_ENABLED
#define ROBO_APP_PROTO_SWITCH_ENABLED 0
#endif

#if ROBO_APP_FREEMASTER_SERIAL_ENABLED ==1
#include "net/robosd_serial.hpp"
#endif

#if ROBO_APP_PROTO_SWITCH_ENABLED ==1
#include "net/robosd_proto_switch.hpp"
#endif

#if ROBO_APP_FREEMASTER_SERIAL_ENABLED ==1
namespace robo {
	class ROBO_EXPORT freemaster {
		robo::net::iserial* serial_;
		static freemaster instance_;
		freemaster(void);
		~freemaster(void);
		void connect_(robo::net::iserial* _serial);
	public:
		static robo::net::iserial* serial(void);
		static void connect(robo::net::iserial* _serial);
		static void recorder(void);
		static void poll(void);
		#if ROBO_APP_PROTO_SWITCH_ENABLED == 1
	class ROBO_EXPORT abonent : public ::robo::net::proto::switcher::abonent {
		protected:
			virtual void stop(void);
			virtual void start(void);
		public:
			virtual size_t get(uint8_t & _data);
			abonent(
				time_us_t _lock_us
				, time_us_t _silence_us
			);
		};
		#endif

	};
}
#endif

#endif
