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
			action_s goal_ = {};
			feedback_s feedback_ = {};
		public:
			devagent(robo::cstr _name,  robo::backend::boardagent & _boardagent) :
				mexo::backend::devagent(_name, _boardagent, goal_, feedback_) {}
			virtual bool do_load(void) {
				ROBO_LBREAKN(mexo::backend::devagent::do_load());
				if (feedback_.agent.state.local == state_s::locals::configure) {
					quest_configure(nullptr);					
					::robo::quest::post();
				}
				return true;
			}

		};



		class boardagent : public robo::backend::boardagent {
			devagent dev_;
		public:
			boardagent(robo::cstr _name, robo::backend::servo& _servo)
				: robo::backend::boardagent(_name, _servo)
				, dev_(RT("dev"),*this) {
			};
			
		};
		

		class servo : public mexo::backend::servo {
			int boards_count_ = 0;
			boardagent** boards_ = nullptr;
		protected:
			bool do_load(void) {
				ROBO_LBREAKN(mexo::backend::servo::do_load());
				ROBO_LBREAKN(robo::ini::load(current_path(), defaults_path(), RT("board_count"), boards_count_))
				if (boards_count_ > 0) {
					boards_ = new boardagent * [boards_count_];
					ROBO_APP_ASSERT(boards_ != nullptr);
					boardagent** pf = boards_;
					robo::string key;
					for (int i = 0; i < boards_count_; ++i, ++pf) {
						*pf = nullptr;
					}
					pf = boards_;
					for (int i = 0; i < boards_count_; ++i, ++pf) {
						key.format(RT("board-%d"), i+1);
						boardagent* f = new boardagent(key, *this);
						ROBO_APP_ASSERT(f != nullptr);
						*pf = f;
					}
				}


				return true;
			}
			virtual void do_clean(void) {
				if (boards_ != nullptr) {
					boardagent** pf = boards_;
					for (int i = 0; i < boards_count_; ++i, ++pf) {
						if (*pf != nullptr) delete* pf;
					}
					delete[] boards_;
					boards_ = nullptr;
				}
				mexo::backend::servo::do_clean();
			}
		public:
			servo(robo::app::module & _module)
				: mexo::backend::servo(RT("servo"), _module)
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
