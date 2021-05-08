#include "core/robosd_system.hpp"
#if ROBO_APP_SYSTEM_DUMMY_ENABLED == 1
namespace robo {
	namespace system {

		bool os::is_frontend(void) {
			return true;
		}
		bool os::is_backend(void) {
			return true;
		}
		void* os::enter(void) {
			return nullptr;
		}
		void os::leave(void* _context) {
			ROBO_UNUSED(_context);
		}
		void os::lock(void) {
		}
		void os::unlock(void) {
		}
		void os::fall(void) {
		}
		void os::comeback(void) {
		}
		time_us_t os::time_us(void) {
			return 0;
		}
		time_us_t os::realtime_us(void) {
			return 0;
		}
		time_ms_t os::time_ms(void) {
			return 0;
		}
		random_t os::rand(random_t _max) {
			return 0;
		}
		void os::wakeup(void) {
		}
		time_us_t os::period_us(void) {
			return 0;
		}
		void os::sleep(void) {
		}
	}
}
#endif

