#ifndef burst_modules_pmsm_front_hpp
#define burst_modules_pmsm_front_hpp
#include "burst++/modules/acw.front.hpp"

namespace burst {
	namespace front {
		namespace  pmsm {
			#pragma pack(push, 1)

			template<typename number> struct action_s {
				acw::action_s<number> cross;
				struct {
					typename number::signal_t voltage;
					typename number::signal_t current;
				} lateral;
				struct {
					typename number::long_signal_t freq;
					typename number::long_signal_t  angle;
				} synchro;
			};

			template<typename number> struct feedback_s {
				acw::feedback_s<number> cross;
				struct {
					typename number::signal_t voltage;
					typename number::signal_t current;
				} lateral;
			};

			struct modes {
				enum {
					synchro_voltage = acw::modes::last+1 //11
					, synchro_current = acw::modes::last + 2//12
					, synchro_hall_estimate = acw::modes::last + 3 //14
					, mode_count = synchro_hall_estimate
				};
			};
			namespace panics {
				struct bits {
					enum {
						phase_break = burst::front::acw::panics::bits::last + 1
						, misalignment = burst::front::acw::panics::bits::last + 2
					};
				};
				struct masks {
					enum {
						phase_break = 1 << bits::phase_break
						, misalignment = 1 << bits::misalignment
					};
				};
			}

			#pragma pack(pop)
		}
	}
}

#endif