#include "im/edev/edev.hpp"
#include "core/robosd_log.hpp"
#include "core/robosd_system.hpp"
#include "core/robosd_ini.hpp"
namespace robo {
	namespace edev {

		agent::block::block(agent& _agent, cstr _name): owner(_agent), name(_name), ref_(*this) {
			ref_.attach_to(_agent.blocks_);
			path.format(RT("%s.%s"), owner.name.c_str(), name.c_str());
		}

		bool agent::block::load_(void) {
			string common;
			path.format(RT("%s.%s"), owner.name.c_str(), name.c_str());
			common.format(RT("%s.%s"), owner.type.c_str(), name.c_str() );
			ROBO_LBREAKN(do_load(path, common.c_str()));
			return true;
		}

		static agent::map& agents_(void) {
			static agent::map agents__;
			return agents__;
		}

		bool agent::attach_(cstr _name, cstr _lib, cstr _type, void* _instance) {
			ref_.set_key(hash(_name));
			ROBO_LBREAKN(ref_.attach_to(agents_()));
			name = _name;
			lib = _lib;
			type = _type;
			lib_instance = _instance;
			return true;
		}

		bool agent::begin_(void) {
			for (block::ref* p = blocks_.first(); p; p = p->next()) {
				ROBO_LBREAKN(p->owner().load_());
			}

			ROBO_LBREAKN(do_begin());
			robo_infolog("emulator agent '%s' was started", name.c_str());


			for (block::ref* p = blocks_.first(); p; p = p->next()) {
				p->owner().do_reconfig();
			}

			return true;
		}

		bool agent::do_begin(void) {
			ROBO_LRET(ini::load(name, type, RT("SAMPLE_TIME_SEC"), sample_time));
		}
		agent* agent::find(int _id) {
			agent* tmp = agents_().find(_id);
			if (tmp == nullptr) {
				robo_errlog("agent for device 0x%x doesn't found", _id);
				return nullptr;
			}
			else {
				return tmp;
			}
		}
		agent* agent::find(cstr _name) {
			agent* tmp = find(hash(_name));
			if (tmp == nullptr) {
				robo_errlog("agent for device with name %s doesn't found", _name);
				return nullptr;
			}
			else {
				return tmp;
			}
		}

		void agent::run( double time) {
			for (agent::ref* p = agents_().first(); p; p = p->next()) {
				p->owner().run_(time);
			}
		}

		void agent::run_(double _time) {
			if (_time >= next_time_) {
				for (block::ref* p = blocks_.first(); p; p = p->next()) {
					p->owner().do_run();
				}
				next_time_ += sample_time;
				do_priotitet_run(_time);
			}
			do_background_run(_time);
		}

		bool agent::try_attach_(cstr _name, cstr _lib, cstr _type, void* _instance) {
			robo_edev_query_f  query = (robo_edev_query_f)system::lib::proc_get(_instance, RT("query_agent"));
			ROBO_LBREAKN(query != nullptr);
			agent* ag = query();
			ROBO_LBREAKN(ag != nullptr);
			ROBO_LBREAKN(ag->attach_(_name, _lib, _type, _instance));
			ag->set_local_ini(robo::system::ini::source());
			return true;
		}
		bool agent::begin(void) {
			int tmp = 0;
			string sec;
			string lib;
			string lib_copy;
			string name;
			string type;
			ROBO_LBREAKN(ini::load(RT("IMITATION_MODEL"), RT("DEV_COUNT"), tmp));
			for (int i = 0; i < tmp; i++) {
				sec.format(RT("IMITATION_MODEL_DEV_%d"), i+1);
				ROBO_LBREAKN(name.load(sec.c_str(), RT("NAME")));
				ROBO_LBREAKN(type.load(sec.c_str(), RT("TYPE")));
				ROBO_LBREAKN(lib.load(type.c_str(), RT("LIB")));
				ROBO_LBREAKN_F(system::lib::exists(lib), "module '%s' isn't found", lib.c_str());
				lib_copy.format(RT("~%s-%s-%d"), name.c_str(), lib.c_str(), i);
				if (system::lib::exists(lib_copy)) {
					ROBO_LBREAKN(system::lib::remove(lib_copy));
				}
				ROBO_LBREAKN(system::lib::copy(lib,lib_copy));
				ROBO_LBREAKN(system::lib::exists(lib_copy));//paranoia
				void* instance = system::lib::load(lib_copy);
				ROBO_LBREAKN_F(instance != nullptr, "lib '%s' isn't found  ", lib_copy.c_str());
				if (!agent::try_attach_(name, lib_copy,  type,  instance)) {
					system::lib::free(instance);
					ROBO_LBREAK_F(RT("agent '%s' isn't attached from lib '%s' "), name.c_str(), lib_copy.c_str());
				} 
			}
			for (agent::ref* p = agents_().first(); p; p = p->next()) {
				ROBO_LBREAKN(p->owner().begin_());
			}
			reconfig_();
			return true;
		}

		void agent::finish(void) {
			agent::ref* p;
			for (p = agents_().first(); p; p = p->next()) {
				p->owner().do_finish();
				robo_infolog("emulator agent '%s' was finished", p->owner().name.c_str());
			}
			agent* ag;
			while ( (ag = agents_().pop()) != nullptr ) {
				void* instance = ag->lib_instance;
				ag = nullptr;//paranoia
				system::lib::free(instance);
			}

		}

		void agent::reconfig_(void) {
			agent::ref* p;
			for (p = agents_().first(); p; p = p->next()) {
				p->owner().do_reconfig();
			}
		}
	}
}
