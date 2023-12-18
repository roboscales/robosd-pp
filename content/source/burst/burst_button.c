#include "burst/burst_button.h"
#if BURST_BUTTON_ENABLED == 1
#ifndef BURST_BTN_MAX_COUNT
#define BURST_BTN_MAX_COUNT 4
#endif
int burst_btn_index_count = 0;
burst_btn_p burst_btn_index[BURST_BTN_MAX_COUNT] = {};

void burst_btn_raise_(burst_btn_p _btn){
	burst_time_us_t us = burst_time_us();
	if( us - _btn->last_us >= _btn->permit_us ){
		_btn->on_raise();
		_btn->last_us = us;
	}
}	
void burst_btn_begin(burst_btn_p _btn, burst_btn_mode_t _mode, burst_time_us_t _permit_us, burst_time_us_t _repeate_us  ){
	_btn->mode = _mode;
	_btn->permit_us = _permit_us;
	_btn->repeate_us = _repeate_us;
	if(burst_btn_index_count < BURST_BTN_MAX_COUNT){
		burst_btn_index[burst_btn_index_count] = _btn;
		burst_btn_index_count++;
		if( _btn->phy_get() == BURST_BTN_PRESSED ){
			_btn->status = BURST_BTN_PRESSED;
		} else{
			_btn->status = BURST_BTN_RELEASED;
		}
	}
}

void burst_btn_poll_(burst_btn_p _btn){
	if( _btn->phy_get() ){
		burst_time_us_t us = burst_time_us();
		if(_btn->status ==  BURST_BTN_RELEASED){
			if(us - _btn->pushdown_us > _btn->permit_us){
				_btn->status =  BURST_BTN_PRESSED;
				burst_btn_raise_(_btn);
				_btn->pushdown_us = burst_time_us();
			}
		}else{
			_btn->pressed_us = us -_btn->pushdown_us;
			if(_btn->repeate_us > 0){
				if( us - _btn->last_repeate_us >= _btn->repeate_us ){
					_btn->last_repeate_us = us;
					burst_btn_raise_(_btn);
				}
			}
		}
	} else {
		if(_btn->status ==  BURST_BTN_PRESSED){
			_btn->on_release();
			_btn->status =  BURST_BTN_RELEASED;
			_btn->pressed_us = _btn->pushdown_us = 0;
		}
	}
}

void burst_btn_poll(void){
	burst_btn_p * p=burst_btn_index;
	for (int i=0;i<burst_btn_index_count;++i,++p)
	{
		burst_btn_poll_(*p);
	}
}
#endif
