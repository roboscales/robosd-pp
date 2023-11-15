#include "burst\modules\pmsm_hall_app.h"
#include "burst\burst_timer.h"
hall_t hall={};
hall_extra_t hall_extra ={};
adc_t adc={};
pmsm_angle_forcer_t angle_forcer={};
pmsm_t motor={};
burst_long_signal_t RPM = 0;

void bldc_mode_pwm_applay_action(burst_dev_ref_p _ref){		
}

void bldc_mode_pwm_start(burst_dev_ref_p _ref){
	swt_begin();
	swt_pwm_start();
}
void bldc_mode_pwm_stop(burst_dev_ref_p _ref){
	swt_pwm_stop();
}
void bldc_mode_pwm_runA(burst_dev_ref_p _ref){
	swt_pwm_run();
	rotcalc( &(motor.inverter.rot), hall.angle );
}

burst_dev_mode_t bldc_mode_pwm = {
	&bldc_mode_pwm_applay_action
	,&bldc_mode_pwm_start
	,&bldc_mode_pwm_stop
	,&bldc_mode_pwm_runA
	,&burst_dev_idle_event
	,&burst_dev_idle_event
	,&burst_dev_idle_event
};


void bldc_mode_current_applay_action(burst_dev_ref_p _ref){		
	swt_current_mode_applay();
}

void bldc_mode_current_start(burst_dev_ref_p _ref){
	swt_begin();
	swt_current_start();
}
void bldc_mode_current_stop(burst_dev_ref_p _ref){
	swt_current_stop();
}

void swt_current_phase_get(int16_t * _A, int16_t * _B, int16_t * _C){
	*_A  = motor.sensor.abc.A;
	*_B  = motor.sensor.abc.B;
	*_C  = motor.sensor.abc.C;
}

void bldc_mode_current_runA(burst_dev_ref_p _ref){
	swt_current_run();
	rotcalc( &(motor.inverter.rot), hall.angle );
}

burst_dev_mode_t bldc_mode_current = {
	&bldc_mode_current_applay_action
	,&bldc_mode_current_start
	,&bldc_mode_current_stop
	,&bldc_mode_current_runA
	,&burst_dev_idle_event
	,&burst_dev_idle_event
	,&burst_dev_idle_event
};


burst_dev_mode_p pmsm_hall_app_modes[ pmsm_mode_count] = {
	&burst_idle_mode
	, &actuator_mode_fault
	, &actuator_mode_voltage
	, &actuator_mode_speed
	, &actuator_mode_position
	, &acwc_mode_voltage_cl
	, &acwc_mode_cl_speed
	, &acwc_mode_cl_position
	, &acwc_mode_current
	, &acwc_mode_speed
	, &acwc_mode_position
	, &pmcm_synchro_voltage
	, &pmcm_synchro_current
	, &burst_idle_mode
	, &bldc_mode_pwm
	, &bldc_mode_current
};


#if BURST_PROTECTION_ENABLED == 1 
#if BURST_PANICS_ACTUATOR_TEMPER_ENABLED == 1 
BURST_WEAK burst_signal_t pmsm_hall_app_motor_temp(void){
	return 0;
}
#endif
#if BURST_PANICS_ACWC_OVERCURRENT_ENABLED ==1
burst_signal_t pmsm_hall_app_motor_current_magnitude(void){
	return burst_pmsm_magnitude_get(&motor);
}
#endif
#endif

void pmsm_hall_app_begin(pmsm_hall_app_config_p _config, pmsm_action_p _action, pmsm_feedback_p _feedback ){
	hall_begin(&hall, &_config->hall);
	adc_begin(&adc, &_config->adc);
	pmsm_begin(
		&motor//pmsm_p _pmsm
		, &(_config->pmsm)//pmsm_config_p _config
		, _action//pmsm_action_p _action
		, _feedback//pmsm_feedback_p _feedback
		, &power//burst_ps_p _ps
		, &enco.ref
		, &speedse.ref
		, &motion
		, &positioner	
		, pmsm_mode_count
		, pmsm_hall_app_modes	
		, &c_cross_flt.ref
		, &c_lat_flt.ref
		,	&c_cross_pi
		,	&c_lat_pi
		,	&c_hi_pi
		,	&c_lo_pi
		, adc.values
		, &angle_forcer.ref
	);
	nikitin_begin(&speedse, &_config->speedse);
	nikitin_begin(&c_cross_flt,&_config->cross_flt);
	nikitin_begin(&c_lat_flt,&_config->lat_flt);
	burst_ps_begin(&power, 0);
	
	hall_extra.hall.pactual = &hall.angle;
	hall_extra.lost = burst_true;
	//angle_forcer.angle.raw = &hall_extra.angle;
	//angle_forcer.speed = &speedse.ref.value;
	
	enco_abs32_begin(&enco, &_config->enco);
	#if PMSM_HALL_APP_ANGLE_SENCE_TYPE == PMSM_HALL_APP_ANGLE_SENCE_TYPE_EXTERN
	pmsm_angle_forcer_begin(&angle_forcer, &_config->angle_forcer,pmsm_hall_app_rotor_pos(),  &speedse.ref.value, 0);
	#endif
	#if PMSM_HALL_APP_ANGLE_SENCE_TYPE == PMSM_HALL_APP_ANGLE_SENCE_TYPE_HALL
	pmsm_angle_forcer_begin(&angle_forcer, &_config->angle_forcer,&hall_extra.angle,  &speedse.ref.value, 0);
	#endif
	
	#if BURST_PROTECTION_ENABLED == 1 
	motor.cross.ac.ref.realtime_protection = burst_pmsm_realtime_protection;
	motor.cross.ac.ref.frontend_protection = burst_pmsm_frontend_protection;
	#if BURST_PANICS_ACTUATOR_TEMPER_ENABLED == 1
	motor.cross.ac.temper_pp = pmsm_hall_app_motor_temp;
	#endif
	#if BURST_PANICS_ACWC_OVERCURRENT_ENABLED ==1
	motor.cross.current.magnitude.get = pmsm_hall_app_motor_current_magnitude;
	#endif
	#endif
}

