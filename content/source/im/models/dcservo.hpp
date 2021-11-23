#ifndef __dcservo_hpp
#define __dcservo_hpp

#include "im/models/models.hpp"
#include "im/models/power.hpp"
#include "im/models/dcmachine.hpp"
#include <armadillo>
namespace robo{
	namespace im{
		namespace dcservo{
			class ideal : public dcmachine::ideal2{
			private:
				float load_inert_kgm2 = 0.01f;
				float load_max_kgsm = 35.f;
				float speed_max_gps = 60.f;
				float position_max_ms = 2.5f;
				float position_min_ms = 0.5f;
				float position_max_g = 180.f;
				float position_min_g = 0.f;
				float voltage_max = 7.f;
				float flux_score = .88f;
				float beta_score = 3.f;
				float prop_score = .3f;
				float load_score = 1.f;

				float Mmax = 1.f;
				float wn = 1.f;
				float Kp = 1.f;
				float Kd = 1.f;
				float Jo = 0.01f;
				float position_gain=1.0f;
				float speed_errint = 0.f;
				float speed_err = 0.f;
				float position_err = 0.f;
				float speed_req = 0.f;
				float Ko;
				class isettings{
					ideal & owner_;
				public:
					block::parametr<float> load_inert_kgm2;
					block::parametr<float> load_max_kgsm;
					block::parametr<float> speed_max_gps;
					block::parametr<float> position_max_ms;
					block::parametr<float> position_min_ms;
					block::parametr<float> position_max_g;
					block::parametr<float> position_min_g;
					block::parametr<float> voltage_max;
					block::parametr<float> flux_score;
					block::parametr<float> beta_score;
					block::parametr<float> prop_score;
					isettings(ideal & _owner);
				};
				friend class isettings;
			protected:
				virtual void reconfig(void);
				virtual bool setup(void);
			public:
				float position_req;
				isettings settings;
				ideal();					
				void set_duty_ms(float  _position_ms);
				virtual void run(void);
			};
		}
	}
}
#endif