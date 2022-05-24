#include "servo/robosd_proto.hpp"

#define MODULE_NAME  flow2serial
#define MODULE_NAME_STR RT("flow2serial")
#include "core/robosd_app.hpp"

#include "servo/mexo_backend.hpp"

namespace MODULE_NAME {

	namespace frontend {
		class servo : public mexo::frontend::servo {
		public:
			servo(robo::app::module & _module)
				:mexo::frontend::servo(RT("fre"), _module)  {}
		};
	}

	namespace backend {
		class devagent : public mexo::backend::devagent {
		public:
			devagent(robo::cstr _name, robo::backend::boardagent& _boardagent, action_s& _goal, feedback_s& _feedback) :
				mexo::backend::devagent(_name, _boardagent, _goal, _feedback) {}
		};


		class servo : public mexo::backend::servo {
		public:
			servo(robo::app::module & _module)
				: mexo::backend::servo(RT("bke"), _module)
//				, yaw_board(RT("yaw"), *this)
			{
			}
		};
	}
		
	class module : public robo::app::module {
		frontend::servo frontend_;
		backend::servo backend_;
		module(void)
			: robo::app::module(MODULE_NAME_STR)
			, frontend_(*this)
			, backend_(*this) {
		}
	protected:

		virtual void backend_loop(void) {}
		virtual void frontend_loop(void) {}
		virtual bool do_load(void) { 
			ROBO_LBREAKN(robo::app::module::do_load());
			return true; 
		}
		virtual bool do_start(void) {
			return true;
		}
		virtual void do_stop(void) {
		}
		virtual void do_clean(void) {
		}
	public:
		static module& instance(void) {
			static module instance_;
			return instance_;
		}
	};
}

#include "core/robosd_system_module_reg.hpp" 
