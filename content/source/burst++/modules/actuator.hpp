#ifndef burst_actuator_hpp
#define burst_actuator_hpp
#include "burst++/burst.hpp"
#include "burst++/math.hpp"
#include "burst++/modules/ps.hpp"

#ifndef BURST_PANICS_ACTUATOR_TEMPER_ENABLED
#define BURST_PANICS_ACTUATOR_TEMPER_ENABLED 0
#endif

namespace burst{
	template <class number> class actuator: public burst::dev{
		public:
		struct config_s{
			burst::dev::config_s ref;
			using signal_t = typename number::signal_t;
			using long_signal_t = typename number::long_signal_t;
			struct{
				time_us_t reset;
				time_us_t set;
			} enco_fault_ticks;
			struct {
				range_s<signal_t> voltage;
				range_s<signal_t> speed;
				range_s<long_signal_t> position;
			} range;
			struct {
				burst_motion_config_t motion;
				burst_positioner_config_t positioner;
			} modes;
			struct {
				#if BURST_PROTECTION_ENABLED == 1
				#if BURST_PANICS_ACTUATOR_TEMPER_ENABLED == 1
				burst_hyst_t temper_pp;		
				#endif
				#endif
			} panic;
		}
	};
}
#endif