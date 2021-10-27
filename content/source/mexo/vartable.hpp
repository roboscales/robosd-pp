#ifndef mexo_vartable_hpp
#define mexo_vartable_hpp
#include "core/robosd_string.hpp"
#include "core/robosd_list.hpp"
namespace mexo{
	namespace var {
		union descriptor{
			struct{
				uint16_t len:8;
				uint16_t bsign:1;
				uint16_t bconst:1;
				uint16_t real:1;
			};
			uint16_t memo;
		};
		constexpr inline int descriptor_enco( uint8_t _len, bool _bsign, bool _bconst, bool _real){
			return (int) ((_len &0xFF)+(_bsign?0x100:0) + (_bconst?0x200:0) + (_real?0x400:0));
		}

		typedef enum{
			uint8 = 				descriptor_enco(1,false,	false,false)
			,int8 = 				descriptor_enco(1,true,		false,false)
			,uint16 = 				descriptor_enco(2,false,	false,false)
			,int16 = 				descriptor_enco(2,true,		false,false)
			,uint32 = 				descriptor_enco(4,false,	false,false)
			,int32 = 				descriptor_enco(4,true,		false,false)
			,uint64 = 				descriptor_enco(8,false,	false,false)
			,int64 = 				descriptor_enco(8,true,		false,false)
			,real =		 			descriptor_enco(4,true,		false,true)
			,ext = 					descriptor_enco(8,true,		false,true)
			,const_uint8 = 			descriptor_enco(1,false,	true,false)
			,const_int8 = 			descriptor_enco(1,true,		true,false)
			,const_uint16 =			descriptor_enco(2,false,	true,false)
			,const_int16 = 			descriptor_enco(2,true,		true,false)
			,const_uint32 =			descriptor_enco(4,false,	true,false)
			,const_int32 = 			descriptor_enco(4,true,		true,false)
			,const_uint64 =			descriptor_enco(8,false,	true,false)
			,const_int64 = 			descriptor_enco(8,true,		true,false)
			,const_real =			descriptor_enco(4,true,		true,true)
			,const_ext = 			descriptor_enco(8,true,		true,true)
		} types;
		
		
		class record{		
			void setup_(
				types _type
				, const void* _addr
				, robo::cstr _name
				, int  _master_key
			);
		protected:
			record(){}

		public:

			const void * addr;
			robo::cstr name;
			descriptor desc;
			int key;


			typedef ::robo::list::unidir_t<record>::item  ref;
			typedef ::robo::list::unidir_t<record> list;
				
			template<typename T>	static ref * create(
						types _type
					, const T & _addr
					, robo::cstr _name
					, int  _master_key
					, list & _list
			){
				descriptor tmp;
				tmp.memo	= _type;
				ROBO_APP_ASSERT(sizeof(T) >= tmp.len);
				ref * r =  new ref;
				ROBO_APP_ASSERT(r != nullptr);
				r->setup_(_type, (void *)&_addr, _name, _master_key);
				_list.push(*r);
				return r;
			}

		};
		
		class machine{
			private:
				const record **  index_=nullptr;
				int index_size_ = 0;
				int count_=0;
				static machine & instance_(void);
				void begin_(int _pool_size);
				int find_(int _key);
				const record * get_(int _index);
				void reg_( const record & _precord	);
				~machine(void);
					
			public:
				enum class mode { config, action, full  } actual_mode_ = mode::full;
				static void begin(int _pool_size){
					instance_().begin_(_pool_size);
				}
				static int find(int _key){
					return instance_().find_(_key);
				}
				static const record * get(int _index){
					return instance_().get_(_index);
				}
				static mode actual_mode(void){ return instance_().actual_mode_; }
				
				static void reg(
						const record & _precord				
				){
					return instance_().reg_(_precord);
				}
		};
	}
}

#endif