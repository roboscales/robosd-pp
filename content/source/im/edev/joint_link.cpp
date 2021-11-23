#include "im/edev/joint_link.hpp"
#include "im/edev/edev.h"
#include "core/robosd_log.hpp"
#include "robosd_target_api.h"
namespace robo{
	namespace im{
		namespace joint{
			link::map &  link::map_(void){
				static map map_;
				return map_;
			};
			bool link::configure(int _id, const robo_string_t _section, float _model_period_sec){
				ROBO_BREAKN(block::configure(_section, _model_period_sec));
				caption.load(_section, RS("link"));
				ref_.set_id(_id);
				return (ref_.attach_to(map_()) == ROBO_SUCCESS);
			}

			iactuator dummy_actuator;
			iload dummy_load;
			link::link(void): ref_(this){
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
			link * link::find(int _id){
				return map_().find(_id);
			}
		}
	}
}