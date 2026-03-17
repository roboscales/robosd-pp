#ifndef __im_edev_joint_pmsmr2_hpp
#define __im_edev_joint_pmsmr2_hpp
#include "im/edev/joint_link.hpp"

#include "im/models/power.hpp"


#include "kinematics/robosd_tractor.hpp"

namespace robo{
	namespace edev{
		namespace joint{
			namespace pmsmr2{
				class ROBO_EXPORT  ideal2 : public agent::block {
				private:
                    
                    constexpr static inline double pi = robo::pi<double>;
                    double a = 0.;
                    double b = 0.;
                    double c = 0.;
                    double d = 0.;
                    double g = 0.;

                   
                    double Ls = 0;
                    double T = 0;
                    double g_factor = 0;
                    double T2 = 0;
                    double T3 = 0;
                    double LsT = 0;
                    double LsT2 = 0;
                    double LsT3 = 0;
                    double twoLs = 0;
                    double twoLsT = 0;
                    double a2 = 0;

                    double LsT2a = 0;
                    double LsT2a2 = 0;
                    double LsT2b = 0;
                    double LsT2c = 0;
                    double LsT2d = 0;

                    double LsTa = 0;
                    double LsTb = 0;
                    double LsTc = 0;
                    double LsTd = 0;

                    double T2a = 0;
                    double T2c = 0;

                    double D = 0;
                    double C1 = 0;

                    double LsT2ab = 0;
                    double LsT2ac = 0;
                    double LsT2bc = 0;
                    double LsT2bd = 0;
                    double LsT2cd = 0;
                    double LsT2d2 = 0;

                    double twoLsTa = 0;
                    double twoLsTb = 0;
                    double twoLsTd = 0;

                    double A_id0 = 0;
                    double A_iq0 = 0;

                    double aT = 0;
                    double aT2 = 0;
                    double aT3 = 0;
                    double c0 = 0;
                    double c1 = 0;


                    void teylor_begin_(void);
                    void teylor_run_(void);
                    void locked_rotor_taylor_(void);
                    void taylor_step_no_current(void);
                protected:
                    int p = 1;
                    double Lf3 = 0.0001;
                    double Rf3 = 2.;
                    double J3 = 0.000001;
                    double Kv3 = 0.;
                    double kL=0.001;

                    double Fm3 = 0.02;
                    double Ls3 = 0.0001;
                    double Lm3 = 0.0001;

					double Rs = 2.;
					double Psi_e = 0.022;
					double Psi_m = 0.022;

					virtual bool do_load(cstr _specific_sect, cstr _common_sect);
					virtual void do_reconfig(void);
					virtual void do_run(void);
				public:

					struct{
						double speed = 0.;
						double phase = 0.;
						double position = 0.;
					} electro;

					power::ph3_t<double> current;
					power::ph3_t<double> voltage;
					joint::iactuator actuator;
					bool powerOn = false;
					ideal2(agent& _agent, cstr _name);
                    void set_line_voltage(double _a_V, double _b_V, double _c_V);
					void set_phase_voltage(double _a_V, double _b_V, double _c_V);
					void set_begin_position(double _pos) {
						actuator.position = _pos;
						electro.position = _pos * p;
					}
					uint8_t poles(void){ return (uint8_t)p; } //todo 
				};
			}
		}
	}
}
#endif