#ifndef burst_store_hpp
#define burst_store_hpp
#include <stdint.h>
#include "burst++\burst_common.hpp"
namespace burst {
	namespace  store {
		enum class result { empty = 0, full = 1, panic = 2};
		enum class statuses { empty = 2, full = 1, panic = 3, busy = 4, unknown = 0};
		enum class commands { save = 1, load =2, reset = 3, none = 0 };
		struct action_s {
			commands command;
		};
		struct present_s {
			statuses status;
		};
		/*		class driver {
				protected:
					virtual states state(void) = 0;
					virtual states clear(void) = 0;
					virtual states save(const uint8_t* _memo, size_t _sz) = 0;
					virtual states load(const uint8_t* _memo, size_t _sz) = 0;
				};*/

	}
	
	template < class D> class store_flash_drive_t : D {
			size_t page_size_ = 0;
			size_t page_count_ = 0;
			size_t size_;
			int read_page_ix=-1;
			int write_page_ix=-1;
		public:
			void eraise(void){
				D::eraise();
				read_page_ix = -1;
				write_page_ix = 0;
			}
			bool begin(void){
				
				for(int i=0;i<page_count_;++i){
					bool isempty = true;
					auto * ptr = &D::memo[i*page_size_];
					for(int j=0;j<page_size_;++j,++ptr){
						if( *ptr != D::empty){
							isempty = false;
							break;
						}
					}
					if(isempty){
						if(i==0){
							read_page_ix = -1;
							write_page_ix = 0;
						} else{
							read_page_ix = i-1;
							if(read_page_ix < page_count_-1){
								write_page_ix = read_page_ix+1;
							} else{
								write_page_ix = -1;
							}
						}
						return true;
					}
				}
				read_page_ix = page_count_-1;
				write_page_ix = -1;
				return true;
			}
			
			bool clear(void){
				return true;
			}
			bool save(const uint8_t * _memo){
				if(write_page_ix<0){
					eraise();
				}
				typename D::memo_t crc = D::crc(_memo);				
				if ( D::write(write_page_ix*page_size_, page_size_,_memo, crc)){
						read_page_ix = write_page_ix;
						if(read_page_ix<page_count_-1){
							write_page_ix= write_page_ix+1;
						} else {
							write_page_ix = -1;
						}
						return true;
				}
				return false;
			}
			
			burst::store::result load(uint8_t * _memo){
				if( read_page_ix>=0 ){
					typename D::memo_t crc_exists = D::memo[ (read_page_ix+1)*page_size_-1];
					typename D::memo_t crc = D::crc((uint8_t *)&D::memo[read_page_ix*page_size_]);
					if(crc==crc_exists){
						std::copy_n( (uint8_t *)&D::memo[read_page_ix*page_size_], size_,_memo);
						return burst::store::result::full;
					} else{
						return burst::store::result::panic;
					}
				} else{
					return burst::store::result::empty;
				}
			}
			store_flash_drive_t(size_t _size): D(_size), size_(_size){
				page_size_ =  (size_/sizeof(typename D::memo_t))+2;				
				page_count_ = D::size/page_size_;
			}

	};
	
	template < class D , typename S> class store_t:  D {
	private:
		store::action_s& action_;
		store::present_s& present_;
		S& content_;		
		const S default_;		
		S tmp;
	public:
		store_t(store::action_s & _action, store::present_s& _present,  S& _content)
	: action_(_action), present_(_present), default_(_content), content_(_content), D(sizeof(S)){}
		
		template <typename T> typename T::present_s& present(void) {
			return reinterpret_cast <typename T::present_s&>(present_);
		}
		template <typename T> typename T::action_s& action(void) {
			return reinterpret_cast <typename T::action_s&>(action_);
		}
		
		//store::states state(void) { return D:state(); };
		bool reset(void) { 
			present_.status =  store::statuses::busy;					
			content_ = default_;
			if(D::clear()){
				present_.status =  store::statuses::empty;
				return save();
			} else {
				present_.status =  store::statuses::panic;
				return false;
			}
		};
		
		bool begin(void){
			return D::begin();
		}
		
		bool save(void) {
			present_.status =  store::statuses::busy;					
			if( D::save( (uint8_t *) &content_) ){
				if( D::load( (uint8_t *) &tmp) == store::result::full ) {
					if ( std::equal((uint8_t *)&tmp,((uint8_t *)&tmp)+sizeof(S),(uint8_t *)&content_) ){
						present_.status =		store::statuses::full;
						return true;
					}
				}
			}
			present_.status = store::statuses::panic ;
			return false;
		}
		
		bool load(void) {
			present_.status = store::statuses::busy;					
			switch(D::load( (uint8_t *) &tmp)){
				case  store::result::full: 
					content_ = tmp;
					present_.status =  store::statuses::full;					
					return true;
				case store::result::empty: 
					content_ = default_;
					return save();
				default:
					break;
			}
			present_.status =  store::statuses::panic;					
			return false;
		}

		void poll(void) {
			switch ((store::commands)action_.command) {
			case store::commands::none:				
				break;
			case store::commands::load:
				load();
				break;
			case store::commands::save:
				save();
				break;
			case store::commands::reset:
				reset();
				break;
			default:
				break;
			}
			action_.command = store::commands::none;
		}

	};
}
#endif