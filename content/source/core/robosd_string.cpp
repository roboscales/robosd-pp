#include "core/robosd_string.hpp"
#include "core/robosd_log.hpp"


namespace robo {
	typedef std::basic_string<char_t, std::char_traits<char_t>, std::allocator<char_t> > stds;
	class string::base_string_ :public stds {};

	char_t string_buffer_frontend[ROBO_STRING_BUFFER_SIZE];
	char_t string_buffer_backend[ROBO_STRING_BUFFER_SIZE];

	string::~string(void) {
		delete value_;
	}

	string::string(void): value_(new base_string_) {
		*((stds*)value_) = RT("");
	}


	string::string(const string& _src) : value_(new base_string_(*(_src.value_))) {}

	#if ROBO_APP_FORMATING_TYPE != ROBO_APP_TYPE_NONE

	string::string(cstr _format, va_list _args) : value_(new base_string_) {
		ROBO_ALARMN(format(_format, _args))
	}

	string::string(cstr _format, ...) : value_(new base_string_) {
		va_list args;
		va_start(args, _format);
		ROBO_ALARMN(format(_format, args))
		va_end(args);
	}
	#endif
		
	#if ROBO_APP_FORMATING_TYPE != ROBO_APP_TYPE_NONE

	bool string::sprintf_backend_(stream_s & _s, cstr _format, va_list _args){
		#if ROBO_APP_ENV_ENABLED == 1
		_s.memo = string_buffer_backend;
		_s.size = system::env::sprintf(string_buffer_backend, ROBO_STRING_BUFFER_SIZE, _format, _args);
		return _s.size>0;
		#else
		return 0;
		#endif
	}
	bool string::sprintf_frontend_(stream_s & _s, cstr _format, va_list _args){
		system::critical c__;
		#if ROBO_APP_ENV_ENABLED == 1
		_s.memo = string_buffer_frontend;
		_s.size = system::env::sprintf(string_buffer_frontend, ROBO_STRING_BUFFER_SIZE, _format, _args);
		return _s.size>0;
		#else
		return 0;
		#endif
	}

	#if ROBO_UNICODE_ENABLED == 1
	bool string::sprintf_backend_(stream_s& _s, const char *  _format, va_list _args) {
		#if ROBO_APP_ENV_ENABLED == 1
		_s.memo = string_buffer_backend;
		_s.size = system::env::sprintf((char *)string_buffer_backend, ROBO_STRING_BUFFER_SIZE * sizeof(char_t)/sizeof(char), _format, _args);
		return _s.size > 0;
		#else
		return false;
		#endif
	}
	bool string::sprintf_frontend_(stream_s& _s, const char* _format, va_list _args) {
		system::critical c__;
		#if ROBO_APP_ENV_ENABLED == 1
		_s.memo = string_buffer_frontend;
		_s.size = system::env::sprintf((char*)string_buffer_frontend, ROBO_STRING_BUFFER_SIZE * sizeof(char_t) / sizeof(char), _format, _args);
		return _s.size > 0;
		#else
		return false;
		#endif
	}
	#endif

	bool string::format(cstr _format, va_list _args) {
		stream_s stream;
		if (system::env::is_backend()) {			
			if(sprintf_backend_(stream, _format, _args)){
				*((stds*)value_) = stream.memo;
				return true;
			}
		}
		else {
			system::critical c__;
			if(sprintf_frontend_(stream, _format, _args)){
				*((stds*)value_) = stream.memo;
				return true;
			}
		}
		return false;
	}
	
	bool string::format(cstr _format, ...) {
		va_list args;
		va_start(args, _format);
		bool ret = format(_format, args);
		ROBO_ALARMN(ret);
		va_end(args);
		return ret;
	}
	#endif
	bool string::load(cstr _section, cstr _key) {
		ROBO_LRET_F(tryload(_section, _key), "error load string %s/%s", _section, _key);
	}

	bool string::load(cstr _first_section, cstr _second_section, cstr _key) {
		if (!tryload(_first_section, _key)) {
			ROBO_LRET_F(tryload(_second_section, _key), "error load string %s/%s", _second_section, _key);
		}
		else {
			return true;
		}		
	}
	
	bool string::tryload(cstr _section, cstr _key) {
		#if ROBO_APP_INI_ENABLED == 1
		if (system::env::is_backend()) {
			if (system::ini::load_str(string_buffer_backend, ROBO_STRING_BUFFER_SIZE, _section, _key)) {
				*((stds *)value_) = string_buffer_backend;
				return true;
			}
		}
		else {
			system::critical c__;
			if (system::ini::load_str(string_buffer_frontend, ROBO_STRING_BUFFER_SIZE, _section, _key)) {
				*((stds*)value_) = string_buffer_frontend;
				return true;
			}
		}
		#endif
		return false;
	}

	bool string::load(delegat::base<bool, uint8_t*, size_t >& _converter) {
		#if ROBO_APP_ENV_ENABLED == 1
		if (system::env::is_backend()) {
			ROBO_LBREAKN(_converter((uint8_t*)string_buffer_backend, ROBO_STRING_BUFFER_SIZE * sizeof(char_t)));
			*((stds*)value_) = string_buffer_backend;
		}
		else {
			system::critical c__;
			ROBO_LBREAKN(_converter((uint8_t*)string_buffer_frontend, ROBO_STRING_BUFFER_SIZE * sizeof(char_t)));
			*((stds*)value_) = string_buffer_frontend;
		}
		return true;
		#else
		return false;
		#endif
	}

	cstr  string::c_str() const { return   value_->c_str(); };

	string::operator  cstr () const { return  value_->c_str(); };
	size_t string::length(void) {
		return value_->length();
	}
	void string::clear(void) {
		value_->clear();
	}

	string& string::operator =(const string& _src) {
		*((stds*)value_) = *((stds*)(_src.value_));
		return *this;
	}

	string& string::operator =(cstr _src) {
		*((stds*)value_) = _src;
		return *this;
	}

}

