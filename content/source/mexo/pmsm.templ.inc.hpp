#define TEMPL_BEGIN
#include "mexo/pmsm.templ.prepare.hpp"
#define ACTUATOR_TEMPLATE_NAME PMSM_ACTUATOR_TEMPLATE_SUB_NAME
#define ACTUATOR_PS_TEMPLATE_SUB_NAME PMSM_ACTUATOR_PS_TEMPLATE_SUB_NAME
#include "mexo/actuator.templ.inc.hpp"
namespace PMSM_TEMPLATE_NAME {

	template <typename types, typename hardwaresys_t>  class dev_t: public PMSM_ACTUATOR_TEMPLATE_SUB_NAME:: dev_t<types,hardwaresys_t> {

	public:
		typedef action_t<types> action_s;
		typedef feedback_t<types> feedback_s;

		typedef PMSM_ACTUATOR_TEMPLATE_SUB_NAME::dev_t<types, hardwaresys_t> actuator_t;

		struct config_s {
			typename actuator_t::config_s actuator;
		};

		
		struct present_s {
			typename actuator_t::present_s actuator;
		};

	protected:
	public:

		dev_t (hardwaresys_t& _hardwaresys, cstr _name, action_s & _action, config_s& _config, present_s& _present, int _slot_index)
			: actuator_t(_hardwaresys, _name, _action.actuator, _config.actuator, _present.actuator, _slot_index)
		{

		}
	};
}

#define TEMPL_FINISH
#include "mexo/pmsm.templ.prepare.hpp"
