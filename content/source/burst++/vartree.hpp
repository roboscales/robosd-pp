#ifndef burst_vartree_hpp
#define burst_vartree_hpp
#include "burst++/burst_common.hpp"

#if ROBO_APP_BURST_VARTREE_ENABLED == 1

#ifndef BURST_VAR_BUFFER_SIZE 
#define BURST_VAR_BUFFER_SIZE 70
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
		#ifndef ROBO_APP_BURST_VARTREE_MODE
		#define ROBO_APP_BURST_VARTREE_MODE full
		#endif
		constexpr enum class mode { none = 0, tuning = 1, action = 2, config = 3, full = 4 } actual_mode = mode::ROBO_APP_BURST_VARTREE_MODE;

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
		#pragma pack(push, 1)
		union descriptor {
			struct {
				uint16_t tag : 2;
				uint16_t len : 10;
				uint16_t bsign : 1;
				uint16_t bconst : 1;
				uint16_t real : 1;
				uint16_t reconfig_need : 1;
				//uint16_t fault : 5;
			};
			uint16_t memo;
			uint8_t bytes[2];
			ref_s ref;
		};
		#pragma pack(pop)	
		constexpr inline uint16_t descriptor_enco(uint8_t _len, bool _bsign, bool _bconst, bool _real) {
			return (uint16_t)(((_len & 0x3FF)<<2) + (_bsign ? 0x1000 : 0) + (_bconst ? 0x2000 : 0) + (_real ? 0x4000 : 0) + 1);
			
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
			, time_us = descriptor_enco(sizeof(time_us_t),false,false,false)
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
			, const_time_us = descriptor_enco(sizeof(time_us_t), false, true, false)
		} types;

		template<typename T> constexpr types const_unsigned_type(const  T & /*_x*/) {
			switch (sizeof(T)) {
			case 1:
				return types::const_uint8;
			case 2:
				return types::const_uint16;
			case 4:
				return types::const_uint32;
			default:
				return types::const_uint64;
			}
		}
		template<typename T> constexpr types const_signed_type(const  T& /*_x*/) {
			switch (sizeof(T)) {
			case 1:
				return types::const_int8;
			case 2:
				return types::const_int16;
			case 4:
				return types::const_int32;
			default:
				return types::const_int64;
			}
		}
		template<typename T> constexpr types unsigned_type(const  T& /*_x*/) {
			switch (sizeof(T)) {
			case 1:
				return types::uint8;
			case 2:
				return types::uint16;
			case 4:
				return types::uint32;
			default:
				return types::uint64;
			}
		}
		template<typename T> constexpr types signed_type(const  T& /*_x*/) {
			switch (sizeof(T)) {
			case 1:
				return types::int8;
			case 2:
				return types::int16;
			case 4:
				return types::int32;
			default:
				return types::int64;
			}
		}
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
				index = 0x1
				, get = 0x2
				, put = 0x3
				//, page_get = 3
				//, page_put = 4
			};
		};
		enum {
			max_len = 0x6
		};
		typedef enum {
			none = 0x0
			, invalid_key = 0x1
			, invalid_index = 0x2
			, invalid_offset = 0x3
			, invalid_length = 0x4
			, invalid_mode = 0x5
		} error;
		#pragma pack(push, 1)
		union header_s{
			struct{
				uint16_t query:2;
				uint16_t error:4;
				uint16_t ix:10;
			};
			uint16_t data;
		};
		#pragma pack(pop)		
		struct record_s {
			descriptor desc;
			robo::cstr name;
			int key;
			const void* addr;			
		};
		void reg(ref_s* _r);

		void push(robo::cstr _name);
		void pop(void);
		extern int var_count;
		template<typename T>	void reg(types _type, const T& _addr, robo::cstr _name,bool _regonfig_need = false) {
			record_s* tmp = new record_s;
			tmp->desc.memo = _type;
			ROBO_ASSERT((_regonfig_need == false) || ((_regonfig_need == true) && (tmp->desc.bconst == 0)))
			auto sz = sizeof(T);
			ROBO_APP_ASSERT(sz >= tmp->desc.len);
			tmp->addr = &_addr;
			tmp->name = _name;
			tmp->key = 0;
			tmp->desc.reconfig_need = _regonfig_need;
			reg( &(tmp->desc.ref) );
			var_count++;
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
		template<typename R > void varreg(robo::cstr _name, const types& t, const range_s<R>& _range, bool _regonfig_need=false) {
			push(_name);
			reg(t, _range.hi, RT("hi"),_regonfig_need);
			reg(t, _range.lo, RT("lo"),_regonfig_need);
			pop();
		}
		template<typename R > void varreg(robo::cstr _name, const types& t, const hyst_t<R>& _hyst, bool _regonfig_need=false) {
			push(_name);
			reg(t, _hyst.overhi, RT("overhi"));
			reg(t, _hyst.hi, RT("hi"),_regonfig_need);
			reg(t, _hyst.lo, RT("lo"),_regonfig_need);
			reg(t, _hyst.ultralo, RT("ultralo"));
			pop();
		}
		#if ROBO_APP_ULTRACOMPACT
		void reindex(void);
		bool if_configure(void);
		void reconfig_query(void);
		#endif	
		
		class showpro {
			robo::char_t* path_ = nullptr;
			robo::char_t* path_ptr_ = nullptr;
			int* path_stack_buffer_ = nullptr;
			int path_sz_ = 0;
			int* path_stack_top_ = 0;
			int path_stack_level_ = 0;
			void end_(void) {
				if (path_) {
					delete[] path_;
					path_ = nullptr;
				}
				if (path_stack_buffer_) {
					delete[] path_stack_buffer_;
					path_stack_buffer_ = nullptr;
				}
			}
		protected:
			burst::var::ref_s** pref = nullptr;
			burst::var::ref_s* ref = nullptr;
			void perform(void) {
				using namespace burst::var;
				int sz;
				ref = *pref;
				switch (ref->tag) {
				case burst::var::tags::push:
					if (path_sz_) {
						if (path_stack_level_ < stack_size) {
							sz = (int)robo::system::sprintf(path_ptr_, path_sz_, RT(".%s"), ((record_s*)(ref))->name);
							*(path_stack_top_) = sz;
							path_ptr_[sz] = 0;
							path_ptr_ += sz;
							path_sz_ -= sz;
							path_stack_top_++;
							path_stack_level_++;
						}
						else {
							end_();
							return;
						}
					}
					break;
				case tags::pop:
					path_stack_top_--;
					path_stack_level_--;
					sz = *(path_stack_top_);
					path_ptr_ -= sz;
					*(path_ptr_) = 0;
					path_sz_ += sz;
					break;
				default:
					sz = (int)robo::system::sprintf(path_ptr_, path_sz_, RT(".%s"), ((record_s*)(ref))->name);
					*(path_stack_top_) = sz;
					path_ptr_[sz] = 0;
					printf((burst::var::record_s*)ref, path_ + 1);

				}
			}

			virtual void printf(burst::var::record_s* _rec, robo::cstr _path) = 0;

			bool do_begin(void) {
				using namespace burst::var;
				pref = burst::var::first();
				if (pref) {
					path_ = new robo::char_t[path_size + 1];
					path_stack_buffer_ = new int[stack_size];

					path_ptr_ = path_;
					path_sz_ = path_size;
					path_stack_top_ = path_stack_buffer_;
					path_stack_level_ = 0;

					perform();
					if (pref != burst::var::last()) {
						return true;
					}
					else {
						end_();
						return false;
					}
				}
				else {
					return false;
				}
			}
			bool do_perform(void) {
				pref++;
				perform();
				if (pref != burst::var::last()) {
					return true;
				}
				else {
					end_();
					return false;
				}
			}
			virtual bool busy(void) { return false;  };
			bool do_loop(void) {
				if (busy()) {
					return true;
				}
				return do_perform();
			}
		};
		class show :public  showpro {
			void (*do_print_)(const char* _buf) = nullptr;
		protected:
			virtual void printf(burst::var::record_s* _rec, robo::cstr _path) {
				robo::string nm;
				nm.format(RT("%s\t%d\t%x\n\r")
					, _path
					, (int)_rec->desc.len
					, uint32_t(((FMSTR_ADDRESS_OFFSET_TYPE)_rec->addr) - FMSTR_ADDRESS_OFFSET));
				nm.ascii([this](const char* _buf) { this->do_print_(_buf); });
			}
		public:
			void run(void (*_do_print)(const char* _buf)) {
				do_print_ = _do_print;
				if (do_begin())
					while (do_perform());
				do_print_ = nullptr;
			}

			show(void)
			{

			}
		};
	}
}
#endif
#endif