#include "core/robosd_string.hpp"
#include "core/robosd_log.hpp"

#if ROBO_UNICODE_ENABLED == 1
#include <codecvt>
#else
#include <cstring>
#endif

namespace robo {
	typedef std::basic_string<char_t, std::char_traits<char_t>, std::allocator<char_t> > stds;
	class string::base_string_ :public stds {};

	char_t string_buffer_frontend[ROBO_STRING_BUFFER_SIZE];
	char_t string_buffer_backend[ROBO_STRING_BUFFER_SIZE];

	string::~string(void) {
		delete value_;
	}

	string::string(void): value_(create_value_()) {
		*((stds*)value_) = RT("");
	}
	size_t string::strlen(cstr _str) {
		#if ROBO_UNICODE_ENABLED
		return wcslen(_str);
		#else
		return ::strlen(_str);
#endif
	}

	string* string::create(void) {
		return new string;
	}


	string::string(const string& _src) : value_(_src.value_==nullptr? create_value_() :new base_string_(*(_src.value_))) {}
	
	string::base_string_ * string::create_value_(void) {
		return new base_string_;
	}

	#if ROBO_APP_FORMATING_TYPE != ROBO_APP_TYPE_NONE
	string::string(cstr _format, va_list _args) : value_(create_value_()) {
		ROBO_ALARMN(format(_format, _args))
	}

	string::string(cstr _format, ...) : value_(create_value_()) {
		va_list args;
		va_start(args, _format);
		ROBO_ALARMN(format(_format, args))
		va_end(args);
	}
	#endif
	

	bool string::operator == (const string& _s) {
		#if ROBO_UNICODE_ENABLED == 1
		return std::wcscmp(c_str(), _s.c_str()) == 0;
		#else
		return std::strcmp(c_str(), _s.c_str()) == 0;
		#endif
	}
	bool string::operator == (cstr _s) {
		#if ROBO_UNICODE_ENABLED == 1
		return std::wcscmp(c_str(), _s) == 0;
		#else
		return std::strcmp(c_str(), _s) == 0;
		#endif
	}


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
		#if ROBO_APP_SYSTEM_ENABLED == 1
		system::critical c__;
		#if ROBO_APP_ENV_ENABLED == 1
		_s.memo = string_buffer_frontend;
		_s.size = system::env::sprintf(string_buffer_frontend, ROBO_STRING_BUFFER_SIZE, _format, _args);
		return _s.size>0;
		#else
		return 0;
		#endif
		#else
		return 0;
		#endif
	}

	#if ROBO_UNICODE_ENABLED == 1
	bool string::sprintf_backend_(stream_s& _s, const char *  _format, va_list _args) {
		#if ROBO_APP_ENV_ENABLED == 1
		_s.memo = string_buffer_backend;
		_s.size = system::env::sprintf((char *)string_buffer_backend, ROBO_STRING_BUFFER_SIZE * sizeof(char_t)/sizeof(char), _format, _args);
		if (_s.size >= 0) {
			*(((char*)string_buffer_backend) + _s.size) = 0;
		}
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
		if (_s.size >= 0) {
			*(((char*)string_buffer_frontend) + _s.size) = 0;
		}
		return _s.size > 0;
		#else
		return false;
		#endif
	}
	#endif
	const char * string::ascii(void)  {
		#if ROBO_UNICODE_ENABLED == 1
		if (system::env::is_backend()) {
			ascii((char*)string_buffer_backend, ROBO_STRING_BUFFER_SIZE * 2);
			return (const char*)string_buffer_backend;
		}
		else {
			system::critical c__;
			ascii((char*)string_buffer_frontend, ROBO_STRING_BUFFER_SIZE * 2);
			return (const char*)string_buffer_frontend;
		}
#else
		return c_str();
#endif
	}

	void string::ascii(char * _buf, size_t _len) const {
		#if ROBO_UNICODE_ENABLED == 1

		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
		std::string utf8_string = convert.to_bytes(c_str());
		const char* p = utf8_string.c_str();
		while (*p && _len>1) {
			*_buf++ = *p++;
			_len--;
		}
		*_buf = 0;
		#else
		const char* p = c_str();
		while (*p && _len > 1) {
			*_buf++ = *p++;
			_len--;
		}
		*_buf = 0;
		#endif
	}
	void string::ascii( ::robo::lambda< void (const char*)> _d) const {
		#if ROBO_UNICODE_ENABLED == 1
		size_t sz =length() + 1;
		char* buf = new char[sz];
		ROBO_APP_ASSERT(buf != nullptr);
		ascii(buf, sz);
		_d(buf);
		delete[] buf;
		#else
		_d(c_str());
		#endif
	}
	void string::asciib(::robo::lambda< void(const uint8_t*, size_t )> _d) const {
		#if ROBO_UNICODE_ENABLED == 1
		size_t sz = length() + 1;
		char* buf = new char[sz];
		ROBO_APP_ASSERT(buf != nullptr);
		ascii(buf, sz);
		_d((const uint8_t*)buf,sz);
		delete[] buf;
		#else
		_d((const uint8_t*)c_str(), length());
		#endif
	}

	bool string::format(cstr _format, va_list _args) {
		stream_s stream;
		#if ROBO_APP_SYSTEM_ENABLED
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
		#else
		if(sprintf_frontend_(stream, _format, _args)){
			*((stds*)value_) = stream.memo;
			return true;
		}
		#endif
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
			ROBO_LRET_F(tryload(_second_section, _key), "error load string %s (or %s) /%s", _first_section, _second_section, _key);
		}
		else {
			return true;
		}		
	}
	

	bool string::tryload(cstr _first_section, cstr _second_section, cstr _key) {
		if (!tryload(_first_section, _key)) {
			return tryload(_second_section, _key);
		}
		else {
			return true;
		}
	}
	bool string::tryload(cstr _section, cstr _key) {
		#if ROBO_APP_INI_ENABLED == 1
		#if ROBO_APP_ENV_ENABLED == 1
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
		#else
		#endif
		if (system::ini::load_str(string_buffer_frontend, ROBO_STRING_BUFFER_SIZE, _section, _key)) {
			*((stds*)value_) = string_buffer_frontend;
			return true;
		}
		#endif
		return false;
	}

	bool string::load(delegat::ref<bool, uint8_t*, size_t >& _converter) {
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
	size_t string::length(void)  const {
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
	#if ROBO_STRING_UTOA_TYPE == ROBO_APP_TYPE_NATIVE
	
	uint8_t  string::utoa_n(uint32_t value, uint8_t _n, char_t *  _r, char_t _space ){
		char_t buffer[11];
		char_t * r;
		if(_space == 0){
			r = &buffer[10];
		} else{
			r= _r+10;
		}
		// 11 байт достаточно для десятичного представления 32-х байтного числа
		// и  завершающего нуля
		uint8_t n = 0;
		do {			
			if(n==_n){
				std::fill_n(_r,_n,RT('#'));
				return n;
			}
			n++;
			*--r = value % 10 + RT('0');
			value /= 10;
		} while (value != 0);		
		
		if(_space == 0){
			std::copy_n(r,n,_r);			
		} else{
			while( n<_n){
				*--r = _space;
				n++;
			}
		}
		return n; 					
	}
	#endif

}

