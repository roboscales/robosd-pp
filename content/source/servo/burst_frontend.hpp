#ifndef burst_frontend_hpp
#define burst_frontend_hpp
#include "servo/robosd_frontend.hpp"
namespace burst {
	namespace frontend {
		class devagent : public robo::frontend::devagent {

		protected:
			typedef ::robo::quest quest;
			robo::string startup_section;
			virtual quest* configure_expansion_create(quest* _quest);
		public:
			devagent(robo::cstr _name, robo::app::node& _owner, config_s& _config, action_s& _action, action_s& _goal, feedback_s& _feedback)
				: robo::frontend::devagent(_name,_owner, _config, _action, _goal, _feedback) {}
			virtual bool do_load(void);
		};
		class servo : public robo::frontend::servo {
		public:
			servo(robo::cstr _name, robo::app::module& _module);
		protected:
		};

	}
}
#endif

