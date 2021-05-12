#ifndef robosd_cstring_hpp
#define robosd_cstring_hpp
#include "core/robosd_delegat.hpp"
#include "core/robosd_log.hpp"
#include <string>
#include <locale.h>
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

		template <typename T> bool to_number(cstr begc, char_t* &endc, T& _value) {
			
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
			ROBO_LBREAKN_F(tmp >= lo && tmp <= hi, "value %f is outside", tmp);
			//todo а ну как округлится?
			_value = (T)tmp; 
			return true;
		}

		template <typename T> bool to_number( T& _value) {
			char_t* endc;
			ROBO_LRET(to_number(c_str(), endc, _value) );
		}
		template <typename T> bool scan_numbers(size_t _max_count, T* _values, size_t & _count ) {
			
			char_t* endc;
			const char_t* begc = c_str();
			_count = 0;
			while( (begc && begc[0]) && _count < _max_count){
				T value;
				ROBO_LBREAKN_F(to_number(begc, endc, value), "error convert number %s [%d]", begc, (int)_count);
				*_values++ = value;
				begc = endc;
				_count++;
			}
			return true;
		}

		template <typename T> bool to_number_array( T* _values, size_t _count) {
			size_t sz;
			ROBO_LBREAKN( scan_numbers(_count, _values, sz))
			ROBO_LRET_F( sz == _count , "error convert string '%s' to %d numbers (%d)",c_str(), (int) _count, (int) sz );
		}
		template <typename T> bool to_number_list(size_t _max_count, T* _values, size_t & _count) {
			ROBO_LRET_F(scan_numbers(_max_count, _values, _count), "error convert string '%s' to numbers", c_str());
		}
	};
}

#endif
