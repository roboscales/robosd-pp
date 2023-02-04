#define TEMPL_BEGIN
#include "mexo/actuator.templ.prepare.hpp"

#define PS_TEMPLATE_NAME ACTUATOR_PS_TEMPLATE_NAME 
#include "mexo/ps.templ.inc.hpp"
 
namespace ACTUATOR_TEMPLATE_NAME {
	template <typename types, typename hardwaresys_t>  class dev_t: public   ACTUATOR_PS_TEMPLATE_NAME::dev_t<types, hardwaresys_t> {
	int slot_index_;
public:
	typedef front::action_t<types> action_s;
	typedef front::feedback_t<types> feedback_s;
	#if ACTUATOR_PREFIX(MOTOR_POSTITION_MEASSURY_ENABLED)
	typedef front::profil_t<types> profil_s;
	#endif
	typedef front::mode mode;
	
	#if ACTUATOR_MOTOR_SPEED_FILTER_ENABLED==1
	typedef ::mexo::function_block_t <
		::mexo::filter<types, true>
		, ::mexo::periodic_subsystem
	>  speed_filter_b;
	speed_filter_b speed_filter;
	#endif 

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
		const action_s& act= ::mexo::dev::action<dev_t>();
		const config_s& conf = ::mexo::dev::config<dev_t>();
		present_s& present = ::mexo::dev::present<dev_t>();

		if (conf.ps.invers) {
			present.speed_deseired = -act.speed;
		}
		else {
			present.speed_deseired = act.speed;
		}
		present.ps.voltage_range_desired.hi = act.ps.voltage;
		present.ps.voltage_range_desired.low = -act.ps.voltage;
		present.ps.current_range_desired.hi = act.ps.current;
		present.ps.current_range_desired.low = -act.ps.current;
	}
	#endif
public:
#if ACTUATOR_SPEED_OV_CURRENT_MODE_ENABLED == 1
protected:	
	void speed_ov_current_mode_start(void) {
		present_s& present = ::mexo::dev::present<dev_t>();
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
		present_s& present = ::mexo::dev::present<dev_t>();
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
		const action_s& act = ::mexo::dev::action<dev_t>();
		const config_s& conf = ::mexo::dev::config<dev_t>();
		present_s& present = ::mexo::dev::present<dev_t>();

		if (conf.ps.invers) {
			present.position_deseired = -act.position;
		}
		else {
			present.position_deseired = act.position;
		}
		present.speed_range_desired.hi = act.speed;
		present.speed_range_desired.low = -act.speed;
		present.ps.voltage_range_desired.hi = act.ps.voltage;
		present.ps.voltage_range_desired.low = -act.ps.voltage;
		present.ps.current_range_desired.hi = act.ps.current;
		present.ps.current_range_desired.low = -act.ps.current;
	}
	#endif
	
