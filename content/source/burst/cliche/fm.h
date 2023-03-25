#include "burst/cliche/_begin.h"
#ifdef CLCH_HEADER
	#include <stdint.h>
	void PREFIX(poll)(void);
	void PREFIX(recorder)(void);
	
	uint8_t  PREFIX(available)(void);
	uint8_t  PREFIX(space)(void);
	uint8_t				PREFIX(get)(void);
	void					PREFIX(put)(uint8_t _data);
#else
#include <stdint.h>
#include "burst/burst_common.h"
BURST_WEAK uint8_t  PREFIX(available)(void){
	return 0;
}
BURST_WEAK uint8_t  PREFIX(space)(void){
	return 0;
}
BURST_WEAK uint8_t				PREFIX(get)(void){
	return 0;
}
BURST_WEAK void					PREFIX(put)(uint8_t _data){
	BURST_UNUSED(_data);
}

#include "freemaster/PE_freemaster.h"


	
void PREFIX(recorder)(void) {
	FMSTR_Recorder();
}
void PREFIX(poll)(void) {
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
	PREFIX(put)(_data);
}
FMSTR_U16 FMSTR_SCI_GETCHAR(void) {
	return PREFIX(get)();
}


void FMSTR_SCI_RE(void) {}
void FMSTR_SCI_RD(void) {}
void FMSTR_SCI_TE(void) {}
void FMSTR_SCI_TD(void) {}
FMSTR_SCISR FMSTR_SCI_RDCLRSR(void) {
	FMSTR_SCISR SciSR = 0;

	if (PREFIX(available)() > 0) {
		SciSR = FMSTR_SCISR_RDRF;
	}

	if (PREFIX(space)() > 0) {
		SciSR |= FMSTR_SCISR_TDRE;
	}
	return SciSR;
}
	
#endif
#include "burst/cliche/_end.h"
