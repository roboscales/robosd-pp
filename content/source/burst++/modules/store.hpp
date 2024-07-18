#ifndef burst_store_hpp
#define burst_store_hpp
#include <stdint.h>
#include "burst++\burst_common.hpp"
namespace burst {
	namespace  store {
		enum class result { empty = 0, full = 1, panic = 2};
		enum class statuses { empty = 2, full = 1, panic = 3, busy = 4, unknown = 0};
		enum class commands { save = 's', load = 'l', reset = 'r', none = 0 };
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
	template < class D , typename S> class store_t:  D {
	private:
		store::action_s& action_;
		store::present_s& present_;
		S& content_;		
		const S default_;		
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
		
		bool save(void) {
			present_.status =  store::statuses::busy;					
			if( D::save( (typename D::memo_t *) &content_) ){
				S tmp;
				if( D::load( (typename D::memo_t *) &tmp) == store::result::full ) {
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
			S tmp;
			present_.status = store::statuses::busy;					
			switch(D::load( (typename D::memo_t *) &tmp)){
				case  store::result::full: 
					content_ = tmp;
					present_.status =  store::statuses::full;					
					return true;
				case store::result::empty: 
					return reset();
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
				action_.command = store::commands::none;
				break;
			case store::commands::save:
				save();
				action_.command = store::commands::none;
				break;
			case store::commands::reset:
				reset();
				action_.command = store::commands::none;
				break;
			}
		}
	};
}
#endif