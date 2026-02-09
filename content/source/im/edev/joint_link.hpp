#ifndef __join_link_hpp
#define __join_link_hpp
#include "im/edev/edev.hpp"
namespace robo{
	namespace edev{
		namespace joint{

			struct ROBO_EXPORT iactuator{
				enum class istate { blocked = 0, start = 1, run = 2};
				float driveng_torque = 0.f;
				float contr_torque = 0.f;
				float speed = 0.f;
				double position = 0;
				istate state = istate::run;
			};

			struct ROBO_EXPORT iload{
				double driveng_position = 0.f;
				float driveng_torque = 0.f;
				float driveng_speed = 0.f;
				float acceleration = 0.f;
				float speed = 0.f;
				double position = 0.f;
				float tension = 0.f;
				float tension_diff = 0.f;
				double offset_rad = 0.;
				bool inverce = 0.;
			};

			class ROBO_EXPORT link:public agent::block {
			public:
				typedef ::robo::list::unique<link, int> map;
				typedef map::ref ref;
			private:
				ref ref_;
			protected:
				virtual bool do_load(cstr _specific_sect, cstr _common_sect);
			public:
				iactuator * actuator;
				iload * load;
				link(agent& _agent, cstr _name);
				void connect_to_actuator(iactuator * _actuator);
				void connect_to_load(iload * _load);
				static link * find(int _id);
				static link* find(cstr _caption);
			};
			
		}
	}
}

#endif

