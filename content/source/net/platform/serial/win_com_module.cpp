#define MODULE_NAME  wincom
#define MODULE_NAME_STR RT("wincom")
#include "core/robosd_app.hpp"
#include "core/robosd_log.hpp"
#include "net/robosd_can_flow_bus.hpp"
#include "net/platform/serial/win_com.hpp"
namespace MODULE_NAME{

	class module : public robo::app::module {
		module(void)
			: robo::app::module(MODULE_NAME_STR) {}
		class win_com : public robo::net::win_com {
		public:
			robo::string name;
			win_com(robo::cstr _key) : robo::net::win_com(){
				name.load(MODULE_NAME_STR,_key);
			}
		};
		win_com  ** win_coms_ = nullptr;
		int com_count_ = 0;

	protected:
		virtual void backend_loop(void) {
		}
		virtual void frontend_loop(void) {
	
		}
		virtual bool do_load(void) { 
			ROBO_LBREAKN(robo::app::module::do_load());
			ROBO_LBREAKN(robo::ini::load(current_path(), RT("count"), com_count_));
			if (com_count_ > 0) {
				win_coms_ = new win_com * [com_count_];
				win_com** b = win_coms_;
				for (int i = 0; i < com_count_; ++i, ++b) {
					(*b) = nullptr;
				}

				b = win_coms_;
				robo::string name;
				for (int i = 0; i < com_count_; ++i, ++b) {
					robo::string key(RT("comm_%d"), i + 1);
					(*b) = new win_com(key);
					ROBO_LBREAKN((*b) != nullptr);					
				}
			}
			return true; 
		}
		virtual bool do_start(void) {
			win_com** b = win_coms_;
			for (int i = 0; i < com_count_; ++i, ++b) {
				(*b)->connect((*b)->name,100);
			}
			return true;
		}
		virtual void do_stop(void) {
			win_com** b = win_coms_;
			for (int i = 0; i < com_count_; ++i, ++b) {
				(*b)->stop();
			}
		}
		virtual void do_clean(void) {
			if (win_coms_ != nullptr) {
				win_com** b = win_coms_;
				for (int i = 0; i < com_count_; ++i,++b) {
					if ((*b)!=nullptr) delete (*b);
				}
				delete[] win_coms_;
				win_coms_ = nullptr;
			}
		}
	public:
		static module& instance(void) {
			static module instance_;
			return instance_;
		}
	};
}

#include "core/robosd_system_module_reg.hpp"
