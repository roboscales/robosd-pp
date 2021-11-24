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
			string section;
			section.format(RT("%s.%s"), owner.type.c_str(), name.c_str() );
			ROBO_LBREAKN(do_load(section.c_str()));
			do_try_load(path.c_str());
			return true;
		}

		static agent::map& agents_(void) {
			static agent::map agents__;
			return agents__;
		}
		cstr im_sect = RT("IMITATION_MODEL");

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
			for (block::ref* p = blocks_.first(); p; p = p->next()) {
				p->owner().do_reconfig();
			}
			ROBO_LRET(do_begin());
		}

		bool agent::do_begin(void) {
			if ( !ini::try_load(name, RT("SAMPLE_TIME_SEC"), sample_time) ) {
				ROBO_LRET(ini::load(im_sect,RT("SAMPLE_TIME_SEC"), sample_time))
			} else return true;
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
			robo_edev_query_f  query = (robo_edev_query_f)system::lib::proc_get(_instance, RT("get_agent"));
			ROBO_LBREAKN(query != nullptr);
			agent* ag = query();
			ROBO_LBREAKN(ag != nullptr);
			ROBO_LBREAKN(ag->attach_(_name, _lib, _type, _instance));
			return true;
		}
		bool agent::begin(void) {
			int tmp = 0;
			string sec;
			string lib;
			string lib_copy;
			string name;
			string type;
			ROBO_LBREAKN(ini::load(im_sect, RT("DEV_COUNT"), tmp));
			for (int i = 0; i < tmp; i++) {
				sec.format(RT("IMITATION_MODEL_DEV_%d"), tmp);
				ROBO_LBREAKN(name.load(sec.c_str(), RT("NAME")));
				ROBO_LBREAKN(type.load(sec.c_str(), RT("TYPE")));
				ROBO_LBREAKN(lib.load(type.c_str(), RT("LIB")));
				ROBO_LBREAKN_F(system::lib::exists(lib), "module isn't found  %s", lib.c_str());
				lib_copy.format(RT("~%s-%s-%d"), name.c_str(), lib.c_str(), i);
				if (system::lib::exists(lib_copy)) {
					ROBO_LBREAKN(system::lib::remove(lib_copy));
				}
				ROBO_LBREAKN(system::lib::copy(lib,lib_copy));
				ROBO_LBREAKN(system::lib::exists(lib_copy));//paranoia
				void* instance = system::lib::load(lib_copy);
				ROBO_LBREAKN(instance != nullptr);
				if (!agent::try_attach_(name, type, lib, instance)) {
					system::lib::free(instance);
					ROBO_LBREAK_F(RT("agent '%s' isn't attached from lib '%s' "), name.c_str(), lib_copy.c_str());
				} 
			}
			for (agent::ref* p = agents_().first(); p; p = p->next()) {
				ROBO_LBREAKN(p->owner().begin_());
			}
			reconfig();
			return true;
		}

		void agent::finish(void) {
			agent::ref* p;
			for (p = agents_().first(); p; p = p->next()) {
				p->owner().do_finish();
			}
			agent* ag;
			while ( (ag = agents_().pop()) != nullptr ) {
				void* instance = ag->lib_instance;
				ag = nullptr;//paranoia
				system::lib::free(instance);
			}
		}

		void agent::reconfig(void) {
			agent::ref* p;
			for (p = agents_().first(); p; p = p->next()) {
				p->owner().do_reconfig();
			}
		}
	}
}

		/*




robo_result_t ROBO_DECL dev_load_p(emu_dev_agent_p _agent, const robo_string_t _lib){
	device_startup_f startup;
 	void *  h;
	ROBO_CHECKRET(dev_reg_p(_agent));
	robo_sprintf(_agent->lib_name, ROBO_LIB_NAME_MAX_LEN, "%s~%d", _lib, _agent->ref.id);
    if (robo_os_lib_exists(_lib) == ROBO_ANSW_YES){
        if (robo_os_lib_exists(_agent->lib_name) == ROBO_ANSW_YES){
            //ROBO_CHECKRET(robo_os_lib_delete(_agent->lib_name));
            robo_os_lib_delete(_agent->lib_name);
		}
		ROBO_CHECKRET(robo_os_lib_copy(_lib, _agent->lib_name));
		ROBO_CHECKRET(robo_os_lib_load(_agent->lib_name, &h));
		_agent->lib_instance = h;
		startup = (device_startup_f)robo_os_lib_proc_get(h, ROBO_EXPORT_FUNCTION_PREFIX "dev_startup" );
		if (!startup){
			robo_os_lib_free(h);
			_agent->lib_instance = 0;
			ROBO_RETERR(" function 'dev_startup' doesn't  found in lib '%s'", _lib);
		}
		ROBO_RETEX(startup(_agent));
	}
	else{
		ROBO_RETERR(" lib '%s' doesn't  found", _lib);
	}
}
robo_result_t ROBO_DECL dev_reg_p(emu_dev_agent_p _agent){
	robo_result_t ret = robo_cell_add_p(&emu_devices, &(_agent->ref));
	if (ret >= ROBO_SUCCESS){
		return ret;
	}
	else{
		ROBO_RETERR("devagent %d registration  error ", _agent->ref.id);
	}
}

void ROBO_DECL dev_unreg_p(emu_dev_agent_p _agent){
	if (_agent->lib_instance){
		robo_os_lib_free(_agent->lib_instance);
		_agent->lib_instance = 0;
		robo_os_lib_delete(_agent->lib_name);
	}
	robo_cell_drop_p(&emu_devices, &(_agent->ref));
}

robo_result_t ROBO_DECL dev_reg_net_driver(int channel_id, emu_chan_driver_p _driver){
	emu_dev_agent_p agent = (emu_dev_agent_p)dev_find_agent(channel_id);
	if (agent == 0){
		ROBO_RETERR("driver registration  to the channel %d error : agent does't found ", channel_id);
	}
	emu_chan_p channel = (emu_chan_p)(agent->instance);
	robo_result_t ret = robo_cell_add_p(&(channel->drivers), &(_driver->ref));
	if (ret >= ROBO_SUCCESS){
		_driver->channel = channel;
		return ret;
	}
	else{
		ROBO_RETERR(" driver registration  to the  channel %d error", channel_id);
	}
}
void ROBO_DECL dev_unreg_net_driver_p(emu_chan_driver_p _driver){
	robo_cell_drop_p(&(_driver->channel->drivers), &(_driver->ref));
}


robo_result_t ROBO_DECL emu_chan_receive(emu_chan_p _channel, robo_byte_p _buf, robo_size_t _len){
	robo_cell_p p;
	for (p = _channel->drivers.first; p; p = p->next){
		emu_chan_driver_p driver  = ((emu_chan_driver_p)p);
		if (driver->request) {
			switch (driver->request(driver, _buf, _len)){
			case EMU_REQUEST_SUCCESS:
				return ROBO_SUCCESS;
			case EMU_REQUEST_CONTINUE:
				continue;
			case EMU_REQUEST_ERROR:
				ROBO_RETERR("request perform error(channel %d, driver  %d)!", _channel->agent->ref.id, driver->ref.id);
				break;
			}
        }else{
			ROBO_RETERR("channel driver  %d doesn't  have recive delegate", _channel->agent->ref.id);
        }
    }
	ROBO_RETERR("channel %d does't found any recive driver", _channel->agent->ref.id);
}
*/

