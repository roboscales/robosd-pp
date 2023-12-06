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

#ifndef PMSM_MODE_SYNCHRO_CURRENT_BUFFER_BITS
#define PMSM_MODE_SYNCHRO_CURRENT_BUFFER_BITS 10
#endif

typedef struct pmsm_mode_hall_estimate_phase_s{
	burst_long_signal_t ae;
	burst_signal_t e;
	burst_signal_t e0;
	burst_long_signal_t af;
	burst_signal_t f;
	burst_signal_t f0;
	uint8_t h_prev;
	burst_signal_t status;
	int ne;
	int nf;
} pmsm_mode_hall_estimate_phase_t;

typedef struct pmsm_mode_hall_estimate_s{
	pmsm_mode_hall_estimate_phase_t A;
	pmsm_mode_hall_estimate_phase_t B;
	pmsm_mode_hall_estimate_phase_t C;
	pmsm_mode_hall_estimate_phase_t nA;
	pmsm_mode_hall_estimate_phase_t nB;
	pmsm_mode_hall_estimate_phase_t nC;
	int dir;
	burst_signal_t angle;
} pmsm_mode_hall_estimate_t;

pmsm_mode_hall_estimate_t pmsm_mode_hall_estimate = {};

void pmsm_mode_hall_estimate_phase_reset_(pmsm_mode_hall_estimate_phase_t * p, uint8_t _h){
	p->ae = 0;
	p->e = 0;
	p->af = 0;
	p->f = 0;
	p-> h_prev = _h;
	p->status = 50;
	p->ne = 0;
	p->nf = 0;
}



burst_signal_t angle_mean_(burst_signal_t a1, burst_signal_t a2){
	burst_signal_t d = a1 - a2;
	return  a1 + (d>>1);
}


void angle_stat_(pmsm_mode_hall_estimate_phase_t * S, burst_signal_t f_prev, burst_signal_t f, uint8_t h, int dir){
	S->status = (S->status*3 + h*100 +2)>>2;
	if(dir>0){
    if(S->h_prev < h ){
			if(S->status<=50){
				if(S->ne == 0){
					S->ae = 0;
					S->e0 = f;
				} else{
					burst_signal_t tmp = angle_mean_(f,f_prev);
					burst_signal_t diff = tmp - S->e0;
					S->ae = S->ae + diff;
				}
				S->ne = S->ne + 1;
			}
		} else {
			if(S->h_prev > h ){
				if(S->status>=50){
					if(S->nf == 0){
						S->af = 0;
						S->f0 = f;
					} else {
						burst_signal_t tmp = angle_mean_(f,f_prev);
						burst_signal_t diff = tmp - S->f0;
						S->af = S->af + diff;
					}
					S->nf = S->nf + 1;
				}
			}
		}
	} else {
		if(dir<0){
			if(S->h_prev < h ){
        if(S->status<=50){
					if(S->nf == 0){
						S->af = 0;
						S->f0 = f;
					} else {
						burst_signal_t tmp = angle_mean_(f,f_prev);
						burst_signal_t diff = tmp - S->f0;
						S->af = S->af + diff;
					}
          S->nf = S->nf + 1;
        }
			} else {
				if(S->h_prev > h ){
					if(S->status>=50){
						if(S->ne == 0){
							S->ae = 0;
							S->e0 = f;
						} else{
							burst_signal_t tmp = angle_mean_(f,f_prev);
							burst_signal_t diff = tmp - S->e0;
							S->ae = S->ae + diff;
						}
            S->ne = S->ne + 1;
					}
				}
			}
		}
	}
	S->h_prev = h;
}

void angle_estimate_(pmsm_mode_hall_estimate_phase_t * S){
	S->e =  S->e0 + (S->ae+ (S->ne>>1)) / S->ne;
	S->f =  S->f0 +(S->af+ (S->nf>>1)) / S->nf;
}

void pmsm_mode_hall_estimate_begin_(void){
	pmsm_mode_hall_estimate_phase_reset_( &(pmsm_mode_hall_estimate.A) ,hall.pins.A);
	pmsm_mode_hall_estimate_phase_reset_( &(pmsm_mode_hall_estimate.B) ,hall.pins.B);
	pmsm_mode_hall_estimate_phase_reset_( &(pmsm_mode_hall_estimate.C) ,hall.pins.C);
	pmsm_mode_hall_estimate.dir = 1;
}
void pmsm_mode_hall_estimate_revert_(void){
	angle_estimate_(&(pmsm_mode_hall_estimate.nA));
	pmsm_mode_hall_estimate_phase_reset_( &(pmsm_mode_hall_estimate.nA) ,hall.pins.A);
	pmsm_mode_hall_estimate_phase_reset_( &(pmsm_mode_hall_estimate.nB) ,hall.pins.B);
	pmsm_mode_hall_estimate_phase_reset_( &(pmsm_mode_hall_estimate.nC) ,hall.pins.C);
	pmsm_mode_hall_estimate.dir = -1;
}

