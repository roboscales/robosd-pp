#ifndef burst_modules_actuator_hpp
#define burst_modules_actuator_hpp
#include "burst++/burst.hpp"
#include "burst++/math.hpp"
#include "burst++/modules/ps.hpp"
#include "burst++/modules/brake.hpp"
#include "burst++/modules/actuator.hpp"
#include "core/robosd_stateflow.hpp"

namespace burst{
	template <class number, class brake_t, class actuator_t, class driver_t>  class motion_machine : public robo::controller, public actor {
    public:
		
        struct  config_s {
			actor::config_s tag;
            //todo  в настройки
            robo::time_us_t startup_pause_us;
            robo::time_us_t  shutdown_pause_us;
            robo::time_us_t  auotostop_pause_us;
            robo::time_us_t  calibrate_timeout_us;
			struct{
				uint32_t positioner;
				uint32_t motion;
				uint32_t calibrate;
			} modes;
        };
		
		struct present_s{
			actor::present_s tag;
			struct {
				uint8_t enable;
				struct {
					signal_t req;
					signal_t prev;
				} speed;
				struct {
					long_signal_t req;
					long_signal_t prev;
				} position;
				struct {
					uint8_t req;
					uint8_t prev;
				} autoblock;
				struct {
					uint8_t req;
					uint8_t prev;
				} command;
			} debug;
		};
    protected:
        class release_task : public  robo::controller::process {
        protected:
			brake_t & brake;
			actuator_t & actuator;
            config_s& settings;
			typename actuator_t::action_s & action;
			typename config_t::config_s & config;

            virtual void onStartup(void) {
                brake.release();
            }
			
            virtual result doStartup(void) {
                return (brake.released() ? result::success : result::wait);
            }
			
            virtual void onShutdown() {
                brake.set();
            }
			
            virtual result doExecute() {
                return result::wait;
            }
			
            virtual result doShutdown() {
                return (brake.fixed() ? result::success : result::wait);
            }
			
        public:
            release_task(config_s& _settings, actuator_t & _actuator, brake_t & _brake) 
			: robo::controller::process()
			, settings(_settings) 
			, actuator(_actuator)
			, brake(_brake)
			, action(_actuator.action())
			{}
            virtual ~release_task() {};
        };

        class move_task : public release_task {
			robo::time_us_t begin_ =0;
        protected:
            bool update_need = false;
            virtual void onPrepare() {
				begin_ = robo::system::time_us();
            }
            virtual result doPrepare() {
				return robo::system::time_us()-begin_>= settings.startup_pause_us ? result::success : result::wait;// tm_dec_();
            }
            virtual void onRelax() {
                begin_ = robo::system::time_us();//tick_ = settings.shutdown_pause;
            }
            virtual result doRelax() {
                return robo::system::time_us()-begin_>= settings.shutdown_pause_us ? result::success : result::wait;//return tm_dec_();
            }
            repeate onFinish() {
				action.ac.dev.mode = burst::front::dev::modes::idle;
				return update_need ? repeate::go : repeate::no;
            }

        public:
            move_task(config_s& _settings, actuator_t & _actuator, brake_t & _brake) 
			: release_task(_settings, _actuator,_brake ) {}
            virtual ~move_task() {};
        };

        class posicioner : public move_task {
			private:
				driver_t & driver_;
				friend class spotter_machine;
				number::signal_t speed_ = 0;
				number::long_signal_t position_ = 0;
				bool autobrake_ = false;
            	robo::time_us_t autostop_begin_us = 0;
			
			protected:
            virtual void onPrepare() {
                action.ac.position = present.enco.ref.position;
                action.ac.speed = 0;
                action.ac.voltage = cfg.range.voltage.hi;
                action.current = config.current.range.hi;
                action.ac.dev.mode = config;
                action.ac.dev.action_actual = true;
				autostop_begin_us = 0;
                move_task::onPrepare();
            }
			
            virtual result doExecute() {
                if (update_need) {
                    action.ac.speed = speed_;
                    action.ac.position = position_;
                    action.ac.dev.action_actual = true;
                    update_need = false;
                }
                else {
                    if (autobrake_)		{											
						if( autostop_begin_us ==0 ){												 
							if ( 
								driver::success();
							//if( driver_t abs(present.speedflt.flt.value ) <=cfg.pmsm.cross.modes.current.positioner.crawlSpeed
							//	&& abs(present.speed.req)
								//&& abs(present.position.req - present.enco.ref.position) <= (cfg.pmsm.cross.modes.current.positioner.deadZone>>2)
							) {
								autostop_begin_us = robo::system::time_us();
								
							}											
						} else {
							if(robo::system::time_us()-autostop_begin_us >= settings.auotostop_pause_us){
								stop();
							}
						}
					}
                }
                return result::wait;
            }
            virtual void onShutdown() {
                action.ac.speed = 0;
                action.ac.position = position_;
                action.ac.dev.action_actual = true;
                move_task::onShutdown();
            }

            posicioner(config_s& _settings, actuator_t & _actuator, brake_t & _brake, driver_t & _driver) 
			: move_task(_settings, _actuator,_brake,_driver)
			{}
        };

        class motion : public move_task {
            friend class spotter_machine;
            number::signal_t speed_ = 0;
            motion(config_s& _settings) : move_task(_settings) {}
            int auto_stop_counter_ = 0;
            virtual void onPrepare() {
                action.ac.position = present.enco.ref.position;
                action.ac.speed = 0;
                action.ac.voltage = cfg.range.voltage.hi;
                action.ac.pmsm.cross.current = cfg.pmsm.cross.current.range.hi;
                action.ac.dev.mode = burst::front::acw::modes::speed;
               // auto_stop_counter_ = settings.auotostop_pause;
            }
			virtual result doPrepare() {
				return result::success;
            }
            virtual result doExecute() {
                if (update_need) {
                    action.ac.speed = speed_;
                    action.ac.dev.action_actual = true;
                    update_need = false;
                }
                return result::wait;
            }
            virtual void onShutdown() {
                action.ac.speed = 0;
                action.ac.dev.action_actual = true;
                move_task::onShutdown();
            }

        };
/*
        class calibrate : public release_task {
            friend class spotter_machine;
            enum class ioperation { stopped = 0, relax, forward, backward, location } operation;
            const unsigned int relax_period_ = SPT_US_TO_TICK(5000000) >> 2;
            const unsigned int speed_scale_shift_ = 0;
            const unsigned int run_period_shift_ = (14 - speed_scale_shift_);
						unsigned int run_period_ = 0;
            unsigned int tick_;
            int64_t acc_forward = 0;
            int64_t acc_backward = 0;
            number::long_signal_t delta_forward = 0;
            number::long_signal_t delta_backward = 0;

            number::long_signal_t pos_el_acc_ = 0;
            number::long_signal_t phase_acc_ = 0;
            number::long_signal_t pos_el_prev_ = 0;
            number::long_signal_t phase_prev_ = 0;
            number::signal_t delta_pos_el_ = 0;
            number::signal_t delta_phase_el_ = 0;
            bool active_ = false;
            const number::long_signal_t speed_ = 65536 << speed_scale_shift_;
            calibrate(config_s& _settings) : release_task(_settings) {}
            virtual ~calibrate() {};

            virtual void onPrepare() {
				run_period_ = (1 << run_period_shift_) * cfg.rotator.pole_count;
                action.ac.speed = 0;
                action.ac.pmsm.synchro.freq = 0;
                action.ac.pmsm.cross.current = 0;
                action.ac.voltage = cfg.range.voltage.hi;

                action.ac.pmsm.lateral.current = (cfg.pmsm.lateral.current.range.hi * 3) >> 2;
                action.ac.pmsm.lateral.voltage = cfg.pmsm.lateral.voltage.range.hi;
                action.ac.dev.mode = burst::front::pmsm::modes::synchro_current;
                action.ac.dev.action_actual = true;

                operation = ioperation::relax;
                tick_ = relax_period_;
                acc_forward = 0;
                acc_backward = 0;
                delta_forward = 0;
                delta_backward = 0;
                pos_el_acc_ = 0;
                phase_acc_ = 0;
                pos_el_prev_ = 0;
                phase_prev_ = 0;
                delta_pos_el_ = 0;
                delta_phase_el_ = 0;
                release_task::onPrepare();
                active_ = true;
            }

            virtual void onExecute() {
                release_task::onExecute();
            }
            virtual void onShutdown() {
                release_task::onShutdown();
                action.ac.dev.mode = burst::front::dev::modes::idle;
                active_ = false;
            }
            virtual result doExecute() {
                delta_pos_el_ = present.rotator.angle.electro - pos_el_prev_;
                pos_el_prev_ = present.rotator.angle.electro;
                pos_el_acc_ += delta_pos_el_;

                delta_phase_el_ = present.rotator.angle.mechanic - phase_prev_;
                phase_prev_ = present.rotator.angle.mechanic;
                phase_acc_ += delta_phase_el_;

                switch (operation) {
								case ioperation::stopped:
								break;
                case ioperation::relax:
                if (tick_ == 0) {
                    operation = ioperation::forward;
                    tick_ = run_period_;
                    present.pmsm.synchro.freq = speed_;
                }
                else {
                    tick_--;
                    break;
                }
                case ioperation::forward:
                if (tick_ == 0) {
                    operation = ioperation::backward;
                    tick_ = run_period_ * 2;
                    present.pmsm.synchro.freq = -speed_;
                }
                else {
                    tick_--;
                    delta_forward = phase_acc_ - pos_el_acc_;
                    acc_forward += delta_forward;
                    break;
                }
                case ioperation::backward:
                if (tick_ == 0) {
                    operation = ioperation::location;
                    tick_ = run_period_;
                    present.pmsm.synchro.freq = speed_;
                }
                else {
                    tick_--;
                    delta_backward = phase_acc_ - pos_el_acc_;
                    acc_backward += delta_backward;
                    break;
                }
                case ioperation::location:
                if (tick_ == 0) {
                    operation = ioperation::stopped;
                    present.pmsm.synchro.freq = 0;
                    cfg.rotator.offset += ((acc_backward + acc_forward) >> (run_period_shift_ + 1 + 1)) / cfg.rotator.pole_count;
										cfg.calibrate_need = false;
                    //todo!!
										//store.save();
										action.ac.store.command = burst::store::commands::save;
//                    mexo_config_save();
                    return result::success;
                }
                else {
                    tick_--;
                    delta_forward = phase_acc_ - pos_el_acc_;
                    acc_forward += delta_forward;
                    break;
                }
                break;
                }
                return result::wait;
            }
        };
*/

        release_task release_task_;
        posicioner posicioner_;
        motion motion_;
//        calibrate calibrate_;
    protected:
        virtual void doTerminate() {
            action.ac.dev.mode = burst::front::dev::modes::idle;
            brake.set();
            calibrate_.active_ = false;
        }


    public:

        bool position_mode(long_signal_t _angle_pp, signal_t _speed_max, bool _autobrake);
        bool speed_mode( signal_t _speed );
        bool brake_set(void);
        bool brake_release(void);

		virtual poll(void){
			ACTOR_CONFIG_S(c);
			ACTOR_PRESENT_S(p)
			if (p.debug.enable) {
				switch (p.debug.command) {
				case 0:
					breakSet();
				break;
				case 1: breakRelease(); 
				break;
				case 3:
					if (p.debug.speed.req != p.debug.speed.prev || p.debug.command.prev != p.debug.command.req) {
						setSpeed(p.debug.speed.req);
						p.debug.speed.prev = p.debug.speed.req;
					}
					break;

				case 4:
					if (p.debug.speed.req != p.debug.speed.prev
						|| p.debug.command.prev != p.debug.command.req
						||p.debug.speed.req != p.debug.speed.prev
						) {
						g_test_req_speed_minpersec_prev = g_test_req_speed_minpersec;
						g_test_req_position_millisec_prev = g_test_req_position_millisec;
						canitf::setPosition(g_test_req_position_millisec, g_test_req_speed_minpersec, g_test_autpblock);
					}
					break;
            case 5:
            if (g_test_command_prev != g_test_command) {
                canitf::calibrate();
            }
            }
            g_test_command_prev = g_test_command;
        }
		}
		
        spotter_machine(config_s& _settings, actuator_t & _actuator, brake_t & _brake, driver_t & _driver)
            : robo::controller()
            , release_task_(_settings, _actuator, _brake)
            , posicioner_(_settings, _actuator, _brake,_driver)
            , motion_(_settings, _actuator, _brake,_driver)
		{}
    };

}