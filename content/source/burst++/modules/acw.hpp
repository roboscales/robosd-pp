#ifndef burst_modules_acw_hpp
#define burst_modules_acw_hpp

#include "burst++/modules/actuator.hpp"
#include "burst++/modules/acw.front.hpp"
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
			typename B::config_s ac;
			struct {
				typename pi_t<number>::config_s pi;
				typename limiter_t<number>::config_s limiter;
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
				signal_t  overcurrent;
				signal_t  overpower;
				signal_t  normpower;
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
				, LIMMITER_CONFIG(b##_LIMMITER)\
				, BURST_RANGE_CONFIG(b##_CURRENT_RANGE)\
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

		pi_t<number> cpi;
	private:
		limiter_t<number> limiter_;
		#if BURST_PROTECTION_ENABLED == 1
		#if BURST_PANICS_ACWC_OVERCURRENT_ENABLED ==1
		#endif
		#endif

	public:
		const signal_t& current;
		#if BURST_PROTECTION_ENABLED == 1
		#if BURST_PANICS_ACWC_OVERCURRENT_ENABLED ==1
		const signal_t& current_mag;
		#endif
		#endif
		struct present_s {
			typename B::present_s ac;
			struct {
				typename pi_t<number>::present_s pi;
				typename limiter_t<number>::present_s limiter;
				signal_t req;
				range_s<signal_t> range;
				struct {
					signal_t actual;
					signal_t delta;
					time_us_t us;
				} magnitude;
			} current;
		};
		
		
		void set_current_range(void) {
			DEV_CONFIG_S(c);
			DEV_PRESENT_S(p);
			DEV_ACTION_S(a);
			range_set(p.current.range, a.current, c.current.range);
		}
		protected:

		void mode_speed_ov_voltage_cl_stop(void) {
			B::mode_speed_stop();
		}

		void mode_speed_ov_voltage_cl_runB(void) {
			B::mode_speed_runB();
		}

		void mode_speed_ov_voltage_cl_applay_action(void) {
			B::mode_speed_applay_action();
			set_current_range();
		}
		void mode_speed_ov_voltage_cl_start(void) {
			DEV_CONFIG_S(c);
			DEV_PRESENT_S(p);
			limiter_.begin();
			B::mode_speed_start(c.modes.voltage_cl.motion, p.ac.voltage.des, p.ac.voltage.range, p.current.limiter.sut_flag);
		}


		private:
		class speed_ov_voltage_cl_mode : public dev::mode {
			friend class acw_t;
			speed_ov_voltage_cl_mode(acw_t& _actuator)
			: dev::mode(front::acw::modes::speed_cl, _actuator) {}
		protected:
			virtual void	applay_action(void) { owner<acw_t>().mode_speed_ov_voltage_cl_applay_action(); }
			virtual void	start(void) { owner<acw_t>().mode_speed_ov_voltage_cl_start(); }
			virtual void	stop(void) { owner<acw_t>().mode_speed_ov_voltage_cl_stop(); }
			virtual void	loopA(void) { owner<acw_t>().limiter_.run();  }
			virtual void	loopB(void) { owner<acw_t>().mode_speed_ov_voltage_cl_runB(); }
			virtual void	loopC(void) {}
			virtual void	frontend_loop(void) {}
		} speed_ov_voltage_cl_mode_;

		protected:

		void mode_voltage_cl_applay_action(void) {
			DEV_PRESENT_S(p);
			DEV_ACTION_S(a);
			p.ac.voltage.des = range_apply(a.ac.voltage, p.ac.voltage.range);
			set_current_range();
		}

		void mode_voltage_cl_start(void) {
			DEV_CONFIG_S(c);
			DEV_PRESENT_S(p);
			p.ac.voltage.range = c.ac.range.voltage;
			B::psc.on();
			limiter_.begin();
		}
		void mode_voltage_cl_stop(void) {
			B::mode_voltage_stop();
		}

		private:
		class voltage_cl_mode : public dev::mode {
			friend class acw_t;
			voltage_cl_mode(acw_t& _actuator)
				: dev::mode(front::acw::modes::voltage_cl, _actuator) {}
		protected:
			virtual void	applay_action(void) { owner<acw_t>().mode_voltage_cl_applay_action(); }
			virtual void	start(void) { owner<acw_t>().mode_voltage_cl_start(); }
			virtual void	stop(void) { owner<acw_t>().mode_voltage_cl_stop(); }
			virtual void	loopA(void) { owner<acw_t>().limiter_.run();  }
			virtual void	loopB(void) { }
			virtual void	loopC(void) {}
			virtual void	frontend_loop(void) {}
		} voltage_cl_mode_;
		protected:
		void mode_position_ov_voltage_cl_stop(void) {
			B::mode_position_stop();
		}

		void mode_position_ov_voltage_cl_runB(void) {
			B::mode_position_runB();
		}
		void mode_position_ov_voltage_cl_applay_action(void) {
			B::mode_position_applay_action();
			set_current_range();
		}
		void mode_position_ov_voltage_cl_start(void) {
			DEV_CONFIG_S(c);
			DEV_PRESENT_S(p);
			limiter_.begin();
			B::mode_position_start(c.modes.voltage_cl.motion, c.modes.voltage_cl.positioner,p.ac.voltage.des, p.ac.voltage.range, p.current.limiter.sut_flag);
		}

		private:
		class position_ov_voltage_cl_mode : public dev::mode {
			friend class acw_t;
			position_ov_voltage_cl_mode(acw_t& _acw)
				: dev::mode(front::acw::modes::position_cl, _acw) {}
		protected:
			virtual void	applay_action(void) { owner<acw_t>().mode_position_ov_voltage_cl_applay_action(); }
			virtual void	start(void) { owner<acw_t>().mode_position_ov_voltage_cl_start(); }
			virtual void	stop(void) { owner<acw_t>().mode_position_ov_voltage_cl_stop(); }
			virtual void	loopA(void) { owner<acw_t>().limiter_.run(); }
			virtual void	loopB(void) { owner<acw_t>().mode_position_ov_voltage_cl_runB(); }
			virtual void	loopC(void) {}
			virtual void	frontend_loop(void) {}
		} position_ov_voltage_cl_mode_;		
		


		//============================================

		void mode_speed_ov_current_stop(void) {
			B::mode_speed_stop();
		}

		void mode_speed_ov_current_runB(void) {
			B::mode_speed_runB();
		}

		void mode_speed_ov_current_applay_action(void) {
			B::mode_speed_applay_action();
			set_current_range();
		}
		void mode_speed_ov_current_start(void) {
			DEV_CONFIG_S(c);
			DEV_PRESENT_S(p);
			cpi.begin();
			B::mode_speed_start(c.modes.current.motion, p.current.req, p.current.range, p.current.pi.satstate);
		}


		private:
			class speed_ov_current_mode : public dev::mode {
				friend class acw_t;
				speed_ov_current_mode(acw_t& _actuator)
					: dev::mode(front::acw::modes::speed, _actuator) {}
			protected:
				virtual void	applay_action(void) { owner<acw_t>().mode_speed_ov_current_applay_action(); }
				virtual void	start(void) { owner<acw_t>().mode_speed_ov_current_start(); }
				virtual void	stop(void) { owner<acw_t>().mode_speed_ov_current_stop(); }
				virtual void	loopA(void) { owner<acw_t>().cpi.run(); }
				virtual void	loopB(void) { owner<acw_t>().mode_speed_ov_current_runB(); }
				virtual void	loopC(void) {}
				virtual void	frontend_loop(void) {}
			} speed_ov_current_mode_;

		protected:

			void mode_current_applay_action(void) {
				DEV_PRESENT_S(p);
				DEV_ACTION_S(a);
				B::set_voltage_range();
				p.current.req = range_apply(a.current, p.current.range);
			}

			void mode_current_start(void) {
				DEV_CONFIG_S(c);
				DEV_PRESENT_S(p);
				p.current.range = c.current.range;
				cpi.begin();
				B::psc.on();
			}
			void mode_current_stop(void) {
				B::psc.off();
			}

		private:
			class current_mode : public dev::mode {
				friend class acw_t;
				current_mode(acw_t& _actuator)
					: dev::mode(front::acw::modes::voltage, _actuator) {}
			protected:
				virtual void	applay_action(void) { owner<acw_t>().mode_current_applay_action(); }
				virtual void	start(void) { owner<acw_t>().mode_current_start(); }
				virtual void	stop(void) { owner<acw_t>().mode_current_stop(); }
				virtual void	loopA(void) { owner<acw_t>().cpi.run(); }
				virtual void	loopB(void) {}
				virtual void	loopC(void) {}
				virtual void	frontend_loop(void) {}
			} current_mode_;
		protected:

			void mode_position_ov_current_stop(void) {
				B::mode_position_stop();
			}

			void mode_position_ov_current_runB(void) {
				B::mode_position_runB();
			}
			void mode_position_ov_current_applay_action(void) {
				B::mode_position_applay_action();
				set_current_range();
			}
			void mode_position_ov_current_start(void) {
				DEV_CONFIG_S(c);
				DEV_PRESENT_S(p);
				cpi.begin();
				B::mode_position_start(c.modes.current.motion, c.modes.current.positioner, p.current.req, p.current.range, p.current.pi.satstate);
			}

		private:
			class position_ov_current_mode : public dev::mode {
				friend class acw_t;
				position_ov_current_mode(acw_t& _acw)
					: dev::mode(front::acw::modes::position, _acw) {}
			protected:
				virtual void	applay_action(void) { owner<acw_t>().mode_position_ov_current_applay_action(); }
				virtual void	start(void) { owner<acw_t>().mode_position_ov_current_start(); }
				virtual void	stop(void) { owner<acw_t>().mode_position_ov_current_stop(); }
				virtual void	loopA(void) { owner<acw_t>().cpi.run(); }
				virtual void	loopB(void) { owner<acw_t>().mode_position_ov_current_runB(); }
				virtual void	loopC(void) {}
				virtual void	frontend_loop(void) {}
			} position_ov_current_mode_;


		public:
		acw_t(
			int _dev_id
			, const config_s & _config
			, present_s& _present
			, action_s & _action
			, feedback_s & _feedback
			, ps::control & _ps
			, const signal_t& _current
			#if BURST_PROTECTION_ENABLED == 1
			#if BURST_PANICS_ACWC_OVERCURRENT_ENABLED ==1
			, const signal_t& _current_mag
			#endif
			#endif
			, const signal_t& _speed
			, const long_signal_t& _position
			#if BURST_PANICS_ACTUATOR_TEMPER_ENABLED == 1 && BURST_PROTECTION_ENABLED == 1
			, const signal_t& _temper
			#endif
			) : B(
				_dev_id
				, _config.ac
				, _present.ac
				, _action.ac
				, _feedback.ac
				, _ps
				, _speed
				, _position
				#if BURST_PANICS_ACTUATOR_TEMPER_ENABLED == 1 && BURST_PROTECTION_ENABLED == 1
				, _temper
				#endif
			)
			, current(_current)
			#if BURST_PROTECTION_ENABLED == 1
			#if BURST_PANICS_ACWC_OVERCURRENT_ENABLED ==1
			, current_mag(_current_mag)
			#endif
			#endif
			, speed_ov_voltage_cl_mode_(*this)
			, voltage_cl_mode_(*this)
			, position_ov_voltage_cl_mode_(*this)
			, speed_ov_current_mode_(*this)
			, current_mode_(*this)
			, position_ov_current_mode_(*this)
			, cpi(
				_config.current.pi //const config_s& _config
				, _present.current.pi// present_s& _present
				, _present.current.req
				, _current
				, nullptr
				, nullptr
				, _present.ac.voltage.range.lo
				, _present.ac.voltage.range.hi
				, _present.ac.voltage.req
				, _ps.satstate()
			)
			, limiter_(
				_config.current.limiter
				, _present.current.limiter
				, _config.current.pi
				, _present.ac.voltage.des
				, _present.ac.voltage.req
				, _current
				, _present.current.range
				, _present.ac.voltage.range
			)
		{
		}

		#if ROBO_APP_BURST_VARTREE_ENABLED == 1
		virtual void regvar_action(robo::cstr _name) {
			using namespace burst::var;
			DEV_ACTION_S(a);
			push(_name);
			{
				B::regvar_action(RT("ac"));
				if (actual_mode >= burst::var::mode::action) {
					reg(number::var::signal, a.current, RT("c"));
				}
			} pop();
		}
		virtual void regvar_present(robo::cstr _name) {
			using namespace burst::var;
			DEV_PRESENT_S(p);
			push(_name);
			{
				B::regvar_present(RT("ac"));
				if (actual_mode >= var::mode::full) {
					push(RT("c"));
					{
						pi_t<number>::regvar_present(RT("pi"), p.current.pi);
						limiter_t<number>::regvar_present(RT("lim"), p.current.limiter);
						reg(number::var::signal, p.current.req, RT("req"));
						varreg(RT("range"), number::var::const_signal, p.current.range);
						push(RT("mag"));
						{
							reg(number::var::const_signal, p.current.magnitude.actual, RT("actual"));
							reg(number::var::const_signal, p.current.magnitude.delta, RT("delta"));
						} pop();
					} pop();
				}
			}pop();
		}

		virtual void regvar_conf(robo::cstr _name) {
			DEV_CONFIG_S(c);
			using namespace burst::var;
			push(_name);{
				B::regvar_conf(RT("ac"));
				if (actual_mode >= var::mode::tuning) {

					push(RT("c"));
					pi_t<number>::regvar_config(RT("pi"), c.current.pi);
					limiter_t<number>::regvar_config(RT("lim"), c.current.limiter);
					varreg(RT("range"), number::var::signal, c.current.range);
					pop();

					push(RT("modes"));
					{
						push(RT("cl"));
						motion_t<number>::regvar_config(RT("mo"), c.modes.voltage_cl.motion);
						positioner_t<number>::regvar_config(RT("po"), c.modes.voltage_cl.positioner);
						pop();

						push(RT("c"));
						motion_t<number>::regvar_config(RT("mo"), c.modes.current.motion);
						positioner_t<number>::regvar_config(RT("po"), c.modes.current.positioner);
						pop();

					} pop();

					#if BURST_PROTECTION_ENABLED == 1
					#if BURST_PANICS_ACWC_OVERCURRENT_ENABLED ==1
					push(RT("panic"));
					{
						reg(number::var::signal, c.panic.overcurrent, RT("overcur"));
						push(RT("power"));
						{
							reg(number::var::signal, c.panic.overpower, RT("over"));
							reg(number::var::signal, c.panic.normpower, RT("norm"));
							reg(types::time_us, c.panic.overpower_tm_us, RT("tm"));
						} pop();
					} pop();
					#endif
					#endif
				}
			}pop();
		}
		#endif
		#if BURST_PROTECTION_ENABLED == 1
		virtual void realtime_protection(void) {
			B::realtime_protection();

			#if BURST_PANICS_ACWC_OVERCURRENT_ENABLED ==1

			DEV_CONFIG_S(cfg);
			DEV_PRESENT_S(p);

			signal_t magnitude = current_mag;
			signal_t delta = magnitude - p.current.magnitude.actual;
			time_us_t now = time_us();
			p.current.magnitude.delta = delta;
			p.current.magnitude.actual = magnitude;

			if (
				magnitude > cfg.panic.overcurrent
				|| (magnitude + delta) > cfg.panic.overcurrent
				) {
				B::raise_panic(front::acw::panics::bits::overcurrent);
			}
			else {
				if (
					magnitude > cfg.panic.overpower
					) {
					if (now - p.current.magnitude.us > cfg.panic.overpower_tm_us) {
						B::raise_panic(front::acw::panics::bits::overcurrent);
						p.current.magnitude.us = 0;
					}
				}
				else {
					if (magnitude < cfg.panic.normpower) {
						p.current.magnitude.us = now;
					}
				}
			}
			#endif
		}
		virtual void frontend_protection(void) {
			B::frontend_protection();
		}
		#endif 
	};
}
#endif