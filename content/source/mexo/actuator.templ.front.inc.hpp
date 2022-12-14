
#define TEMPL_BEGIN
#include "mexo/actuator.templ.prepare.hpp"

#define PS_TEMPLATE_NAME ACTUATOR_PS_TEMPLATE_NAME 
#include "mexo/ps.templ.front.inc.hpp"
namespace ACTUATOR_TEMPLATE_NAME {
	namespace front {
		struct mode : public ACTUATOR_PS_TEMPLATE_NAME::front::mode {
			enum {
				cl_ofset = 16 
				#if ACTUATOR_SPEED_OV_CURRENT_MODE_ENABLED == 1  
				,speed_ov_current =3 
				#endif
				#if ACTUATOR_POSITION_OV_CURRENT_MODE_ENABLED == 1  
				,position_ov_current =4
				#endif
				#if ACTUATOR_SPEED_OV_VOLTAGE_CL_MODE_ENABLED == 1
				,speed_ov_voltage_cl = 1 + cl_ofset
				#endif
				#if ACTUATOR_POSITION_OV_VOLTAGE_CL_MODE_ENABLED == 1
				,position_ov_voltage_cl = 2 + cl_ofset
				#endif
			};
		};
		template<typename types> struct action_t {
			ACTUATOR_PS_TEMPLATE_NAME::front::action_t<types> ps;

			#if ACTUATOR_SPEED_OV_CURRENT_MODE_ENABLED == 1  \
			|| ACTUATOR_SPEED_OV_VOLTAGE_CL_MODE_ENABLED == 1 
			mexo_proto_signal_t speed;
			#endif
			#if ACTUATOR_POSITION_OV_CURRENT_MODE_ENABLED == 1  \
			|| ACTUATOR_POSITION_OV_VOLTAGE_CL_MODE_ENABLED == 1 
			mexo_proto_long_signal_t position;
			#endif
		};
		template<typename types>struct feedback_t {
			ACTUATOR_PS_TEMPLATE_NAME::front::feedback_t<types> ps;
			#if ACTUATOR_MOTOR_POSTITION_MEASSURY_ENABLED == 1
			mexo_proto_signal_t crawl_speed;
			mexo_proto_long_signal_t dead_zone;
			mexo_proto_signal_t speed;
			mexo_proto_long_signal_t position;
			#endif
		};
		template<typename types>struct profil_t {
			#if ACTUATOR_PREFIX(MOTOR_POSTITION_MEASSURY_ENABLED)
			mexo_proto_signal_t crawl_speed;
			mexo_proto_long_signal_t dead_zone;
			#endif
		};
	}
}

#define TEMPL_FINISH
#include "mexo/actuator.templ.prepare.hpp"