#ifndef burst_actuator_hpp
#define burst_actuator_hpp

#include "burst++/modules/actuator.hpp"
#include "burst++/modules/acw.hpp"
#include "burst++/modules/pi.hpp"
namespace burst{
	template <class number> class acw_t: public actuator_t<number>{
		using B = actuator_t<number>;
		public:
		using parameter_t = typename number::parameter_t;
		using signal_t = typename number::signal_t;
		using long_signal_t = typename number::long_signal_t;
		typedef front::acw::action_s<number>  action_s;
		typedef front::acw::feedback_s<number>  feedback_s;

		struct config_s{
			B::config_s actuator;
			struct {
				pi_t<number>::config_s pi;
				range_s<signal_t> range;
			} current;
			struct {
				struct {
					typename motion_t<number>::config_s motion;
					typename positioner_t<number>::config_s positioner;
				} voltage_cl;
				struct {
					typename motion_t<number>::config_s motion;
					typename positioner_t<number>::config_s positioner;
				} current;
			} modes;
			#if BURST_PROTECTION_ENABLED == 1
			#if BURST_PANICS_ACWC_OVERCURRENT_ENABLED ==1
			struct {
				signal_t  overcurrent_pp;
				signal_t  overpower_pp;
				signal_t  normpower_pp;
				time_us_t  overpower_tm_us;
			} panic;
			#endif
			#endif
		};
	
		#if BURST_PANICS_ACWC_OVERCURRENT_ENABLED ==1 && 	BURST_PROTECTION_ENABLED == 1

		#define BURST_PANICS_ACWC_OVERCURRENT_CO(a)\
		,{\
			a##_PANICS_ACWC_OWERCURRENT_PP\
			, a##_PANICS_ACWC_OWERPOWER_PP\
			, a##_PANICS_ACWC_NORMPOWER_PP\
			, a##_PANICS_ACWC_OWERPOWER_TM_US\
		}
		#else
		#define BURST_PANICS_ACWC_OVERCURRENT_CO(a)
		#endif

		#define ACWC_CONFIG(a,b) ACWC_CONFIG_(a,b)
		#define ACWC_CONFIG_(a,b)\
		{\
			ACTUATOR_CONFIG(a)\
			,{\
				PI_CONFIG(b##_CURRENT_PI)\
				, RANGE_CONFIG(b##_CURRENT_RANGE)\
			}\
			,{\
				{\
					MOTION_CONFIG(a##_MOTION_OV_VOLTAGE_CL)\
					,POSITIONER_CONFIG(a##_POSITIONER_OV_VOLTAGE_CL)\
				}\
				,{\
					MOTION_CONFIG(a##_MOTION_OV_CURRENT)\
					,POSITIONER_CONFIG(a##_POSITIONER_OV_CURRENT)\
				}\
			}\
			BURST_PANICS_ACWC_OVERCURRENT_CO(a)\
		}
		
		pi_t<number> dir;
		limiter_t<number> limiter;

		struct present_s {		
			dev::present_s ref;
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
			typename motion_t<number>::present_s motion;
			typename positioner_t<number>::present_s positioner;		
		};
		
		
		#if BURST_PANICS_ACTUATOR_TEMPER_ENABLED == 1 && BURST_PROTECTION_ENABLED == 1
		virtual signal_t temper_pp() {
			return temper;
		};
		#endif
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
		void mode_speed_start(void) {
			DEV_CONFIG_S(c);
			DEV_PRESENT_S(p);
			p.speed.range = c.range.speed;
			p.position.range = c.range.position;
			motion.setup(
				&c.modes.motion
				, nullptr
				, nullptr
				, &p.voltage.range.lo
				, &p.voltage.range.hi
				, nullptr
				, nullptr
				, nullptr
				, &p.voltage.req
				, &psc.satstate()
			);
			motion.reset();
			psc.on();
		}

		void mode_speed_stop(void) {
			psc.off();
		}

		void mode_speed_runB(void) {
			motion.run();
		}
		private:
		class speed_ov_voltage_mode : public dev::mode {
			friend class actuator_t;
			speed_ov_voltage_mode(actuator_t & _actuator)
			: dev::mode(front::actuator::modes::speed, _actuator) {}
		protected:
			virtual void	applay_action(void) { owner<actuator_t>().mode_speed_applay_action(); }
			virtual void	start(void) { owner<actuator_t>().mode_speed_start(); }
			virtual void	stop(void) { owner<actuator_t>().mode_speed_stop(); }
			virtual void	loopA(void) {  }
			virtual void	loopB(void) { owner<actuator_t>().mode_speed_runB(); }
			virtual void	loopC(void) {}
			virtual void	frontend_loop(void) {}
		} speed_ov_voltage_mode_;
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
		void mode_position_start(void) {
			DEV_CONFIG_S(c);
			DEV_PRESENT_S(p);
			DEV_ACTION_S(a);
			p.position.range = c.range.position;
			motion.setup(
				&c.modes.motion
				, nullptr
				, nullptr
				, &p.voltage.range.lo
				, &p.voltage.range.hi
				, nullptr
				, nullptr
				, nullptr
				, &p.voltage.req
				, &psc.satstate()
			);
			positioner.setup(
				&c.modes.positioner
				, nullptr
				, nullptr
				, &p.speed.range.lo
				, &p.speed.range.hi
				, &p.speed.req
			);
			motion.reset();
			//positioner.reset();
			psc.on();
		}

		void mode_position_stop(void) {
			psc.off();
		}

		void mode_position_runB(void) {
			motion.run();
		}
		private:
		class position_ov_voltage_mode : public dev::mode {
			friend class actuator_t;
			position_ov_voltage_mode(actuator_t& _actuator)
				: dev::mode(front::actuator::modes::position, _actuator) {}
		protected:
			virtual void	applay_action(void) { owner<actuator_t>().mode_position_applay_action(); }
			virtual void	start(void) { owner<actuator_t>().mode_position_start(); }
			virtual void	stop(void) { owner<actuator_t>().mode_position_stop(); }
			virtual void	loopA(void) {}
			virtual void	loopB(void) { owner<actuator_t>().mode_position_runB(); }
			virtual void	loopC(void) {}
			virtual void	frontend_loop(void) {}
		} position_ov_voltage_mode_;		
		public:
		actuator_t(
			int _dev_id
			, const config_s & _config
			, present_s& _present
			, action_s & _action
			, feedback_s & _feedback
			, ps::control & _ps
			, signal_t& _speed
			, long_signal_t& _position
			#if BURST_PANICS_ACTUATOR_TEMPER_ENABLED == 1 && BURST_PROTECTION_ENABLED == 1
			, signal_t & _temper
			#endif
		) : dev(_dev_id, _config.ref, _present.ref, _action.ref, _feedback.ref)
			, psc(_ps)
			#if BURST_PANICS_ACTUATOR_TEMPER_ENABLED == 1 && BURST_PROTECTION_ENABLED == 1
			, temper(_temper)
			#endif
			, motion(_present.motion, _present.speed.req, _speed)
			, positioner(_present.positioner, _present.position.req, _position)
			, speed_ov_voltage_mode_(*this)
			, voltage_mode_(*this) 
			, position_ov_voltage_mode_(*this) 
		{

		}
		#if ROBO_APP_BURST_VARTREE_ENABLED
		virtual void do_regvar_present(void) {
			DEV_PRESENT_S(p);
			dev::do_regvar_present();
			using namespace burst::var;
			if (actual_mode >= var::mode::full) {
				push(RT("voltage"));
				reg(number::var::const_signal, p.voltage.des, RT("des"));
				reg(number::var::const_signal, p.voltage.req, RT("req"));
				varreg(RT("range"), number::var::const_signal, p.voltage.range);
				pop();
				push(RT("speed"));
				reg(number::var::const_signal, p.speed.req, RT("req"));
				varreg(RT("range"), number::var::const_signal, p.speed.range);
				pop();
				push(RT("position"));
				reg(number::var::const_long_signal, p.position.req, RT("req"));
				varreg(RT("range"), number::var::const_long_signal, p.position.range);
				pop();

				motion_t<number>::regvar_present("motion", p.motion);
				positioner_t<number>::regvar_present("positioner", p.positioner);

			}
		}

		virtual void do_regvar_conf(void) {
			DEV_CONFIG_S(c);
			dev::do_regvar_conf();
			using namespace burst::var;
			if (actual_mode >= var::mode::tuning) {
				push(RT("enco_fault_ticks"));
				reg(types::time_us, c.enco_fault_ticks.reset, RT("reset"));
				reg(types::time_us, c.enco_fault_ticks.set, RT("set"));
				pop();

				push(RT("range"));
				varreg(RT("voltage"), number::var::signal, c.range.voltage);
				varreg(RT("speed"), number::var::signal, c.range.speed);
				varreg(RT("position"), number::var::long_signal, c.range.position);
				pop();

				push(RT("modes"));
				motion_t<number>::regvar_config("mo_ov_v", c.modes.motion);
				positioner_t<number>::regvar_config("po_ov_v", c.modes.positioner);
				pop();

				#if BURST_PROTECTION_ENABLED == 1
				#if BURST_PANICS_ACTUATOR_TEMPER_ENABLED == 1
				push(RT("panic"));
				varreg(RT("temper_pp"), number::var::signal, c.panic.temper_pp);
				pop();
				#endif
				#endif

				pop();
			}
		}
		#endif

		#if BURST_PROTECTION_ENABLED == 1
		virtual void realtime_protection(void) {
			dev::realtime_protection();
		}
		virtual void frontend_protection(void) {
			dev::frontend_protection();
			using namespace front::actuator::panics;
			#if BURST_PANICS_ACTUATOR_TEMPER_ENABLED == 1
			DEV_CONFIG_S(c);
			DEV_PRESENT_S(p);
			signal_t temper = temper_pp();
			if (temper >= c.panic.temper_pp.overhi) {
				raise_panic(bits::overtemp);
			}
			else {
				if (temper <= c.panic.temper_pp.hi) {
					if ( (p.ref.panic & masks::overtemp) == masks::overtemp) {
						reset_panic(bits::overtemp);
					}
				}
			}
			if (temper <= c.panic.temper_pp.ultralo) {
				raise_panic(bits::lotemp);
			}
			else {
				if (temper >= c.panic.temper_pp.lo) {
					if ( (p.ref.panic & masks::lotemp) == masks::lotemp) {
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