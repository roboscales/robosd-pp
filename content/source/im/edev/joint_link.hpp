#ifndef __join_common_hpp
#define __join_common_hpp
#include "core/robosd_common.h"
#include "im/models/models.hpp"
#include "core/robosd_list.hpp"
#include "core/robosd_cstring.hpp"
namespace robo{
	namespace im{
		namespace joint{
			struct ROBO_EXPORT iactuator{
				enum class istate { blocked, start, run };
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
			};

			class ROBO_EXPORT link :public robo::models::block{
			public:
				typedef robo::list::ref<link> ref;
				typedef robo::list::map<link> map;
			private:
				ref ref_;
				static map &  map_(void);
			public:
				iactuator * actuator;
				iload * load;
				robo::string caption;
				link(void);
				bool configure(int _id, const robo_string_t _section, float _model_period_sec);
				void connect_to_actuator(iactuator * _actuator);
				void connect_to_load(iload * _load);
				static link * find(int _id);
			};
			typedef robo::models::block::parametr<float> parametr;
		}
	}
}

#endif

