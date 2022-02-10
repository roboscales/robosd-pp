#ifdef  TEMPL_BEGIN
 
#ifndef BRAKE_TEMPLATE_NAME
#define BRAKE_TEMPLATE_NAME brake
#define BRAKE_PS_TEMPLATE_NAME brake_ps
#define brake_CURRENT_MODE_ENABLED 1
#define brake_VOLTAGE_CL_MODE_ENABLED 1
#define brake_VOLTAGE_MODE_ENABLED 1
#define brake_ps_CURRENT_REGULATOR_ENABLED 1
#define brake_ps_CURRENT_LIMMITER_ENABLED 1
#define brake_ps_VOLTAGE_REGULATOR_ENABLED 1
#endif

#define BRAKE_PREFIX(name)  _BRAKE_PREFIX(name,BRAKE_TEMPLATE_NAME)
#define _BRAKE_PREFIX(name,prfx)  __BRAKE_PREFIX(name,prfx)
#define __BRAKE_PREFIX(name,prfx) prfx##_##name

#define BRAKE_PS_PREFIX(name)  _BRAKE_PS_PREFIX(name,BRAKE_PS_TEMPLATE_NAME)
#define _BRAKE_PS_PREFIX(name,prfx)  __BRAKE_PS_PREFIX(name,prfx)
#define __BRAKE_PS_PREFIX(name,prfx) prfx##_##name

#define BRAKE_CURRENT_MODE_ENABLED  (BRAKE_PS_PREFIX(CURRENT_REGULATOR_ENABLED)==1)

#define BRAKE_VOLTAGE_MODE_ENABLED (BRAKE_PS_PREFIX(VOLTAGE_REGULATOR_ENABLED)==1) 

#undef TEMPL_BEGIN

#else
	#ifdef  TEMPL_FINISH
		#undef BRAKE_TEMPLATE_NAME
		#undef BRAKE_PS_TEMPLATE_NAME
		#undef BRAKE_PS_TEMPLATE_SUB_NAME

		#undef BRAKE_CURRENT_MODE_ENABLED
		#undef BRAKE_VOLTAGE_MODE_ENABLED

		#undef  BRAKE_PREFIX
		#undef  _BRAKE_PREFIX
		#undef  __BRAKE_PREFIX

		#undef BRAKE_PS_PREFIX
		#undef _BRAKE_PS_PREFIX
		#undef __BRAKE_PS_PREFIX
		
		#undef TEMPL_FINISH
		
	#else
		#error  invalid prepare mode
	#endif
#endif

