#include "burst/burst_common.h"
#include "burst/cliche/_begin.h"
#ifdef CLCH_HEADER
#include "burst/modules/drv8301_reg.h"
void PREFIX(exchange)(uint16_t _deseired, uint16_t * _actual);
void PREFIX(begin)(drv8301_config_p _config);
void PREFIX(set_default)(drv8301_config_p _config);
void PREFIX(status_poll)(void);
void PREFIX(delay)(void);
void PREFIX(crash)(void);
void PREFIX(cs_on)(void);
void PREFIX(cs_off)(void);
void PREFIX(put)(uint16_t);
uint16_t PREFIX(get)(void);
int PREFIX(complete)(void);
void PREFIX(on_status_receive)(void);
#else
#include "burst/modules/drv8301_reg.h"
BURST_WEAK void PREFIX(cs_on)(void){
}
BURST_WEAK void PREFIX(cs_off)(void){
}
BURST_WEAK void PREFIX(put)(uint16_t _data){
	BURST_UNUSED(_data);
}
BURST_WEAK uint16_t PREFIX(get)(void){
	return 0xFFFF;
}
BURST_WEAK int PREFIX(complete)(void){
	return 0;
}
BURST_WEAK void PREFIX(on_status_receive)(void){
}
BURST_WEAK void  PREFIX(delay)(void){
}
BURST_WEAK void PREFIX(crash)(void){
}

void PREFIX(exchange)(uint16_t _deseired, uint16_t * _actual){
	PREFIX(cs_off)();
	PREFIX(delay)();
	PREFIX(put)(_deseired);
	while(PREFIX(complete)() == 0 );
	PREFIX(cs_on)();
	PREFIX(delay)();
	*_actual =PREFIX(get)();
}
drv8301_regs_t PREFIX(actual);
drv8301_regs_t PREFIX(deseired);
drv8301_poll_status_t PREFIX(poll_status);

void  PREFIX(phy_read_)(uint16_t * _reg ){
	uint16_t dummy=0xFFF1;
	uint16_t answer=0xFFF1;
	uint16_t addr = ((drv8301_registr_p)_reg)->address;
	addr = addr<<11;
	uint16_t request = (1<<15) | addr;

	PREFIX(exchange)(request,&dummy);
	PREFIX(exchange)(0,&answer);
	if( (answer & 0x8000) == 0 && (answer & 0x7800)==addr){
		((drv8301_registr_p)_reg)->data = answer;
	}
}

void PREFIX(phy_write_)(uint16_t  _deseired, uint16_t * _actual){
	int rpt =10;
	do {

		uint16_t dummy=0xFFF1;
		uint16_t answer=0xFFF1;
		
		uint16_t addr = ((drv8301_registr_p)&_deseired)->address;
		uint16_t request = (1<<15) | (addr<<11);

		PREFIX(exchange)(_deseired,&dummy);

		PREFIX(exchange)(request,&dummy);
		PREFIX(exchange)(0,&answer);

		if( answer==_deseired){
			(*_actual) = answer;
			return;
		}
		for(int i=0;i<1000;++i) PREFIX(delay)();
	} while(--rpt);
	PREFIX(crash)();	
}

void PREFIX(begin)(drv8301_config_p _config){
	PREFIX(poll_status) = DRV8301_POLL_STATUS_NONE;
	uint16_t * d = PREFIX(deseired).regs;
	for(int i=0; i<DRV8301_REG_COUNT; ++i,++d ){
		*d = (i<<11);
	}
	
	uint16_t * a = PREFIX(actual).regs;
	for(int i=0; i<DRV8301_REG_COUNT; ++i,++a ){
		*a = (i<<11)  ;
	}

	PREFIX(deseired).control1.GATE_CURRENT = _config->CONTROL1.GATE_CURRENT;
	PREFIX(deseired).control1.GATE_RESET = _config->CONTROL1.GATE_RESET;
	PREFIX(deseired).control1.PWM_MODE = _config->CONTROL1.PWM_MODE;
	PREFIX(deseired).control1.OCP_MODE = _config->CONTROL1.OCP_MODE;
	PREFIX(deseired).control1.OC_ADJ_SET = _config->CONTROL1.OC_ADJ_SET;


	PREFIX(deseired).control2.OCTW_MODE = _config->CONTROL2.OCTW_MODE;
	PREFIX(deseired).control2.GAIN = _config->CONTROL2.GAIN;
	PREFIX(deseired).control2.DC_CAL_CH1 = _config->CONTROL2.DC_CAL_CH1;
	PREFIX(deseired).control2.DC_CAL_CH2 = _config->CONTROL2.DC_CAL_CH2;
	PREFIX(deseired).control2.OC_TOFF = _config->CONTROL2.OC_TOFF;

	PREFIX(phy_read_)( &(PREFIX(actual).status1.value));
	PREFIX(phy_read_)( &(PREFIX(actual).status2.value));
	PREFIX(phy_write_)( PREFIX(deseired).control1.value, &(  PREFIX(actual).control1.value));
	PREFIX(phy_write_)( PREFIX(deseired).control2.value, &(  PREFIX(actual).control2.value));
}

void PREFIX(set_default)(drv8301_config_p _config){
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

void PREFIX(status_poll)(void){
	PREFIX(actual).status1.FAULT= 1;
	PREFIX(actual).status2.unused1=1;
	PREFIX(phy_read_)(&(PREFIX(actual).status1.value));
	PREFIX(phy_read_)(&(PREFIX(actual).status2.value));
	PREFIX(on_status_receive)();
}

#endif
#include "burst/cliche/_end.h"
