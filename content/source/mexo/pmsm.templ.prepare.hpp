#ifdef  TEMPL_BEGIN

#ifndef PMSM_TEMPLATE_NAME
#define PMSM_TEMPLATE_NAME pmsm
#endif

#define PMSM_PREFIX(name)  _PMSM_PREFIX(name,PMSM_TEMPLATE_NAME)
#define _PMSM_PREFIX(name,prfx)  __PMSM_PREFIX(name,prfx)
#define __PMSM_PREFIX(name,prfx) prfx##_##name

#define PMSM_ACTUATOR_PS_TEMPLATE_SUB_NAME PMSM_PS_CROSS_SUB_NAME
#define PMSM_ACTUATOR_TEMPLATE_SUB_NAME PMSM_PREFIX(PMSM_ACTUATOR_SUB_NAME)


#undef TEMPL_BEGIN

#else
	#ifdef  TEMPL_FINISH
		#undef PMSM_TEMPLATE_NAME
		#undef  PMSM_PREFIX
		#undef  _PMSM_PREFIX
		#undef  __PMSM_PREFIX

		#undef  PMSM_ACTUATOR_PS_TEMPLATE_SUB_NAME
		#undef  PMSM_ACTUATOR_TEMPLATE_SUB_NAME


		#undef TEMPL_FINISH
		
	#else
		#error  invalid prepare mode
	#endif
#endif

