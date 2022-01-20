#include "mexo/vartable.hpp"
#if ROBO_APP_MEXO_VAR_ENABLED
#include <algorithm>
namespace mexo {
	namespace var {
		#ifdef ROBO_APP_MEXO_SIDE
		void record::setup_(
			types _type
			, const void* _addr
			, robo::cstr _name
			, int  _master_key
		) {
			addr = _addr;
			name = _name;
			key = robo::hash(RT("."), _master_key);
			key = robo::hash(_name, key);
			desc.memo = _type;
			//machine::reg(this);
		}

		machine& machine::instance_(void) {
			static machine machine_;
			return machine_;
		}

		void machine::begin_(int _pool_size) {
			if (index_ != nullptr) {
				delete[] index_;
				index_ = nullptr;
			}
			index_size_ = _pool_size;
			if (index_size_ > 0) {
				index_ = new const record * [index_size_];
			}
			count_ = 0;
		}

		machine::~machine(void) {
			if (index_ != nullptr) delete[] index_;
		}

		int machine::find_(int _key) {
			const record** r = index_;
			for (int i = 0; i < count_; ++i, ++r) {
				if ((*r)->key == _key) {
					return i;
				}
			}
			return -1;
		}
		const record* machine::get_(int _index) {
			if (_index >= 0 && _index < count_) {
				return index_[_index];
			}
			else return nullptr;
		}
		void  machine::reg_(
			const record& _precord
		) {
			ROBO_APP_ASSERT(count_ < index_size_);
			int ix = find_(_precord.key);
			ROBO_APP_ASSERT(ix < 0);
			index_[count_] = &_precord;
			count_++;
		}

		int  machine::proto_(const  uint8_t* _buf_in, uint8_t* _buf_out) {
			request   query = (request)_buf_in[0];
			int   ix;
			uint8_t offset;
			uint8_t len;

			switch (query) {
			case request::index:
			{
				int32_t  key = *(int32_t*)(_buf_in + 1);
				ix = find_(key);

				if (ix >= 0 && ix < count_) {
					const record* r = index_[ix];
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
				len = _buf_in[2];
				_buf_out[1] = ix;
				if (ix < count_) {
					const record* r = index_[ix];
					if (len <= max_len && len == r->desc.len) {
						_buf_out[0] = query;
						{
							::robo::system::guard g__;
							std::copy_n((uint8_t*)r->addr, len, _buf_out + 2);
						}
					}
					else {
						_buf_out[0] = query | invalid_length; ;
					}
				}
				else {
					_buf_out[0] = query | invalid_index; ;
					std::fill_n(_buf_out + 2, len, 0xFF);
				}
				return 2 + len;
			}
			case request::put:
			{
				ix = _buf_in[1];
				len = _buf_in[2];
				_buf_out[1] = ix;
				if (ix < count_) {
					const record* r = index_[ix];
					if (len <= max_len && len == r->desc.len) {
						{
							::robo::system::guard g__;
							std::copy_n(_buf_in + 3, len, (uint8_t*)r->addr);
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
			case request::page_get:
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
			}
			}

			return 0;
		}

		size_t record::sprintf(::robo::char_t* buf, size_t _max_sz) {
			switch (desc.memo) {
			case uint8:
			case const_uint8:
			return ::robo::system::sprintf(buf, _max_sz, RT("%hu"), (unsigned short)*(uint8_t*)addr);
			case int8:
			case const_int8:
			return ::robo::system::sprintf(buf, _max_sz, RT("%hd"), (short)*(int8_t*)addr);
			case uint16:
			case const_uint16:
			return ::robo::system::sprintf(buf, _max_sz, RT("%hu"), (unsigned short)*(uint16_t*)addr);
			case int16:
			case const_int16:
			return ::robo::system::sprintf(buf, _max_sz, RT("%hd"), (short)*(int16_t*)addr);
			case uint32:
			case const_uint32:
			return ::robo::system::sprintf(buf, _max_sz, RT("%lu"), (unsigned long)*(uint32_t*)addr);
			case int32:
			case const_int32:
			return ::robo::system::sprintf(buf, _max_sz, RT("%ld"), (long)*(int32_t*)addr);
			case uint64:
			case const_uint64:
			return ::robo::system::sprintf(buf, _max_sz, RT("%llu"), (unsigned long long) * (uint64_t*)addr);
			case int64:
			case const_int64:
			return ::robo::system::sprintf(buf, _max_sz, RT("%lld"), (long long)*(int64_t*)addr);
			case real:
			case const_real:
			return ::robo::system::sprintf(buf, _max_sz, RT("%f"), (float)*(float*)addr);
			case ext:
			case const_ext:
			return ::robo::system::sprintf(buf, _max_sz, RT("%f"), (double)*(double*)addr);
			}
			return 0;
		}

		#endif
	}
}
#endif