#include "burst++/vartree.hpp"
#include "burst++/burst.hpp"
#include "core/robosd_ring_buf.hpp"
#include "core/robosd_common.hpp"

#if BURST_VAR_ENABLED
#include <algorithm>
namespace burst {
	namespace var {
		robo::char_t path[path_size + 1] = {};

		ref_s** index = new ref_s * [pool_size];
		int count = 0;
		void reg(ref_s* _r) {
			ROBO_APP_ASSERT(count < pool_size);
			index[count] = _r;
			count++;
		}
		void push(robo::cstr _name) {
			node_s* n = new node_s;
			n->name = _name;
			n->ref.tag = tags::push;
			reg(&(n->ref));
		}
		void pop(void) {
			ref_s* r = new ref_s;
			r->tag = tags::pop;
			reg(r);
		}

		#ifndef BURST_VAR_STACK_SIZE
		#define BURST_VAR_STACK_SIZE 10
		#endif
		
		void reindex(void) {
			//todo посчитать
			robo::char_t * path = new robo::char_t [path_size + 1];
			int * path_stack_buffer = new int [stack_size];

			ref_s** index_ptr = index;
			int psz = path_size;
			robo::char_t* path_ptr = path;
			int* path_stack_top = path_stack_buffer;
			int path_stack_level = 0;

			for (uint8_t i = 0; i < count; ++i, ++index_ptr) {
				if ((*index_ptr)->tag == tags::var ) {
					((record_s *)*index_ptr)->key = 0xFFFF;
				}
			}
			index_ptr = index;
			int index_count = count;
			while (index_count > 0) {
				int sz;
				ref_s * ref = *((index_ptr)++);
				index_count--;
				switch (ref->tag) {
				case tags::push:
				if (psz) {
					if (path_stack_level < stack_size) {
						sz = robo::system::sprintf(path_ptr, psz, RT(".%s"), ((record_s *)(ref))->name);
						*(path_stack_top) = sz;
						path_ptr[sz] = 0;
						path_ptr += sz;
						psz -= sz;
						path_stack_top++;
						path_stack_level++;
					}
					else {
						delete[] path;
						delete[] path_stack_buffer;
						return;
					}
				}
				break;
				case tags::pop:
				path_stack_top--;
				path_stack_level--;
				sz = *(path_stack_top);
				path_ptr -= sz;
				*(path_ptr) = 0;
				psz += sz;
				break;
				default:
				sz = robo::system::sprintf(path_ptr, psz, RT(".%s"), ((record_s *)(ref))->name);
				*(path_stack_top) = sz;
				path_ptr[sz] = 0;
				((record_s *)(ref))->key = robo::hash(path + 1, 0);
				}	
			}
			delete[] path;
			delete[] path_stack_buffer;
		}
		burst::board::slot::simple start(
			burst::board::slot::kind::start
			, [] {
				reindex();
			}
		);
		void free(void) {
			ref_s** r = index;
			for (int i = 0; i < count; ++i,++r) {
				delete *r;
			}
		}
		int find(int _key) {
			ref_s * * r = index;
			for (int i = 0; i < count; ++i,++r) {
				if ((*r)->tag == tags::var) {
					record_s* var = (record_s*)(*r);
					if (var->key == _key) {
						return i;
					}
				}
				
			}
			return -1;
		}
		const record_s* get(int _index) {
			if (_index >= 0 && _index < count) {
				ref_s** r = index + _index;
				if ((*r)->tag == tags::var) {
					return (record_s*)(*r);
				}
			}
			return nullptr;
		}


		

