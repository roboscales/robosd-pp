#ifndef mexopp_drive_hpp
#define mexopp_drive_hpp
#include "mexo++drive.common.hpp"
#include "mexo++/mexo.hpp"
#include "prf/clock_ns.hpp"
using slot = ::mexo::machine::slot;
namespace mppd{
	namespace prf{
		struct clock{
			static uint32_t tick(void);
			static void idle(void);
			constexpr  static uint32_t clock_hz(){ return MPPD_CORE_CLOCK_HZ; }
		};
	}
	typedef ::robo::prf::clock_ns_t<::robo::prf::clock32_drv,prf::clock> clock;
}
#endif