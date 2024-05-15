#ifndef burst_modules_pmsm_hpp
#define burst_modules_pmsm_hpp
#include "burst++/modules/pmsm.front.hpp"
#include "burst++/modules/acw.hpp"
#include "burst++/modules/power3ph.hpp"

namespace burst {
	template <class number> class pmsm_t : public acw_t<number> {
		using B = acw_t<number>;
	public:
		using parameter_t = typename number::parameter_t;
		using signal_t = typename number::signal_t;
		using long_signal_t = typename number::long_signal_t;
		typedef front::pmsm::action_s<number>  action_s;
		typedef front::pmsm::feedback_s<number>  feedback_s;

		struct config_s {
			typename B::config_s cross;
			struct {
				struct {
					typename pi_t<number>::config_s pi;
					range_s<signal_t> range;
				} current;
				struct {
					range_s<signal_t> range;
				} voltage;
			} lateral;
			
			#if BURST_PROTECTION_ENABLED == 1
			struct {
				#if BURST_PANICS_PMSM_MISSALIGMENT_ENABLED == 1 &&  BURST_PROTECTION_ENABLED == 1
				signal_t  current_misalignment_lim;
				#endif
			} panic;
			#endif
		};
		#if BURST_PANICS_PMSM_MISSALIGMENT_ENABLED == 1 &&  BURST_PROTECTION_ENABLED == 1
		#define PANICS_PMSM_CURRENT_MISSALIGMENT_CO(a)\
			a##_PANICS_PMSM_CURRENT_MISSALIGMENT_PP
		#else
		#define PANICS_PMSM_CURRENT_MISSALIGMENT_CO(a)
		#endif

		#define PMSM_CONFIG(a) PMSM_CONFIG_(a)
		#define PMSM_CONFIG_(a)\
		{\
			ACWC_CONFIG(a,a##_CROSS)\
			,{\
				{\
					PI_CONFIG(a##_LATERAL_CURRENT_PI)\
					, RANGE_CONFIG(a##_LATERAL_CURRENT_RANGE)\
				}\
				,{\
					RANGE_CONFIG(a##_LATERAL_VOLTAGE_RANGE)\
				}\
			}\
			,{\
				PANICS_PMSM_CURRENT_MISSALIGMENT_CO(a)\
			}\
		}

		struct present_s {
			typename B::present_s cross;
			struct {
				struct {
					signal_t req;
					range_s<signal_t> range;
					typename pi_t<number>::present_s pi;
				} current;
				struct {
					signal_t req;
					range_s<signal_t> range;
				} voltage;
			}lateral;
			struct {
				long_signal_t freq;
				long_signal_t angle32;
				signal_t angle;
			} synchro;
		};
	private:
		const signal_t& current_lateral_;
		signal_t  current_mag_ = 0;

	public:
		pi_t<number> lpi;
		typename powe3ph<number>::rotator_t& rotator;
		typename powe3ph<number>::inverter& inverter;
		pmsm_t(
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
		) : B(
			_dev_id
			, _config.cross
			, _present.cross
			, _action.cross
			, _feedback.cross
			, _ps
			, _current_cross
			, current_mag_
			, _speed
			, _position
			#if BURST_PANICS_ACTUATOR_TEMPER_ENABLED == 1 && BURST_PROTECTION_ENABLED == 1
			, _temper
			#endif
		)
			, current_lateral_(_current_lateral)
			
			, lpi(
				_config.lateral.current.pi //const config_s& _config
				, _present.lateral.current.pi// present_s& _present
				, _present.lateral.current.req
				, _current_lateral
				, nullptr
				, nullptr
				, _present.lateral.current.range.lo
				, _present.lateral.current.range.hi
				, _present.lateral.voltage.req
				, _ps.satstate()
			)

			, rotator(_rotator)
			, inverter(_inverter)
			, synchro_voltage_mode_(*this)
			, synchro_current_mode_(*this) {}

		protected:
		void mode_synchro_voltage_applay_action(void) {
			DEV_PRESENT_S(p);
			DEV_ACTION_S(a);

			p.cross.ac.voltage.req = range_apply(a.cross.ac.voltage, p.cross.ac.voltage.range);
			p.lateral.voltage.req = range_apply(a.lateral.voltage, p.lateral.voltage.range);
			p.synchro.freq = a.synchro.freq;
			p.synchro.angle = a.synchro.angle;
		}
		void mode_synchro_voltage_start(void) {
			DEV_PRESENT_S(p);
			DEV_CONFIG_S(cfg);
			p.cross.ac.voltage.range = cfg.cross.ac.range.voltage;
			p.lateral.voltage.range = cfg.lateral.voltage.range;
			rotator.switch_to_synchro();
			psc.on();
		}
		void mode_synchro_voltage_stop(void) {
			rotator.switch_to_enco();
			psc.off();
		}
		void mode_synchro_voltage_runA(void) {
			p.synchro.angle32 += p.synchro.freq;
			p.synchro.angle = (signal_t)fast::rsh(p.synchro.angle32, 16);
		}
	private:
		class synchro_voltage_mode : public dev::mode {
			friend class pmsm_t;
			synchro_voltage_mode(actuator_t& _actuator)
				: dev::mode(front::actuator::modes::speed, _actuator) {}
		protected:
			virtual void	applay_action(void) { owner<pmsm_t>().mode_synchro_voltage_applay_action(); }
			virtual void	start(void) { owner<pmsm_t>().mode_synchro_voltage_start(); }
			virtual void	stop(void) { owner<pmsm_t>().mode_synchro_voltage_stop(); }
			virtual void	loopA(void) {}
			virtual void	loopB(void) {}
			virtual void	loopC(void) {}
			virtual void	frontend_loop(void) {}
		} synchro_voltage_mode_;
	protected:
		void mode_synchro_current_applay_action(void) {
			DEV_PRESENT_S(p);
			DEV_ACTION_S(a);
			DEV_CONFIG_S(c);
			range_set(p.cross.ac.voltage.range, a.cross.ac.voltage, c.cross.ac.range.voltage);
			range_set(p.lateral.voltage.range, a.lateral.voltage, c.lateral.voltage.range);

			p.cross.current.req = range_apply(a.cross.current, p.cross.current.range);
			p.lateral.current.req = range_apply(a.lateral.current, p.lateral.current.range);
			p.synchro.freq = a.synchro.freq;
			p.synchro.angle32 = a.synchro.angle;
			p.synchro.angle = (signal_t)fast::rsh(p.synchro.angle32, 16);
		}
		void mode_synchro_current_start(void) {
			DEV_PRESENT_S(p);
			DEV_CONFIG_S(c);

			p.cross.current.range = c.cross.current.range;

			p.lateral.current.range = c.lateral.current.range;

			lpi.begin();
			B::cpi.begin();

			rotator.switch_to_synchro();
			B::psc.on();
		}
		void mode_synchro_current_stop(void) {
			rotator.switch_to_enco();
			B::psc.off();
		}
		void mode_synchro_current_runA(void) {
			lpi.run();
			B::cpi.run();
			p.synchro.angle32 += p.synchro.freq;
			p.synchro.angle = (signal_t)fast::rsh(p.synchro.angle32, 16);
		}
		private:
			class synchro_current_mode : public dev::mode {
				friend class pmsm_t;
				synchro_current_mode(actuator_t& _actuator)
					: dev::mode(front::actuator::modes::speed, _actuator) {}
			protected:
				virtual void	applay_action(void) { owner<pmsm_t>().mode_synchro_current_applay_action(); }
				virtual void	start(void) { owner<pmsm_t>().mode_synchro_current_start(); }
				virtual void	stop(void) { owner<pmsm_t>().mode_synchro_current_stop(); }
				virtual void	loopA(void) {}
				virtual void	loopB(void) { }
				virtual void	loopC(void) {}
				virtual void	frontend_loop(void) {}
			} synchro_current_mode_;
		public:
		virtual void switch_to_mode(int _mode) {
			DEV_PRESENT_S(p);
			dev::switch_to_mode(_mode);
			if (p.cross.ac.dev.mode > front::dev::modes::idle && p.cross.ac.dev.mode <= front::acw::modes::last) {
				DEV_CONFIG_S(cfg);
				//резетим контур продольного тока
				p.lateral.current.req = 0;
				p.lateral.current.range = cfg.lateral.current.range;
				p.lateral.voltage.range = cfg.lateral.voltage.range;
				lpi.begin();
			}
		}
		virtual void loopA(void)
		{
			DEV_PRESENT_S(p);
			dev::loopA();
			if (p.cross.ac.dev.mode > front::dev::modes::idle && p.cross.ac.dev.mode <= front::acw::modes::last) {
				lpi.run();
			}
		}

	#if 0
		

		#if ROBO_APP_BURST_VARTREE_ENABLED
		virtual void do_regvar_present(void) {
			DEV_PRESENT_S(p);
			B::do_regvar_present();
			using namespace burst::var;
			if (actual_mode >= var::mode::full) {
				push(RT("c"));
				{
					pi_t<number>::regvar_present(RT("pi"), p.current.pi);
					limiter_t<number>::regvar_present(RT("lim"), p.current.limiter);
					reg(number::var::const_signal, p.current.req, RT("req"));
					push(RT("mag"));
					{
						reg(number::var::const_signal, p.current.magnitude.actual, RT("actual"));
						reg(number::var::const_signal, p.current.magnitude.delta, RT("delta"));
					} pop();
					varreg(RT("range"), number::var::const_signal, p.current.range);
				} pop();
			}
		}

		virtual void do_regvar_conf(void) {
			DEV_CONFIG_S(c);
			B::do_regvar_conf();
			using namespace burst::var;
			if (actual_mode >= var::mode::tuning) {

				push(RT("c"));
				pi_t<number>::regvar_config(RT("pi"), c.current.pi);
				limiter_t<number>::regvar_config(RT("lim"), c.current.limiter);
				varreg(RT("range"), number::var::signal, c.current.range);
				pop();

				push(RT("modes"));
				{
					push(RT("cl"));
					motion_t::regvar_config(RT("mo"), c.modes.voltage_cl.motion);
					positioner_t::regvar_config(RT("po"), c.modes.voltage_cl.positioner);
					pop();

					push(RT("c"));
					motion_t::regvar_config(RT("mo"), c.modes.current.motion);
					positioner_t::regvar_config(RT("po"), c.modes.current.positioner);
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
						reg(types::time_us, c.panic.normpower, RT("tm"));
					} pop();
				} pop();
				#endif
				#endif
			}
		}
		#endif
		#if BURST_PROTECTION_ENABLED == 1
		virtual void realtime_protection(void) {
			B::realtime_protection();

			#if BURST_PANICS_ACWC_OVERCURRENT_ENABLED ==1

			DEV_CONFIG_S(cfg);
			DEV_PRESENT_S(p);

			signal_t magnitude = current_mag_;
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
		#endif
	};
}

#endif