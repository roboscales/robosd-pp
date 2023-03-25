#include "burst/modules/drv8301.h"
unsigned int delay = 50;


void drv8301_exchange(drv8301_p _drv, uint16_t _deseired, uint16_t * _actual){
	_drv->cs_off();
	drv8301_phy_delay_ns(delay);
	_drv->put(_deseired);
	while(_drv->complete() == 0 );
	_drv->cs_on();
	drv8301_phy_delay_ns(delay);
	*_actual =_drv->get();
}

void drv8301_phy_read_(drv8301_p _drv ,uint16_t * _reg ){

	uint16_t dummy=0xFFF1;
	uint16_t answer=0xFFF1;
	uint16_t addr = ((drv8301_registr_p)_reg)->address;
	addr = addr<<11;
	uint16_t request = (1<<15) | addr;

	drv8301_exchange(_drv,request,&dummy);
	drv8301_exchange(_drv,0,&answer);
	if( (answer & 0x8000) == 0 && (answer & 0x7800)==addr){
		((drv8301_registr_p)_reg)->data = answer;
	}
}

void drv8301_phy_write_(drv8301_p _drv, uint16_t  _deseired, uint16_t * _actual){
	int rpt =3;
	do {

		uint16_t dummy=0xFFF1;
		uint16_t answer=0xFFF1;
		
		uint16_t addr = ((drv8301_registr_p)&_deseired)->address;
		uint16_t request = (1<<15) | (addr<<11);

		drv8301_exchange(_drv,_deseired,&dummy);

		drv8301_exchange(_drv,request,&dummy);
		drv8301_exchange(_drv, 0,&answer);

		if( answer==_deseired){
			(*_actual) = answer;
			return;
		}
		drv8301_phy_delay_ns(50000);
	} while(--rpt);
	drv8301_phy_crash();	
}
#include <string.h>
void drv8301_begin(drv8301_p _drv, drv8301_config_p _config){
	_drv->poll_status = DRV8301_POLL_STATUS_NONE;
	uint16_t * d = _drv->deseired.regs;
	for(int i=0; i<DRV8301_REG_COUNT; ++i,++d ){
		*d = (i<<11);
	}
	
	uint16_t * a = _drv->actual.regs;
	for(int i=0; i<DRV8301_REG_COUNT; ++i,++a ){
		*a = (i<<11)  ;
	}

	_drv->deseired.control1.GATE_CURRENT = _config->CONTROL1.GATE_CURRENT;
	_drv->deseired.control1.GATE_RESET = _config->CONTROL1.GATE_RESET;
	_drv->deseired.control1.PWM_MODE = _config->CONTROL1.PWM_MODE;
	_drv->deseired.control1.OCP_MODE = _config->CONTROL1.OCP_MODE;
	_drv->deseired.control1.OC_ADJ_SET = _config->CONTROL1.OC_ADJ_SET;


	_drv->deseired.control2.OCTW_MODE = _config->CONTROL2.OCTW_MODE;
	_drv->deseired.control2.GAIN = _config->CONTROL2.GAIN;
	_drv->deseired.control2.DC_CAL_CH1 = _config->CONTROL2.DC_CAL_CH1;
	_drv->deseired.control2.DC_CAL_CH2 = _config->CONTROL2.DC_CAL_CH2;
	_drv->deseired.control2.OC_TOFF = _config->CONTROL2.OC_TOFF;

	drv8301_phy_read_(_drv, &(_drv->actual.status1.value));
	drv8301_phy_read_(_drv, &(_drv->actual.status2.value));
	drv8301_phy_write_(_drv, _drv->deseired.control1.value, &(_drv->actual.control1.value));
	drv8301_phy_write_(_drv, _drv->deseired.control2.value, &(_drv->actual.control2.value));

}
	
