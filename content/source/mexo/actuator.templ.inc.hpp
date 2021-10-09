#ifndef ACTUATOR_TEMPLATE_NAME
#define ACTUATOR_TEMPLATE_NAME actuator
#define ACTUATOR_PREFIX(name)  _ACTUATOR_PREFIX(name,ACTUATOR_TEMPLATE_NAME)
#define _ACTUATOR_PREFIX(name,prfx)  __ACTUATOR_PREFIX(name,prfx)
#define __ACTUATOR_PREFIX(name,prfx) prfx##_##name
#define actuator_MOTOR_POSTITION_MEASSURY_ENABLED 1
#define actuator_MOTOR_SPEED_FILTER_ENABLED 1
#define actuator_SPEED_OV_CURRENT_MODE_ENABLED 1
#define actuator_SPEED_OV_VOLTAGE_CL_MODE_ENABLED 1
#define actuator_POSITION_MODE_OV_CURRENT_ENABLED 1
#define actuator_POSITION_MODE_OV_VOLTAGE_CL_ENABLED 1
#define actuator_ps_CURRENT_LIMMITER_ENABLED 1
#define actuator_ps_CURRENT_REGULATOR_ENABLED 1
#define PS_TEMPLATE_SUB_NAME ps
#endif
#ifndef PS_TEMPLATE_SUB_NAME
#define PS_TEMPLATE_SUB_NAME ps
#endif
#define PS_TEMPLATE_NAME ACTUATOR_PREFIX(PS_TEMPLATE_SUB_NAME)
#define POWER_SUPPLY_PREFIX(name)  _POWER_SUPPLY_PREFIX(name,ACTUATOR_TEMPLATE_NAME)
#define _POWER_SUPPLY_PREFIX(name,prfx)  __POWER_SUPPLY_PREFIX(name,prfx)
#define __POWER_SUPPLY_PREFIX(name,prfx) prfx##_ps_##name


#define ACTUATOR_MOTOR_POSTITION_MEASSURY_ENABLED ACTUATOR_PREFIX(MOTOR_POSTITION_MEASSURY_ENABLED)
#if ACTUATOR_MOTOR_POSTITION_MEASSURY_ENABLED ==1
#define ACTUATOR_MOTOR_SPEED_FILTER_ENABLED ACTUATOR_PREFIX(MOTOR_SPEED_FILTER_ENABLED)
#define ACTUATOR_SPEED_OV_CURRENT_MODE_ENABLED (( ACTUATOR_PREFIX(SPEED_OV_CURRENT_MODE_ENABLED) == 1) && (POWER_SUPPLY_PREFIX(CURRENT_REGULATOR_ENABLED)==1))
#define ACTUATOR_SPEED_OV_VOLTAGE_CL_MODE_ENABLED ((ACTUATOR_PREFIX(SPEED_OV_VOLTAGE_CL_MODE_ENABLED)==1) && (POWER_SUPPLY_PREFIX(CURRENT_LIMMITER_ENABLED)==1))
#define ACTUATOR_POSITION_MODE_OV_CURRENT_ENABLED ((ACTUATOR_PREFIX(POSITION_MODE_OV_CURRENT_ENABLED)==1) && (ACTUATOR_SPEED_OV_CURRENT_MODE_ENABLED==1))
#define ACTUATOR_POSITION_MODE_OV_VOLTAGE_CL_ENABLED ((ACTUATOR_PREFIX(POSITION_MODE_OV_VOLTAGE_CL_ENABLED)==1) && (ACTUATOR_SPEED_OV_VOLTAGE_CL_MODE_ENABLED==1))
#else
#define ACTUATOR_SPEED_OV_CURRENT_MODE_ENABLED 0
#define ACTUATOR_SPEED_OV_VOLTAGE_CL_MODE_ENABLED 0
#define ACTUATOR_POSITION_MODE_OV_CURRENT_ENABLED 0
#define ACTUATOR_POSITION_MODE_OV_VOLTAGE_CL_ENABLED 0
#endif 


#include "mexo/ps.templ.inc.hpp"

