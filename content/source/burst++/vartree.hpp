#ifndef burst_vartree_hpp
#define burst_vartree_hpp
#include "burst++/burst_common.hpp"

#if BURST_VAR_ENABLED == 1

#ifndef BURST_VAR_BUFFER_SIZE 
#define BURST_VAR_BUFFER_SIZE 50
#endif

#include "core/robosd_string.hpp"
#include "core/robosd_list.hpp"
namespace burst {
	namespace var {
		struct tags { enum { push = 0, var = 1, pop = 2 }; };
		struct ref_s {
			uint8_t tag : 2;
			uint8_t reserv : 6;
		};

		#ifndef BURST_VAR_POOL_SIZE
		#define BURST_VAR_POOL_SIZE 20
		#endif
		#ifndef BURST_VAR_STACK_SIZE
		#define BURST_VAR_STACK_SIZE 10
		#endif
		enum { pool_size = BURST_VAR_POOL_SIZE, path_size = BURST_VAR_BUFFER_SIZE, stack_size = BURST_VAR_STACK_SIZE};

		struct node_s {
			ref_s ref;
			robo::cstr name;
		};

		union descriptor {
			struct {
				uint16_t tag : 2;
				uint16_t len : 11;
				uint16_t bsign : 1;
				uint16_t bconst : 1;
				uint16_t real : 1;
				//uint16_t fault : 5;
			};
			uint16_t memo;
			uint8_t bytes[2];
			ref_s ref;
		};

		constexpr inline uint16_t descriptor_enco(uint8_t _len, bool _bsign, bool _bconst, bool _real) {
			return (uint16_t)(((_len & 0x7FF)<<2) + (_bsign ? 0x2000 : 0) + (_bconst ? 0x4000 : 0) + (_real ? 0x8000 : 0) + 1);
		}

		typedef enum {
			uint8 = descriptor_enco(1, false, false, false)
			, int8 = descriptor_enco(1, true, false, false)
			, uint16 = descriptor_enco(2, false, false, false)
			, int16 = descriptor_enco(2, true, false, false)
			, uint32 = descriptor_enco(4, false, false, false)
			, int32 = descriptor_enco(4, true, false, false)
			, uint64 = descriptor_enco(8, false, false, false)
			, int64 = descriptor_enco(8, true, false, false)
			, real = descriptor_enco(4, true, false, true)
			, ext = descriptor_enco(8, true, false, true)
			, const_uint8 = descriptor_enco(1, false, true, false)
			, const_int8 = descriptor_enco(1, true, true, false)
			, const_uint16 = descriptor_enco(2, false, true, false)
			, const_int16 = descriptor_enco(2, true, true, false)
			, const_uint32 = descriptor_enco(4, false, true, false)
			, const_int32 = descriptor_enco(4, true, true, false)
			, const_uint64 = descriptor_enco(8, false, true, false)
			, const_int64 = descriptor_enco(8, true, true, false)
			, const_real = descriptor_enco(4, true, true, true)
			, const_ext = descriptor_enco(8, true, true, true)
		} types;

		static inline robo::cstr type_name(const descriptor & d) {
			int ix = descriptor_enco(d.len, d.bsign, d.bconst, d.real);
			switch (ix) {
			case uint8: return RT("u8");
			case int8: return RT("i8");
			case uint16: return RT("u16");
			case int16: return RT("i16");
			case uint32: return RT("u32");
			case int32: return RT("i32");
			case uint64: return RT("u54");
			case int64: return RT("i64");
			case real: return RT("real");
			case ext: return RT("ext");
			case const_uint8: return RT("const u8");
			case const_int8: return RT("const i8");
			case const_uint16: return RT("const u16");
			case const_int16: return RT("const i16");
			case const_uint32: return RT("const u32");
			case const_int32: return RT("const i32");
			case const_uint64: return RT("const u54");
			case const_int64: return RT("const i64");
			case const_real: return RT("const real");
			case const_ext: return RT("const ext");
			default: return RT("unknown");
			}
		}
		struct request {
			enum {
				index = 0
				, get = 1
				, put = 2
				//, page_get = 3
				//, page_put = 4
			};
		};
		enum {
			error_mask = 0xf0
			, max_len = 0x6
		};
		typedef enum {
			invalid_key = 0x10
			, invalid_index = 0x20
			, invalid_offset = 0x30
			, invalid_length = 0x40
		} error;

		struct record_s {
			descriptor desc;
			robo::cstr name;
			int key;
			const void* addr;
		};
		void reg(ref_s* _r);

		void push(robo::cstr _name);
		void pop(void);
		template<typename T>	void reg(types _type, const T& _addr, robo::cstr _name) {
			record_s* tmp = new record_s;
			tmp->desc.memo = _type;
			ROBO_APP_ASSERT(sizeof(T) >= tmp->desc.len);
			tmp->addr = &_addr;
			tmp->name = _name;
			tmp->key = 0;
			reg( &(tmp->desc.ref) );
		}

		void free(void);
		int find(int _key);
		const record_s* get(int _index);
		int proto(const uint8_t* _buf_in, uint8_t* _buf_out);
		#if ROBO_APP_FORMATING_TYPE != ROBO_APP_TYPE_NONE
		size_t sprintf(record_s& _rec, ::robo::char_t* buf, size_t _max_sz);
		#endif
		ref_s** first(void);
		ref_s** last(void);
	}
}
#endif
#endif