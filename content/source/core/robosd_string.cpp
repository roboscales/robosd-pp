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
	}


	string::string(const string& _src) : value_(new base_string_(*(_src.value_))) {}

	string::string(cstr _format, va_list _args)  {
		ROBO_ALARMN(format(_format, _args))
	}

	string::string(cstr _format, ...) {
		va_list args;
		va_start(args, _format);
		ROBO_ALARMN(format(_format, args))
		va_end(args);
	}
	
	bool string::printf_backend_(stream_s & _s, cstr _format, va_list _args){
		#if ROBO_APP_ENV_ENABLED == 1
		_s.memo = string_buffer_backend;
		_s.size = system::env::sprintf(string_buffer_backend, ROBO_STRING_BUFFER_SIZE, _format, _args);
		return _s.size>0;
		#else
		return 0;
		#endif
	}
	bool string::printf_frontend_(stream_s & _s, cstr _format, va_list _args){
		#if ROBO_APP_ENV_ENABLED == 1
		_s.memo = string_buffer_frontend;
		_s.size = system::env::sprintf(string_buffer_frontend, ROBO_STRING_BUFFER_SIZE, _format, _args);
		return _s.size>0;
		#else
		return 0;
		#endif
	}

	bool string::format(cstr _format, va_list _args) {
		stream_s stream;
		if (system::env::is_backend()) {			
			if(printf_backend_(stream, _format, _args)){
				*((stds*)value_) = stream.memo;
				return true;
			}
		}
		else {
			system::critical c__;
			if(printf_frontend_(stream, _format, _args)){
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

	bool string::load(cstr _section, cstr _key) {
		ROBO_LRET_F(tryload(_section, _key), "error load string %s/%s", _section, _key);
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

}

