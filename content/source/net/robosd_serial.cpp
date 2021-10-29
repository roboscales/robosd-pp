#include "net/robosd_serial.hpp"

namespace robo {
	namespace net {
		iserial::list& iserial::list_(void) {
			static list instance_;
			return instance_;
		}
		iserial::iserial(void) :ref_(*this, 0) {
			//int hash = robo_string_hash(_serial->caption_);
		}
		iserial::~iserial(void) {}

		bool iserial::reg(cstr _caption) {
			int key = hash(_caption, 0);
			ref_.set_key(key);
			ref_.attach_to(list_());
			return ref_.attached();
		}
		void iserial::unreg(void) {
			ref_.dettach();
		}

		::robo::net::iserial* iserial::query(cstr _caption) {
			::robo::net::iserial* s = list_().find(hash(_caption, 0));
			if (s) {
				s->ref_.dettach();
				return s;
			}
			else {
				return 0;
			}
		}
		::robo::net::iserial* iserial::find(cstr _caption) {
			return list_().find(hash(_caption, 0));
		}

		iserial& iserial::query_ref(cstr  _caption) {
			::robo::net::iserial* s = list_().find(hash(_caption, 0));
			if (s) {
				s->ref_.dettach();
				return *s;
			}
			else {
				robo_errlog("serial '%s' is't found !", _caption);
				return serial_dummy::instance();
			}

		}

		void iserial::forall(lambda<void(iserial&)>& _operator) {
			for (ref* r = list_().first(); r; r = r->next())
				_operator(r->owner());
		}


		void iserial::release(void) {
			ref_.attach_to(list_());
		}


		size_t serial_dummy::available(void) {
			return 0;
		}

		size_t serial_dummy::space(void) {
			return 0;
		}
		size_t serial_dummy::space_max(void) {
			return 1;
		}

		size_t serial_dummy::get(uint8_t* /*_data*/, size_t /*_max_size*/) {
			return 0;
		}

		bool serial_dummy::put(const  uint8_t* /*_data*/, size_t /*_max_size*/) {
			return true;
		}

		size_t serial_dummy::get(uint8_t & /*_data*/) {
			return 0;
		}

		bool  serial_dummy::put(uint8_t) {
			return true;
		}

		void serial_dummy::reset(void) {}

		serial_dummy& serial_dummy::instance(void) {
			static serial_dummy instance_;
			return instance_;
		}

	}
}
