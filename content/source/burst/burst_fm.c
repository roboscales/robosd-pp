#include "burst/burst_fm.h"

#if BURST_APP_FREEMASTER_SERIAL_ENABLED == 1

#include "freemaster/PE_freemaster.h"

burst_serial_p burst_fm_serial_  = & burst_serial_dummy;

void burst_fm_connect(burst_serial_p _serial) {
	if (burst_fm_serial_) {
		burst_fm_serial_->reset();
	}
	if (_serial) {
		burst_fm_serial_ = _serial;
	}
	else {
		burst_fm_serial_ = & burst_serial_dummy;
	}
}
	
void burst_fm_recorder(void) {
	FMSTR_Recorder();
}
void burst_fm_poll(void) {
	FMSTR_Poll();
}

/*
Все файлы подключаются так, чтобы упростить  работу с проектом
*/
#include "freemaster/PE_freemaster_default.c"
#include "freemaster/PE_freemaster_fastrec.c"
#include "freemaster/PE_freemaster_protocol.c"
#include "freemaster/PE_freemaster_rec.c"
#include "freemaster/PE_freemaster_scope.c"
#include "freemaster/PE_freemaster_serial.c"
#include "freemaster/PE_freemaster_sfio.c"
#include "freemaster/PE_freemaster_tsa.c"

void FMSTR_SCI_PUTCHAR(FMSTR_U8 _data) {
	burst_fm_serial_->put(_data);
}
FMSTR_U16 FMSTR_SCI_GETCHAR(void) {
	return burst_fm_serial_->get();
}


void FMSTR_SCI_RE(void) {}
void FMSTR_SCI_RD(void) {}
void FMSTR_SCI_TE(void) {}
void FMSTR_SCI_TD(void) {}
FMSTR_SCISR FMSTR_SCI_RDCLRSR(void) {
	FMSTR_SCISR SciSR = 0;

	if (burst_fm_serial_->available() > 0) {
		SciSR = FMSTR_SCISR_RDRF;
	}

	if (burst_fm_serial_->space() > 0) {
		SciSR |= FMSTR_SCISR_TDRE;
	}
	return SciSR;
}

void freemaster_init() {}

#endif
