#ifndef mexo_vartable_hpp
#define mexo_vartable_hpp
#include "core/robosd_common.hpp"

#ifndef ROBO_APP_MEXO_VAR_ENABLED
#define ROBO_APP_MEXO_VAR_ENABLED 0
#endif

#if ROBO_APP_MEXO_VAR_ENABLED == 1
#include "core/robosd_string.hpp"
#include "core/robosd_list.hpp"
namespace mexo {
	namespace var {
		union descriptor {
			struct {
				uint16_t len : 8;
				uint16_t bsign : 1;
				uint16_t bconst : 1;
				uint16_t real : 1;
				uint16_t fault : 5;
			};
			uint16_t memo;
			uint8_t bytes[2];
		};
		constexpr inline int descriptor_enco(uint8_t _len, bool _bsign, bool _bconst, bool _real) {
			return (int)((_len & 0xFF) + (_bsign ? 0x100 : 0) + (_bconst ? 0x200 : 0) + (_real ? 0x400 : 0));
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
		typedef enum {
			index = 0
			, get = 1
			, put = 2
			, page_get = 3
			, page_put = 4
		} request;
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
		#ifdef ROBO_APP_MEXO_SIDE
		class record {
			void setup_(
				types _type
				, const void* _addr
				, robo::cstr _name
				, int  _master_key
			);
		protected:
			record() {}

		public:

			const void* addr;
			robo::cstr name;
			descriptor desc;
			int key;
			#if ROBO_APP_FORMATING_TYPE != ROBO_APP_TYPE_NONE
			size_t sprintf(::robo::char_t* buf, size_t _max_sz);
			#endif

			typedef ::robo::list::unidir_t<record>::item  ref;
			typedef ::robo::list::unidir_t<record> list;

			template<typename T>	static ref* create(
				types _type
				, const T& _addr
				, robo::cstr _name
				, int  _master_key
				, list& _list
			) {
				descriptor tmp;
				tmp.memo = _type;
				ROBO_APP_ASSERT(sizeof(T) >= tmp.len);
				ref* r = new ref;
				ROBO_APP_ASSERT(r != nullptr);
				r->setup_(_type, (void*)&_addr, _name, _master_key);
				_list.push(*r);
				return r;
			}
			static int root_key(void);
			static list& root_vars(void);
			
			template<typename T>	static ref* create(
				types _type
				, const T& _addr
				, robo::cstr _name
			) {				
				return create<T>(_type,_addr, _name, root_key(), root_vars() );
			}

		};
		#ifndef ROBO_APP_MEXO_VAR_MODE
		#define ROBO_APP_MEXO_VAR_MODE tuning
		#endif
		class machine {
		public:
			enum class mode { none = 0, tuning = 1, action = 2, config = 3, full = 4 } actual_mode_ = mode::ROBO_APP_MEXO_VAR_MODE;
		private:
			const record** index_ = nullptr;
			int index_size_ = 0;
			int count_ = 0;
			static machine& instance_(void);
			void begin_(int _pool_size);
			int find_(int _key);
			const record* get_(int _index);
			void reg_(const record& _precord);
			int proto_(const  uint8_t* _buf_in, uint8_t* _buf_out);
			~machine(void);

		public:
			static void begin(int _pool_size) {
				instance_().begin_(_pool_size);
			}
			static int find(int _key) {
				return instance_().find_(_key);
			}
			static const record* get(int _index) {
				return instance_().get_(_index);
			}
			static mode actual_mode(void) { return instance_().actual_mode_; }

			static void reg(const record& _precord) { return instance_().reg_(_precord); }
			static int proto(const uint8_t* _buf_in, uint8_t* _buf_out) { return instance_().proto_(_buf_in, _buf_out); }
		};
		#endif
	}
}
#endif
#endif