template <typename types, typename hardwaresys_t>  class ACTUATOR_PREFIX(t) : public PS_TEMPLATE_NAME < types, hardwaresys_t > {
	int slot_index_;
public:
	
	#if ACTUATOR_SPEED_OV_CURRENT_MODE_ENABLED == 1 || \
		ACTUATOR_SPEED_OV_VOLTAGE_CL_MODE_ENABLED == 1 || \
		ACTUATOR_MOTOR_SPEED_FILTER_ENABLED==1
	typedef ::mexo::controller_task_t <
		::mexo::motion<types>
		, ::mexo::periodic_subsystem
		, const typename types::signal_t&
	> speed_regulator_b;
	#endif
	#if ACTUATOR_SPEED_OV_CURRENT_MODE_ENABLED == 1
	speed_regulator_b speed_regulator_ov_current;
	#endif
	#if ACTUATOR_SPEED_OV_VOLTAGE_CL_MODE_ENABLED == 1 
	speed_regulator_b speed_regulator_ov_voltage_cl;
	#endif
	#if ACTUATOR_MOTOR_SPEED_FILTER_ENABLED==1
	typedef ::mexo::function_block_t <
		::mexo::filter<types>
		, ::mexo::periodic_subsystem
	>  speed_filter_b;
	speed_filter_b speed_filter;
	#endif 
	typedef PS_TEMPLATE_NAME < types, hardwaresys_t > ps_t;

	struct action_s {
		typename ps_t::action_s ps;
		#if ACTUATOR_SPEED_OV_CURRENT_MODE_ENABLED == 1  \
		|| ACTUATOR_SPEED_OV_VOLTAGE_CL_MODE_ENABLED == 1 
			typename types::signal_t speed;
		#endif
		#if ACTUATOR_POSITION_MODE_OV_CURRENT_ENABLED == 1  \
		|| ACTUATOR_POSITION_MODE_OV_VOLTAGE_CL_ENABLED == 1 
			typename types::signal_t position;
		#endif
	};

	struct config_s {
		typename ps_t::config_s ps;
		#if ACTUATOR_SPEED_OV_CURRENT_MODE_ENABLED == 1  
		typename speed_regulator_b::config_s speed_ov_current;
		#endif
		#if ACTUATOR_SPEED_OV_VOLTAGE_CL_MODE_ENABLED == 1  
		typename speed_regulator_b::config_s speed_ov_voltage_cl;
		#endif
		#if ACTUATOR_MOTOR_SPEED_FILTER_ENABLED == 1
		typename speed_filter_b::config_s speed_filter;
		#endif
	};

	struct present_s {
		typename ps_t::present_s ps;
		#if ACTUATOR_SPEED_OV_CURRENT_MODE_ENABLED == 1  \
		|| ACTUATOR_SPEED_OV_VOLTAGE_CL_MODE_ENABLED == 1 
		typename speed_regulator_b::present_s speed_regulator;
		#endif
		#if ACTUATOR_MOTOR_SPEED_FILTER_ENABLED == 1
		typename speed_filter_b::present_s speed_filter;
		#endif
		#if ACTUATOR_MOTOR_POSTITION_MEASSURY_ENABLED == 1
		typename types::signal_t speed_deseired;
		typename types::signal_t position_deseired;
		#endif
	};

protected:
	#if ACTUATOR_SPEED_OV_CURRENT_MODE_ENABLED == 1 || \
		ACTUATOR_SPEED_OV_VOLTAGE_CL_MODE_ENABLED == 1
	void speed_mode_applay_action(void) {
		const action_s& action = ::mexo::dev::action_cast<action_s>();
		present_s& present = ::mexo::dev::present_cast<present_s>();

		if (action.ps.invers) {
			present.speed_deseired = -action.speed;
		}
		else {
			present.speed_deseired = action.speed;
		}
		present.ps.voltage_range_desired.hi = action.ps.voltage;
		present.ps.voltage_range_desired.low = -action.ps.voltage;
		present.ps.current_range_desired.hi = action.ps.current;
		present.ps.current_range_desired.low = -action.ps.current;
	}
	#endif
public:
#if ACTUATOR_SPEED_OV_CURRENT_MODE_ENABLED == 1
protected:	
	void speed_ov_current_mode_start(void) {
		present_s& present = ::mexo::dev::present_cast<present_s>();
		speed_regulator_ov_current.set_output(&present.ps.current_deseired);
		speed_regulator_ov_current.set_input(&present.speed_deseired);
		PS_TEMPLATE_NAME < types, hardwaresys_t > ::mode_current_start();
		speed_regulator_ov_current.start();
	}
	void speed_ov_current_mode_stop(void) {
		speed_regulator_ov_current.stop();
		PS_TEMPLATE_NAME < types, hardwaresys_t > ::mode_current_stop();
		speed_regulator_ov_current.set_output(nullptr);
		speed_regulator_ov_current.set_input(nullptr);
	}

	public:
	friend class speed_ov_current_mode_t;
	class speed_ov_current_mode_t :public ::mexo::ps::dev::mode {
	protected:
		ACTUATOR_PREFIX(t) & owner(void) { return owner_cast<ACTUATOR_PREFIX(t)>(); }

		virtual void applay_action(void) {
			owner().speed_mode_applay_action();
		}

		virtual void do_start(void) {
			owner().speed_ov_current_mode_start();
		}

		virtual void do_stop(void) {
			owner().speed_ov_current_mode_stop();
		}

	public:
		speed_ov_current_mode_t(int _index, ACTUATOR_PREFIX(t)& _owner) :
			::mexo::ps::dev::mode(_index, RT("mod_sp_c"), _owner) {}
	} speed_ov_current_mode;
	#endif
	#if ACTUATOR_SPEED_OV_VOLTAGE_CL_MODE_ENABLED == 1
protected:
	void speed_ov_voltage_cl_mode_start(void) {
		present_s& present = ::mexo::dev::present_cast<present_s>();
		speed_regulator_ov_voltage_cl.set_output(&present.ps.voltage_deseired);
		speed_regulator_ov_voltage_cl.set_input(&present.speed_deseired);
		PS_TEMPLATE_NAME < types, hardwaresys_t > ::mode_limmiter_start();
		speed_regulator_ov_voltage_cl.start();
	}
	void speed_ov_voltage_cl_mode_stop(void) {
		speed_regulator_ov_voltage_cl.stop();
		PS_TEMPLATE_NAME < types, hardwaresys_t > ::mode_limmiter_stop();
		speed_regulator_ov_voltage_cl.set_output(nullptr);
		speed_regulator_ov_voltage_cl.set_input(nullptr);
	}

public:
	friend class speed_ov_voltage_cl_mode_t;
	class speed_ov_voltage_cl_mode_t :public ::mexo::ps::dev::mode {
	protected:
		ACTUATOR_PREFIX(t)& owner(void) { return owner_cast<ACTUATOR_PREFIX(t)>(); }

		virtual void applay_action(void) {
			owner().speed_mode_applay_action();
		}

		virtual void do_start(void) {
			owner().speed_ov_voltage_cl_mode_start();
		}

		virtual void do_stop(void) {
			owner().speed_ov_voltage_cl_mode_stop();
		}

	public:
		speed_ov_voltage_cl_mode_t(int _index, ACTUATOR_PREFIX(t) & _owner) :
			::mexo::ps::dev::mode(_index, RT("mod_sp_cl"), _owner) {}
	} speed_ov_voltage_cl_mode;
	#endif
	ACTUATOR_PREFIX(t)(hardwaresys_t& _hardwaresys, cstr _name, action_s& _action, config_s& _config, present_s& _present, int _slot_index)
		: ps_t(_hardwaresys, _name, _action.ps, _config.ps, _present.ps )
		, slot_index_(_slot_index)
		#if ACTUATOR_SPEED_OV_CURRENT_MODE_ENABLED == 1
		, speed_regulator_ov_current(
			RT("sp_c_r")
			, this
			, _config.speed_ov_current
			, _present.speed_regulator
			, _present.ps.current_range_desired
			, _present.ps.current_regulator.cb.satstate.actual
			#if ACTUATOR_MOTOR_SPEED_FILTER_ENABLED == 1
			, _present.speed_filter.fb.output
			#else
			, _hardwaresys.enco().diff()
			#endif

		)
		#endif 
		#if ACTUATOR_SPEED_OV_VOLTAGE_CL_MODE_ENABLED == 1
		, speed_regulator_ov_voltage_cl(
			RT("sp_cl_r")
			, this
			, _config.speed_ov_voltage_cl
			, _present.speed_regulator
			, _present.ps.voltage_range_desired
			, _present.ps.current_limmiter.cb.satstate.actual
			#if ACTUATOR_MOTOR_SPEED_FILTER_ENABLED == 1
			, _present.speed_filter.fb.output
			#else
			, _hardwaresys.enco().diff()
			#endif

		)
		#endif 		
		#if ACTUATOR_MOTOR_SPEED_FILTER_ENABLED ==1
		, speed_filter(RT("sp_f"), &_hardwaresys.periodic_subsystem(), _config.speed_filter, _present.speed_filter, _hardwaresys.enco_block().delta())
		#endif
		#if ACTUATOR_SPEED_OV_CURRENT_MODE_ENABLED == 1
		, speed_ov_current_mode(3, *this)
		#endif
		#if ACTUATOR_SPEED_OV_VOLTAGE_CL_MODE_ENABLED == 1
		, speed_ov_voltage_cl_mode(17,*this)
		#endif
	{
		#if ACTUATOR_SPEED_OV_CURRENT_MODE_ENABLED == 1
		_config.speed_ov_current = 
			#define MOTION_PROP_GAIN ACTUATOR_PREFIX(SPEED_OV_CURRENT_PROP_GAIN)
			#define MOTION_MODEL_GAIN ACTUATOR_PREFIX(SPEED_OV_CURRENT_MODEL_GAIN)
			#define MOTION_CONTROL_SHIFT ACTUATOR_PREFIX(SPEED_OV_CURRENT_CONTROL_SHIFT)
			#define MOTION_MODEL_SHIFT ACTUATOR_PREFIX(SPEED_OV_CURRENT_MODEL_SHIFT)
			#define MOTION_FORCE_GAIN ACTUATOR_PREFIX(SPEED_OV_CURRENT_FORCE_GAIN)
			#define MOTION_FORCE_LIM ACTUATOR_PREFIX(SPEED_OV_CURRENT_FORCE_LIM)
			#define MOTION_REF_GAIN ACTUATOR_PREFIX(SPEED_OV_CURRENT_REF_GAIN)
			#define MOTION_REF_PRESC_SHIFT ACTUATOR_PREFIX(SPEED_OV_CURRENT_REF_PRESC_SHIFT)
			#include "mexo/motion.templ.settings.inc.hpp"
		;
		#endif
		#if ACTUATOR_SPEED_OV_VOLTAGE_CL_MODE_ENABLED == 1
		_config.speed_ov_voltage_cl =
			#define MOTION_PROP_GAIN ACTUATOR_PREFIX(SPEED_OV_VOLTAGE_CL_PROP_GAIN)
			#define MOTION_MODEL_GAIN ACTUATOR_PREFIX(SPEED_OV_VOLTAGE_CL_MODEL_GAIN)
			#define MOTION_CONTROL_SHIFT ACTUATOR_PREFIX(SPEED_OV_VOLTAGE_CL_CONTROL_SHIFT)
			#define MOTION_MODEL_SHIFT ACTUATOR_PREFIX(SPEED_OV_VOLTAGE_CL_MODEL_SHIFT)
			#define MOTION_FORCE_GAIN ACTUATOR_PREFIX(SPEED_OV_VOLTAGE_CL_FORCE_GAIN)
			#define MOTION_FORCE_LIM ACTUATOR_PREFIX(SPEED_OV_VOLTAGE_CL_FORCE_LIM)
			#define MOTION_REF_GAIN ACTUATOR_PREFIX(SPEED_OV_VOLTAGE_CL_REF_GAIN)
			#define MOTION_REF_PRESC_SHIFT ACTUATOR_PREFIX(SPEED_OV_VOLTAGE_CL_REF_PRESC_SHIFT)
			#include "mexo/motion.templ.settings.inc.hpp"
			;
		#endif
		#if ACTUATOR_MOTOR_SPEED_FILTER_ENABLED == 1
		_config.speed_filter =
			#define FILTER_GAIN ACTUATOR_PREFIX(SPEED_FILTER_GAIN)
			#define FILTER_SHIFT_GAIN ACTUATOR_PREFIX(SPEED_FILTER_SHIFT_GAIN)
			#define FILTER_SHIFT_PRESC ACTUATOR_PREFIX(SPEED_FILTER_SHIFT_PRESC)
			#define FILTER_SHIFT_VALUE ACTUATOR_PREFIX(SPEED_FILTER_SHIFT_VALUE)
			#include "mexo/filter.templ.settings.inc.hpp"
			;
		#endif
		#if ACTUATOR_SPEED_OV_CURRENT_MODE_ENABLED == 1
		speed_regulator_ov_current.setup(_slot_index);
		#endif
		#if ACTUATOR_SPEED_OV_VOLTAGE_CL_MODE_ENABLED == 1
		speed_regulator_ov_voltage_cl.setup(_slot_index);
		#endif
	}
};

#undef  POWER_SUPPLY_PREFIX
#undef  _POWER_SUPPLY_PREFIX
#undef  __POWER_SUPPLY_PREFIX
#undef PS_TEMPLATE_NAME

#undef ACTUATOR_MOTOR_POSTITION_MEASSURY_ENABLED
#undef ACTUATOR_MOTOR_SPEED_FILTER_ENABLED
#undef ACTUATOR_SPEED_OV_CURRENT_MODE_ENABLED
#undef ACTUATOR_SPEED_OV_VOLTAGE_CL_MODE_ENABLED
#undef ACTUATOR_POSITION_MODE_OV_CURRENT_ENABLED
#undef ACTUATOR_POSITION_MODE_OV_VOLTAGE_CL_ENABLED