		int proto(const  uint8_t* _buf_in, uint8_t* _buf_out) {
			int   query = _buf_in[0];
			int   ix;
			

			switch (query) {
			case request::index:
			{
				int32_t  key = *(int32_t*)(_buf_in + 1);
				ix = find(key);
				const record_s* r = get(ix);
				if (r) {					
					_buf_out[0] = query;
					_buf_out[1] = (uint8_t)ix;
					_buf_out[2] = r->desc.bytes[0];
					_buf_out[3] = r->desc.bytes[1];
				}
				else {
					_buf_out[0] = query | invalid_key; //query
					_buf_out[1] = 0xff;//ix
					_buf_out[2] = 0xff;
					_buf_out[3] = 0xff;
				}
				return 4;
			}
			case request::get:
			{
				ix = _buf_in[1];
				//len = _buf_in[2];
				_buf_out[1] = ix;
				const record_s* r = get(ix);
				if (r) {
					if ( r->desc.len<=max_len) {
						_buf_out[0] = query;
						{
							::robo::system::guard g__;
							std::copy_n((uint8_t*)r->addr, r->desc.len, _buf_out + 2);
						}
					}
					else {
						_buf_out[0] = query | invalid_length; ;
					}
					return 2 + r->desc.len;
				}
				else {
					_buf_out[0] = query | invalid_index; ;
					//todo мы не знаем, что за переменная - сервер получит сообщение неверной длины, но  такое возможно при неверном desc
					return 1;
				}
			}
			case request::put:
			{
				ix = _buf_in[1];
				//len = _buf_in[2];
				_buf_out[1] = ix;
				const record_s* r = get(ix);
				if ( r ) {
					if (r->desc.len <= max_len) {
						{
							::robo::system::guard g__;
							std::copy_n(_buf_in + 2, r->desc.len, (uint8_t*)r->addr);
						}
						_buf_out[0] = query;
					}
					else {
						_buf_out[0] = query | invalid_length; ;
					}
				}
				else {
					_buf_out[0] = query | invalid_index; ;
				}
				return 2;
			}
			/*case request::page_get:
			{
				ix = _buf_in[1];
				len = _buf_in[2];
				offset = _buf_in[3];
				_buf_out[1] = ix;
				_buf_out[2] = offset;
				if (ix < count_) {
					const record* r = index_[ix];
					if (len <= max_len - 1 && len + offset <= r->desc.len) {
						_buf_out[0] = query;
						{
							::robo::system::guard g__;
							std::copy_n(((uint8_t*)r->addr) + offset, len, _buf_out + 3);
						}
					}
					else {
						_buf_out[0] = query | invalid_length; ;
					}
				}
				else {
					_buf_out[0] = query | invalid_index; ;
					std::fill_n(_buf_out + 3, len, 0xFF);
				}
				return 3 + len;
			}
			case request::page_put:
			{
				ix = _buf_in[1];
				offset = ((uint16_t)_buf_in[3])*256 +  _buf_in[2];
				_buf_out[2] = _buf_in[2];
				_buf_out[3] = _buf_in[3];
				if (ix < count_) {
					const record* r = index_[ix];
					if (len <= max_len - 1 && len + offset <= r->desc.len) {
						_buf_out[0] = query;
						{
							::robo::system::guard g__;
							std::copy_n(_buf_out + 3, len, ((uint8_t*)r->addr) + offset);
						}
					}
					else {
						_buf_out[0] = query | invalid_length; ;
					}
				}
				else {
					_buf_out[0] = query | invalid_index; ;
				}
				return 3;
			}*/
			}

			return 0;
		}

		#if ROBO_APP_FORMATING_TYPE != ROBO_APP_TYPE_NONE
		size_t sprintf(record_s & _rec, ::robo::char_t* buf, size_t _max_sz) {
			switch (_rec.desc.memo) {
			case uint8:
			case const_uint8:
			return ::robo::system::sprintf(buf, _max_sz, RT("%u"), (unsigned int)*(uint8_t*)_rec.addr);
			case int8:
			case const_int8:
			return ::robo::system::sprintf(buf, _max_sz, RT("%d"), (int)*(int8_t*)_rec.addr);
			case uint16:
			case const_uint16:
			return ::robo::system::sprintf(buf, _max_sz, RT("%u"), (unsigned int)*(uint16_t*)_rec.addr);
			case int16:
			case const_int16:
			return ::robo::system::sprintf(buf, _max_sz, RT("%d"), (int)*(int16_t*)_rec.addr);
			case uint32:
			case const_uint32:
			return ::robo::system::sprintf(buf, _max_sz, RT("%lu"), (unsigned long)*(uint32_t*)_rec.addr);
			case int32:
			case const_int32:
			return ::robo::system::sprintf(buf, _max_sz, RT("%ld"), (long)*(int32_t*)_rec.addr);
			case uint64:
			case const_uint64:
			return ::robo::system::sprintf(buf, _max_sz, RT("%llu"), (unsigned long long) * (uint64_t*)_rec.addr);
			case int64:
			case const_int64:
			return ::robo::system::sprintf(buf, _max_sz, RT("%lld"), (long long)*(int64_t*)_rec.addr);
			case real:
			case const_real:
			return ::robo::system::sprintf(buf, _max_sz, RT("%f"), (float)*(float*)_rec.addr);
			case ext:
			case const_ext:
			return ::robo::system::sprintf(buf, _max_sz, RT("%f"), (double)*(double*)_rec.addr);
			}
			return 0;
		}
		#endif
		ref_s* * first(void) {
			if(count>0){
				return  index;
			}
			else {
				return nullptr;
			}
		}
		ref_s** last(void) {
			if (count > 0) {
				return  index+count-1;
			}
			else {
				return nullptr;
			}
		}

	}
}
#endif