	#if ACTUATOR_POSITION_OV_CURRENT_MODE_ENABLED == 1
protected:
	void position_ov_current_mode_start(void) {
		present_s& present = ::mexo::dev::present<dev_t>();
		
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
		present_s& present = ::mexo::dev::present<dev_t>();

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
	dev_t (hardwaresys_t& _hardwaresys, cstr _name, action_s& _action, feedback_s& _feedback, config_s& _config, present_s& _present, int _slot_index)
		: ps_t(_hardwaresys, _name, _action.ps, _feedback.ps, _config.ps, _present.ps )
		, slot_index_(_slot_index)
		#if ACTUATOR_MOTOR_SPEED_FILTER_ENABLED ==1
		, speed_filter(RT("sp_f"), &_hardwaresys.periodic_subsystem, _config.speed_filter, _present.speed_filter, _hardwaresys.motor_enco_block.delta_acc_ref())
		#endif
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
		#if ACTUATOR_SPEED_OV_CURRENT_MODE_ENABLED == 1
		, speed_ov_current_mode(mode::speed_ov_current, *this)
		#endif
		#if ACTUATOR_SPEED_OV_VOLTAGE_CL_MODE_ENABLED == 1
		, speed_ov_voltage_cl_mode(mode::speed_ov_voltage_cl,*this)
		#endif
		#if ACTUATOR_POSITION_OV_CURRENT_MODE_ENABLED == 1
		, position_ov_current_mode(mode::position_ov_current,*this)
		#endif
		#if ACTUATOR_POSITION_OV_VOLTAGE_CL_MODE_ENABLED == 1
		, position_ov_voltage_cl_mode(mode::position_ov_voltage_cl,*this)
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
	public:
		#if ACTUATOR_PREFIX(MOTOR_POSTITION_MEASSURY_ENABLED)
		const profil_s profil(int _mode) {
			config_s& cf = ps_t::template config< dev_t >();
			profil_s p;
			p.crawl_speed = 0;
			p.dead_zone = 0;
			#if ACTUATOR_MOTOR_POSTITION_MEASSURY_ENABLED == 1
			switch (_mode) {
			#if ACTUATOR_SPEED_OV_VOLTAGE_CL_MODE_ENABLED == 1
			case mode::speed_ov_voltage_cl:
			#if ACTUATOR_POSITION_OV_VOLTAGE_CL_MODE_ENABLED == 1
			p.crawl_speed = cf.positioner_ov_voltage_cl.crawlSpeed;
			p.dead_zone = cf.positioner_ov_voltage_cl.deadZone;
			#endif
			break;
			#endif
			#if ACTUATOR_POSITION_OV_VOLTAGE_CL_MODE_ENABLED == 1
			case mode::position_ov_voltage_cl:
			p.crawl_speed = cf.positioner_ov_voltage_cl.crawlSpeed;
			p.dead_zone = cf.positioner_ov_voltage_cl.deadZone;
			break;
			#endif
			#if ACTUATOR_SPEED_OV_CURRENT_MODE_ENABLED == 1
			case mode::speed_ov_current:
			#if ACTUATOR_POSITION_OV_CURRENT_MODE_ENABLED == 1
			p.crawl_speed = cf.positioner_ov_current.crawlSpeed;
			#endif
			break;
			#endif
			#if ACTUATOR_POSITION_OV_CURRENT_MODE_ENABLED == 1
			case mode::position_ov_current:
			p.crawl_speed = cf.positioner_ov_current.crawlSpeed;
			break;
			#endif
			}
			return p;
		}
		#endif
		#endif
	protected:
		virtual void do_update_feedback(void) {
			feedback_s& fb = ps_t::template feedback< dev_t >();
			#if ACTUATOR_MOTOR_POSTITION_MEASSURY_ENABLED == 1
			present_s& pr = ps_t::template present< dev_t >();
			config_s& cf = ps_t::template config< dev_t >();

			ps_t::do_update_feedback();
			if (cf.ps.invers) {				
				#if ACTUATOR_MOTOR_SPEED_FILTER_ENABLED == 1
				fb.speed = -pr.speed_filter.fb.output;
				#else
				fb.speed = -ps_t::hardwaresys.motor_enco_block.delta_acc_ref();
				#endif
				fb.position = -ps_t::hardwaresys.motor_enco_block.position_ref();
			}
			else {
				#if ACTUATOR_MOTOR_SPEED_FILTER_ENABLED == 1
				fb.speed = pr.speed_filter.fb.output;
				#else
				fb.speed = ps_t::hardwaresys.motor_enco_block.delta_acc_ref();
				#endif
				fb.position = ps_t::hardwaresys.motor_enco_block.position_ref();
			}
			#endif
		}
		#if ROBO_APP_MEXO_VAR_ENABLED == 1
		void do_create_vars(void) {
			ps_t::do_create_vars();
			if (::mexo::var::machine::actual_mode() >= ::mexo::var::machine::mode::action) {
				const action_s& act = ps_t::template action<dev_t>();

				#if ACTUATOR_SPEED_OV_CURRENT_MODE_ENABLED == 1  \
				|| ACTUATOR_SPEED_OV_VOLTAGE_CL_MODE_ENABLED == 1 
				::mexo::var::record::create(types::var::signal, act.speed, RT("act.sp"), ps_t::key(), ps_t::vars);
				#endif
				#if ACTUATOR_POSITION_OV_CURRENT_MODE_ENABLED == 1  \
				|| ACTUATOR_POSITION_OV_VOLTAGE_CL_MODE_ENABLED == 1 
				::mexo::var::record::create(types::var::long_signal, act.position, RT("act.po"), ps_t::key(), ps_t::vars);
				#endif
			}
			if (::mexo::var::machine::actual_mode() >= ::mexo::var::machine::mode::full) {
				const present_s& prsnt = ps_t::template present<dev_t>();
				#if ACTUATOR_SPEED_OV_CURRENT_MODE_ENABLED == 1  \
				|| ACTUATOR_SPEED_OV_VOLTAGE_CL_MODE_ENABLED == 1 
				::mexo::var::record::create(types::var::signal, prsnt.speed_deseired, RT("desrd.sp"), ps_t::key(), ps_t::vars);
				#endif
				#if ACTUATOR_POSITION_OV_CURRENT_MODE_ENABLED == 1  \
				|| ACTUATOR_POSITION_OV_VOLTAGE_CL_MODE_ENABLED == 1 
				::mexo::var::record::create(types::var::long_signal, prsnt.position_deseired, RT("desrd.po"), ps_t::key(), ps_t::vars);
				#endif
				/*

				#if POWER_SUPPLY_VOLTAGE_REGULATOR_ENABLED == 1 ||  POWER_SUPPLY_CURRENT_REGULATOR_ENABLED == 1 || POWER_SUPPLY_CURRENT_LIMMITER_ENABLED == 1
				::mexo::var::record::create(typename types::var::signal, present.voltage_required, RT("req.v"), ps_t::key(), ps_t::vars);
				#endif		

				#if POWER_SUPPLY_VOLTAGE_REGULATOR_ENABLED == 1 ||  POWER_SUPPLY_CURRENT_LIMMITER_ENABLED == 1
				::mexo::var::record::create(typename types::var::signal, present.voltage_deseired, RT("desrd.v"), ps_t::key(), ps_t::vars);
				#endif*/
			}

		}
		#endif
};
}

#define TEMPL_FINISH
#include "mexo/actuator.templ.prepare.hpp"
