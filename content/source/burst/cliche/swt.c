#include "swt.h"

void swt_reset(burst_swt_t * _swt, int _pwm){
	_swt->pwm.control = _swt->pwm.actual = _pwm;
	_swt->model = _pwm<<7;
	_swt->model32 = _swt->model << 10;
}

void swt_run(burst_swt_t * _swt, burst_swt_config_t * _config,int _requried,  int _actual){
	int error = _requried - _actual;
	int model;
	if ( !( (error>0 && _swt->pwm.control == _config->pwm.max ) || (error<0 && _swt->pwm.control == 0))){
		int model32 = _swt->model32 + error * _config->gain.model;
		model = model32 >> 10;
		_swt->model = model;
		_swt->model32 = model32;
	} else{
		model = _swt->model32 >> 10;
	}
	int pwm = ((error  - _actual) * _config->gain.prop + model ) >> 7;
	if( pwm < _swt->pwm.min) {
		pwm = _swt->pwm.min;
	} else if ( pwm > _swt->pwm.max) {
		pwm = _swt->pwm.max;
	}
	_swt->pwm.control = pwm;	
	_swt->error = error;
}

void swt_set_control(burst_swt_t * _swt, burst_swt_config_t * _config){
	if( _swt->pwm.control>=0){
		_swt->pwm.actual = _swt->pwm.control;
	} else {		
		_swt->pwm.actual = (uint16_t)((int16_t)_config->pwm.max + _swt->pwm.control);
	}
}
#if 0

int swt_pwm = 899;
int swt_cur_req = 100;

//int swt_model_reset2(swt_config_t * _conf,  int _pwm1, int _pwm2 ){
	//return (((_pwm1 + _pwm2) >> 1) *_conf->gain.iprop10)>>10 ;
//}

//int swt_model_reset(swt_config_t * _conf,  int _pwm){
	//return ( _pwm *_conf->gain.iprop10 ) >> 10 ;
//}



swt_t swtA={};
swt_t swtB={};
swt_t swtC={};
swt_config_t swt_cfg={
	{
		15
		,2000
	}
};
//motor.sensor.abc.A
int swt_pwmA = 0;
int swt_pwmB = 0;
int swt_pwmC = 0;

int swm_sector_offset = 0;
void swt_loop(void){
	if(motor.cross.ac.ref.panic){
		swt_enable = 0;
	}
	
	if(swt_enable){
		int sector = hall.sector+swm_sector_offset;
		if (sector >=6){
			sector = sector-6;
		}
		static int  sector_prev = -1;
		int sectror_change = sector != sector_prev ;
		sector_prev = sector;
		
		if(swt_enable == 1 ){
			switch(sector){
				case 0: //A
					TIM1->CCER = 0x1055;
					swt_pwmA = 0;
					if(sectror_change){
						swt_reset(&swtB,swtA.pwm>>1);
					}
					swt_run(&swtB,&swt_cfg, swt_cur_req, motor.sensor.abc.B);
					swt_pwmB = swtB.pwm;
					swt_pwmC = 0;
					break;
				case 1://-C
					TIM1->CCER = 0x1550;
					if(sectror_change){
						swt_reset(&swtB,swtB.pwm>>1);
					}
					swt_pwmA = 0;
					swt_run(&swtB,&swt_cfg, swt_cur_req, motor.sensor.abc.B);
					swt_pwmB = swtB.pwm;
					swt_pwmC = 0;
					break;
				case 2://B
					TIM1->CCER = 0x1505;
					swt_pwmA = 0;
					swt_pwmB = 0;
					if(sectror_change){
						swt_reset(&swtC,swtB.pwm>>1);
					}
					swt_run(&swtC,&swt_cfg, swt_cur_req, motor.sensor.abc.C);
					swt_pwmC = swtC.pwm;
					break;
				case 3://-A
					TIM1->CCER = 0x1055;
					swt_pwmA = 0;
					swt_pwmB = 0;
					if(sectror_change){
						swt_reset(&swtC,swtC.pwm>>1);
					}
					swt_run(&swtC,&swt_cfg, swt_cur_req, motor.sensor.abc.C);
					swt_pwmC = swtC.pwm;
					break;
				case 4://C
					TIM1->CCER = 0x1550;
					if(sectror_change){
						swt_reset(&swtA,swtC.pwm>>1);
					}
					swt_run(&swtA,&swt_cfg, swt_cur_req, motor.sensor.abc.A);
					swt_pwmA = swtA.pwm;
					swt_pwmB = 0;
					swt_pwmC = 0;
					break;
				case 5: //-B
					TIM1->CCER = 0x1505;
					if(sectror_change){
						swt_reset(&swtA,swtA.pwm>>1);
					}
					swt_run(&swtA,&swt_cfg, swt_cur_req, motor.sensor.abc.A);
					swt_pwmA = swtA.pwm;
					//swtB.pwm = swtC.pwm 
					swt_pwmB = 0;
					swt_pwmC = 0;

					break;
			}
		}
		
	if(swt_enable == 2 ){
			switch(sector){
				case 0: //A
					TIM1->CCER = 0x1055;
					swt_pwmA = swt_pwm>>1;
					swt_pwmB = swt_pwm;
					swt_pwmC = 0;
					break;
				case 1://-C
					TIM1->CCER = 0x1550;
					swt_pwmA = 0;
					swt_pwmB = swt_pwm;
					swt_pwmC = swt_pwm>>1;
					break;
				case 2://B
					TIM1->CCER = 0x1505;
					swt_pwmA = 0;
					swt_pwmB = swt_pwm>>1;
					swt_pwmC = swt_pwm;
					break;
				case 3://-A
					TIM1->CCER = 0x1055;
					swt_pwmA = swt_pwm>>1;
					swt_pwmB = 0;
					swt_pwmC =  swt_pwm;
					break;
				case 4://C
					TIM1->CCER = 0x1550;
					swt_pwmA = swt_pwm;
					swt_pwmB = 0;
					swt_pwmC = swt_pwm>>1;
					break;
				case 5: //-B
					TIM1->CCER = 0x1505;
					swt_pwmA = swt_pwm;
					swt_pwmB = swt_pwm>>1;
					swt_pwmC = 0;

					break;
			}
		}
		
		TIM1->CCR3 = swt_pwmA;
		TIM1->CCR2 = swt_pwmB;
		TIM1->CCR1 = swt_pwmC;
		if(swt_enable_prev == 0){
			//TIM1->CCER = 0x1555;
			//SD_GPIO_Port->BRR = SD_Pin;
		}

	} else{
		if(swt_enable_prev){
			//SD_GPIO_Port->BSRR = SD_Pin;
			TIM1->CCR1 = 0;
			TIM1->CCR2 = 0;
			TIM1->CCR3 = 0;
			TIM1->CCER = 0x1000;
			swt_reset(&swtA,0);
			swt_reset(&swtB,0);
			swt_reset(&swtC,0);
		}
	}
	swt_enable_prev = swt_enable;
}
#endif