void pmsm_mode_hall_estimate_finish_(void){
	angle_estimate_(&(pmsm_mode_hall_estimate.A));
	angle_estimate_(&(pmsm_mode_hall_estimate.B));
	angle_estimate_(&(pmsm_mode_hall_estimate.C));
	angle_estimate_(&(pmsm_mode_hall_estimate.nA));
	angle_estimate_(&(pmsm_mode_hall_estimate.nB));
	angle_estimate_(&(pmsm_mode_hall_estimate.nC));	
}

void pmsm_mode_hall_estimate_applay_action(burst_dev_ref_p _ref){	
	pmsm_mode_synchro_current_applay_action(_ref);
	
}

void pmsm_mode_hall_estimate_start(burst_dev_ref_p _ref){
	pmsm_mode_hall_estimate_begin_();
	pmsm_mode_synchro_current_start(_ref);
}

void pmsm_mode_hall_estimate_stop(burst_dev_ref_p _ref){
	pmsm_mode_hall_estimate_finish_();
	pmsm_mode_synchro_current_stop(_ref);
}
pmsm_hall_app_config_p pmsm_hall_app_config = 0;
burst_bool_t pmsm_mode_hall_estimate_end_(void){
	if(pmsm_mode_hall_estimate.dir > 0){
		if(
			pmsm_mode_hall_estimate.A.ne >  pmsm_hall_app_config->estimate_revert_count
			&& pmsm_mode_hall_estimate.A.nf >  pmsm_hall_app_config->estimate_revert_count
			&& pmsm_mode_hall_estimate.B.ne >  pmsm_hall_app_config->estimate_revert_count
			&& pmsm_mode_hall_estimate.B.nf >  pmsm_hall_app_config->estimate_revert_count
			&& pmsm_mode_hall_estimate.C.ne >  pmsm_hall_app_config->estimate_revert_count
			&& pmsm_mode_hall_estimate.C.nf >  pmsm_hall_app_config->estimate_revert_count
			){
				return burst_true;
		}
	} else if(pmsm_mode_hall_estimate.dir < 0){
		if(
			pmsm_mode_hall_estimate.nA.ne >  pmsm_hall_app_config->estimate_revert_count
			&& pmsm_mode_hall_estimate.nA.nf >  pmsm_hall_app_config->estimate_revert_count
			&& pmsm_mode_hall_estimate.nB.ne >  pmsm_hall_app_config->estimate_revert_count
			&& pmsm_mode_hall_estimate.nB.nf >  pmsm_hall_app_config->estimate_revert_count
			&& pmsm_mode_hall_estimate.nC.ne >  pmsm_hall_app_config->estimate_revert_count
			&& pmsm_mode_hall_estimate.nC.nf >  pmsm_hall_app_config->estimate_revert_count
			){
				return burst_true;
		}
	}
	return burst_false;
}

void pmsm_mode_hall_estimate_runA(burst_dev_ref_p _ref){
	pmsm_mode_synchro_current_runA(_ref);
	burst_signal_t angle = (motor.synchro.angle)>>16;
	if(pmsm_mode_hall_estimate.dir>0){
		angle_stat_(&pmsm_mode_hall_estimate.A, pmsm_mode_hall_estimate.angle,angle, hall.pins.A, 1);
		angle_stat_(&pmsm_mode_hall_estimate.B, pmsm_mode_hall_estimate.angle,angle, hall.pins.B, 1);
		angle_stat_(&pmsm_mode_hall_estimate.C, pmsm_mode_hall_estimate.angle,angle, hall.pins.C, 1);
	} else{
		angle_stat_(&pmsm_mode_hall_estimate.nA, pmsm_mode_hall_estimate.angle,angle, hall.pins.A, -1);
		angle_stat_(&pmsm_mode_hall_estimate.nB, pmsm_mode_hall_estimate.angle,angle, hall.pins.B, -1);
		angle_stat_(&pmsm_mode_hall_estimate.nC, pmsm_mode_hall_estimate.angle,angle, hall.pins.C, -1);
	}
	pmsm_mode_hall_estimate.angle = angle;
}

void pmsm_mode_hall_estimate_check(burst_dev_ref_p _ref){
	if(pmsm_mode_hall_estimate_end_() ==  burst_true){
		if(pmsm_mode_hall_estimate.dir > 0 ){
			pmsm_mode_hall_estimate_revert_();
			motor.synchro.freq = -motor.synchro.freq;
		} else{
			motor.cross.ac.ref.action->mode = actuator_mode_idle_ix;
		}
	}
}
burst_dev_mode_t pmcm_synchro_hall_statistic_current = {
	&pmsm_mode_hall_estimate_applay_action
	,&pmsm_mode_hall_estimate_start
	,&pmsm_mode_hall_estimate_stop
	,&pmsm_mode_hall_estimate_runA
	,&burst_dev_idle_event
	,&burst_dev_idle_event
	,&pmsm_mode_hall_estimate_check
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
	, &pmcm_synchro_hall_statistic_current
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
	pmsm_hall_app_config = _config;
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
	if(motor.cross.ac.ref.mode == bldc_mode_pwm_ix){
			swt_pwm_run();
	}
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
		burst_long_signal_t delta = pos - last_rpm_pos;
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
