#include "im/edev/joint_link.hpp"
#include "im/edev/edev.hpp"
#include "core/robosd_log.hpp"

namespace robo{
	namespace edev{
		namespace joint{
			link::map &  links_(void){
				static link::map links__;
				return links__;
			};

			bool link::do_load(cstr _specific_sect, cstr _common_sect){
				ref_.set_key(hash(_specific_sect));

				ROBO_LRET(ref_.attach_to(links_()));
			}

			iactuator dummy_actuator;
			iload dummy_load;

			link::link(agent& _agent, cstr _name): agent::block(_agent, _name), ref_(*this,-1){
				actuator = &dummy_actuator;
				load = &dummy_load;
			}

			void link::connect_to_actuator(iactuator * _actuator){
				if (_actuator){
					actuator = _actuator;
				}
				else{
					actuator = &dummy_actuator;
				}
			}

			void link::connect_to_load(iload * _load){
				if (_load){
					load = _load;
				}
				else{
					load = &dummy_load;
				}
			}

			link* link::find(int _id) {
				link* tmp = links_().find(_id);
				if (tmp == nullptr) {
					robo_errlog("link with id  0x%x doesn't found", _id);
					return nullptr;
				}
				else {
					return tmp;
				}
			}

			link* link::find(cstr _name) {
				link* tmp = find(hash(_name));
				if (tmp == nullptr) {
					robo_errlog("link with name %s doesn't found", _name);
					return nullptr;
				}
				else {
					return tmp;
				}
			}
		}
	}
}