void drv8301_set_default(drv8301_config_p _config){
	_config->CONTROL1.GATE_CURRENT = DRV8301_GATE_CURRENT_1700mA;
	_config->CONTROL1.GATE_RESET =  DRV8301_GATE_RESET_NORMAL_MODE;
	_config->CONTROL1.OCP_MODE = DRV8301_OCP_MODE_CURRENT_LIMIT;
	_config->CONTROL1.OC_ADJ_SET = DRV8301_OC_ADJ_SET_0060mV;
	_config->CONTROL1.PWM_MODE = DRV8301_PWM_MODE_6_INPUTS;
	
	_config->CONTROL2.DC_CAL_CH1 = DRV8301_DC_CAL_CH1_NORMAL;
	_config->CONTROL2.DC_CAL_CH2 = DRV8301_DC_CAL_CH2_NORMAL;
	_config->CONTROL2.GAIN = DRV8301_GAIN_10VdV;
	_config->CONTROL2.OCTW_MODE = DRV8301_OCTW_MODE_BOTH;
	_config->CONTROL2.OC_TOFF = DRV8301_OC_TOFF_CYCLE_BY_CYCLE;	
}
/*
void drv8301_status1_query(drv8301_p _drv){
	drv8301_phy_delay_ns(1000000);
	_drv->cs_off();
	drv8301_phy_delay_ns(delay);
	_drv->actual.status1.FAULT= 1;
	_drv->put(0x8000);
	_drv->poll_status = DRV8301_POLL_STATUS_QUERY1;
}
void drv8301_status2_query(drv8301_p _drv){
	drv8301_phy_delay_ns(1000000);
	_drv->cs_off();
	drv8301_phy_delay_ns(delay);
	_drv->actual.status2.unused1=1;
	_drv->put(0x8800);
	_drv->poll_status = DRV8301_POLL_STATUS_QUERY2;
}
*/

void drv8301_status_poll(drv8301_p _drv){
	/*if(_drv->poll_status== DRV8301_POLL_STATUS_NONE){
			drv8301_status1_query(_drv);
	} else {
		if( _drv->complete() ){
			switch(_drv->poll_status){
				case DRV8301_POLL_STATUS_QUERY1: 
				_drv->cs_on();
				drv8301_phy_delay_ns(delay);
				_drv->cs_off();
				drv8301_phy_delay_ns(delay);
				_drv->put(0);
				_drv->poll_status = DRV8301_POLL_STATUS_GET1;
				break;
				case DRV8301_POLL_STATUS_GET1: 
				_drv->cs_on();
				drv8301_phy_delay_ns(delay);
				{
					uint16_t answer = _drv->get();
					if( (answer & 0x8000) == 0 && (answer & 0x7800)==0){
						_drv->actual.status1.value=answer;
					}
				}
				drv8301_status2_query(_drv);
				break;
				case DRV8301_POLL_STATUS_QUERY2: 
				_drv->cs_on();
				drv8301_phy_delay_ns(delay);
				_drv->cs_off();
				drv8301_phy_delay_ns(delay);
				_drv->put(0);
				_drv->poll_status = DRV8301_POLL_STATUS_GET2;
				break;
				case DRV8301_POLL_STATUS_GET2: 
				_drv->cs_on();
				uint16_t answer = _drv->get();
				if( (answer & 0x8000) == 0 && (answer & 0x7800)==0x800){
					_drv->actual.status2.value=answer;
				}
				_drv->on_status_receive();
				drv8301_phy_delay_ns(delay);
				drv8301_status1_query(_drv);
				break;
				case DRV8301_POLL_STATUS_NONE: 
				break;
			}
		}
	}
	*/
	_drv->actual.status1.FAULT= 1;
	_drv->actual.status2.unused1=1;
	drv8301_phy_read_(_drv, &(_drv->actual.status1.value));
	drv8301_phy_read_(_drv, &(_drv->actual.status2.value));
	if(_drv->on_status_receive){
		_drv->on_status_receive();
	}
}
