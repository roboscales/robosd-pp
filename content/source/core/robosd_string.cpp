#include "core/robosd_string.hpp"
#include "core/robosd_log.hpp"
#include "core/robosd_system.hpp"

//#include "core/robosd_ini.h"




namespace robo{	

	char_t string_buffer[ROBO_STRING_BUFFER_SIZE];

	string::~string(void){
	}
	
	string::string(void) : string_base() {
	}

	
	string::string(const string &  _src) : string_base(_src) {
	}

	string::string(cstr _format, va_list _args) : string_base(){
		ROBO_ALARMN( format(_format, _args))
	}

	string::string(cstr _format, ...) : string_base(){
		va_list args;
		va_start(args, _format);
		ROBO_ALARMN(format(_format, args))
		va_end(args);
	}
	
	bool string::format(cstr _format, va_list _args){
		system::guard g__;
		if (system::os::sprintf(string_buffer, ROBO_STRING_BUFFER_SIZE, _format, _args)) {
			*( (string_base *)this) = string_buffer;
			return true;
		}
		else {
			return false;
		}
	}
	bool string::format( cstr _format, ... ){
		va_list args;
		va_start(args, _format);
		bool ret = format(_format, args);
		ROBO_ALARMN(ret);
		va_end(args);
		return ret;
	}

	bool string::load(cstr _section, cstr _key){
		ROBO_LRET(tryload(_section,_key));
	}
	
	bool string::tryload(cstr _section, cstr _key){
		system::guard g__;
		if (system::os::ini_load_str(string_buffer, ROBO_STRING_BUFFER_SIZE, _section, _key)) {
			*((string_base*)this) = string_buffer;
			return true;
		}
		else {
			return false;
		}
	}
	bool string::load(delegat::base<bool, uint8_t *, size_t > & _converter){
		ROBO_LBREAKN( _converter( (uint8_t*)string_buffer, ROBO_STRING_BUFFER_SIZE*sizeof(char_t)) );
		*((string_base*)this) = string_buffer;
		return true;
	}

}

