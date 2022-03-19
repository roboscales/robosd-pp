#define TEMPL_BEGIN
#include "mexo/actuator.templ.prepare.hpp"

#define PS_TEMPLATE_NAME ACTUATOR_PS_TEMPLATE_NAME 
#include "mexo/ps.templ.inc.hpp"
 
namespace ACTUATOR_TEMPLATE_NAME {
	template <typename types, typename hardwaresys_t>  class dev_t: public   ACTUATOR_PS_TEMPLATE_NAME::dev_t<types, hardwaresys_t> {
	int slot_index_;
public:
	typedef ::mexo::front::ACTUATOR_TEMPLATE_NAME::action_t<types> action_s;
	typedef ::mexo::front::ACTUATOR_TEMPLATE_NAME::feedback_t<types> feedback_s;
	typedef ::mexo::front::ACTUATOR_TEMPLATE_NAME::mode mode;

	#if ACTUATOR_SPEED_OV_CURRENT_MODE_ENABLED == 1 || \
		ACTUATOR_SPEED_OV_VOLTAGE_CL_MODE_ENABLED == 1 || \
		ACTUATOR_POSITION_OV_CURRENT_MODE_ENABLED==1 || \
		ACTUATOR_POSITION_OV_VOLTAGE_CL_MODE_ENABLED==1
	typedef ::mexo::controller_task_t <
		::mexo::motion<types>
		, ::mexo::periodic_subsystem
		, const typename types::signal_t&
	> motion_b;
	#endif
	#if ACTUATOR_SPEED_OV_CURRENT_MODE_ENABLED == 1
	motion_b motion_ov_current;
	#endif
	#if ACTUATOR_SPEED_OV_VOLTAGE_CL_MODE_ENABLED == 1 
	motion_b motion_ov_voltage_cl;
	#endif
		
	#if ACTUATOR_MOTOR_SPEED_FILTER_ENABLED==1
	typedef ::mexo::function_block_t <
		::mexo::filter<types, true>
		, ::mexo::periodic_subsystem
	>  speed_filter_b;
	speed_filter_b speed_filter;
	#endif 

	#if ACTUATOR_POSITION_OV_CURRENT_MODE_ENABLED == 1 || \
		ACTUATOR_POSITION_OV_VOLTAGE_CL_MODE_ENABLED == 1
	typedef ::mexo::controller_task_t <
		::mexo::positioner<types>
		, ::mexo::periodic_subsystem
		, const typename types::long_signal_t&
		, const typename types::signal_t&
		, const typename types::signal_t&
	> positioner_b;
	#endif
		
	#if ACTUATOR_POSITION_OV_CURRENT_MODE_ENABLED == 1
		positioner_b positioner_ov_current;
	#endif

	#if ACTUATOR_POSITION_OV_VOLTAGE_CL_MODE_ENABLED == 1
		positioner_b positioner_ov_voltage_cl;
	#endif
		
	typedef ACTUATOR_PS_TEMPLATE_NAME :: dev_t<types,hardwaresys_t> ps_t;

	struct config_s {
		typename ps_t::config_s ps;
		#if ACTUATOR_SPEED_OV_CURRENT_MODE_ENABLED == 1  
		typename motion_b::config_s motion_ov_current;
		#endif
		#if ACTUATOR_SPEED_OV_VOLTAGE_CL_MODE_ENABLED == 1  
		typename motion_b::config_s motion_ov_voltage_cl;
		#endif
		#if ACTUATOR_MOTOR_SPEED_FILTER_ENABLED == 1
		typename speed_filter_b::config_s speed_filter;
		#endif
		#if ACTUATOR_POSITION_OV_CURRENT_MODE_ENABLED == 1
		typename positioner_b::config_s positioner_ov_current;
		#endif
		#if ACTUATOR_POSITION_OV_VOLTAGE_CL_MODE_ENABLED == 1
		typename positioner_b::config_s positioner_ov_voltage_cl;
		#endif
	};

	struct present_s {
		typename ps_t::present_s ps;
		#if ACTUATOR_SPEED_OV_CURRENT_MODE_ENABLED == 1  \
		|| ACTUATOR_SPEED_OV_VOLTAGE_CL_MODE_ENABLED == 1 
		typename motion_b::present_s motion;
		#endif
		#if ACTUATOR_MOTOR_SPEED_FILTER_ENABLED == 1
		typename speed_filter_b::present_s speed_filter;
		#endif
		#if ACTUATOR_POSITION_OV_CURRENT_MODE_ENABLED == 1 \
		||	ACTUATOR_POSITION_OV_VOLTAGE_CL_MODE_ENABLED == 1
			typename positioner_b::present_s positioner;
		#endif
		#if ACTUATOR_MOTOR_POSTITION_MEASSURY_ENABLED == 1
		::mexo::range_s<typename types::signal_t> speed_range_desired;
		//::mexo::range_s<typename types::long_signal_t> position_range_desired;
		typename types::signal_t speed_deseired;
		typename types::signal_t speed_force;
		typename types::long_signal_t position_deseired;
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
		motion_ov_current.set_output(&present.ps.current_deseired);
		motion_ov_current.set_input(&present.speed_deseired);
		dev_t < types, hardwaresys_t > ::mode_current_start();
		motion_ov_current.start();
	}
	void speed_ov_current_mode_stop(void) {
		motion_ov_current.stop();
		dev_t < types, hardwaresys_t > ::mode_current_stop();
		motion_ov_current.set_output(nullptr);
		motion_ov_current.set_input(nullptr);
	}

	public:
	friend class speed_ov_current_mode_t;
	class speed_ov_current_mode_t :public ::mexo::ps::dev::mode {
	protected:
		dev_t & owner(void) { return owner_cast<dev_t>(); }

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
		speed_ov_current_mode_t(int _index, dev_t& _owner) :
			::mexo::ps::dev::mode(_index, RT("mod_sp_c"), _owner) {}
	} speed_ov_current_mode;
	#endif
	#if ACTUATOR_SPEED_OV_VOLTAGE_CL_MODE_ENABLED == 1
protected:
	void speed_ov_voltage_cl_mode_start(void) {
		present_s& present = ::mexo::dev::present_cast<present_s>();
		motion_ov_voltage_cl.set_output(&present.ps.voltage_deseired);
		motion_ov_voltage_cl.set_input(&present.speed_deseired);
		dev_t < types, hardwaresys_t > ::mode_limmiter_start();
		motion_ov_voltage_cl.start();
	}
	void speed_ov_voltage_cl_mode_stop(void) {
		motion_ov_voltage_cl.stop();
		dev_t < types, hardwaresys_t > ::mode_limmiter_stop();
		motion_ov_voltage_cl.set_output(nullptr);
		motion_ov_voltage_cl.set_input(nullptr);
	}

public:
	friend class speed_ov_voltage_cl_mode_t;
	class speed_ov_voltage_cl_mode_t :public ::mexo::ps::dev::mode {
	protected:
		dev_t < types, hardwaresys_t > & owner(void) { return owner_cast<dev_t < types, hardwaresys_t >>(); }

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
		speed_ov_voltage_cl_mode_t(int _index, dev_t < types, hardwaresys_t > & _owner) :
			::mexo::ps::dev::mode(_index, RT("mod_sp_cl"), _owner) {}
	} speed_ov_voltage_cl_mode;
	#endif
	
	#if ACTUATOR_POSITION_OV_CURRENT_MODE_ENABLED == 1 || \
		ACTUATOR_POSITION_OV_VOLTAGE_CL_MODE_ENABLED == 1
	void position_mode_applay_action(void) {
		const action_s& action = ::mexo::dev::action_cast<action_s>();
		present_s& present = ::mexo::dev::present_cast<present_s>();

		if (action.ps.invers) {
			present.position_deseired = -action.position;
		}
		else {
			present.position_deseired = action.position;
		}
		present.speed_range_desired.hi = action.speed;
		present.speed_range_desired.low = -action.speed;
		present.ps.voltage_range_desired.hi = action.ps.voltage;
		present.ps.voltage_range_desired.low = -action.ps.voltage;
		present.ps.current_range_desired.hi = action.ps.current;
		present.ps.current_range_desired.low = -action.ps.current;
	}
	#endif
	
	#if ACTUATOR_POSITION_OV_CURRENT_MODE_ENABLED == 1
protected:
	void position_ov_current_mode_start(void) {
		present_s& present = ::mexo::dev::present_cast<present_s>();
		
		positioner_ov_current.set_output(&present.speed_deseired);
		positioner_ov_current.set_input(&present.position_deseired);			
		positioner_ov_current.start();		
		speed_ov_current_mode_start();
	}
	void position_ov_current_mode_stop(void) {
		speed_ov_current_mode_stop();
		positioner_ov_current.stop();
		positioner_ov_current.set_output(nullptr);
		positioner_ov_current.set_input(nullptr);
	}

public:
	friend class position_ov_current_mode_t;
	class position_ov_current_mode_t :public ::mexo::ps::dev::mode {
	protected:
		dev_t& owner(void) { return owner_cast<dev_t>(); }

		virtual void applay_action(void) {
			owner().position_mode_applay_action();
		}

		virtual void do_start(void) {
			owner().position_ov_current_mode_start();
		}

		virtual void do_stop(void) {
			owner().position_ov_current_mode_stop();
		}

	public:
		position_ov_current_mode_t(int _index, dev_t & _owner) :
			::mexo::ps::dev::mode(_index, RT("mod_po_c"), _owner) {}
	} position_ov_current_mode;
	#endif
	
	#if ACTUATOR_POSITION_OV_VOLTAGE_CL_MODE_ENABLED == 1
protected:
	void position_ov_voltage_cl_mode_start(void) {
		present_s& present = ::mexo::dev::present_cast<present_s>();

		positioner_ov_voltage_cl.set_output(&present.speed_deseired);
		positioner_ov_voltage_cl.set_input(&present.position_deseired);
	
		positioner_ov_voltage_cl.start();		
		speed_ov_voltage_cl_mode_start();
	}
	void position_ov_voltage_cl_mode_stop(void) {
		speed_ov_voltage_cl_mode_stop();
		positioner_ov_voltage_cl.stop();
		positioner_ov_voltage_cl.set_output(nullptr);
		positioner_ov_voltage_cl.set_input(nullptr);
	}

public:
	friend class position_ov_voltage_cl_mode_t;
	class position_ov_voltage_cl_mode_t :public ::mexo::ps::dev::mode {
	protected:
		dev_t& owner(void) { return owner_cast<dev_t>(); }

		virtual void applay_action(void) {
			owner().position_mode_applay_action();
		}

		virtual void do_start(void) {
			owner().position_ov_voltage_cl_mode_start();
		}

		virtual void do_stop(void) {
			owner().position_ov_voltage_cl_mode_stop();
		}

	public:
		position_ov_voltage_cl_mode_t(int _index, dev_t & _owner) :
			::mexo::ps::dev::mode(_index, RT("mod_po_cl"), _owner) {}
	} position_ov_voltage_cl_mode;
	#endif
	dev_t (hardwaresys_t& _hardwaresys, cstr _name, action_s& _action, config_s& _config, present_s& _present, int _slot_index)
		: ps_t(_hardwaresys, _name, _action.ps, _config.ps, _present.ps )
		, slot_index_(_slot_index)
		#if ACTUATOR_SPEED_OV_CURRENT_MODE_ENABLED == 1
		, motion_ov_current(
			RT("sp_c_r")
			, this
			, _config.motion_ov_current
			, _present.motion
			, _present.ps.current_range_desired
			, _present.ps.current_regulator.cb.satstate.actual
			#if ACTUATOR_MOTOR_SPEED_FILTER_ENABLED == 1
			, _present.speed_filter.fb.output
			#else
			, _hardwaresys.motor_enco_block.delta_acc_ref()
			#endif
		)
		#endif 
		#if ACTUATOR_SPEED_OV_VOLTAGE_CL_MODE_ENABLED == 1
		, motion_ov_voltage_cl(
			RT("sp_cl_r")
			, this
			, _config.motion_ov_voltage_cl
			, _present.motion
			, _present.ps.voltage_range_desired
			, _present.ps.current_limmiter.cb.satstate.actual
			#if ACTUATOR_MOTOR_SPEED_FILTER_ENABLED == 1
			, _present.speed_filter.fb.output
			#else
			, _hardwaresys.motor_enco_block.delta_acc_ref()
			#endif
		)
		#endif 
		#if ACTUATOR_POSITION_OV_CURRENT_MODE_ENABLED == 1
		, positioner_ov_current(
			RT("po_c_r")
			, this
			, _config.positioner_ov_current
			, _present.positioner
			, _present.speed_range_desired
			, _present.motion.cb.satstate.actual
			, _hardwaresys.motor_enco_block.position_ref()
			#if ACTUATOR_MOTOR_SPEED_FILTER_ENABLED == 1
			, _present.speed_filter.fb.output
			#else
			, _hardwaresys.motor_enco_block.delta_acc_ref()
			#endif
			, _present.speed_force
		)
		#endif 
		#if ACTUATOR_POSITION_OV_VOLTAGE_CL_MODE_ENABLED == 1
		,  positioner_ov_voltage_cl(
			RT("po_cl_r")
			, this
			, _config.positioner_ov_voltage_cl
			, _present.positioner
			, _present.speed_range_desired
			, _present.motion.cb.satstate.actual
			, _hardwaresys.motor_enco_block.position_ref()
			#if ACTUATOR_MOTOR_SPEED_FILTER_ENABLED == 1
			, _present.speed_filter.fb.output
			#else
			, _hardwaresys.motor_enco_block.delta_acc_ref()
			#endif
			, _present.speed_force
		)
		#endif 		
		#if ACTUATOR_MOTOR_SPEED_FILTER_ENABLED ==1
		, speed_filter(RT("sp_f"), &_hardwaresys.periodic_subsystem, _config.speed_filter, _present.speed_filter, _hardwaresys.motor_enco_block.delta_acc_ref())
		#endif
		#if ACTUATOR_SPEED_OV_CURRENT_MODE_ENABLED == 1
		, speed_ov_current_mode(mode::speed_ov_current, *this)
		#endif
		#if ACTUATOR_SPEED_OV_VOLTAGE_CL_MODE_ENABLED == 1
		, speed_ov_voltage_cl_mode(17,*this)
		#endif
		#if ACTUATOR_POSITION_OV_CURRENT_MODE_ENABLED == 1
		, position_ov_current_mode(mode::position_ov_current,*this)
		#endif
		#if ACTUATOR_POSITION_OV_VOLTAGE_CL_MODE_ENABLED == 1
		, position_ov_voltage_cl_mode(18,*this)
		#endif
	{
		#if ACTUATOR_SPEED_OV_CURRENT_MODE_ENABLED == 1
		_config.motion_ov_current = 
			#define MOTION_PROP_GAIN ACTUATOR_PREFIX(SPEED_OV_CURRENT_PROP_GAIN)
			#define MOTION_MODEL_GAIN ACTUATOR_PREFIX(SPEED_OV_CURRENT_MODEL_GAIN)
			#define MOTION_CONTROL_SHIFT ACTUATOR_PREFIX(SPEED_OV_CURRENT_CONTROL_SHIFT)
			#define MOTION_MODEL_SHIFT ACTUATOR_PREFIX(SPEED_OV_CURRENT_MODEL_SHIFT)
			#define MOTION_FORCE_GAIN ACTUATOR_PREFIX(SPEED_OV_CURRENT_FORCE_GAIN)
			#define MOTION_FORCE_LIM ACTUATOR_PREFIX(SPEED_OV_CURRENT_FORCE_LIM)
			#include "mexo/motion.templ.settings.inc.hpp"
		;
		#endif
		#if ACTUATOR_SPEED_OV_VOLTAGE_CL_MODE_ENABLED == 1
		_config.motion_ov_voltage_cl =
			#define MOTION_PROP_GAIN ACTUATOR_PREFIX(SPEED_OV_VOLTAGE_CL_PROP_GAIN)
			#define MOTION_MODEL_GAIN ACTUATOR_PREFIX(SPEED_OV_VOLTAGE_CL_MODEL_GAIN)
			#define MOTION_CONTROL_SHIFT ACTUATOR_PREFIX(SPEED_OV_VOLTAGE_CL_CONTROL_SHIFT)
			#define MOTION_MODEL_SHIFT ACTUATOR_PREFIX(SPEED_OV_VOLTAGE_CL_MODEL_SHIFT)
			#define MOTION_FORCE_GAIN ACTUATOR_PREFIX(SPEED_OV_VOLTAGE_CL_FORCE_GAIN)
			#define MOTION_FORCE_LIM ACTUATOR_PREFIX(SPEED_OV_VOLTAGE_CL_FORCE_LIM)
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
		
		#if ACTUATOR_POSITION_OV_CURRENT_MODE_ENABLED == 1
		_config.positioner_ov_current = 
		#define POSITIONER_PROP_GAIN ACTUATOR_PREFIX(POSITIONER_OV_CURRENT_PROP_GAIN)
		#define POSITIONER_DIFF_GAIN ACTUATOR_PREFIX(POSITIONER_OV_CURRENT_DIFF_GAIN)
		#define POSITIONER_CONTROL_SHIFT ACTUATOR_PREFIX(POSITIONER_OV_CURRENT_CONTROL_SHIFT)
		#define POSITIONER_DIFF_QUADR_GAIN ACTUATOR_PREFIX(POSITIONER_OV_CURRENT_DIFF_QUADR_GAIN)
		#define POSITIONER_DIFF_QUADR_SHIFT ACTUATOR_PREFIX(POSITIONER_OV_CURRENT_DIFF_QUADR_SHIFT)
		#define POSITIONER_DEAD_ZONE ACTUATOR_PREFIX(POSITIONER_OV_CURRENT_DEAD_ZONE)
		#define POSITIONER_CRAWL_SPEED ACTUATOR_PREFIX(POSITIONER_OV_CURRENT_CRAWL_SPEED)
		#include "mexo/positioner.templ.settings.inc.hpp"
		;
		#endif

		#if ACTUATOR_POSITION_OV_VOLTAGE_CL_MODE_ENABLED == 1
		_config.positioner_ov_voltage_cl = 
		#define POSITIONER_PROP_GAIN ACTUATOR_PREFIX(POSITIONER_OV_VOLTAGE_CL_PROP_GAIN)
		#define POSITIONER_DIFF_GAIN ACTUATOR_PREFIX(POSITIONER_OV_VOLTAGE_CL_DIFF_GAIN)
		#define POSITIONER_CONTROL_SHIFT ACTUATOR_PREFIX(POSITIONER_OV_VOLTAGE_CL_CONTROL_SHIFT)
		#define POSITIONER_DIFF_QUADR_GAIN ACTUATOR_PREFIX(POSITIONER_OV_VOLTAGE_CL_DIFF_QUADR_GAIN)
		#define POSITIONER_DIFF_QUADR_SHIFT ACTUATOR_PREFIX(POSITIONER_OV_VOLTAGE_CL_DIFF_QUADR_SHIFT)
		#define POSITIONER_DEAD_ZONE ACTUATOR_PREFIX(POSITIONER_OV_VOLTAGE_CL_DEAD_ZONE)
		#define POSITIONER_CRAWL_SPEED ACTUATOR_PREFIX(POSITIONER_OV_VOLTAGE_CL_CRAWL_SPEED)
		#include "mexo/positioner.templ.settings.inc.hpp"
		;
		#endif
		
		//_action.speed

		#if ACTUATOR_SPEED_OV_CURRENT_MODE_ENABLED == 1
		motion_ov_current.setup(_slot_index);
		#endif
		#if ACTUATOR_SPEED_OV_VOLTAGE_CL_MODE_ENABLED == 1
		motion_ov_voltage_cl.setup(_slot_index);
		#endif
		#if ACTUATOR_POSITION_OV_VOLTAGE_CL_MODE_ENABLED == 1
		positioner_ov_voltage_cl.setup(_slot_index);
		#endif
		#if ACTUATOR_POSITION_OV_CURRENT_MODE_ENABLED == 1
		positioner_ov_current.setup(_slot_index);
		#endif
	}
	protected:
		#if ROBO_APP_MEXO_VAR_ENABLED == 1
		void do_create_vars(void) {
			ps_t::do_create_vars();
			if (::mexo::var::machine::actual_mode() >= ::mexo::var::machine::mode::action) {
				const action_s& action = action_cast<action_s>();

				#if ACTUATOR_SPEED_OV_CURRENT_MODE_ENABLED == 1  \
				|| ACTUATOR_SPEED_OV_VOLTAGE_CL_MODE_ENABLED == 1 
				::mexo::var::record::create(typename types::var::signal, action.speed, RT("act.sp"), key(), vars);
				#endif
				#if ACTUATOR_POSITION_OV_CURRENT_MODE_ENABLED == 1  \
				|| ACTUATOR_POSITION_OV_VOLTAGE_CL_MODE_ENABLED == 1 
				::mexo::var::record::create(typename types::var::long_signal, action.position, RT("act.po"), key(), vars);
				#endif
			}
			if (::mexo::var::machine::actual_mode() >= ::mexo::var::machine::mode::full) {
				const present_s& present = present_cast<present_s>();
				#if ACTUATOR_SPEED_OV_CURRENT_MODE_ENABLED == 1  \
				|| ACTUATOR_SPEED_OV_VOLTAGE_CL_MODE_ENABLED == 1 
				::mexo::var::record::create(typename types::var::signal, present.speed_deseired, RT("desrd.sp"), key(), vars);
				#endif
				#if ACTUATOR_POSITION_OV_CURRENT_MODE_ENABLED == 1  \
				|| ACTUATOR_POSITION_OV_VOLTAGE_CL_MODE_ENABLED == 1 
				::mexo::var::record::create(typename types::var::long_signal, present.position_deseired, RT("desrd.po"), key(), vars);
				#endif
				/*

				#if POWER_SUPPLY_VOLTAGE_REGULATOR_ENABLED == 1 ||  POWER_SUPPLY_CURRENT_REGULATOR_ENABLED == 1 || POWER_SUPPLY_CURRENT_LIMMITER_ENABLED == 1
				::mexo::var::record::create(typename types::var::signal, present.voltage_required, RT("req.v"), key(), vars);
				#endif		

				#if POWER_SUPPLY_VOLTAGE_REGULATOR_ENABLED == 1 ||  POWER_SUPPLY_CURRENT_LIMMITER_ENABLED == 1
				::mexo::var::record::create(typename types::var::signal, present.voltage_deseired, RT("desrd.v"), key(), vars);
				#endif*/
			}

		}
		#endif
};
}

#define TEMPL_FINISH
#include "mexo/actuator.templ.prepare.hpp"
