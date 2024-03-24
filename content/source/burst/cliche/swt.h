#ifndef burst_swt_h
#define burst_swt_h
#include "burst/burst.h"
#include "burst/burst_signal.h"
typedef struct {
	struct{
		int prop;
		int model;
	} gain;
	struct{
		int32_t ramp32;
		uint16_t min;
		uint16_t max;
	} pwm;
	struct{
		int32_t ramp32;
		int16_t min;
		int16_t max;
	} current;
	uint8_t sector_offset;
}burst_swt_config_t;


typedef struct {
	int error;
	int model;
	int model32;
	struct{
		int16_t control;
		uint16_t actual;
		int16_t min;
		int16_t max;
	} pwm;
}burst_swt_t;
void swt_reset(burst_swt_t * _swt, int _pwm);
void swt_run(burst_swt_t * _swt, burst_swt_config_t * _config,int _requried,  int _actual);
void swt_set_control(burst_swt_t * _swt, burst_swt_config_t * _config);
#endif

/*typedef struct {
	burst_swt_t A;
	burst_swt_t B;
	burst_swt_t C;
}burst_swt_machine_t;
*/


#if 0
#define CLCH_NAME c
#define c_CONFIG_GAIN_PROP 0
#define c_CONFIG_GAIN_MODEL 0
#define c_CONFIG_PWM_RAMP32 0
#define c_CONFIG_PWM_MIN 0
#define c_CONFIG_PWM_MAX 100
#define c_CONFIG_CURRENT_RAMP32 0
#define c_CONFIG_CURRENT_MIN -100
#define c_CONFIG_CURRENT_MAX 100
#define c_CONFIG_SECTOR_OFFSET 0
#endif


#ifdef CLCH_NAME
#include "burst/cliche/_begin.h"
	void PREFIX(begin)(void);
	void PREFIX(pwm_start)(void);
	void PREFIX(pwm_stop)(void);
	void PREFIX(pwm_run)(void);
	void PREFIX(current_start)(void);
	void PREFIX(current_stop)(void);
	void PREFIX(current_run)(void);
	void PREFIX(current_phase_get)(int16_t * _A, int16_t * _B, int16_t * _C);
	void PREFIX(phy_A_set_pwm)(uint16_t _pwm);
	void PREFIX(phy_B_set_pwm)(uint16_t _pwm);
	void PREFIX(phy_C_set_pwm)(uint16_t _pwm);
	void PREFIX(phy_A_set_lo)(void);
	void PREFIX(phy_B_set_lo)(void);
	void PREFIX(phy_C_set_lo)(void);
	void PREFIX(phy_A_off)(void);
	void PREFIX(phy_B_off)(void);
	void PREFIX(phy_C_off)(void);
	void PREFIX(phy_A_on)(uint16_t _pwm);
	void PREFIX(phy_B_on)(uint16_t _pwm);
	void PREFIX(phy_C_on)(uint16_t _pwm);
	uint8_t PREFIX(phy_sector_get)(void);
	void PREFIX(current_mode_applay)(void);
