#include "net/robosd_can.hpp"
#include "core/robosd_log.hpp"

namespace robo {
	namespace net {
		static ican::map & cans_(void) {
			static ican::map cans__;
			return cans__;
		}
		delegat::dummy<void, ican&, uint32_t, const uint8_t*, uint8_t   > can_dummy_on_receive;
		delegat::dummy<void, ican&, ican::event> can_dummy_on_event;

		ican::ican(void) 
			: ref_(*this, -1) {
			on_receive = &can_dummy_on_receive;
			on_event = &can_dummy_on_event;
		}
		void ican::set_on_receive(on_receive_f* _on_receive) {
			on_receive = _on_receive == nullptr ? &can_dummy_on_receive : _on_receive;
		}
#if ROBO_AUTONUM_ENABLED == 1
		void ican::set_on_receive( void (* _simple)( ican&, uint32_t, const uint8_t*, uint8_t) ){
			set_on_receive(robo::delegat::ausimple(_simple));
		}
#endif
		void ican::set_on_event(on_event_f* _on_event) {
			on_event = _on_event == nullptr ? &can_dummy_on_event : _on_event;
		}
#if ROBO_AUTONUM_ENABLED == 1
		void ican::set_on_event(void (*_simple)(ican&, event)) {
			set_on_event(robo::delegat::ausimple(_simple));
		}
#endif

		bool ican::reg(cstr _caption) {
			ref_.set_key(hash(_caption));
			ROBO_LRET(ref_.attach_to(cans_()));
		}
		void ican::unreg(void) {
			ref_.dettach();
		}
		ican* ican::find(cstr _caption) {
			return cans_().find(hash(_caption));
		}
		ican* ican::query(cstr _caption) {
			ican* s = cans_().find(hash(_caption, 0));
			if (s) {
				s->ref_.dettach();
				return s;
			}
			else {
				robo_errlog("can '%s' is't found !", _caption);
				return 0;
			}
		}
		ican& ican::query_ref(cstr _caption) {
			ican* s = cans_().find(hash(_caption, 0));
			if (s) {
				s->ref_.dettach();
				return *s;
			}
			else {
				robo_errlog("can '%s' is't found !", _caption);
				return can_dummy::instance();
			}
		}
		void  ican::release(void) {
			ref_.attach_to(cans_());
		}
		void ican::forall(lambda<void(ican&)>& _operator) {
			for (ref* r = cans_().first(); r; r = r->next())
				_operator(r->owner());
		}
		
		can_dummy& can_dummy::instance(void) {
			static can_dummy instance_;
			return instance_;
		}

	}
}
