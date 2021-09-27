#ifndef ACTUATOR_TEMPLATE_NAME
#define ACTUATOR_TEMPLATE_NAME actuator
#define ACTUATOR_PREFIX(name)  _ACTUATOR_PREFIX(name,ACTUATOR_TEMPLATE_NAME)
#define _ACTUATOR_PREFIX(name,prfx)  __ACTUATOR_PREFIX(name,prfx)
#define __ACTUATOR_PREFIX(name,prfx) prfx##_##name
#endif

#define POWER_SUPPLY_PREFIX(name)  _POWER_SUPPLY_PREFIX(name,ACTUATOR_TEMPLATE_NAME)
#define _POWER_SUPPLY_PREFIX(name,prfx)  __POWER_SUPPLY_PREFIX(name,prfx)
#define __POWER_SUPPLY_PREFIX(name,prfx) prfx##_ps_##name

#define PS_TEMPLATE_NAME ACTUATOR_PREFIX(ps)

#include "mexo/ps.templ.inc.hpp"

template <typename types, typename hardwaresys_t>  class ACTUATOR_PREFIX(t) : public PS_TEMPLATE_NAME < types, hardwaresys_t > {
public:
	typedef PS_TEMPLATE_NAME < types, hardwaresys_t > ps_t;
	struct action_s {
		typename ps_t::action_s ps;
	};

	struct config_s {
		typename ps_t::config_s ps;
	};

	struct present_s {
		typename ps_t::present_s ps;
	};
	struct standalone_s {
		typename ps_t::standalone_s ps;
	};

	ACTUATOR_PREFIX(t)(hardwaresys_t & _hardwaresys, cstr _name, action_s & _action, config_s & _config, standalone_s & _standalone, present_s & _present)
		: ps_t(_hardwaresys, _name, _action.ps, _config.ps, _standalone.ps, _present.ps) {}
};
#undef  POWER_SUPPLY_PREFIX
#undef  _POWER_SUPPLY_PREFIX
#undef  __POWER_SUPPLY_PREFIX
#undef PS_TEMPLATE_NAME
