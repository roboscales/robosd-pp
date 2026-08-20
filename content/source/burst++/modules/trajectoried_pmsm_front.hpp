#ifndef burst_modules_tpmsm_front_hpp
#define burst_modules_tpmsm_front_hpp

#include "burst++/modules/actuator.front.hpp"
#include "burst++/modules/acw.front.hpp"
#include "burst++/modules/pmsm.front.hpp"

namespace burst 
{
    namespace front
    {
        namespace tpmsm
        {
            #pragma pack(push, 1)

            template<typename number> struct action_s
            {
                pmsm::action_s<number> pmsm;

                struct 
                {
                    typename number::signal_t acceleration;
                    typename number::signal_t deceleration;
                    typename number::parameter_t profile_number;
                } speed_trajectory_opt;
                struct 
                {
                    typename number::signal_t acceleration;
                    typename number::signal_t deceleration;
                    typename number::signal_t speed;
                    typename number::parameter_t profile_number;
                } pos_trajectory_opt;
            };

            template<typename number> struct feedback_s
            {
                pmsm::feedback_s<number> pmsm;
//                struct
//                {
//                    typename number::long_signal_t time;
//                    typename number::long_signal_t point_des;
//                } trajectory_status;
            };

            struct modes 
            {
				enum 
                {
                    speed_traj = pmsm::modes::mode_count + 1, //14
                    position_traj = pmsm::modes::mode_count + 2, //15
                    mode_count = position_traj,
				};
			};
        }
    }
}

#endif /*burst_modules_tpmsm_front_hpp*/
