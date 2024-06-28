#ifndef burst_store_hpp
#define burst_store_hpp
#include <stdint.h>
#include "burst++\burst_common.hpp"
namespace burst {
	namespace  store {
		enum class statuses { empty = 'e', full = 'f', panic = 'P', busy = 'b', unknown = 0};
		enum class commands { save = 's', load = 'l', clear = 'c', none = 0 };
		struct action_s {
			uint8_t command;
		};
		struct present_s {
			uint8_t status;
		};
		/*		class driver {
				protected:
					virtual states state(void) = 0;
					virtual states clear(void) = 0;
					virtual states save(const uint8_t* _memo, size_t _sz) = 0;
					virtual states load(const uint8_t* _memo, size_t _sz) = 0;
				};*/

	}
	template < class D , class S> class store_t:  D {
	private:
		store::action_s& action_;
		store::present_s& present_;
		S& content_;
	public:
		store_t(store::action_s & _action, store::present_s& _present, S& _content)
			: action_(_action), present_(_present), content_(_content){}
		template <typename T> typename T::present_s& present(void) {
			return reinterpret_cast <typename T::present_s&>(present_);
		}
		template <typename T> typename T::action_s& action(void) {
			return reinterpret_cast <typename T::action_s&>(action_);
		}

		//store::states state(void) { return D:state(); };
		store::statuses clear(void) { 
			auto status = D::status();
			if ( status  == store::statuses::full ) {
				status = D::clear();
			} 
			present_.status = (uint8_t)status;
			return status;
		};
		store::statuses load( uint8_t* _memo, size_t _ofset, size_t _sz ) {
			auto status = D::status();
			if (status == store::statuses::full) {
				status = D::load(_memo, _ofset, _sz);
			}
			present_.status = (uint8_t)status;
			return status;
		}
		store::statuses save(const uint8_t* _memo, size_t _ofset, size_t _sz) {
			auto status = D::status();
			if (status != store::statuses::panic && status != store::statuses::busy) {
				status = D::save(_memo, _ofset, _sz);
			}
			present_.status = (uint8_t)status;
			return status;
		}
		store::statuses load(void) {
			return load((uint8_t *)&content_, 0, sizeof(S));
		}
		store::statuses save(void) {
			return save((const uint8_t*)&content_, 0, sizeof(S));
		}
		void poll(void) {
			switch ((store::commands)action_.command) {
			case store::commands::none:				
				break;
			case store::commands::load:
				if (load() != store::statuses::busy) {
					action_.command = (uint8_t)store::commands::none;
				}
				break;
			case store::commands::save:
				if (save() != store::statuses::busy) {
					action_.command = (uint8_t)store::commands::none;
				}
				break;
			case store::commands::clear:
				if (clear() != store::statuses::busy) {
					action_.command = (uint8_t)store::commands::none;
				}
				break;
			}
		}
	};
}
#endif