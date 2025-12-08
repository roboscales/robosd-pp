#ifndef burst_modules_actuator_hpp
#define burst_modules_actuator_hpp
#include "burst++/burst.hpp"
#include "burst++/math.hpp"
#include "burst++/modules/filter.hpp"
#include "burst++/modules/ps.hpp"
#include "burst++/modules/enco.hpp"
#include "burst++/modules/motion.hpp"
#include "burst++/modules/positioner.hpp"
#include "burst++/modules/actuator.front.hpp"

#ifndef BURST_PANICS_ACTUATOR_TEMPER_ENABLED
#define BURST_PANICS_ACTUATOR_TEMPER_ENABLED 0
#endif

namespace burst{
	template <class number> class actuator_t: public dev{
		public:
		using parameter_t = typename number::parameter_t;
		using signal_t = typename number::signal_t;
		using long_signal_t = typename number::long_signal_t;
		typedef front::actuator::action_s<number>  action_s;
		typedef front::actuator::feedback_s<number>  feedback_s;
		typedef motion_t<number>  motion_s;
		typedef positioner_t<number>  positioner_s;
		struct config_s{
			burst::dev::config_s dev;
			struct{
				time_us_t reset;
				time_us_t set;
			} enco_fault_ticks;
			struct {
				range_s<signal_t> voltage;
				range_s<signal_t> speed;
				range_s<long_signal_t> position;
			} range;
			#ifndef BURST_ACTUATOR_MOVE_OV_VOLTAGE_MODE_ENABLED
			#define BURST_ACTUATOR_MOVE_OV_VOLTAGE_MODE_ENABLED 0
			#endif
			#if BURST_ACTUATOR_MOVE_OV_VOLTAGE_MODE_ENABLED ==1
			struct {
				typename motion_s::config_s motion;
				typename positioner_s::config_s positioner;
			} modes;
			#endif
			struct {
				#if BURST_PROTECTION_ENABLED == 1
				#if BURST_PANICS_ACTUATOR_TEMPER_ENABLED == 1
				hyst_t<signal_t> temper;
				#endif
				#endif
			} panic;
		};
	
		#if BURST_PANICS_ACTUATOR_TEMPER_ENABLED == 1 && BURST_PROTECTION_ENABLED == 1
		#define BURST_PANICS_ACTUATOR_TEMPER_CO(a)\
		{\
			a##_PANICS_ACTUATOR_TEMPER_OVERHI_PP\
			, a##_PANICS_ACTUATOR_TEMPER_HI_PP\
			, a##_PANICS_ACTUATOR_TEMPER_LO_PP\
			, a##_PANICS_ACTUATOR_TEMPER_ULTRALO_PP\
		}
		#else
		#define BURST_PANICS_ACTUATOR_TEMPER_CO(a)
		#endif

		#if BURST_ACTUATOR_MOVE_OV_VOLTAGE_MODE_ENABLED ==1
		#define BURST_ACTUATOR_MODES_CO(a)\
		,{\
					MOTION_CONFIG(a##_MOTION_OV_VOLTAGE)\
					,POSITIONER_CONFIG(a##_POSITIONER_OV_VOLTAGE)\
			}	
		#else
		#define BURST_ACTUATOR_MODES_CO(a)
		#endif
		
		#define ACTUATOR_CONFIG(a) ACTUATOR_CONFIG_(a)
		#define ACTUATOR_CONFIG_(a)\
		{\
			DEV_CONFIG(a##_REF)\
			,{\
				a##_ENCO_FAULT_TICKS_RESET\
				,a##_ENCO_FAULT_TICKS_SET\
			}\
			,{\
				BURST_RANGE_CONFIG(a##_RANGE_VOLTAGE)\
				, BURST_RANGE_CONFIG(a##_RANGE_SPEED)\
				, BURST_RANGE_CONFIG(a##_RANGE_POSITION)\
			}\
			BURST_ACTUATOR_MODES_CO(a)\
			,{\
				BURST_PANICS_ACTUATOR_TEMPER_CO(a)\
			}\
		}
	protected:
		//int def_mode;
		
		ps::control & psc;	
		#if BURST_PANICS_ACTUATOR_TEMPER_ENABLED == 1 && BURST_PROTECTION_ENABLED == 1
		const signal_t& temper;
		#endif
	private:
		motion_s motion_;
		positioner_s positioner_;
	public:
		struct present_s {		
			burst::dev::present_s dev;
			struct {
				signal_t des;
				signal_t req;
				range_s<signal_t> range;
			} voltage;
			struct {
				signal_t req;
				range_s<signal_t> range;
			} speed;
			struct {
				long_signal_t req;
				range_s<long_signal_t> range;
			} position;
			typename motion_s::present_s motion;
			typename positioner_s::present_s positioner;
		};
		
		
		void set_voltage_range(void) {
			DEV_CONFIG_S(c);
			DEV_PRESENT_S(p);
			DEV_ACTION_S(a);
			range_set(p.voltage.range, a.voltage, c.range.voltage);
		}
		void set_speed_range(void) {
			DEV_CONFIG_S(c);
			DEV_PRESENT_S(p);
			DEV_ACTION_S(a);
			range_set(p.speed.range, a.speed, c.range.speed);
		}
		protected:
		void mode_speed_applay_action(void) {
			DEV_PRESENT_S(p);
			DEV_ACTION_S(a);
			set_voltage_range();
			p.speed.req = range_apply(a.speed, p.speed.range);
		}
		void mode_speed_ov_voltage_start(void) {
			DEV_CONFIG_S(c);
			DEV_PRESENT_S(p);
			mode_speed_start(c.modes.motion, p.voltage.req, p.voltage.range, psc.satstate());
		}

		void mode_speed_start(
			const typename motion_s::config_s& _motion
			, signal_t& _control
			, const range_s<signal_t>& _control_range
			, const satstates& _master_sut_flag
		) {
			DEV_CONFIG_S(c);
			DEV_PRESENT_S(p);
			p.speed.range = c.range.speed;
			p.position.range = c.range.position;
			motion_.setup(
				&_motion
				, nullptr
				, nullptr
				, &_control_range.lo
				, &_control_range.hi
				, nullptr
				, nullptr
				, nullptr
				, &_control
				, &_master_sut_flag
			);
			motion_.begin();
			psc.on();
		}
		void mode_speed_stop(void) {
			psc.off();
		}

		void mode_speed_runB(void) {
			motion_.run();
		}
		#if BURST_ACTUATOR_MOVE_OV_VOLTAGE_MODE_ENABLED
		private:
		class speed_ov_voltage_mode : public dev::mode {
			friend class actuator_t;
			speed_ov_voltage_mode(actuator_t & _actuator)
			: dev::mode(front::actuator::modes::speed, _actuator) {}
		
		protected:
			virtual void	applay_action(void) { owner<actuator_t>().mode_speed_applay_action(); }
			virtual void	start(void) { owner<actuator_t>().mode_speed_ov_voltage_start(); }
			virtual void	stop(void) { owner<actuator_t>().mode_speed_stop(); }
			virtual void	loopA(void) {  }
			virtual void	loopB(void) { owner<actuator_t>().mode_speed_runB(); }
			virtual void	loopC(void) {}
			virtual void	frontend_loop(void) {}
		} speed_ov_voltage_mode_;
		#endif
		protected:
		void mode_voltage_applay_action(void) {
			DEV_PRESENT_S(p);
			DEV_ACTION_S(a);
			p.voltage.req = range_apply(a.voltage, p.voltage.range);
		}

		void mode_voltage_start(void) {
			DEV_CONFIG_S(c);
			DEV_PRESENT_S(p);
			p.voltage.range = c.range.voltage;
			psc.on();
		}

		void mode_voltage_stop(void) {
			psc.off();
		}
		private:
		class voltage_mode : public dev::mode {
			friend class actuator_t;
			voltage_mode(actuator_t& _actuator)
				: dev::mode(front::actuator::modes::voltage, _actuator) {}
		protected:
			virtual void	applay_action(void) { owner<actuator_t>().mode_voltage_applay_action(); }
			virtual void	start(void) { owner<actuator_t>().mode_voltage_start(); }
			virtual void	stop(void) { owner<actuator_t>().mode_voltage_stop(); }
			virtual void	loopA(void) {}
			virtual void	loopB(void) { }
			virtual void	loopC(void) {}
			virtual void	frontend_loop(void) {}
		} voltage_mode_;
		protected:
		
		void mode_position_applay_action(void) {
			DEV_PRESENT_S(p);
			DEV_ACTION_S(a);
			set_voltage_range();
			set_speed_range();
			p.position.req = range_apply(a.position, p.position.range);
		}

		void mode_position_ov_voltage_start(void) {
			DEV_CONFIG_S(c);
			DEV_PRESENT_S(p);
			mode_position_start(c.modes.motion, c.modes.positioner, p.voltage.req, p.voltage.range, psc.satstate());
		}

		void mode_position_start(
			const typename motion_s::config_s & _motion
			, const typename positioner_s::config_s& _positioner
			, signal_t & _control
			, const range_s<signal_t> & _control_range
			, const satstates & _master_sut_flag
		) {
			DEV_CONFIG_S(c);
			DEV_PRESENT_S(p);
			DEV_ACTION_S(a);
			p.position.range = c.range.position;
			motion_.setup(
				&_motion
				, nullptr
				, nullptr
				, &_control_range.lo
				, &_control_range.hi
				, nullptr
				, nullptr
				, nullptr
				, &_control
				, &_master_sut_flag
			);
			positioner_.setup(
				&_positioner
				,& motion_.signal
				, nullptr
				, &p.speed.range.lo
				, &p.speed.range.hi
				, &p.speed.req
			);
			motion_.begin();
			//positioner.reset();
			psc.on();
		}

		void mode_position_stop(void) {
			psc.off();
		}

		void mode_position_runB(void) {
			motion_.run();
			positioner_.run();
		}
		#if BURST_ACTUATOR_MOVE_OV_VOLTAGE_MODE_ENABLED
		private:
		class position_ov_voltage_mode : public dev::mode {
			friend class actuator_t;
			position_ov_voltage_mode(actuator_t& _actuator)
				: dev::mode(front::actuator::modes::position, _actuator) {}
		protected:
			virtual void	applay_action(void) { owner<actuator_t>().mode_position_applay_action(); }
			virtual void	start(void) { owner<actuator_t>().mode_position_ov_voltage_start(); }
			virtual void	stop(void) { owner<actuator_t>().mode_position_stop(); }
			virtual void	loopA(void) {}
			virtual void	loopB(void) { owner<actuator_t>().mode_position_runB(); }
			virtual void	loopC(void) {}
			virtual void	frontend_loop(void) {}
		} position_ov_voltage_mode_;		
		#endif
		public:
		actuator_t(
			int _dev_id
			, const config_s & _config
			, present_s& _present
			, action_s & _action
			, feedback_s & _feedback
			, ps::control & _ps
			, const signal_t& _speed
			, const long_signal_t& _position
			#if BURST_PANICS_ACTUATOR_TEMPER_ENABLED == 1 && BURST_PROTECTION_ENABLED == 1
			, const signal_t & _temper
			#endif
		) : dev(_dev_id, _config.dev, _present.dev, _action.dev, _feedback.dev)
			, psc(_ps)
			#if BURST_PANICS_ACTUATOR_TEMPER_ENABLED == 1 && BURST_PROTECTION_ENABLED == 1
			, temper(_temper)
			#endif
			, motion_(_present.motion, _present.speed.req, _speed)
			, positioner_(_present.positioner, _present.position.req, _position)
				#if BURST_ACTUATOR_MOVE_OV_VOLTAGE_MODE_ENABLED
			, speed_ov_voltage_mode_(*this)
				#endif
			, voltage_mode_(*this) 
				#if BURST_ACTUATOR_MOVE_OV_VOLTAGE_MODE_ENABLED
			, position_ov_voltage_mode_(*this) 
				#endif
		{
			using namespace front::actuator::panics;
			noreset_panic_mask |= (masks::overtemp | masks::lotemp) ;
		}
		#if ROBO_APP_BURST_VARTREE_ENABLED == 1
		virtual void regvar_action(robo::cstr _name) {
			using namespace burst::var;
			DEV_ACTION_S(a);
			push(_name);{
				dev::regvar_action(RT("dev"));
				if (actual_mode >= burst::var::mode::action) {
					reg(number::var::signal, a.voltage, RT("v"));
					reg(number::var::signal, a.speed, RT("sp"));
					reg(number::var::long_signal, a.position, RT("po"));				
				}
			} pop();
		}
		virtual void regvar_present(robo::cstr _name) {
			DEV_PRESENT_S(p);
			DEV_ACTION_S(a);
			using namespace burst::var;
			push(_name);
			{
				dev::regvar_present(RT("dev"));
				if (actual_mode >= var::mode::full) {
					push(RT("v"));
					reg(number::var::const_signal, p.voltage.des, RT("des"));
					reg(number::var::signal, p.voltage.req, RT("req"));
					varreg(RT("range"), number::var::const_signal, p.voltage.range);
					pop();
					push(RT("sp"));
					reg(number::var::signal, p.speed.req, RT("req"));
					varreg(RT("range"), number::var::const_signal, p.speed.range);
					pop();
					push(RT("po"));
					reg(number::var::long_signal, p.position.req, RT("req"));
					varreg(RT("range"), number::var::const_long_signal, p.position.range);
					pop();
					motion_s::regvar_present(RT("motion"), p.motion);
					positioner_s::regvar_present(RT("positioner"), p.positioner);
				}
			} pop();
		}

		virtual void regvar_conf(robo::cstr _name) {
			DEV_CONFIG_S(c);
			using namespace burst::var;
			push(_name); {
				dev::regvar_conf(RT("dev"));
				if (actual_mode >= var::mode::tuning) {
					push(RT("enco_fault_ticks"));
					reg(types::time_us, c.enco_fault_ticks.reset, RT("reset"));
					reg(types::time_us, c.enco_fault_ticks.set, RT("set"));
					pop();

					push(RT("range"));
					varreg(RT("v"), number::var::signal, c.range.voltage);
					varreg(RT("sp"), number::var::signal, c.range.speed);
					varreg(RT("po"), number::var::long_signal, c.range.position);
					pop();
					#if BURST_ACTUATOR_MOVE_OV_VOLTAGE_MODE_ENABLED
					push(RT("modes"));
					{
						push(RT("v"));
						motion_t::regvar_config(RT("mo"), c.modes.motion);
						positioner_t::regvar_config(RT("po"), c.modes.positioner);
						pop();
					} pop();
					#endif
					#if BURST_PROTECTION_ENABLED == 1
					#if BURST_PANICS_ACTUATOR_TEMPER_ENABLED == 1
					push(RT("panic"));
					reg(number::var::signal, c.panic.temper, RT("temper"));
					pop();
					#endif
					#endif

				}
			} pop();
		}
		#endif

		#if BURST_PROTECTION_ENABLED == 1
		unsigned int get_hall_state();
		virtual void realtime_protection(void) {
			dev::realtime_protection();
		}
		virtual void frontend_protection(void) {
			dev::frontend_protection();
			using namespace front::actuator::panics;
			
			#if BURST_PANICS_ACTUATOR_HALL_ENABLED == 1
			unsigned int hall_state = get_hall_state();
			if(hall_state == 0 || hall_state == 7)
			{
				raise_panic(bits::hall_failure);
			}
			else if (hall_state >= 1 || hall_state <=6)
			{
				reset_panics(bits::hall_failure);
			}
			#endif

			#if BURST_PANICS_ACTUATOR_TEMPER_ENABLED == 1
			DEV_CONFIG_S(c);
			DEV_PRESENT_S(p);
			if (temper >= c.panic.temper.overhi) {
				raise_panic(bits::overtemp);
			}
			else {
				if (temper <= c.panic.temper.hi) {
					if ( (p.dev.panic & masks::overtemp) == masks::overtemp) {
						reset_panic(bits::overtemp);
					}
				}
			}
			if (temper <= c.panic.temper.ultralo) {
				raise_panic(bits::lotemp);
			}
			else {
				if (temper >= c.panic.temper.lo) {
					if ( (p.dev.panic & masks::lotemp) == masks::lotemp) {
						reset_panic(bits::lotemp);
					}
				}
			}
			#endif
		}
		#endif 

	};
}
#endif