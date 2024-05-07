#ifndef burst_actuator_hpp
#define burst_actuator_hpp
#include "burst++/burst.hpp"
#include "burst++/math.hpp"
#include "burst++/modules/filter.hpp"
#include "burst++/modules/ps.hpp"
#include "burst++/modules/enco.hpp"
#include "burst++/modules/motion.hpp"
#include "burst++/modules/positioner.hpp"

#ifndef BURST_PANICS_ACTUATOR_TEMPER_ENABLED
#define BURST_PANICS_ACTUATOR_TEMPER_ENABLED 0
#endif

namespace burst{
	template <class number> class actuator: public burst::dev{
		public:
		using parametr_t = typename number::parametr_t;
		using signal_t = typename number::signal_t;
		using long_min = typename number::long_min;
		using long_signal_t = typename number::long_signal_t;

		struct config_s{
			burst::dev::config_s ref;
			struct{
				time_us_t reset;
				time_us_t set;
			} enco_fault_ticks;
			struct {
				range_s<signal_t> voltage;
				range_s<signal_t> speed;
				range_s<long_signal_t> position;
			} range;
			struct {
				typename motion_t<number>::config_s motion;
				typename positioner_t<number>::config_s positioner;
			} modes;
			struct {
				#if BURST_PROTECTION_ENABLED == 1
				#if BURST_PANICS_ACTUATOR_TEMPER_ENABLED == 1
				burst_hyst_t temper_pp;		
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

		#define ACTUATOR_CONFIG(a) ACTUATOR_CONFIG_(a)
		#define ACTUATOR_CONFIG_(a)\
		{\
			DEV_CONFIG(a##_REF)\
			,{\
				a##_ENCO_FAULT_TICKS_RESET\
				,a##_ENCO_FAULT_TICKS_SET\
			}\
			,{\
				RANGE_CONFIG(a##_RANGE_VOLTAGE)\
				, RANGE_CONFIG(a##_RANGE_SPEED)\
				, RANGE_CONFIG(a##_RANGE_POSITION)\
			}\
			,{\
					MOTION_CONFIG(a##_MOTION_OV_VOLTAGE)\
					,POSITIONER_CONFIG(a##_POSITIONER_OV_VOLTAGE)\
			}\
			,{\
				BURST_PANICS_ACTUATOR_TEMPER_CO(a)\
			}\
		}
		
		int def_mode;
		
		ps::control & psc;	
		enco_t<number> & enco;
		filter_t<signal_t> & speedflt;

		motion_t<number> motion;
		positioner_t<number> positioner;
		
		struct present_s {		
			burst::dev::present_s ref;
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
		virtual signal_t temper_pp()=0;
		#endif
		
		//void mode_speed_applay_action(void);
		//void actuator_mode_voltage_applay_action(void);
	
		//void actuator_mode_position_applay_action(burst_dev_ref_p _ref);

		//void actuator_event_update_feedback(burst_dev_ref_p _dev);

		actuator (
			const config_s & _config
			, action_s & _action
			, feedback_s & _feedback
			, ps::control & _ps
			, enco_t<number> & _enco
			, filter_t<signal_t> & _spf
		) : burst::dev( _config.ref, _present.ref, _action.ref, _feedback.ref)
			, psc(_ps), 
			;

		#if BURST_PROTECTION_ENABLED == 1
		virtual void realtime_protection(void);
		virtual void frontend_protection(void);	
		#endif 
		
	};
}
#endif