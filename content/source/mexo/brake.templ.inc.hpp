#define TEMPL_BEGIN
#include "mexo/brake.templ.prepare.hpp"

#define PS_TEMPLATE_NAME BRAKE_PS_TEMPLATE_NAME 
#include "mexo/ps.templ.inc.hpp"

#include "mexo/mexo_brake.hpp"
#include "core/robosd_system.hpp"

namespace BRAKE_TEMPLATE_NAME {
	template <typename types, typename hardwaresys_t>  class dev_t
		: public   BRAKE_PS_TEMPLATE_NAME::dev_t<types, hardwaresys_t> 
		, public ::mexo::brake::itf
	{
public:
	
	typedef BRAKE_PS_TEMPLATE_NAME :: dev_t<types,hardwaresys_t> ps_t;
	typedef ::mexo::front::BRAKE_TEMPLATE_NAME::power_mode power_mode;
	typedef ::mexo::front::BRAKE_PS_TEMPLATE_NAME::mode mode;
	typedef typename types::signal_t signal_t;
	struct config_s {
		typename ps_t::config_s ps;
		#if BRAKE_VOLTAGE_MODE_ENABLED == 1  
		struct {
			signal_t force;
			signal_t normal;
		} voltage;
		#endif
		#if BRAKE_CURRENT_MODE_ENABLED == 1  
		struct {
			signal_t force;
			signal_t normal;
		} current;
		#endif

		struct {
			::robo::time_us_t relax_us;
			::robo::time_us_t force_us;
		} timeout;
		uint8_t power_mode;
	};

	struct present_s {
		typename ps_t::present_s ps;
		#if BRAKE_VOLTAGE_MODE_ENABLED == 1  \
		|| BRAKE_CURRENT_MODE_ENABLED == 1
			//typename motion_b::present_s motion;
		#endif
		command_t command;
		status_t status;
	};
	typedef typename ps_t::action_s action_s;
	private:
	
	void brake_force(void) {
		const config_s& config = ps_t::template config_cast<config_s>();
		present_s& present = ps_t::template present_cast<present_s>();
		action_s& action = ps_t::template action_cast<action_s>();

		switch(config.power_mode){
		case power_mode::dummy:
		break;
		#if BRAKE_VOLTAGE_MODE_ENABLED == 1
		case power_mode::voltage:
		present.ps.voltage_deseired = config.voltage.force;
		action.dev.mode = mode::voltage;

		break;
		#endif
		#if BRAKE_CURRENT_MODE_ENABLED == 1  
		case power_mode::current:
		present.ps.current_deseired = config.current.force;
		present.ps.voltage_range_desired = ps_t::hardwaresys.power_supply_block.pwm_voltage_limits();
		action.dev.mode = mode::current;
		break;
		#endif
		}
	}

	void brake_normal(void) {
		const config_s& config = ps_t::template config_cast<config_s>();
		present_s& present = ps_t::template present_cast<present_s>();

		switch (config.power_mode) {
//		case power_mode::configure:
		case power_mode::dummy:
		break;
		#if BRAKE_VOLTAGE_MODE_ENABLED == 1
		case power_mode::voltage:
		present.ps.voltage_deseired = config.voltage.normal;
		break;
		#endif
		#if BRAKE_CURRENT_MODE_ENABLED == 1  
		case power_mode::current:
		present.ps.current_deseired = config.current.normal;
		break;		
		#endif
		}
	}

	void brake_set(void) {
//		present_s& present = ps_t::present_cast<present_s>();
		action_s& action = ps_t::template action_cast<action_s>();

		action.dev.mode = mode::idle;
	}
	::robo::time_us_t last_us_ = 0;
	void poll_(void) {
		#if ROBO_APP_ENV_ENABLED ==1
		::robo::time_us_t now_us_ = ::robo::system::env::time_us();
		present_s& present = ps_t::template present_cast<present_s>();
		const config_s& config = ps_t::template config_cast<config_s>();

		switch (present.status) {
		case status_t::fixed:
		if (present.command == command_t::release) {
			present.status = status_t::releasing;
			brake_force();
			last_us_ = now_us_;
		}
		else {
			break;
		}

		case status_t::releasing:
		if (present.command == command_t::set) {
			present.status = status_t::fixing;
			brake_set();
			last_us_ = now_us_;
		}
		else {
			if ((now_us_ - last_us_) >= config.timeout.force_us) {
				present.status = status_t::released;
				brake_normal();
			}
		}
		break;

		case status_t::released:
		if (present.command == command_t::set) {
			present.status = status_t::fixing;
			brake_set();
			last_us_ = now_us_;
		}
		else {
			break;
		}

		case status_t::fixing:
		if ((now_us_ - last_us_) >= config.timeout.relax_us) {
			present.status = status_t::fixed;
		}
		break;
		}
		#endif
	}

	#if BRAKE_CURRENT_MODE_ENABLED == 1 || 	BRAKE_VOLTAGE_MODE_ENABLED==1 
	::mexo::machine::slot::member<dev_t> poller_;
	#endif
public:
	
	dev_t(hardwaresys_t& _hardwaresys, cstr _name, action_s & _action, config_s& _config, present_s& _present, int _slot_index)
		: ps_t(_hardwaresys, _name, _action, _config.ps, _present.ps)
		, itf(_present.command, _present.status)
		#if BRAKE_CURRENT_MODE_ENABLED == 1 || 	BRAKE_VOLTAGE_MODE_ENABLED==1 
		, poller_(_slot_index,*this,&dev_t::poll_)
		#endif
	{
		ps_t::action_disable();
		#if BRAKE_VOLTAGE_MODE_ENABLED == 1  
		_config.voltage.force = BRAKE_PREFIX(VOLTAGE_FORCE);
		_config.voltage.normal = BRAKE_PREFIX(VOLTAGE_NORMAL);
		#endif
		#if BRAKE_CURRENT_MODE_ENABLED == 1  
		_config.current.force = BRAKE_PREFIX(CURRENT_FORCE);
		_config.current.normal = BRAKE_PREFIX(CURRENT_NORMAL);
		#endif
		_config.timeout.relax_us = BRAKE_PREFIX(TIMEOUT_RELAX_US);
		_config.timeout.force_us = BRAKE_PREFIX(TIMEOUT_FORCE_US);
		_config.power_mode = power_mode::BRAKE_PREFIX(POWER_MODE);
	}
protected:
	virtual bool do_reconfig(void) { 
		ps_t::enable();
		::mexo::brake::itf::enable();
		return true; 
	};
	#if ROBO_APP_MEXO_VAR_ENABLED == 1
	void do_create_vars(void) {
		ps_t::do_create_vars();
		const config_s& config = ps_t::template config_cast<config_s>();
		present_s& present = ps_t::template present_cast<present_s>();
		if (::mexo::var::machine::actual_mode() >= ::mexo::var::machine::mode::tuning) {
			::mexo::var::record::create(::mexo::var::uint32, config.timeout.relax_us, RT("tm.relax_us"), key(), vars);
			::mexo::var::record::create(::mexo::var::uint32, config.timeout.force_us, RT("tm.force_us"), key(), vars);
			#if BRAKE_VOLTAGE_MODE_ENABLED == 1  
			::mexo::var::record::create(types::var::signal, config.voltage.force, RT("v.force"), key(), vars);
			::mexo::var::record::create(types::var::signal, config.voltage.normal, RT("v.normal"), key(), vars);
			#endif
			#if BRAKE_CURRENT_MODE_ENABLED == 1  
			::mexo::var::record::create(types::var::signal, config.current.force, RT("c.force"), key(), vars);
			::mexo::var::record::create(types::var::signal, config.current.normal, RT("c.normal"), key(), vars);
			#endif
			::mexo::var::record::create(::mexo::var::uint8, config.power_mode, RT("pm"), key(), vars);
		}
		if (::mexo::var::machine::actual_mode() >= ::mexo::var::machine::mode::full) {
			::mexo::var::record::create(::mexo::var::uint8, present.command, RT("cmd"), key(), vars);
			::mexo::var::record::create(::mexo::var::const_uint8, present.status, RT("stat"), key(), vars);
		}
	}
	#endif
};
}

#define TEMPL_FINISH
#include "mexo/brake.templ.prepare.hpp"
