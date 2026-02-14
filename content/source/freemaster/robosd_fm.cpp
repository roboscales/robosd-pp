#include "freemaster/robosd_fm.hpp"

#include "freemaster/PE_freemaster.h"

#if ROBO_APP_FREEMASTER_SERIAL_ENABLED == 1
namespace robo {
	freemaster freemaster::instance_;

	freemaster::freemaster(void)
		: serial_(&net::serial_dummy::instance()) {
	}
	freemaster::~freemaster(void) {

	}
	void freemaster::connect_(robo::net::iserial* _serial) {
		if (serial_) {
			serial_->reset();
		}
		if (_serial) {
			serial_ = _serial;
		}
		else {
			serial_ = &robo::net::serial_dummy::instance();
		}
	}
	void freemaster::connect(net::iserial* _serial) {
		instance_.connect_(_serial);
	}
	
	void freemaster::connect(cstr _name){
		instance_.connect_(net::link::query<net::iserial>(_name));
	}
	
	robo::net::iserial* freemaster::serial(void) {
		return instance_.serial_;
	}

	void freemaster::recorder(void) {
		FMSTR_Recorder();
	}
	void freemaster::poll(void) {
		FMSTR_Poll();
	}

	#if ROBO_APP_PROTO_SWITCH_ENABLED == 1

	void freemaster::abonent::stop(void) {
		connect((net::iserial*)nullptr);
	}
	void freemaster::abonent::start(void) {
		connect(this);
	}
	uint8_t abonent_marker[3] = { 0x2b, 0xC0, 0x40 };

	freemaster::abonent::abonent(
		time_us_t _lock_us
		, time_us_t _silence_us
	) : robo::net::proto::switcher::abonent(
		abonent_marker
		, 3
		, _lock_us
		, _silence_us
	) {}

	size_t  freemaster::abonent::get(uint8_t & _data)	{
		alive();		
		return robo::net::proto::switcher::abonent::get(_data);
	};
	#endif
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
	robo::freemaster::serial()->put(_data);
}
FMSTR_U16 FMSTR_SCI_GETCHAR(void) {
	uint8_t tmp = 0;
	robo::freemaster::serial()->get(tmp);
	return tmp;
}


void FMSTR_SCI_RE(void) {}
void FMSTR_SCI_RD(void) {}
void FMSTR_SCI_TE(void) {}
void FMSTR_SCI_TD(void) {}
FMSTR_SCISR FMSTR_SCI_RDCLRSR(void) {
	FMSTR_SCISR SciSR = 0;

	if (robo::freemaster::serial()->available() > 0) {
		SciSR = FMSTR_SCISR_RDRF;
	}

	if (robo::freemaster::serial()->space() > 0) {
		SciSR |= FMSTR_SCISR_TDRE;
	}
	return SciSR;
}

void freemaster_init() {}

#endif

#if ROBO_APP_FREEMASTER_DIRRECT_ENABLED == 1
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
	robo::prf::freemaster::put(_data);
}
FMSTR_U16 FMSTR_SCI_GETCHAR(void) {
	return robo::prf::freemaster::get();
}


void FMSTR_SCI_RE(void) {
	robo::prf::freemaster::re();
}
void FMSTR_SCI_RD(void) {
	robo::prf::freemaster::rd();
}
void FMSTR_SCI_TE(void) {
	robo::prf::freemaster::te();	
}
void FMSTR_SCI_TD(void) {
	robo::prf::freemaster::td();	
}
FMSTR_SCISR FMSTR_SCI_RDCLRSR(void) {
	FMSTR_SCISR SciSR = 0;

	if (robo::prf::freemaster::available()) {
		SciSR = FMSTR_SCISR_RDRF;
	}

	if (robo::prf::freemaster::space() ) {
		SciSR |= FMSTR_SCISR_TDRE;
	}
	return SciSR;
}
void robo::freemaster::recorder(void) {
	FMSTR_Recorder();
}
void robo::freemaster::poll(void) {
	FMSTR_Poll();
}
#endif
