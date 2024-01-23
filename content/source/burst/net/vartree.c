#include "burst/net/vartree.h"
#include "burst/burst.h"
#include <stdio.h>

burst_varreg_p * burst_varreg_top = 0;
uint16_t burst_varreg_count =0;

void _memcopy(const uint8_t * src,uint8_t * dst, uint8_t len){
	while (len>0){
			*dst=*src;
			dst++;
			src++;
			len--;
	}
}

uint8_t burst_var_name_len(const char * s){
	uint8_t ret;
	for (ret = 0; ret<255;ret++,s++){
			if(*s==0) break;
	}
	return ret;
}


int burst_vartable_perform( burst_vartable_p _vartable,  const uint8_t * _buf_in, uint8_t * _buf_out){
	int   query = _buf_in[0];
	int   ix;

	switch (query) {
	case BURST_VAR_QUERY_INDEX:
	{
		int32_t  key = *(int32_t*)(_buf_in + 1);
		ix = burst_vartable_hash_to_id(_vartable,key);

		if (ix >= 0 && ix < _vartable->count) {
			const burst_var_p  r = (burst_var_p) _vartable->table [ix];
			_buf_out[0] = query;
			_buf_out[1] = (uint8_t)ix;
			_buf_out[2] = r->desc.bytes[0];
			_buf_out[3] = r->desc.bytes[1];
		}
		else {
			_buf_out[0] = query | BURST_VAR_QUERY_INVALID_KEY; //query
			_buf_out[1] = 0xff;//ix
			_buf_out[2] = 0xff;
			_buf_out[3] = 0xff;
		}
		return 4;
	}
	case BURST_VAR_QUERY_GET:
	{
		ix = _buf_in[1];
		//len = _buf_in[2];
		_buf_out[1] = ix;
		if (ix <  _vartable->count) {
			const burst_var_p  r = (burst_var_p) _vartable->table [ix];
			if ( r->desc.len<=BURST_VAR_MAX_LEN) {
				_buf_out[0] = query;
				{
					_memcopy( (uint8_t*)r->address,_buf_out + 2, r->desc.len );
				}
			}
			else {
				_buf_out[0] = query | BURST_VAR_QUERY_INVALID_LENGTH;
			}
			return 2 + r->desc.len;
		}
		else {
			_buf_out[0] = query | BURST_VAR_QUERY_INVALID_INDEX;
			//todo мы не знаем, что за переменная - сервер получит сообщение неверной длины, но  такое возможно при неверном desc
			return 1;
		}
	}
	case BURST_VAR_QUERY_PUT:
	{
		ix = _buf_in[1];
		_buf_out[1] = ix;
		if (ix <  _vartable->count) {
			const burst_var_p  r = (burst_var_p) _vartable->table [ix];
			if (r->desc.len <= BURST_VAR_MAX_LEN) {
				{
					_memcopy(_buf_in + 2,(uint8_t*)r->address, r->desc.len );
				}
				_buf_out[0] = query;
			}
			else {
				_buf_out[0] = query | BURST_VAR_QUERY_INVALID_LENGTH;
			}
		}
		else {
			_buf_out[0] = query | BURST_VAR_QUERY_INVALID_INDEX; ;
		}
		return 2;
	}
	}
	return 0;
}

void burst_vartable_reg(burst_vartable_p _vartable, burst_varreg_p _var){
	if (_vartable->tableSize > 0){
		_vartable->table[_vartable->count++] = _var;
		_vartable->tableSize--;
	}
}

void burst_vartable_init(burst_vartable_p _vartable, burst_varreg_p  * _vars ,  uint8_t _size){
	if (_size > 0){
		_vartable->table = _vars;
		_vartable->tableSize = _size;
		_vartable->count = 0;
	}
	else{
		_vartable->table = 0;
		_vartable->tableSize = 0;
		_vartable->count = 0;
	}
}

void burst_vartable_deinit(burst_vartable_p _vartable){
	_vartable->table = 0;
	_vartable->tableSize = 0;
	_vartable->count = 0;

}

uint8_t burst_vartable_hash_to_id(burst_vartable_p _vartable, int32_t _hash){
	burst_varreg_p * reg = _vartable->table;
	burst_var_p v;
	for (uint8_t i = 0; i < _vartable->count; ++i, ++reg){
		if ((*reg)->type == BURST_VAR_TYPE_VAR){
			v = (burst_var_p)(*reg);
			if (v->full_path_hash == _hash){
				return i;
			}
		}
	}
	return 0xFF;
}


#ifndef BURST_TERMO_VT_BUFFER_SIZE
#define BURST_TERMO_VT_BUFFER_SIZE 50
#endif

#ifndef BURST_TERMO_VT_STACK_SIZE
#define BURST_TERMO_VT_STACK_SIZE 10
#endif

void burst_vartable_create_index(burst_vartable_p _vartable){
	char path_content[BURST_TERMO_VT_BUFFER_SIZE + 1];
	int path_stack_buffer[BURST_TERMO_VT_STACK_SIZE];
	uint8_t index_count = _vartable->count;
	burst_varreg_p * index_ptr = _vartable->table;

	burst_size_t path_size = BURST_TERMO_VT_BUFFER_SIZE;
	char * path_ptr = path_content;
	int * path_stack_top = path_stack_buffer;
	int path_stack_level = 0;

	for (uint8_t i = 0; i < index_count; ++i, ++index_ptr){
		if ( (*index_ptr)->type == BURST_VAR_TYPE_VAR){
			((burst_var_p)*index_ptr)->full_path_hash = 0xFFFF;
		}
	}
	index_ptr = _vartable->table;
	while (index_count>0){
			burst_size_t sz;
			burst_varreg_p varreg = *((index_ptr)++);
			index_count--;
			switch (varreg->type){
			case BURST_VAR_TYPE_PUSH:
				if (path_size){
					if (path_stack_level < BURST_TERMO_VT_STACK_SIZE){
						sz = BURST_STD_SNPRINTF(path_ptr, path_size, ".%s", ((burst_varnode_p)(varreg))->name);
						*(path_stack_top) = sz;
						path_ptr[sz] = 0;
						path_ptr += sz;
						path_size -= sz;
						path_stack_top++;
						path_stack_level++;
					}
					else{
						return;
					}
				}
				break;
			case BURST_VAR_TYPE_POP:
				path_stack_top--;
				path_stack_level--;
				sz = *(path_stack_top);
				path_ptr -= sz;
				*(path_ptr) = 0;
				path_size += sz;
				break;
			default:
				//volatile uintptr_t ptr = ((uintptr_t)(((burst_var_p)(varreg))->address) - addr_offset_);
				//itf::printf("%s.%s\t%d\t%x\n\r", path_.content + 1, ((burst_varnode_p)(varreg))->name, ((burst_var_p)(varreg))->len, (unsigned int)ptr);
				sz = BURST_STD_SNPRINTF(path_ptr, path_size, ".%s", ((burst_varnode_p)(varreg))->name);
				*(path_stack_top) = sz;
				path_ptr[sz] = 0; 
				((burst_var_p)(varreg))->full_path_hash = burst_string_hash(path_content+1, 0);
			}		
	}
}

