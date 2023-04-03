#ifndef mexopp_drive_hpp
#define mexopp_drive_hpp
#include "mexo++drive.common.hpp"
#include "mexo++/mexo.hpp"
using slot = ::mexo::machine::slot;
namespace mppd{
	namespace periphery{
		struct clock{
			typedef uint32_t ns_t;
			typedef uint32_t us_t;
			typedef uint32_t tick_t;
			static tick_t tick(void);
			static void idle(void);
			static tick_t ns2tick(ns_t _ns);
			static ns_t tick2ns(tick_t _tick);
			static tick_t us2tick(us_t _ns);
			static us_t tick2us(tick_t _tick);
		};
	}
	typedef ::mexo::clock_us_t<periphery::clock> clock;

}
#endif