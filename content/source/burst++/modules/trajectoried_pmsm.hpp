#ifndef burst_modules_tpmsm_hpp
#define burst_modules_tpmsm_hpp

#include "burst++/modules/acw.hpp"
#include "burst++/modules/power3ph.hpp"
#include "burst++/modules/pmsm.hpp"
#include "trajectory_handler.hpp"
#include "trajectoried_pmsm_front.hpp"
#include "stspin32_ps.hpp"

namespace burst
{
    template<class number> class tpmsm_t : public pmsm_t<number>
    {
        using super = pmsm_t<number>; // обращение к классу родителю

        // Типы траекторного генератора
        using traj_init_t = TrajectoryInit<number>;
        using traj_point_t = TrajectoryPoint<traj_init_t, number>;
        using traj_handler_t = TrajectoryHandler<traj_init_t, traj_point_t, number>;

        public:
        	volatile uint8_t Debug_trajectory_inited_flag = 0;
            //сокращения 
            using parameter_t = typename number::parameter_t;
            using signal_t = typename number::signal_t;
            using long_signal_t = typename number::long_signal_t;
            typedef front::tpmsm::action_s<number> action_s;
            typedef front::tpmsm::feedback_s<number> feedback_s;

			#pragma pack(push, 4)
            struct config_s
            {
                typename super::config_s pmsm;
                typename traj_handler_t::config_s thandler;
                long_signal_t time_scale;
            };
            
            struct 
            {
                #if BURST_PANICS_STSPIN_ENABLED == 1
                bool & driver_fault;
                #endif
            } panic;

        #define TPMSM_CONFIG(a) TPMSM_CONFIG_(a)
		#define TPMSM_CONFIG_(a)\
		{\
            PMSM_CONFIG(a) \
			, ACTOR_CONFIG(a) \
            , a##_TIME_SCALE \
		}

		#pragma pack(push, 4)
        struct present_s
        {
            typename super::present_s pmsm;
            typename traj_handler_t::present_s thandler;
            struct
            {
            	long_signal_t out;
                long_signal_t acceleration;
                long_signal_t deceleration;
                long_signal_t overall_time;
            } speed;
            struct
            {
            	long_signal_t out;
                long_signal_t speed;
                long_signal_t acceleration;
                long_signal_t deceleration;
            } position;
        };
        public:
        	traj_init_t traj_init;
			traj_handler_t traj_handler_;
			LinearSpeedProfile<number, traj_init_t> linear_speed_profile_;
			LinearPositionProfile<number, traj_init_t> linear_position_profile_;
			signal_t & speedflt_value;
			long_signal_t & position_sensor_value;
#if BURST_PANICS_STSPIN_ENABLED == 1
			panic panic_;
#endif
			tpmsm_t(
				int _dev_id
				, const config_s& _config
				, present_s& _present
				, action_s& _action
				, feedback_s& _feedback
				, ps::control& _ps
				, typename powe3ph<number>::rotator_t& _rotator
				, typename powe3ph<number>::inverter& _inverter
				, signal_t& _current_cross
				, signal_t& _current_lateral
				, signal_t& _speed
				, long_signal_t& _position
				#if BURST_PANICS_ACTUATOR_TEMPER_ENABLED == 1 && BURST_PROTECTION_ENABLED == 1
				, signal_t& _temper
				#endif
				#if BURST_PROTECTION_ENABLED == 1
				#if BURST_PANICS_ACWC_OVERCURRENT_ENABLED ==1
				, const signal_t& _current_mag
				#endif
				#endif
				, typename traj_handler_t::present_s & _phandler
				, typename traj_handler_t::config_s & _chandler
			) : super(
				_dev_id
				, _config.pmsm
				, _present.pmsm
				, _action.pmsm
				, _feedback.pmsm
				, _ps
				, _rotator
				, _inverter
				, _current_cross
				, _current_lateral
				, _speed
				, _position
				#if BURST_PANICS_ACTUATOR_TEMPER_ENABLED == 1 && BURST_PROTECTION_ENABLED == 1
				, _temper
				#endif
				#if BURST_PROTECTION_ENABLED == 1
				#if BURST_PANICS_ACWC_OVERCURRENT_ENABLED ==1
				, _current_mag
				#endif
				#if BURST_PANICS_STSPIN_ENABLED == 1
				, _fault_flag
				#endif
				#endif
			)
			, speed_traj_mode_(*this)
			, position_traj_mode_(*this)
			, traj_handler_(_chandler, _phandler)
			, linear_speed_profile_(traj_handler_.profileDict)
			, linear_position_profile_(traj_handler_.profileDict)
            , speedflt_value(_speed)
			, position_sensor_value(_position)
#if BURST_PANICS_STSPIN_ENABLED == 1
				panic_.driver_fault
			,
#endif
			{
				traj_handler_.begin();
			}

        protected:

			/*
			 * Режим траекторный скоростной
			 */

			/*
			 * Применение структуры action (вызывается при update == true)
			 */
            void mode_speed_traj_applay_action(void)
            {
                DEV_ACTION_S(a);
                DEV_PRESENT_S(p);
                DEV_CONFIG_S(c);

                p.speed.acceleration = a.speed_trajectory_opt.acceleration;
                p.speed.deceleration = a.speed_trajectory_opt.deceleration;

                traj_init.acceleration = p.speed.acceleration;
                traj_init.deceleration = p.speed.deceleration;

                traj_init.start = speedflt_value;
                traj_init.end = a.pmsm.cross.ac.speed;

                traj_init.scale = 1000000;
                traj_init.profile_type = a.speed_trajectory_opt.profile_number;

                traj_handler_.approximer(traj_init, time_us());
                this->mode_speed_ov_voltage_cl_applay_action();
            };

            void mode_speed_traj_start(void)
            {
                this->mode_speed_ov_voltage_cl_start();
                traj_handler_.SetState(traj_handler_t::Trajectory_States::AWAITING);
            };

            void mode_speed_traj_stop(void)
            {
            	this->mode_speed_ov_voltage_cl_stop();
            	traj_handler_.SetState(traj_handler_t::Trajectory_States::AWAITING);
            };

            void mode_speed_traj_runB(void)
            {

                this->mode_speed_ov_voltage_cl_runB();
            }

            void mode_speed_traj_trajectory_loop(void)
            {
            	DEV_ACTION_S(a);
                DEV_PRESENT_S(p);
                traj_handler_.run(p.speed.out, time_us());
                p.pmsm.cross.ac.speed.req = p.speed.out;
            }

        private:

            class speed_traj_mode : public dev::mode
            {
                friend class tpmsm_t;
                speed_traj_mode(tpmsm_t& _actuator) :
                    dev::mode(front::tpmsm::modes::speed_traj, _actuator) {}

                protected:
                virtual void applay_action(void) { owner<tpmsm_t>().mode_speed_traj_applay_action(); }
                virtual void start(void) { owner<tpmsm_t>().mode_speed_traj_start(); }
                virtual void stop(void) { owner<tpmsm_t>().mode_speed_traj_stop(); }
                virtual void loopA(void) { owner<tpmsm_t>().mode_speed_ov_voltage_cl_runA(); }
                virtual void loopB(void) { owner<tpmsm_t>().mode_speed_traj_runB(); }
                virtual void loopC(void) {   }
                virtual void frontend_loop(void) { }
                virtual void trajectory_loop(void) { owner<tpmsm_t>().mode_speed_traj_trajectory_loop(); }

            } speed_traj_mode_;

		protected:
			/*
			 * Режим траекторный позиционный
			 */

			/*
			 * Применение структуры action (вызывается при update == true)
			 */
            void mode_position_traj_applay_action(void)
            {
                DEV_ACTION_S(a);
                DEV_PRESENT_S(p);
                DEV_CONFIG_S(c);

                p.position.speed = a.pos_trajectory_opt.speed;
                p.position.acceleration = a.pos_trajectory_opt.acceleration;
                p.position.deceleration = a.pos_trajectory_opt.deceleration;

                traj_init.speed = p.position.speed;
                traj_init.acceleration = p.position.acceleration;
                traj_init.deceleration = p.position.deceleration;

                traj_init.start = position_sensor_value;
                traj_init.end = a.pmsm.cross.ac.position;

                traj_init.scale = 1000;
                traj_init.profile_type = a.pos_trajectory_opt.profile_number;

                traj_handler_.approximer(traj_init, time_ms());
                this->mode_position_ov_voltage_cl_applay_action();
            };

            void mode_position_traj_start(void)
            {
                this->mode_position_ov_voltage_cl_start();
                traj_handler_.SetState(traj_handler_t::Trajectory_States::AWAITING);
            };

            void mode_position_traj_stop(void)
            {
            	this->mode_position_ov_voltage_cl_stop();
            	traj_handler_.SetState(traj_handler_t::Trajectory_States::AWAITING);
            };

            void mode_position_traj_runB(void)
            {
                this->mode_position_ov_voltage_cl_runB();
            }

            void mode_position_traj_trajectory_loop(void)
            {
            	DEV_ACTION_S(a);
                DEV_PRESENT_S(p);
                traj_handler_.run(p.position.out, time_ms());
                p.pmsm.cross.ac.position.req = p.position.out;
            }

        private:

            class position_traj_mode : public dev::mode
            {
                friend class tpmsm_t;
                position_traj_mode(tpmsm_t& _actuator) :
                    dev::mode(front::tpmsm::modes::position_traj, _actuator) {}

                protected:
                virtual void applay_action(void) { owner<tpmsm_t>().mode_position_traj_applay_action(); }
                virtual void start(void) { owner<tpmsm_t>().mode_position_traj_start(); }
                virtual void stop(void) { owner<tpmsm_t>().mode_position_traj_stop(); }
                virtual void loopA(void) { owner<tpmsm_t>().mode_position_ov_voltage_cl_runA(); }
                virtual void loopB(void) { owner<tpmsm_t>().mode_position_traj_runB(); }
                virtual void loopC(void) {  }
                virtual void frontend_loop(void) {  }
                virtual void trajectory_loop(void) { owner<tpmsm_t>().mode_position_traj_trajectory_loop(); }

            } position_traj_mode_;

        public:

            virtual void switch_to_mode(int _mode)
            {
            	DEV_PRESENT_S(p);

            	super::switch_to_mode(_mode);

            	if(p.pmsm.cross.ac.dev.mode > front::pmsm::modes::mode_count && p.pmsm.cross.ac.dev.mode <= front::tpmsm::modes::mode_count)
            	{
                    DEV_CONFIG_S(cfg);
                    p.pmsm.lateral.current.req = 0;
                    p.pmsm.lateral.current.range = cfg.pmsm.lateral.current.range;
                    p.pmsm.lateral.voltage.range = cfg.pmsm.lateral.voltage.range;
                    this->lpi.begin();
            	}
            }

            virtual void loopA(void) {
                DEV_PRESENT_S(p);
                super::loopA();   // вызывает dev::loopA() + lpi.run() для acw-режимов
                // Дополнительно: lpi.run() для траекторных режимов
                if (p.pmsm.cross.ac.dev.mode > front::pmsm::modes::mode_count && p.pmsm.cross.ac.dev.mode <= front::tpmsm::modes::mode_count) {
                	this->lpi.run();
                }
            }

    		virtual void realtime_protection(void) {
    			super::realtime_protection();
#if BURST_PANICS_STSPIN_ENABLED == 1
    			if(panic_.driver_fault == 1)
    			{
    				raise_panic(bits::stspin32_fault);
    			}
    			else
    			{
    				reset_panic(bits::stspin32_fault);
    			}
#endif

    		}
    };
}

#endif /*burst_modules_tpmsm_hpp*/
