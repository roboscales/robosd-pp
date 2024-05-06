#include "net/robosd_serial.hpp"

namespace robo {
	namespace net {
		bool  iserial::puts(const char *_s){
				return put( (const uint8_t *)_s, strlen(_s) );
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
