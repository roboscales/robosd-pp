
#ifndef robosd_cstring_hpp
#define robosd_cstring_hpp
#include "core/robosd_delegat.hpp"
#include "core/robosd_log.hpp"
#include "core/robosd_system.hpp"
#include <string>
#include <locale.h>
#ifndef ROBO_STRING_BUFFER_SIZE
#define ROBO_STRING_BUFFER_SIZE 255
#endif

namespace robo {

	class ROBO_EXPORT string
	{
	private:
		class base_string_;
		base_string_ * value_ = nullptr;
		//std::basic_string<char_t, std::char_traits<char_t>, std::allocator<char_t> > value_;
	public:
		virtual ~string(void);
		string(void);
		string(const string& _src);
		#if ROBO_APP_FORMATING_TYPE != ROBO_APP_TYPE_NONE
		string(cstr _format, va_list _args);
		string(cstr _format, ...);
		#endif
		string(cstr _template, cstr* _map, size_t _size);
		string& operator =(const string& _src);
		string& operator =(cstr _src);
		#if ROBO_APP_FORMATING_TYPE != ROBO_APP_TYPE_NONE
		bool format(cstr _format, va_list _args);
		bool format(cstr _format, ...);
		#endif
		bool load(cstr _section, cstr _key);
		bool tryload(cstr _section, cstr _key);

		bool load(delegat::base<bool, uint8_t*, size_t >& _converter);
		size_t length(void);
		cstr c_str() const;
		operator cstr () const; //todo осмыслить
		void clear(void);
		private:
			struct stream_s{
				const char_t * memo;
				size_t size;
			};
			#if ROBO_APP_FORMATING_TYPE != ROBO_APP_TYPE_NONE
			static bool printf_backend_(stream_s & _s, cstr _format, va_list _args);
			static bool printf_frontend_(stream_s & _s, cstr _format, va_list _args);
			#endif
		public:
			#if ROBO_APP_FORMATING_TYPE != ROBO_APP_TYPE_NONE
			template< typename B> static bool format_stream(B & _b,  cstr _format, va_list _args) {
				stream_s stream;
				if (system::env::is_backend()) {
					return ( printf_backend_(stream,_format,_args)  && _b.put((uint8_t *)stream.memo, stream.size*sizeof(char_t)) );			
				}
				else {
					system::critical c__;
					return ( printf_frontend_(stream,_format,_args)  && _b.put((uint8_t *)stream.memo, stream.size*sizeof(char_t)) );			
				}
			};
			#endif
		template <typename T> bool to_number(cstr begc, char_t*& endc, T& _value) {

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

		template <typename T> bool to_number(T& _value) {
			char_t* endc;
			ROBO_LRET(to_number(c_str(), endc, _value));
		}
		template <typename T> bool scan_numbers(size_t _max_count, T* _values, size_t& _count) {

			char_t* endc;
			const char_t* begc = c_str();
			_count = 0;
			while ((begc && begc[0]) && _count < _max_count) {
				T value;
				ROBO_LBREAKN_F(to_number(begc, endc, value), "error convert number %s [%d]", begc, (int)_count);
				*_values++ = value;
				begc = endc;
				_count++;
			}
			return true;
		}

		template <typename T> bool to_number_array(T* _values, size_t _count) {
			size_t sz;
			ROBO_LBREAKN(scan_numbers(_count, _values, sz))
				ROBO_LRET_F(sz == _count, "error convert string '%s' to %d numbers (%d)", c_str(), (int)_count, (int)sz);
		}
		template <typename T> bool to_number_list(size_t _max_count, T* _values, size_t& _count) {
			ROBO_LRET_F(scan_numbers(_max_count, _values, _count), "error convert string '%s' to numbers", c_str());
		}
	};
}

#endif
