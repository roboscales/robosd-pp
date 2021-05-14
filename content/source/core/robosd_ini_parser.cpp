#include "core/robosd_ini_parser.h"
#if ROBO_APP_INI_TYPE == ROBO_APP_TYPE_NATIVE
#include "core/robosd_system.hpp"
#include "core/robosd_log.hpp"
#include <algorithm>

namespace robo {
	typedef struct robo_ipa_entry_s{
	unsigned short offset;
	unsigned short len;
} robo_ipa_entry_t;
typedef robo_ipa_entry_t  * robo_ipa_entry_p;


typedef struct robo_ipa_section_s{
	robo_ipa_entry_t name;
	size_t fiRTt_cell;
	size_t count;
} robo_ipa_section_t;
typedef robo_ipa_section_t  * robo_ipa_section_p;

typedef struct robo_ipa_cell_s{
	robo_ipa_entry_t name;
	robo_ipa_entry_t value;
} robo_ipa_cell_t;
typedef robo_ipa_cell_t  * robo_ipa_cell_p;

typedef struct robo_ipa_env_s{
	uint8_t * data;
	size_t size;
	robo_ipa_section_p sections;
	robo_ipa_cell_p cells;
	size_t sections_count;
	size_t cells_count;
	size_t sections_ix;
	size_t cells_ix;
	size_t cells_counter;
	robo_ipa_section_p current_section;
	robo_ipa_cell_p current_cell;
} robo_ipa_env_t;
typedef robo_ipa_env_t  * robo_ipa_env_p;


size_t robo_ipa_str_get(robo_ipa_env_p _env, robo_ipa_entry_p _entry, char_t * _dst, size_t _size);
size_t robo_ipa_str_get(robo_ipa_env_p _env, robo_ipa_entry_p _entry, char_t * _dst, size_t _size){
	char_t * src = (char_t *)(_env->data + _entry->offset);
	size_t m = _entry->len;
	if (m >= _size) {
		size_t l = _size - 1;
		while (l--)
		{
			*_dst++ = *src++;
		}
		*_dst = 0;
		return _size;
	}
	else{
		size_t l = m;
		while (l--)
		{
			*_dst++ = *src++;
		}
		*_dst = 0;
		return m;
	}
}

//size_t robo_ipa_str_get(robo_ipa_env_p _env, robo_ipa_entry_p _entry, uint8_t * _dst, size_t _size);


typedef enum { FETCH_SECTION, FETCH_END_SECTION, FETCH_END_LINE, FETCH_KEY, FETCH_END_KEY, FETCH_VALUE, FETCH_END_VALUE } robo_ipa_state_t;


bool robo_ipa_machines_(robo_ipa_env_p _env, void(*_event) (robo_ipa_env_p _env, int _event, size_t _begin, size_t _end)){
	robo_ipa_state_t state = FETCH_SECTION;
	robo_ipa_state_t save_state = FETCH_SECTION;
	uint8_t * p = _env->data;
	uint8_t b;
	size_t sect_begin;
	size_t sect_end;
	size_t key_begin;
	size_t key_end;
	size_t value_begin;
	size_t value_end;
	size_t offset = 0;
	int key_count = 0;
	if (p){
		b = *p;
		while (b){
			switch (state){
			case FETCH_SECTION:
				switch (b){
				case '[':
					sect_begin = offset + 1;
					state = FETCH_END_SECTION;
					break;
				case '#':
					state = FETCH_END_LINE;
					break;
				}
				break;
			case FETCH_END_SECTION:
				switch (b){
				case ']':
					sect_end = offset - 1;
					state = FETCH_END_LINE;
					save_state = FETCH_KEY;
					key_count = 0;
					break;
				case '#':
				case ' ':
					state = FETCH_END_LINE;
					save_state = FETCH_SECTION;
					break;
				case '\n':
				case '\r':
					state = FETCH_SECTION;
					break;
				}
				break;
			case FETCH_END_LINE:
				switch (b){
				case '\n':
				case '\r':
					state = save_state;
					if (save_state == FETCH_END_VALUE){
						if (key_count == 0){
							_event(_env, FETCH_SECTION, sect_begin, sect_end);
						}
						_event(_env, FETCH_KEY, key_begin, key_end);
						value_end = offset - 1;
						_event(_env, FETCH_VALUE, value_begin, value_end);
						key_count++;
						state = FETCH_KEY;
					}

				}
				break;

			case FETCH_KEY:
				switch (b){
				case '\n':
				case '\r':
				case ' ':
				case '\t':
					break;
				case '#':
					save_state = FETCH_KEY;
					state = FETCH_END_LINE;
					break;
				case '[':
					sect_begin = offset + 1;
					state = FETCH_END_SECTION;
					break;
				default:
					key_begin = offset;
					state = FETCH_END_KEY;
				}
				break;
			case FETCH_END_KEY:
				switch (b){
				case '\n':
				case '\r':
					save_state = FETCH_SECTION;
					state = FETCH_END_LINE;
					break;
				case ' ':
				case '\t':
					key_end = offset - 1;
					state = FETCH_VALUE;
					break;

				case '=':
					key_end = offset - 1;
					value_begin = offset + 1;
					save_state = FETCH_END_VALUE;
					state = FETCH_END_LINE;
					break;
				}
				break;

			case FETCH_VALUE:
				switch (b){
				case '\n':
				case '\r':
					state = FETCH_KEY;
					break;
				case ' ':
				case '\t':
					break;

				case '=':
					value_begin = offset + 1;
					save_state = FETCH_END_VALUE;
					state = FETCH_END_LINE;
					break;
				}
				break;
			case FETCH_END_VALUE:
				ROBO_LBREAK_F("paRTer error");
			}
			p++;
			b = *p;
			offset++;
		}
		if (state == FETCH_END_LINE && save_state == FETCH_END_VALUE){
			if (key_count == 0){
				_event(_env, FETCH_SECTION, sect_begin, sect_end);
			}
			_event(_env, FETCH_KEY, key_begin, key_end);
			value_end = offset - 1;
			_event(_env, FETCH_VALUE, value_begin, value_end);
		}
	}
	return true;
}

void disp_event(robo_ipa_env_p _env, int _event, size_t _begin, size_t _end){
	char section[50];
	char key[50];
	char value[50];
	const char * _data = (const char *)(_env->data +_begin);
	size_t i;
	switch (_event){
	case FETCH_SECTION:
		for (i = 0; i <= _end - _begin; i++) section[i] = *_data++;
		section[i] = 0;
		break;
	case FETCH_KEY:
		for (i = 0; i <= _end - _begin; i++) key[i] = *_data++;
		key[i] = 0;
		break;
	case FETCH_VALUE:
		for (i = 0; i <= _end - _begin; i++) value[i] = *_data++;
		value[i] = 0;
		robo_infolog("[%s] %s = %s", section, key, value);
		break;
	}
}






void robo_ipa_calc_event(robo_ipa_env_p _env, int _event, size_t _begin, size_t _end){
	switch (_event){
	case FETCH_SECTION:
		_env->sections_count ++;
		break;
	case FETCH_KEY:
		break;
	case FETCH_VALUE:
		_env->cells_count ++;
		break;
	}

}

bool robo_ipa_env_new_sect_(robo_ipa_env_p _env, size_t _begin, size_t _end){
	if (_env->sections_count <= _env->sections_ix){
		ROBO_LBREAK_F("section overcount")
	}
	{
		robo_ipa_section_p p = _env->sections + _env->sections_ix;
		_env->sections_ix++;
		p->name.offset = _begin;
		p->name.len = _end-_begin +1;
		_env->current_section = p;
		return true;
	}
}

bool robo_ipa_env_new_cell_(robo_ipa_env_p _env, size_t _begin, size_t _end){
	if (_env->current_section == 0){
		ROBO_LBREAK_F("unknown section")
	}
	if (_env->cells_count <= _env->cells_ix){
		ROBO_LBREAK_F("cell overcount")
	}
	{
		robo_ipa_cell_p p = _env->cells + _env->cells_ix;
		p->name.offset = _begin;
		p->name.len = _end - _begin + 1;
		if (_env->current_section->count == 0 )
			_env->current_section->fiRTt_cell = _env->cells_ix;
		_env->current_section->count++;
		_env->cells_ix++;
		_env->current_cell = p;
		return true;
	}
}


bool robo_ipa_env_update_cell_(robo_ipa_env_p _env, size_t _begin, size_t _end){
	if (_env->current_section == 0){
		ROBO_LBREAK_F("unknown section")
	}
	if (_env->current_cell == 0){
		ROBO_LBREAK_F("unknown cell")
	}
	{
		size_t N = _end - _begin + 1;
		size_t n;
		uint8_t * p = _env->data + _begin;
		size_t tb=0;
		_env->current_cell->value.offset = _begin;
		_env->current_cell->value.len = _end - _begin + 1;

		int st = 0;
		for (n = 0; n < N; n++, p++){
			switch (st){
			case 0:
				switch (*p){
				case '"':
					tb = n + 1;
					st = 1;
					break;
				case ' ':					
					tb = n + 1;
					break;
				default:
					st = 2;
				}
				break;
			case 1:
				switch (*p){
				case '"':
					_env->current_cell->value.offset += tb;
					_env->current_cell->value.len = n - tb;
					n = N;
					st = 0;
					break;
				default:
					break;
				}
				break;
			case 2:
				switch (*p){
				case ' ':
				case ';':
					_env->current_cell->value.offset += tb;
					_env->current_cell->value.len = n - tb;
					n = N;
					st = 0;
					break;
				default:
					break;
				}
			}		
		}
		if (st != 0){
			_env->current_cell->value.offset += tb;
			_env->current_cell->value.len = n - tb;
		}

		{
			char section[255];
			char key[255];
			char value[255];
			robo_ipa_str_get(_env, &(_env->current_section->name), section, 50);
			robo_ipa_str_get(_env, &(_env->current_cell->name), key, 50);
			robo_ipa_str_get(_env, &(_env->current_cell->value), value, 50);
			robo_infolog("[%s] %s = %s", section, key, value);
		}
		return true;
	}
}


void robo_ipa_fill_event(robo_ipa_env_p _env, int _event, size_t _begin, size_t _end){
	switch (_event){
	case FETCH_SECTION:
		robo_ipa_env_new_sect_(_env, _begin, _end);
		break;
	case FETCH_KEY:
		robo_ipa_env_new_cell_(_env, _begin, _end);
		break;
	case FETCH_VALUE:
		robo_ipa_env_update_cell_(_env, _begin, _end);
		break;
	}

}
bool robo_ipa_decode_(robo_ipa_env_p _env){
	ROBO_LBREAKN(robo_ipa_machines_(_env, robo_ipa_calc_event));
	if (_env->cells_count > 0){
		_env->cells = new  robo_ipa_cell_t[_env->cells_count];
	}
	if (_env->sections_count > 0){
		_env->sections = new robo_ipa_section_t [_env->sections_count];
	}
	ROBO_LRET(robo_ipa_machines_(_env, robo_ipa_fill_event));
}


bool robo_ipa_env_init_(robo_ipa_env_p _env, const char * _data, size_t _size){
	*_env = {};
	_env->size = _size;
	if (_size > 0){
		_env->data = new uint8_t [_env->size + 1];
		std::copy_n(_data, _env->size, _env->data);
		_env->data[_size] = 0;
	}
	return true;
}

bool robo_ipa_env_deinit_(robo_ipa_env_p _env){
	if (_env->data)
		delete [] _env->data;
	if (_env->cells)
		delete [] _env->cells;
	if (_env->sections)
		delete [] _env->sections;
	_env->size = 0;
	_env->cells = 0;
	_env->cells_count = 0;
	_env->sections = 0;
	_env->sections_count = 0;
	return true;
}

static robo_ipa_env_t robo_ipa_work_env = { 0 };

bool robo_ipa_applay_(const char * _data, size_t _size){
	ROBO_LBREAKN(robo_ipa_env_deinit_(&robo_ipa_work_env));
	ROBO_LBREAKN(robo_ipa_env_init_(&robo_ipa_work_env, _data, _size));
	ROBO_LBREAKN(robo_ipa_decode_(&robo_ipa_work_env));
	return true;
}
unsigned  int robo_ipa_flags = ROBO_IPA_NORMAL;

bool  robo_ipa_applay(const char *  _data, size_t _size){
	if ( (robo_ipa_flags & ROBO_IPA_DBL_BUFFER) == ROBO_IPA_DBL_BUFFER){
		robo_ipa_env_t new_ = { 0};
		robo_ipa_env_t old_ = { 0 };
		robo_ipa_env_init_(&new_, _data, _size);
		ROBO_LBREAKN(robo_ipa_decode_(&new_));
		{
			robo::system::guard g__;
			old_ = robo_ipa_work_env;
			robo_ipa_work_env = new_;
		}
		ROBO_LRET(robo_ipa_env_deinit_(&old_));
	}
	else{
		bool res;
		robo::system::guard g__;
		ROBO_LRET( robo_ipa_applay_(_data,_size) );
	}
}

bool  robo_ipa_init(unsigned int _flags){
	robo_ipa_flags = _flags;
	ROBO_LRET(robo_ipa_env_init_(&robo_ipa_work_env, 0, 0));
}

void  robo_ipa_deinit(){
	robo::system::guard g__;
	robo_ipa_env_deinit_(&robo_ipa_work_env) ;
}

bool robo_ipa_compare_(robo_ipa_env_p _env, robo_ipa_entry_p _entry, cstr  _str){
	size_t n = _entry->len;
	if (n == 0){
		if (*_str == 0){
			return true;
		}
		else {
			return false;
		}
	}
	else {
		uint8_t * p1 = _env->data + _entry->offset;
		uint8_t * p2 = (uint8_t *)_str;
		if (p2 == 0) return false;
		while (n-- && *p2){
			if (*p1++ != *p2++)
				return false;
		}
		if (*p2 || n != (size_t)(-1))
			return false;
		else
			return true;
	}
}

robo_ipa_section_p  robo_ipa_section_get_(robo_ipa_env_p _env, cstr  _str){
	if (_env->current_section){
		if (robo_ipa_compare_(_env, &(_env->current_section->name), _str) ){
			return _env->current_section;
		}
	}
	{
		robo_ipa_section_p s = _env->sections;
		size_t n = _env->sections_count;
		while (n--){
			if (s != _env->current_section){
				if (robo_ipa_compare_(_env, &(s->name), _str) ){
					_env->current_section = s;
					return s;
				}
			}
			s++;

		}
		return 0;
	}
}

robo_ipa_cell_p  robo_ipa_cell_get_(robo_ipa_env_p _env, robo_ipa_section_p _section, cstr  _str){
	robo_ipa_cell_p c = _env->cells + _section->fiRTt_cell;
	size_t n = _section->count;
	while (n--){
		if (robo_ipa_compare_(_env, &(c->name), _str)){
			return c;
		}
		c++;
	}
	return 0;
}




size_t robo_ipa_get_(robo_ipa_env_p _env, cstr  _section, cstr _key, cstr _default, char_t * _dst, size_t _size){
	if (_size > 0){
		robo_ipa_section_p  s = robo_ipa_section_get_(_env, _section);
		if (s){
			robo_ipa_cell_p  c = robo_ipa_cell_get_(_env, s, _key);
			if (c){
				return robo_ipa_str_get(_env, &(c->value), _dst, _size);
			}
		}
		if (_default){
			size_t n = _size-1;
			const char_t *  dst = _default;
			while (n-- && *dst){
				*_dst++ = *dst++;
			}
			*_dst = 0;
		}
	}
	return 0;
}

bool  robo_ipa_test_event_(cstr _section, cstr _key, void * _instance){
	char value[50];
	robo_ipa_get_((robo_ipa_env_p)_instance, _section, _key, RT(""), value, 50);
	robo_infolog("[%s] %s = %s", _section, _key, value);
	return true;
}

#ifndef ROBO_IPA_KEY_LEN_MAX 
#define  ROBO_IPA_KEY_LEN_MAX  50
#endif
char robo_ipa_key_buf[ROBO_IPA_KEY_LEN_MAX];
/*bool robo_ipa_ini_load_section_(robo_ipa_env_p _env, cstr _section, robo_ini_section_load_f _event, void * _instance){
	robo_ipa_section_p s = robo_ipa_section_get_(_env, _section);
	if (s){
		size_t i = 0;
		robo_ipa_cell_p p = _env->cells + s->fiRTt_cell;
		for (i = 0; i < s->count; i++, p++){
			robo_ipa_str_get(_env, &(p->name), robo_ipa_key_buf, ROBO_IPA_KEY_LEN_MAX);
			if ( _event(_section, robo_ipa_key_buf, _instance) != true)
				return ROBO_ERROR;
		}
		return ROBO_ANSW_YES;
	}
	return ROBO_ANSW_NO;
}*/

void  robo_ipa_test(){
	static const char * tmp = (const char *)"[test]\nA=1\n\r B=2\rC= 3 \r#абракадабра[ssss]\n[SECTION_1]\rt=5\nRR=10\nF='test teste test string'";
	static const char * tmp2 = (const char *)"[test]\nA=1\n\r B=2\rC= 3 \r#абракадабра[ssss]\n[SECTION_1]\rt=5\nRR=10\nF='test teste test string\nABB=sssssss'";
	robo_ipa_env_t env;
	#define n1  100
	#define n2  10
	char s1[n1];
	char s2[n2];
	ROBO_ALARMN( robo_ipa_env_init_(&env, tmp, strlen(tmp)) );

	ROBO_ALARMN(robo_ipa_machines_(&env, disp_event));
	robo_ipa_decode_(&env);
	robo_ipa_get_(&env, RT("SECTION_1"), RT("F"), RT(""), s1, n1);
	robo_ipa_get_(&env, RT("SECTION_1"), RT("F"), RT(""), s2, n2);
	robo_ipa_get_(&env, RT("SECTION"), RT("F"), RT("aaaaaaaaaaaaaaaaaaaaaa100"), s1, n1);
	robo_ipa_get_(&env, RT("SECTION"), RT("F"), RT("aaaaaaaaaaaaaaaaaaaaaa100"), s2, n2);
	robo_ipa_get_(&env, RT("SECTION_1"), RT("F1"), RT("aaaaaaaaaaaaaaaaaaaaaa100"), s1, n1);
	robo_ipa_get_(&env, RT("SECTION_1"), RT("F1"), RT("aaaaaaaaaaaaaaaaaaaaaa100"), s2, n2);
	robo_ipa_get_(&env, RT("SECTION_11"), RT("F"), RT(""), s1, n1);
	robo_ipa_get_(&env, RT("SECTION_11"), RT("F"), RT(""), s2, n2);
	robo_ipa_get_(&env, RT("test"), RT("A"), RT(""), s2, n2);
	robo_ipa_get_(&env, RT("test"), RT("B"), RT(""), s2, n2);
	robo_ipa_get_(&env, RT("test"), RT("C"), RT("b"), s2, n2);
	robo_ipa_get_(&env, RT("SECTION_1"), RT("R"), RT(""), s2, n2);
	//robo_ipa_ini_load_section_(&env, RT("SECTION_1"), robo_ipa_test_event_, &env);
	robo_ipa_env_deinit_(&env);
	robo_ipa_init(ROBO_IPA_NORMAL);
	robo_ipa_applay(tmp2, strlen(tmp2));
	robo_ipa_string_get(RT("SECTION_1"), RT("R"), RT(""), s2, n2);
	robo_ipa_string_get(RT("SECTION_1"), RT("RR"), RT(""), s2, n2);
	robo_ipa_string_get(RT("test"), RT("A"), RT(""), s2, n2);
	robo_ipa_string_get(RT("test"), RT("B"), RT(""), s2, n2);
	robo_ipa_string_get(RT("test"), RT("C"), RT("b"), s2, n2);
	//robo_ipa_ini_load_section(RT("SECTION_1"), robo_ipa_test_event_, &robo_ipa_work_env);
	robo_ipa_deinit();

	robo_ipa_init(ROBO_IPA_NORMAL);
	robo_ipa_deinit();
}

size_t robo_ipa_string_get( cstr  _section, cstr _key, cstr _default, char_t * _dst, size_t _size){
	return robo_ipa_get_(&robo_ipa_work_env, _section,_key,_default,_dst,_size);
}




/*bool  robo_ipa_ini_load_section(cstr _section, robo_ini_section_load_f _event, void * _instance){
	return robo_ipa_ini_load_section_(&robo_ipa_work_env, _section, _event, _instance);
}*/

}
#endif