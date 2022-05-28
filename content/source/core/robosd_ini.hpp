#ifndef __robo_ini_hpp
#define  __robo_ini_hpp
#include "core/robosd_log.hpp"
#include "core/robosd_string.hpp"
#include  <limits>


namespace robo {
	namespace ini {
		template <typename T>	bool try_load(cstr _sect, cstr _key, T& _value) {
			string value;
			return value.tryload(_sect, _key) && value.to_number(_value);
		}

		template <typename T>	bool try_load(cstr _sect_first, cstr _sect_second, cstr _key, T& _value) {
			return
				try_load(_sect_first, _key, _value)
				||
				try_load(_sect_second, _key, _value);
		}

		template <typename T>	bool load(cstr _sect, cstr _key, T& _value) {
			ROBO_LRET_F( try_load(_sect, _key, _value), "error load number param %s/%s", _sect, _key);
		}

		template <typename T>	bool load(cstr _sect_first, cstr _sect_second, cstr _key, T& _value) {
			if (try_load(_sect_first, _key, _value)) {
				return true;
			}
			else {
				ROBO_LRET_F(try_load(_sect_second, _key, _value), "error load number param %s(or%s)/%s", _sect_first,_sect_second, _key);
			}
		}

		template<typename T> bool try_load_arr(cstr _section, cstr _key, T * _values, size_t _count) {
			string values;
			return values.tryload(_section, _key) && values.to_number_array(_values, _count);
		}

		template <typename T>	bool load_arr(cstr _sect, cstr _key, T* _values, size_t _count) {
			ROBO_LRET_F(try_load_arr(_sect, _key, _values, _count), "error load array %s/%s", _sect, _key);
		}

		template<typename T> bool try_load_list(cstr _section, cstr _key, size_t _max_count, T* _values, size_t & _count) {
			string values;
			return values.tryload(_section, _key) && values.to_number_list(_max_count, _values, _count);
		}

		template <typename T>	bool load_list(cstr _section, cstr _key, size_t _max_count, T* _values, size_t& _count) {
			ROBO_LRET_F(try_load_list(_section, _key, _max_count, _values, _count), "error load list %s/%s", _section, _key);
		}
	}
}

#endif
