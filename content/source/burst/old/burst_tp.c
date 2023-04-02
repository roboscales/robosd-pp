#include "burst/burst_tp.h"
void burst_tp_on_(burst_tp_p _tp, int _verb){
	if(_tp->verb == _verb){
		_tp->status = BURST_TP_ON;
		_tp->phy_on();
	}
}
void burst_tp_off_(burst_tp_p _tp, int _verb){
	if(_tp->verb == _verb){
		_tp->status = BURST_TP_OFF;
		_tp->phy_off();
	}
}
void burst_tp_toggle_(burst_tp_p _tp, int _verb){
	if(_tp->verb == _verb){
		if( _tp->status == BURST_TP_OFF){
			_tp->phy_on();
			_tp->status = BURST_TP_ON;
		} else {
			_tp->status = BURST_TP_OFF;
			_tp->phy_off();
		}
	}
}

