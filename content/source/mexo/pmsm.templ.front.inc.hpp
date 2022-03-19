#define TEMPL_BEGIN
#include "mexo/pmsm.templ.prepare.hpp"
#define ACTUATOR_TEMPLATE_NAME PMSM_ACTUATOR_TEMPLATE_SUB_NAME
#define ACTUATOR_PS_TEMPLATE_NAME PMSM_PS_CROSS_TEMPLATE_NAME
#include "mexo/actuator.templ.front.inc.hpp"
namespace mexo {
	namespace front {
		namespace PMSM_TEMPLATE_NAME {
			struct mode : public PMSM_ACTUATOR_TEMPLATE_SUB_NAME::mode {
				enum {
					sync_ofset = 32
					#if PMSM_SYNC_VOLTAGE_MODE_ENABLED == 1  
					, sync_voltage = sync_ofset+1
					#endif
					#if PMSM_SYNC_CURRENT_MODE_ENABLED == 1  
					, sync_current = sync_ofset + 2
					#endif
				};
			};

			template<typename types> struct action_t {
				PMSM_ACTUATOR_TEMPLATE_SUB_NAME::action_t<types> actuator;
				#if PMSM_SYNC_VOLTAGE_MODE_ENABLED == 1 || PMSM_SYNC_CURRENT_MODE_ENABLED == 1
				typename types::long_signal_t freq;
				typename types::long_signal_t angle;
				struct{
					typename types::signal_t voltage;
					typename types::signal_t current;
				} lateral;
				#endif
			};
			template<typename types>struct feedback_t {
				PMSM_ACTUATOR_TEMPLATE_SUB_NAME::action_t<types> actuator;
			};
		};
	}
}

#define TEMPL_FINISH
#include "mexo/pmsm.templ.prepare.hpp"