void pmsm_hall_app_start(void){
	while( adc.ready == burst_false ){
		BURST_NOP();
	}	
}

void pmsm_hall_app_realtime_loop(void){	
	enco.ref.run();
	pmsm_angle_forcer_run(&angle_forcer);
	pmsm_sence_run(&motor);
	c_cross_flt.ref.run();
	c_lat_flt.ref.run();
}

void pmsm_hall_app_backend_loop(void){		
	burst_dev_runA(&motor.cross.ac.ref);	
	#if PMSM_HALL_APP_EXTRA_TYPE == PMSM_HALL_APP_EXTRA_TYPE_NONE
	hall_dummy_interp(&hall_extra);
	#else
	hall_qubic_interp(&hall_extra);
	#endif
	pmsm_inverter_run(&motor);
	power.run();
}
void pmsm_hall_app_frontend_loop(void){
	#if PMSM_HALL_APP_EXTRA_TYPE == PMSM_HALL_APP_EXTRA_TYPE_REGRESS
	hall_regres_poll(&hall_extra);
	#endif
}
void pmsm_hall_app_update_feedback(void){
	burst_dev_ref_p r = (burst_dev_ref_p)&motor;
	r->update_feedback(r);
}
static int pmsm_hall_app_presc = 0;
void pmsm_hall_app_control_step_1(void){
	pmsm_hall_app_presc++;
	if(pmsm_hall_app_presc== ((pmsm_hall_app_config_p)( motor.cross.ac.ref.config))->controlPresc){
		speedse.ref.run();
		enco.ref.delta_acc = 0;
		pmsm_hall_app_presc = 0;
	}
}
void pmsm_hall_app_control_step_2(void){
	if(pmsm_hall_app_presc==0){
		burst_dev_runB(&motor.cross.ac.ref);
	}
}

#ifndef PMSM_HALL_RPM_FILTER_VALUE_PRESC
#define PMSM_HALL_RPM_FILTER_VALUE_PRESC 5
#endif
#ifndef PMSM_HALL_RPM_VALUE_TOTAL_PRESC
#define PMSM_HALL_RPM_VALUE_TOTAL_PRESC 10
#endif
#ifndef PMSM_HALL_RPM_ENCODER_DEFRES 
#define PMSM_HALL_RPM_ENCODER_DEFRES 8
#endif

#ifndef PMSM_HALL_RPM_BASE_PERIOD_US 
#define PMSM_HALL_RPM_BASE_PERIOD_US BURST_TIMER_TICK_US
#endif

#ifndef PMSM_HALL_RPM_FILTER_GAIN_PRESC 
#define PMSM_HALL_RPM_FILTER_GAIN_PRESC 5
#endif

#define PMSM_HALL_RPM_GAIN ( ((int32_t)( (60000000./PMSM_HALL_RPM_BASE_PERIOD_US))>>enco_RESOLUTION_ACTUAL))

#define PMSM_HALL_RPM_PRESC (PMSM_HALL_RPM_VALUE_TOTAL_PRESC - PMSM_HALL_RPM_FILTER_VALUE_PRESC + PMSM_HALL_RPM_ENCODER_DEFRES - enco_RESOLUTION_ACTUAL) 



void pmsm_hall_app_control_step_3(void){
	static burst_time_us_t last_rpm_us = 0;
	static burst_long_signal_t last_rpm_pos = 0;
	static burst_long_signal_t delta_flt = 0;
	burst_time_us_t now = burst_time_us();
	if(  now - last_rpm_us  >= (1<<PMSM_HALL_RPM_PRESC)*PMSM_HALL_RPM_BASE_PERIOD_US ){
		burst_long_signal_t pos = enco.ref.position;
		burst_signal_t delta = pos - last_rpm_pos;
		delta_flt = delta_flt*((1<<PMSM_HALL_RPM_FILTER_GAIN_PRESC)-1) + delta*(1<<PMSM_HALL_RPM_FILTER_VALUE_PRESC);
		delta_flt >>= PMSM_HALL_RPM_FILTER_GAIN_PRESC;
		//60*1000000 / 256 (pp.об) / 16 = 14648
		
		RPM = (delta_flt * PMSM_HALL_RPM_GAIN ) >> (PMSM_HALL_RPM_FILTER_VALUE_PRESC+PMSM_HALL_RPM_PRESC);			
		last_rpm_us = now;
		last_rpm_pos = pos;
	}
}

uint32_t enco_encode(void){
	return (uint32_t)hall.angle;
}
