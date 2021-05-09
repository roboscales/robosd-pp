#ifndef robosd_cstring_hpp
#define robosd_cstring_hpp
#include "core/robosd_delegat.hpp"
#include <string>
#ifndef ROBO_STRING_BUFFER_SIZE
#define ROBO_STRING_BUFFER_SIZE 255
#endif

namespace robo{
	typedef std::basic_string<char_t, std::char_traits<char_t>, std::allocator<char_t> > string_base;
	class ROBO_EXPORT string 
		: public  string_base {
	public:
		virtual ~string(void);
		string(void);
		string(const string &  _src);
		string(cstr _format, va_list _args);
		string(cstr _format, ...);
		string(cstr _template, cstr * _map, size_t _size);

		bool format(cstr _format, va_list _args);
		bool format( cstr _format, ...);

		bool load(cstr _section, cstr _key);
		bool tryload( cstr _section, cstr _key);
		
		bool load(delegat::base<bool, uint8_t*, size_t >& _converter);

		inline  cstr c_str() const { return   string_base::c_str();  };
        inline operator  cstr () const { return c_str(); }; //todo осмыслить

		template <typename T> bool to_number(T & _value) {
			cstr begc = c_str();
			char_t* endc;

			setlocale(LC_NUMERIC, "C");

#if ROBO_UNICODE_ENABLED ==1
			double tmp = wcstod(begc, &endc);
#else
			double tmp = strtod(begc, &endc);
#endif
			ROBO_LBREAKN(begc != endc);
			setlocale(LC_NUMERIC, "");
			double lo = (double)std::numeric_limits<T>::lowest();
			double hi = (double)std::numeric_limits<T>::max();
			ROBO_LBREAKN_F(tmp >=lo  && tmp <= hi, "value %f is outside", tmp);
			//todo а ну как округлится?
			_value = (T)tmp;
			return true;
		}
	};
}

#endif
