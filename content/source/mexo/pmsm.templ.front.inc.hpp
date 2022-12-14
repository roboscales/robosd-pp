#define TEMPL_BEGIN
#include "mexo/pmsm.templ.prepare.hpp"
#define ACTUATOR_TEMPLATE_NAME PMSM_ACTUATOR_TEMPLATE_SUB_NAME
#define ACTUATOR_PS_TEMPLATE_NAME PMSM_PS_CROSS_TEMPLATE_NAME
#include "mexo/actuator.templ.front.inc.hpp"
namespace PMSM_TEMPLATE_NAME {
	namespace front {
		struct mode : public PMSM_ACTUATOR_TEMPLATE_SUB_NAME::front::mode {
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
			PMSM_ACTUATOR_TEMPLATE_SUB_NAME::front::action_t<types> actuator;
			#if PMSM_SYNC_VOLTAGE_MODE_ENABLED == 1 || PMSM_SYNC_CURRENT_MODE_ENABLED == 1
			mexo_proto_long_signal_t freq;
			mexo_proto_long_signal_t angle;
			struct{
				mexo_proto_signal_t voltage;
				#if LAT_CURRENT_REGULATOR_ENABLED == 1
				mexo_proto_signal_t current;
				#endif
			} lateral;
			#endif
		};
		template<typename types>struct feedback_t {
			PMSM_ACTUATOR_TEMPLATE_SUB_NAME::front::feedback_t<types> actuator;
			struct {
				mexo_proto_signal_t voltage;
				#if LAT_CURRENT_MEASSURY_ENABLED == 1
				mexo_proto_signal_t current;
				#endif
			} lateral;			
		};
		
		template<typename types>struct profil_t{
			PMSM_ACTUATOR_TEMPLATE_SUB_NAME::front::profil_t<types> actuator;
		};
	};
}

#define TEMPL_FINISH
#include "mexo/pmsm.templ.prepare.hpp"