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
			
			struct {
				#if BURST_PROTECTION_ENABLED == 1
				#if BURST_PANICS_PMSM_MISSALIGMENT_ENABLED == 1 &&  BURST_PROTECTION_ENABLED == 1
				signal_t  current_misalignment_lim;
				#endif
				#endif
			} panic;
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
					, BURST_RANGE_CONFIG(a##_LATERAL_CURRENT_RANGE)\
				}\
				,{\
					BURST_RANGE_CONFIG(a##_LATERAL_VOLTAGE_RANGE)\
				}\
			}\
			,{\
				PANICS_PMSM_CURRENT_MISSALIGMENT_CO(a)\
			},\
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
	public:
		const signal_t& current_lateral;
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
			#if BURST_PROTECTION_ENABLED == 1
			#if BURST_PANICS_ACWC_OVERCURRENT_ENABLED ==1
			, const signal_t& _current_mag
			#endif
			#endif
		) : B(
			_dev_id
			, _config.cross
			, _present.cross
			, _action.cross
			, _feedback.cross
			, _ps
			, _current_cross
			#if BURST_PROTECTION_ENABLED == 1
			#if BURST_PANICS_ACWC_OVERCURRENT_ENABLED ==1
			, _current_mag			
			#endif
			#endif
			, _speed
			, _position
			#if BURST_PANICS_ACTUATOR_TEMPER_ENABLED == 1 && BURST_PROTECTION_ENABLED == 1
			, _temper
			#endif
		)
			, current_lateral(_current_lateral)
			
			, lpi(
				_config.lateral.current.pi //const config_s& _config
				, _present.lateral.current.pi// present_s& _present
				, _present.lateral.current.req
				, _current_lateral
				, nullptr
				, nullptr
				, _present.lateral.voltage.range.lo
				, _present.lateral.voltage.range.hi
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
			p.synchro.angle32 = a.synchro.angle;
		}
		void mode_synchro_voltage_start(void) {
			DEV_PRESENT_S(p);
			DEV_CONFIG_S(cfg);
			p.cross.ac.voltage.range = cfg.cross.ac.range.voltage;
			p.lateral.voltage.range = cfg.lateral.voltage.range;
			rotator.switch_to_synchro();
			B::psc.on();
		}
		void mode_synchro_voltage_stop(void) {
			rotator.switch_to_enco();
			B::psc.off();
		}
		void mode_synchro_voltage_runA(void) {
			DEV_PRESENT_S(p);
			p.synchro.angle32 += p.synchro.freq;
			p.synchro.angle = (signal_t)robo::digit::rsh(p.synchro.angle32, 16);
		}
	private:
		class synchro_voltage_mode : public dev::mode {
			friend class pmsm_t;
			synchro_voltage_mode(pmsm_t& _actuator)
				: dev::mode(front::pmsm::modes::synchro_voltage, _actuator) {}
		protected:
			virtual void	applay_action(void) { owner<pmsm_t>().mode_synchro_voltage_applay_action(); }
			virtual void	start(void) { owner<pmsm_t>().mode_synchro_voltage_start(); }
			virtual void	stop(void) { owner<pmsm_t>().mode_synchro_voltage_stop(); }
			virtual void	loopA(void) { owner<pmsm_t>().mode_synchro_voltage_runA(); }
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
			p.synchro.angle = (signal_t)robo::digit::rsh(p.synchro.angle32, 16);
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
			DEV_PRESENT_S(p);
			lpi.run();
			B::cpi.run();
			p.synchro.angle32 += p.synchro.freq;
			p.synchro.angle = (signal_t)robo::digit::rsh(p.synchro.angle32, 16);
		}
		private:
			class synchro_current_mode : public dev::mode {
				friend class pmsm_t;
				synchro_current_mode(pmsm_t& _actuator)
					: dev::mode(front::pmsm::modes::synchro_current, _actuator) {}
			protected:
				virtual void	applay_action(void) { owner<pmsm_t>().mode_synchro_current_applay_action(); }
				virtual void	start(void) { owner<pmsm_t>().mode_synchro_current_start(); }
				virtual void	stop(void) { owner<pmsm_t>().mode_synchro_current_stop(); }
				virtual void	loopA(void) { owner<pmsm_t>().mode_synchro_current_runA(); }
				virtual void	loopB(void) { }
				virtual void	loopC(void) {}
				virtual void	frontend_loop(void) {}
			} synchro_current_mode_;
		public:
		virtual void switch_to_mode(int _mode) {
			DEV_PRESENT_S(p);
			dev::switch_to_mode(_mode);
			if (p.cross.ac.dev.mode > front::actuator::modes::voltage && p.cross.ac.dev.mode <= front::acw::modes::last) {
				DEV_CONFIG_S(cfg);
				//������� ������ ����������� ����
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
			if (p.cross.ac.dev.mode > front::actuator::modes::voltage && p.cross.ac.dev.mode <= front::acw::modes::last) {
				lpi.run();
			}
		}
		#if ROBO_APP_BURST_VARTREE_ENABLED == 1
		virtual void regvar_action(robo::cstr _name) {
			using namespace burst::var;
			DEV_ACTION_S(a);
			push(_name);
			{
				B::regvar_action(RT("cross"));
				if (actual_mode >= burst::var::mode::action) {
					push(RT("lat"));{
						reg(number::var::signal, a.lateral.current, RT("c"));
						reg(number::var::signal, a.lateral.voltage, RT("v"));
					} pop();
					push(RT("synchro"));
					{
						reg(number::var::long_signal, a.synchro.freq, RT("freq"));
						reg(number::var::long_signal, a.synchro.angle, RT("angle"));
					} pop();
				}
			} pop();
		}

		virtual void regvar_present(robo::cstr _name) {
			DEV_PRESENT_S(p);
			using namespace burst::var;
			push(_name);{				
				B::regvar_present(RT("cross"));
				if (actual_mode >= var::mode::full) {
					push(RT("lat"));{
						push(RT("c"));{
							reg(number::var::signal, p.lateral.current.req, RT("req"));
							pi_t<number>::regvar_present(RT("pi"), p.lateral.current.pi);
							varreg(RT("range"), number::var::const_signal, p.lateral.current.range);
						}pop();
						push(RT("v"));{
							reg(number::var::signal, p.lateral.voltage.req, RT("req"));
							varreg(RT("range"), number::var::const_signal, p.lateral.voltage.range);
						}pop();
					}pop();
					push(RT("synchro"));{
						reg(number::var::long_signal, p.synchro.freq, RT("freq"));
						reg(number::var::long_signal, p.synchro.angle32, RT("angle32"));
						reg(number::var::const_signal, p.synchro.angle, RT("angle"));
					} pop();
				};
			} pop();
		}

		virtual void regvar_conf(robo::cstr _name) {
			DEV_CONFIG_S(c);
			using namespace burst::var;
			push(_name);{
				B::regvar_conf(RT("cross"));			
				if (actual_mode >= var::mode::tuning) {
					push(RT("lat"));
					{
						push(RT("c"));
						{
							pi_t<number>::regvar_config(RT("pi"), c.lateral.current.pi);
							varreg(RT("range"), number::var::signal, c.lateral.current.range);
						} pop();
						push(RT("v"));
						{
							varreg(RT("range"), number::var::signal, c.lateral.current.range);
						} pop();
					} pop();

					#if BURST_PROTECTION_ENABLED == 1
					push(RT("panic"));{				
						#if BURST_PANICS_PMSM_MISSALIGMENT_ENABLED == 1 &&  BURST_PROTECTION_ENABLED == 1
						reg(number::var::signal, c.panic.overpower, RT("misalignment_lim"));
						#endif
					} pop();
					#endif
				}
			}pop();
		} 
		#endif
	};
}

#endif