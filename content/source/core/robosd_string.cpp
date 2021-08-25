#include "core/robosd_string.hpp"
#include "core/robosd_log.hpp"
#include "core/robosd_system.hpp"

namespace robo{	

	char_t string_buffer_frontend[ROBO_STRING_BUFFER_SIZE];
	char_t string_buffer_backend[ROBO_STRING_BUFFER_SIZE];

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
#if ROBO_APP_ENV_ENABLED == 1
		char_t * _buffer;
		if( system::env::is_backend() ){
			_buffer = string_buffer_backend; 
		} else {
			system::critical c__;
			_buffer = string_buffer_frontend; 
		}
		if (system::env::sprintf(_buffer, ROBO_STRING_BUFFER_SIZE, _format, _args)) {
			*( (string_base *)this) = _buffer;
			return true;
		}
		else {
			return false;
		}
#else
		return false;
#endif
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
		ROBO_LRET_F(tryload(_section,_key),"error load string %s/%s", _section, _key);
	}
	
	bool string::tryload(cstr _section, cstr _key){
#if ROBO_APP_INI_ENABLED == 1
		if( system::env::is_backend() ){
			if (system::ini::load_str(string_buffer_backend, ROBO_STRING_BUFFER_SIZE, _section, _key)) {
				*((string_base*)this) = string_buffer_backend;
				return true;
			}
		} else {
			system::critical c__;
			if (system::ini::load_str(string_buffer_frontend, ROBO_STRING_BUFFER_SIZE, _section, _key)) {
				*((string_base*)this) = string_buffer_frontend;
				return true;
			}
		}
#endif
		return false;
	}
	
	bool string::load(delegat::base<bool, uint8_t *, size_t > & _converter){
#if ROBO_APP_ENV_ENABLED == 1
		if( system::env::is_backend() ){
			ROBO_LBREAKN( _converter( (uint8_t*)string_buffer_backend, ROBO_STRING_BUFFER_SIZE*sizeof(char_t)) );
			*((string_base*)this) = string_buffer_backend;
		} else {
			system::critical c__;
			ROBO_LBREAKN( _converter( (uint8_t*)string_buffer_frontend, ROBO_STRING_BUFFER_SIZE*sizeof(char_t)) );
		*((string_base*)this) = string_buffer_frontend;
		}
		return true;
#else
		return false;
#endif
	}
}

