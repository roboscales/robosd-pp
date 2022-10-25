#include "burst/modules/drv8711.h"
void drv8711_set_default(drv8711_config_p _config){
	_config->CTRL.ENBL= DRV8711_CTRL_ENBL_DEFAULT;
	_config->CTRL.RDIR= DRV8711_CTRL_RDIR_DEFAULT;
	_config->CTRL.RSTEP= DRV8711_CTRL_RSTEP_DEFAULT;
	_config->CTRL.MODE= DRV8711_CTRL_MODE_DEFAULT;
	_config->CTRL.EXSTALL= DRV8711_CTRL_EXSTALL_DEFAULT;
	_config->CTRL.ISGAIN= DRV8711_CTRL_ISGAIN_DEFAULT;
	_config->CTRL.DTIME= DRV8711_CTRL_DTIME_DEFAULT;

	_config->TORQUE.TORQUE= DRV8711_TORQUE_TORQUE_DEFAULT;
	_config->TORQUE.SMPLTH= DRV8711_TORQUE_SMPLTH_DEFAULT;

	_config->OFF.TOFF= DRV8711_OFF_TOFF_DEFAULT;
	_config->OFF.PWMMODE= DRV8711_OFF_PWMMODE_DEFAULT;

	_config->BLANK.TBLANK= DRV8711_BLANK_TBLANK_DEFAULT;
	_config->BLANK.ABT= DRV8711_BLANK_ABT_DEFAULT;

	_config->DECAY.TDECAY= DRV8711_DECAY_TDECAY_DEFAULT;
	_config->DECAY.DECMOD= DRV8711_DECAY_DECMOD_DEFAULT;

	_config->STALL.SDTHR= DRV8711_STALL_SDTHR_DEFAULT;
	_config->STALL.SDCNT= DRV8711_STALL_SDCNT_DEFAULT;
	_config->STALL.VDIV= DRV8711_STALL_VDIV_DEFAULT;

	_config->DRIVE.OCPTH= DRV8711_DRIVE_OCPTH_DEFAULT;
	_config->DRIVE.OCPDEG= DRV8711_DRIVE_OCPDEG_DEFAULT;
	_config->DRIVE.TDRIVEN= DRV8711_DRIVE_TDRIVEN_DEFAULT;
	_config->DRIVE.TDRIVEP= DRV8711_DRIVE_TDRIVEP_DEFAULT;
	_config->DRIVE.IDRIVEN= DRV8711_DRIVE_IDRIVEN_DEFAULT;
	_config->DRIVE.IDRIVEP= DRV8711_DRIVE_IDRIVEP_DEFAULT;
}

void drv8711_phy_read_(drv8711_p _drv ,uint16_t * _reg ){
	_drv->cs_on();
  _drv->put(* _reg);
	drv8711_phy_delay_us(57);
	_drv->get(_reg);
	_drv->cs_off();
}

void drv8711_phy_write_(drv8711_p _drv, uint16_t  _deseired, uint16_t * _actual){
	int rpt =3;
	do {
		_drv->cs_on();
		_drv->put(_deseired);
		drv8711_phy_delay_us(57);
		uint16_t dummy;
		_drv->get(&dummy);
		_drv->cs_off();
		drv8711_phy_read_(_drv,_actual);
		if( ((0x7FFF) & (* _actual)) ==_deseired){
			return;
		}
	} while(--rpt);
	drv8711_phy_crash();
}

void drv8711_begin_(drv8711_p _drv,drv8711_config_p _config){
	
	uint16_t * d = _drv->deseired.values;
	for(int i=0; i<DRV8711_REG_COUNT; ++i,++d ){
		*d = (i<<12);
	}
	
	uint16_t * a = _drv->actual.values;
	for(int i=0; i<DRV8711_REG_COUNT; ++i,++a ){
		*a = (i<<12) | (1<<15) ;
	}

	_drv->deseired.CTRL.ENBL = DRV8711_CTRL_ENBL_OFF;//_config->CTRL.ENBL;
	_drv->deseired.CTRL.RDIR = _config->CTRL.RDIR;
	_drv->deseired.CTRL.RSTEP = _config->CTRL.RSTEP;
	_drv->deseired.CTRL.MODE = _config->CTRL.MODE;
	_drv->deseired.CTRL.EXSTALL = _config->CTRL.EXSTALL;
	_drv->deseired.CTRL.ISGAIN = _config->CTRL.ISGAIN;
	_drv->deseired.CTRL.DTIME = _config->CTRL.DTIME;
	
	_drv->deseired.TORQUE.TORQUE = _config->TORQUE.TORQUE;
	_drv->deseired.TORQUE.SMPLTH = _config->TORQUE.SMPLTH;
	
	_drv->deseired.OFF.TOFF = _config->OFF.TOFF;
	_drv->deseired.OFF.PWMMODE = _config->OFF.PWMMODE;

	_drv->deseired.BLANK.TBLANK = _config->BLANK.TBLANK;
	_drv->deseired.BLANK.ABT = _config->BLANK.ABT;

	_drv->deseired.DECAY.TDECAY = _config->DECAY.TDECAY;
	_drv->deseired.DECAY.DECMOD = _config->DECAY.DECMOD;

	_drv->deseired.STALL.SDTHR = _config->STALL.SDTHR;
	_drv->deseired.STALL.SDCNT = _config->STALL.SDCNT;
	_drv->deseired.STALL.VDIV = _config->STALL.VDIV;

	_drv->deseired.DRIVE.OCPTH = _config->DRIVE.OCPTH;
	_drv->deseired.DRIVE.OCPDEG = _config->DRIVE.OCPDEG;
	_drv->deseired.DRIVE.TDRIVEN = _config->DRIVE.TDRIVEN;
	_drv->deseired.DRIVE.TDRIVEP = _config->DRIVE.TDRIVEP;
	_drv->deseired.DRIVE.IDRIVEN = _config->DRIVE.IDRIVEN;
	_drv->deseired.DRIVE.IDRIVEP = _config->DRIVE.IDRIVEP;


	a = _drv->actual.values;
	d = _drv->deseired.values;
	for(int i=0; i<DRV8711_REG_COUNT; ++i,++a,++d ){
		drv8711_phy_write_(_drv, *d, a);
	}

	_drv->deseired.CTRL.ENBL = _config->CTRL.ENBL;
	drv8711_phy_write_(_drv, _drv->deseired.CTRL.value, &_drv->actual.CTRL.value);
}
void drv8711_status_query_(drv8711_p _drv){
		_drv->cs_on();
		_drv->actual.STATUS.NotAvailabled = 1;
		_drv->put(_drv->actual.STATUS.value);
}
burst_bool_t drv8711_status_check_(drv8711_p _drv){
	if( _drv->complete()){
		_drv->get(&_drv->actual.STATUS.value);
		_drv->cs_off();
		return burst_true;
	} else {
		return burst_false;
	}
}
