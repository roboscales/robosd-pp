#include "burst\modules\pmsm_hall_app.h"
#include "burst\burst_timer.h"
hall_t hall={};
hall_extra_t hall_extra ={};
adc_t adc={};
pmsm_angle_forcer_t angle_forcer={};
pmsm_t motor={};
burst_long_signal_t RPM = 0;
	
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
};


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
}

void pmsm_hall_app_start(void){
	while( adc.ready == burst_false ){
		BURST_NOP();
	}	
}

void pmsm_hall_app_realtime_loop(void){	
	c_cross_flt.ref.run();
	c_lat_flt.ref.run();
	pmsm_angle_forcer_run(&angle_forcer);
	pmsm_sence_run(&motor);
	pmsm_protector_run (&motor);
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
void pmsm_hall_app_control_step_1(void){
	enco.ref.run();
}
static int presc = 0;
void pmsm_hall_app_control_step_2(void){
	presc++;
	if(presc== ((pmsm_hall_app_config_p)( motor.cross.ac.ref.config))->controlPresc){
		speedse.ref.run();
		enco.ref.delta_acc = 0;
		presc = 0;
	}
}
void pmsm_hall_app_control_step_3(void){
	if(presc==0){
		burst_dev_runB(&motor.cross.ac.ref);
	}
	{
		static burst_time_us_t last_rpm_us = 0;
		static burst_long_signal_t last_rpm_pos = 0;
		static burst_long_signal_t delta_flt = 0;
		burst_time_us_t now = burst_time_us();
		if(  now - last_rpm_us  >= (1024)*16 ){
			burst_long_signal_t pos = enco.ref.position;
			burst_signal_t delta = pos - last_rpm_pos;
			delta_flt = delta_flt*31 + delta*32;
			delta_flt >>= 5;
			//60*1000000 / 256 (pp.об) / 16 = 14648
			
			RPM = (14648L * delta_flt) >> (15+( ((enco_abs32_config_p)(enco.ref.config))->resolution.actual - 8   )  );			
			last_rpm_us = now;
			last_rpm_pos = pos;
		}
	}
}

uint32_t enco_encode(void){
	return (uint32_t)hall.angle;
}
