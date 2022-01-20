#include "im/edev/edev.hpp"
#include "addon_template.hpp"
namespace robo {
	namespace addon_template {
		class agent :public edev::agent {
		protected:
			virtual void do_background_run(double _time) {	
			}
			virtual void do_priotitet_run(double _time) {
			}

			virtual bool do_begin(void) {
				ROBO_LBREAKN(edev::agent::do_begin())
				robo_infolog("addon_template '%s' was started", name.c_str());
				return  true;
			}
			virtual void do_reconfig(void) {};
			virtual void do_finish(void) {
				robo_infolog("addon_template '%s' was finished", name.c_str());
			}
		public:
			agent(void) {}
			~agent() {}
			virtual void set_local_ini(cstr _ini) { ::robo::system::ini::begin(_ini); }
		} agent_;
	}	
}

extern "C" {
	ROBO_EXPORT_RUNTIME robo::edev::agent* ROBO_EXPORT_RUNTIME_DECL query_agent(void) {
		return & robo::addon_template::agent_;
	}
}

