#ifndef __join_gearbox_hpp
#define __join_gearbox_hpp

#include "im/edev/joint_link.hpp"

namespace robo{
	namespace im{
		namespace joint{
			namespace gearbox{
				namespace elastic{
					class ROBO_EXPORT ideal : public link{
					protected:
						virtual bool setup(void);
						virtual void reconfig(void);
					public:
						struct{
							float elastic_torque = 0.f;
							float speed = 0.f;
							double position = 0.f;
							float tension = 0.f;
							float tension_diff = 0.f;
						} observer;
						float hook_gain = 0.f;
						float load_viscous_gain = 0.f;
						float gear_ratio = 1.f;
						virtual void run(void);
						ideal(void);
					};
					class ROBO_EXPORT nonline : public link{
					protected:
						virtual bool setup(void);
						virtual void reconfig(void);

					public:

						struct{
							float speed = 0.f;
							double position = 0.f;
							float elastic_torque = 0.f;
							float tension = 0.f;
							float tension_diff = 0.f;
							float tension_max = 0.f;
							float hook_gain = 0.f;
							float load_viscous_gain = 0.f;
							float gear_ratio = 1.f;
							float dead_zone = 0.f;
							struct{
								float dead_zone = 0.f;
								float tension_max = 0.f;
								float torque_max = 0.f;
							} config;
						} driver;

					

						struct{
							double min = 0.f;
							double max = 0.f;
							float gain = 0.f;
							float torque = 0.f;
							float tension = 0.f;
							float tension_diff = 0.f;
							float tension_max = 0.f;
							float desipation_gain = 0.f;
							struct{
								float min = 0.f;
								float max = 0.f;
								float tension_max = 0.f;
								float torque_max = 0.f;
							} config;
							double position = 0.f;
							int enabled;
						} supply;

						virtual void run(void);
						nonline(void);
					};

					class ROBO_EXPORT friction : public nonline{
						enum class result { STOP, START, RUN };
						result run_(float _fric);
					protected:
						virtual bool setup(void);
						virtual void reconfig(void);

					public:
						float rest = 0.f;
						float dry = 0.f;
						float crawl_speed = 0.f;
						float scale = 0.f;
						struct{
							float total = 0.f;
							float friction = 0.f;
						} torque;
						struct {
							float crawl_speed = 0;
							float crawl_speed_grad = 0;
							float rest = 0.f;
							float dry = 0.f;
							float tension_gain = 0.f;
						} base;
						virtual void run(void);
						friction(void);
					};
					class ROBO_EXPORT brake : public friction{
						float set_time_us;
						float release_time_us;
						float set_time;
						float release_time;
						float set_dry;
						float release_dry;
						float set_rest;
						float release_rest;
						bool fast_set;
						bool fast_release;
						float release_beta;
						float release_gama;
						float set_beta;
						float set_gama;
						enum class state { set, release } state_;
					protected:
						virtual bool setup(void);
						virtual void reconfig(void);

					public:
						virtual void run(void);
						brake(void);
						void set(void);
						void release(void);
					};
				}
			}
		}
	}
}

#endif

