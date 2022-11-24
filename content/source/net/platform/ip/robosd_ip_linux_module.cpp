#define MODULE_NAME  winip
#define MODULE_NAME_STR RT("linuxip")
#include "core/robosd_app.hpp"
#include "core/robosd_log.hpp"
#include "net/robosd_can_flow_bus.hpp"
#include "net/platform/ip/robosd_udp_linux.hpp"
namespace MODULE_NAME{
	class module : public robo::app::module {
		module(void): robo::app::module(MODULE_NAME_STR) {}
		linux::udp_poll ** udps_ = nullptr;
		int udps_count_ = 0;
	protected:
		virtual void backend_loop(void) {			
		}
		virtual void frontend_loop(void) {
		}
		virtual bool do_load(void) { 
			ROBO_LBREAKN(robo::app::module::do_load());
			ROBO_LBREAKN(robo::ini::load(current_path(), RT("udp_count"), udps_count_));
			if (udps_count_ > 0) {
				udps_ = new linux::udp_poll * [udps_count_];
				linux::udp_poll** b = udps_;
				for (int i = 0; i < udps_count_; ++i, ++b) {
					(*b) = nullptr;
				}

				b = udps_;

				for (int i = 0; i < udps_count_; ++i, ++b) {
					robo::string name(RT("udp-%d"), i + 1);
					(*b) = new linux::udp_poll(false);
					ROBO_LBREAKN((*b) != nullptr);					
					ROBO_LBREAKN( (*b)->begin(name) );
				}
			}
			return true; 
		}
		virtual bool do_start(void) {
			linux::udp_poll** b = udps_;
			for (int i = 0; i < udps_count_; ++i, ++b) {
				(*b)->start();
			}
			return true;
		}
		virtual void do_stop(void) {
			linux::udp_poll** b = udps_;
			for (int i = 0; i < udps_count_; ++i, ++b) {
				(*b)->stop();
			}
		}
		virtual void do_clean(void) {
			if (udps_ != nullptr) {
				linux::udp_poll** b = udps_;
				for (int i = 0; i < udps_count_; ++i, ++b) {
					if ((*b)!=nullptr) delete (*b);
				}
				delete[] udps_;
				udps_ = nullptr;
			}
			udps_count_ = 0;
		}
	public:
		static module& instance(void) {
			static module instance_;
			return instance_;
		}
	};
}

#include "core/robosd_system_module_reg.hpp"
