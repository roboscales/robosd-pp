#include "core/robosd_system.hpp"
#if ROBO_APP_SYSTEM_ENABLED == 1
namespace robo {
	namespace system {

		int itf_lock_count_ = 0;
		int itf_guest_count_ = 0;
		enum  class state { enabled = 178, unknown = -178 };
		state state_ = state::unknown;

		guard::guard(void) {
			if (state_ == state::enabled) {
				if (os::is_frontend()) {
					context_ = os::enter();
				}
				else {
					if (itf_lock_count_ == 0) {
						os::lock();
					}
					itf_lock_count_++;
					context_ = nullptr;
				}
			}
		}

		guard::~guard(void) {
			if (state_ == state::enabled) {
				if (os::is_frontend()) {
					os::leave(context_);
				}
				else {
					if (itf_lock_count_ > 0) {
						itf_lock_count_--;
						if (itf_lock_count_ == 0) {
							os::unlock();
						}
					}
				}
			}
		}
		void enable(void) {
			state_ = state::enabled;
		}
		void disable(void) {
			state_ = state::unknown;
		}

		system::lazzyboy::lazzyboy(void)
			: sleep_us_(system::os::time_us()) {
		}

		system::lazzyboy::~lazzyboy(void) {
		}

		robo::time_us_t	 system::lazzyboy::idle_us(void) {
			return  os::time_us() - sleep_us_;
		}


		system::fall::fall(void) {
			os::fall();
		}

		system::fall::~fall(void) {
			os::comeback();
		}
	}
}
#endif

