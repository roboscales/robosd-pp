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
	pmsm_angle_forcer_begin(&angle_forcer, &_config->angle_forcer,&hall_extra.angle,  &speedse.ref.value, 0);
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
		if(  now - last_rpm_us  >= 1024*16 ){
			burst_long_signal_t pos = enco.ref.position;
			burst_long_signal_t delta = pos - last_rpm_pos;
			delta_flt = delta_flt*31 + delta*32;
			delta_flt >>= 5;
			//60*1000000 / 256 (pp.об) / 16 = 14648
			
			RPM = (14648L * delta_flt) >> (15+( ((enco_abs32_config_p)(enco.ref.config))->resolution.actual - 8 ) );			
			last_rpm_us = now;
			last_rpm_pos = pos;
		}
	}
}

uint32_t enco_encode(void){
	return (uint32_t)hall.angle;
}

#if 0
void machine(void){
	if(requried.mode != actual.mode){
		actual.mode = requried.mode;
		switch(actual.mode){
			case 0:
				power_command = clch_ps_command_off;
				break;
			case 1:
				power_command = clch_ps_command_on;
				actual.angle32 = 0;
				actual.angle = 0;
				break;
			case 2:
				power_command = clch_ps_command_on;
				break;
			case 3:
				power_command = clch_ps_command_on;
				break;
		}		
	}
	
	switch(actual.mode){
			case 1:
			actual.angle32 += requried.freq;
			actual.angle =  actual.angle32 >> 16;
			inv3ph_run(&inverter,requried.voltage.cross, requried.voltage.lateral, actual.angle);
			break;
			case 2:
			{
				actual.angle =  hall.angle;
				if(inverter.dq.cross<requried.voltage.cross){
					inverter.dq.cross++;
				} else if(inverter.dq.cross>requried.voltage.cross){
					inverter.dq.cross--;
				}
				burst_signal_t v = BURST_ABS(250 * speedse.ref.value);
				if(inverter.dq.cross>0){
					if(v>inverter.dq.cross){
						v=inverter.dq.cross;
					}
				} else if(inverter.dq.cross<0) {
					if(v<inverter.dq.cross){
						v=inverter.dq.cross;
					}				
				} else v = 0;
				inverter.dq.lateral = -v;
				//inv3ph_run(&inverter,hall_extra.angle32>>16);
			}
			break;
			case 3:
			{
				actual.angle =  hall.angle;
				eds = 1200L*speedse.ref.value;				
				voltage_min = eds-6000;
				voltage_max = eds+6000;
				if(requried.voltage.cross>voltage_max){
					inverter.dq.cross = voltage_max;
				} else if(requried.voltage.cross<voltage_min){
					inverter.dq.cross = voltage_min;
				}  else{
					if(inverter.dq.cross<requried.voltage.cross){
						inverter.dq.cross++;
					} else if(inverter.dq.cross>requried.voltage.cross){
						inverter.dq.cross--;
					}
				}
				lat_current_pi.run();				
				force_angle = (3823L* speedse.ref.value)>>5;
				//inv3ph_run(&inverter,(hall_extra.angle32>>16) + force_angle);
			}
			break;
	}

	current3ph_run(&curse);
	hall_qubic_interp(&hall_extra);
}


void burst_sw_begin(void){
	hall_begin(&hall, &burst_config.hall);
	adc_begin(&adc, &burst_config.adc);
	inv3ph_begin(&inverter, &burst_config.inv3ph);
	current3ph_begin(&curse, &burst_config.current3ph, &inverter, adc.values );
	nikitin_begin(&speedse,&burst_config.speedse);
	power_status = clch_ps_status_off;
	hall_extra.hall.pactual = &hall.angle;
	hall_extra.lost = burst_true;
	lat_current_pi.setup(
	&burst_config.lat_current_pi 				//burst_pi_config_p _config
	,	&requried.current.lateral 	//burst_signal_p				_signal_req
	, &curse.dq.lateral 					//burst_signal_p				_signal
	, &inverter.dq.lateral				//burst_signal_p			  _control
	, 0														//burst_signal_t 				_start_control
	, &power_satstate							//burst_satstate_t *		_master_sut_flag
	, 0														//burst_signal_p				_controlMax
	, 0														//burst_signal_p				_controlMin
	, 0														//burst_signal_p				_signal_diff
	, 0														//burst_signal_p				_signal_force
	);
}
void burst_sw_start(void){
	adc_start();
	while( adc.ready == burst_false ){
		BURST_NOP();
	}

}

void burst_sw_realtime_loop(void){	
	power_run();
	fm_recorder();
}

void burst_sw_backend_loop(void){	
	power_run();
	fm_recorder();
	/*if(power_command == clch_ps_command_on){
		power_command = clch_ps_command_off;
	}*/
	//inv3ph_test();
	machine();
	fm_poll();
}
void burst_sw_frontend_loop(void){	
	hall_regres_poll(&hall_extra);
}
void burst_sw_slot_0(void){
	speedse.ref.run();
	hall.delta_acc = 0;
}
#endif
