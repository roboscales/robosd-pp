#include "im/edev/edev.hpp"
#include "core/robosd_log.hpp"
#include "core/robosd_system.hpp"
#include "core/robosd_ini.hpp"
namespace robo {
	namespace edev {
		static agent::map& agents_(void) {
			static agent::map agents__;
			return agents__;
		}

		agent* find_agent(int _id) {
			agent* tmp = agents_().find(_id);
			if (tmp == nullptr) {
				robo_errlog("agent for device 0x%x doesn't found", _id);
				return nullptr;
			}
			else {
				return tmp;
			}
		}
		agent* find_agent(cstr _name) {
			agent* tmp = find_agent(hash(_name));
			if (tmp == nullptr) {
				robo_errlog("agent for device with name %s doesn't found", _name);
				return nullptr;
			}
			else {
				return tmp;
			}
		}

		void run(double period, double time) {
			for (agent::ref* p = agents_().first(); p; p = p->next()) {
				p->owner().run(period, time);

			}
		}

		bool begin(void) {
			int tmp = 0;
			string sec;
			string lib;
			string lib_copy;
			string name;
			string type;
			ROBO_BREAKN(ini::load(RT("IMITATION_MODEL"), RT("DEV_COUNT"), tmp));
			for (int i = 0; i < tmp; i++) {
				sec.format(RT("IMITATION_MODEL_DEV_%d"), tmp);
				ROBO_BREAKN(name.load(sec.c_str(), RT("NAME")));
				ROBO_BREAKN(type.load(sec.c_str(), RT("TYPE")));

				ROBO_BREAKN(lib.load(type.c_str(), RT("LIB")));
				ROBO_LBREAKN_F(system::lib::exists(lib), "module isn't found  %s", lib.c_str());
				lib_copy.format(RT("~%s-%s-%d"), name.c_str(), lib.c_str(), i);
				if (system::lib::exists(lib_copy)) {
					system::lib::
				}


			}
			for (agent::ref* p = agents_().first(); p; p = p->next()) {
				ROBO_LBREAKN(p->owner().begin());
			}
			return true;
		}

		void finish(double period, double time) {
			agent::ref* p;
			for (p = agents_().first(); p; p = p->next()) {
				p->owner().finish();
			}
			while ( (p = agents_().first()) != nullptr ) {
				agent* ag = & ( p->owner() );
				if (ag->lib_instance) {
					system::lib::free(ag->lib_instance);
				}
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