#ifndef CLCH_HEADER
	burst_swt_config_t PREFIX(config) = {
		{//gain
			PREFIX(CONFIG_GAIN_PROP)//int prop;
			,PREFIX(CONFIG_GAIN_MODEL)//int model;
		}// gain;
		,{//pwm{
			PREFIX(CONFIG_PWM_RAMP32)//uint16_t ramp32;
			,PREFIX(CONFIG_PWM_MIN)//uint16_t min;
			,PREFIX(CONFIG_PWM_MAX)//uint16_t max;
		}//pwm;
		,{//current{
			PREFIX(CONFIG_CURRENT_RAMP32)//uint16_t ramp32;
			,PREFIX(CONFIG_CURRENT_MIN)//int16_t min;
			,PREFIX(CONFIG_CURRENT_MAX)//int16_t max;
		}//current;
		,PREFIX(CONFIG_SECTOR_OFFSET)//uint8_t sector_offset;
	};

	static int  PREFIX(sector_actual) = -1;
	static burst_signal_t  PREFIX(current_actual) = -1;

	struct{
		struct{
			burst_signal_t requried;
			burst_signal_t actual;
			burst_long_signal_t actual32;
		} pwm;
		struct{
			burst_signal_t deseired;
			burst_signal_t requried;
			burst_long_signal_t requried32;
			int mode;
			burst_swt_t pi;
			/*
			struct{
				burst_swt_t pi;
				int16_t actual;
			} A;
			struct{
				burst_swt_t pi;
				int16_t actual;
			} B;
			struct{
				burst_swt_t pi;
				int16_t actual;
			} C;*/
			struct{
				burst_signal_t A;
				burst_signal_t B;
				burst_signal_t C;
			} actual;
		} current;
	} CLCH_NAME = {};
	void PREFIX(begin)(void){
	}
	void PREFIX(current_mode_applay)(void){
		/*
		CLCH_NAME.current.A.pi.pwm.min = - CLCH_NAME.pwm.requried;
		CLCH_NAME.current.A.pi.pwm.max = CLCH_NAME.pwm.requried;
		CLCH_NAME.current.B.pi.pwm.min = - CLCH_NAME.pwm.requried;
		CLCH_NAME.current.B.pi.pwm.max = CLCH_NAME.pwm.requried;
		CLCH_NAME.current.C.pi.pwm.min = - CLCH_NAME.pwm.requried;
		CLCH_NAME.current.C.pi.pwm.max = CLCH_NAME.pwm.requried;*/
		CLCH_NAME.current.pi.pwm.min = - CLCH_NAME.pwm.requried;
		CLCH_NAME.current.pi.pwm.max = CLCH_NAME.pwm.requried;
	}
	void PREFIX(pwm_start)(void){
		PREFIX(sector_actual) = -1;
	}
	burst_bool_t PREFIX(voltage_dir_actual) = burst_false;
	void PREFIX(pwm_stop)(void){
		PREFIX(phy_A_off)();
		PREFIX(phy_B_off)();
		PREFIX(phy_C_off)();
		CLCH_NAME.pwm.actual32 = 0;
		CLCH_NAME.pwm.actual = 0;
	}
	#define SWT_PHASE_VOLTAGE_DIRECT(A,B,C) _SWT_PHASE_VOLTAGE_DIRECT(A,B,C)
	#define _SWT_PHASE_VOLTAGE_DIRECT(A,B,C)\
		if(sectror_change){\
			PREFIX(phy_##A##_off)();\
			PREFIX(phy_##C##_set_lo)();\
			PREFIX(phy_##B##_on)((uint16_t)pwm  );\
		} else{\
			PREFIX(phy_##B##_set_pwm)(pwm  );\
		}

	#define SWT_PHASE_VOLTAGE_REVERT(A,B,C) _SWT_PHASE_VOLTAGE_REVERT(A,B,C)
	#define _SWT_PHASE_VOLTAGE_REVERT(A,B,C)\
		if(sectror_change){\
			PREFIX(phy_##C##_off)();\
			PREFIX(phy_##A##_on)(PREFIX(config).pwm.max-pwm-PREFIX(config).pwm.min);\
			PREFIX(phy_##B##_on)(PREFIX(config).pwm.max);\
		} else{\
			PREFIX(phy_##A##_set_pwm)(PREFIX(config).pwm.max-pwm-PREFIX(config).pwm.min);\
		}		

	void PREFIX(pwm_run_)(int sector){
		burst_bool_t voltage_dir_actual = CLCH_NAME.pwm.actual >= 0 ? burst_true:burst_false;
		int sectror_change = sector != PREFIX(sector_actual) || (voltage_dir_actual != PREFIX(voltage_dir_actual));
		PREFIX(sector_actual) = sector;
		PREFIX(voltage_dir_actual) = voltage_dir_actual;
		uint16_t pwm = (uint16_t)( CLCH_NAME.pwm.actual >= 0 ? CLCH_NAME.pwm.actual : -CLCH_NAME.pwm.actual );
		if( pwm>0 ){
			pwm+=PREFIX(config).pwm.min;
			if(pwm > PREFIX(config).pwm.max){
				pwm = PREFIX(config).pwm.max;
			}
		}
		

		if(voltage_dir_actual){
			switch(sector){
				case 0: //A
					SWT_PHASE_VOLTAGE_DIRECT(A,B,C)
					break;
				case 1://-C
					SWT_PHASE_VOLTAGE_REVERT(A,B,C)
					//SWT_PHASE_VOLTAGE_DIRECT(C,B,A)
					break;
				case 2://B
					SWT_PHASE_VOLTAGE_DIRECT(B,C,A)
					break;
				case 3://-A
					SWT_PHASE_VOLTAGE_REVERT(B,C,A)
					//SWT_PHASE_VOLTAGE_DIRECT(A,C,B)
					break;
				case 4://C
					SWT_PHASE_VOLTAGE_DIRECT(C,A,B)
					break;
				case 5: //-B
					SWT_PHASE_VOLTAGE_REVERT(C,A,B)
					//SWT_PHASE_VOLTAGE_DIRECT(B,A,C)
					break;
			}
		} else {
			switch(sector){
				case 0: //A
					SWT_PHASE_VOLTAGE_DIRECT(A,C,B)
					break;
				case 1://-C
					SWT_PHASE_VOLTAGE_REVERT(B,A,C)
					break;
				case 2://B
					SWT_PHASE_VOLTAGE_DIRECT(B,A,C)
					break;
				case 3://-A
					SWT_PHASE_VOLTAGE_REVERT(C,B,A)
					break;
				case 4://C
					SWT_PHASE_VOLTAGE_DIRECT(C,B,A)
					break;
				case 5: //-B
					SWT_PHASE_VOLTAGE_REVERT(A,C,B)
					break;
			}
		}
	}
	uint8_t PREFIX(sector) = -1;
	
	void PREFIX(pwm_run)(void){
		
		if(CLCH_NAME.pwm.requried>PREFIX(config).pwm.max){
			CLCH_NAME.pwm.requried = PREFIX(config).pwm.max;
		}
		if(CLCH_NAME.pwm.requried<-PREFIX(config).pwm.max){
			CLCH_NAME.pwm.requried = -PREFIX(config).pwm.max;
		}
		
		if(CLCH_NAME.pwm.actual < CLCH_NAME.pwm.requried ){
			CLCH_NAME.pwm.actual32 +=  PREFIX(config).pwm.ramp32;
			CLCH_NAME.pwm.actual = (int16_t)(CLCH_NAME.pwm.actual32>>16);
			if( CLCH_NAME.pwm.actual > CLCH_NAME.pwm.requried){
				CLCH_NAME.pwm.actual = CLCH_NAME.pwm.requried ;
			}
		} else if(CLCH_NAME.pwm.actual > CLCH_NAME.pwm.requried) {
			CLCH_NAME.pwm.actual32 -=  PREFIX(config).pwm.ramp32;
			CLCH_NAME.pwm.actual = (int16_t)(CLCH_NAME.pwm.actual32>>16);
			if( CLCH_NAME.pwm.actual < CLCH_NAME.pwm.requried){
				CLCH_NAME.pwm.actual = CLCH_NAME.pwm.requried ;
			}
		}
		
		PREFIX(sector) = PREFIX(phy_sector_get)() + PREFIX(config).sector_offset;
		if (PREFIX(sector) >=6){
			PREFIX(sector) = PREFIX(sector)-6;
		}
		PREFIX(pwm_run_)(PREFIX(sector));
		
		//CLCH_NAME.pwm.actual = CLCH_NAME.pwm.requried;
		
		
		//CLCH_NAME.pwm.actual = CLCH_NAME.pwm.requried;
	}
	void PREFIX(current_start)(void){
		PREFIX(sector_actual) = -1;
	}
	
	void PREFIX(current_stop)(void){
		PREFIX(pwm_stop)();
		CLCH_NAME.current.requried = 0;
		CLCH_NAME.current.requried32 = 0;
		/*
		swt_reset(&CLCH_NAME.current.A.pi,0);
		swt_reset(&CLCH_NAME.current.B.pi,0);
		swt_reset(&CLCH_NAME.current.C.pi,0);
		*/
		swt_reset(&CLCH_NAME.current.pi,0);
	}
	
	#define SWT_PHASE_CURRENT(A,B,C) _SWT_PHASE_CURRENT(A,B,C)
	


	void PREFIX(current_run)(void){
		
		int sector = PREFIX(phy_sector_get)() + PREFIX(config).sector_offset;
		if (sector >=6){
			sector = sector-6;
		}
		if(CLCH_NAME.current.deseired>PREFIX(config).current.max){
			CLCH_NAME.current.deseired = PREFIX(config).current.max;
		} else 
		if(CLCH_NAME.current.deseired<PREFIX(config).current.min){
			CLCH_NAME.current.deseired = PREFIX(config).current.min;
		}
		if(CLCH_NAME.current.requried < CLCH_NAME.current.deseired ){
			CLCH_NAME.current.requried32 +=  PREFIX(config).current.ramp32;
			CLCH_NAME.current.requried = (int16_t)(CLCH_NAME.current.requried32>>16);
			if( CLCH_NAME.current.requried > CLCH_NAME.current.deseired){
				CLCH_NAME.current.requried = CLCH_NAME.current.deseired ;
			}
		} else if(CLCH_NAME.current.requried > CLCH_NAME.current.deseired) {
			CLCH_NAME.current.requried32 -=  PREFIX(config).current.ramp32;
			CLCH_NAME.current.requried = (int16_t)(CLCH_NAME.current.requried32>>16);
			if( CLCH_NAME.current.requried < CLCH_NAME.current.deseired){
				CLCH_NAME.current.requried = CLCH_NAME.current.deseired ;
			}
		}
		burst_signal_t r = CLCH_NAME.current.requried;
		PREFIX(current_phase_get)(&CLCH_NAME.current.actual.A, &CLCH_NAME.current.actual.B, &CLCH_NAME.current.actual.C);
		
		switch(sector){
			case 0: 
				 PREFIX(current_actual) = CLCH_NAME.current.actual.B;
				break;
			case 1: 
				 PREFIX(current_actual) = -CLCH_NAME.current.actual.A;
				break;
			case 2: 
				 PREFIX(current_actual) = CLCH_NAME.current.actual.C;
				break;
			case 3: 
				 PREFIX(current_actual) = -CLCH_NAME.current.actual.B;
				break;
			case 4: 
				 PREFIX(current_actual) = CLCH_NAME.current.actual.A;
				break;
			case 5: 
				 PREFIX(current_actual) = -CLCH_NAME.current.actual.C;
				break;

		}
		swt_run(&CLCH_NAME.current.pi,&PREFIX(config), r,  PREFIX(current_actual));
		CLCH_NAME.pwm.actual = CLCH_NAME.current.pi.pwm.control;
		PREFIX(pwm_run_)(sector);
	}
		
	
#endif
#include "burst/cliche/_end.h"
#endif
