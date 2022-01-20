#include "mexo-drive.hpp"
#include "core/robosd_system.hpp"
#include <cstdlib>
#include <cstdio>
#include "net/robosd_serial.hpp"
#include "net/robosd_flow.hpp"
#include "net/robosd_flow_id.h"
#include "freemaster/robosd_fm.hpp"
#include "prf/as5048a/as5048a.hpp"
#include "terminal/robosd_termo.hpp"
#include "core/robosd_log.hpp"
#include "core/robosd_ini.hpp"
#include "im/edev/edev.hpp"
#include "net/platform/can/emulator/emu_can.hpp"

namespace mexo_drive {
	static time_us_t period_us_ = 50;
	static time_us_t us_ = 0;
	static time_ms_t ms_ = 0;

	void tick_(void) {
		static time_us_t us_acc_ = 0;
		us_ += period_us_;
		us_acc_ += period_us_;
		if (us_acc_ >= 1000) {
			us_acc_ -= 1000;
			ms_++;
		}
	}

	class agent : public edev::agent {
		robo::net::emu_can::port can_;
	public:
		virtual void do_priotitet_run(double _time){
			tick_();
			::mexo::machine::priority_loop();
			::mexo::machine::backend_loop();
		}
		virtual void do_background_run(double _time){
			::mexo::machine::frontend_loop();
		}
		virtual bool do_begin(void) {
			ROBO_LBREAKN(edev::agent::do_begin());

			ROBO_LBREAKN(robo::ini::load(name,type, RT("CHANNEL"), can_.channel));
			ROBO_LBREAKN(robo::ini::load(name, type, RT("REPEAT_MAX_COUNT"), can_.repeat_max_count));
			can_.open();

			::mexo::machine::begin();
			::mexo::machine::start();
			return true;
		}
		virtual void do_reconfig(void) {
		}
		virtual void do_finish(void) {
			can_.close();
		}
		virtual void set_local_ini(cstr _ini) { system::ini::begin(_ini); }

	} agent_;
}

extern "C" {
	ROBO_EXPORT_RUNTIME robo::edev::agent* ROBO_EXPORT_RUNTIME_DECL query_agent(void) {
		return &mexo_drive::agent_;
	}
}

namespace robo {

	#if ROBO_APP_ENV_TYPE == ROBO_APP_TYPE_SPECIFIC

	void  system::env::abort(void) {
		::abort();
	}



	void* system::env::critical_enter(void) {
		return nullptr;
	}

	void system::env::critical_leave(void* /*context_*/) {}

	system::context current_context_ = system::context::frontend;
	bool locked_ = false;
	bool system::env::is_frontend(void) {
		return current_context_ == context::frontend;
	}

	bool system::env::is_backend(void) {
		return current_context_ == context::backend;
	}

	void* system::env::enter(void) {
		return nullptr;

	}

	void system::env::leave(void* /*_context*/) {
	}


	void system::env::lock(void) {
		ROBO_APP_ASSERT(locked_ == false);
		locked_ = true;
	}
	void system::env::unlock(void) {
		ROBO_APP_ASSERT(locked_ == true);
		locked_ = false;
	}

	void system::env::fall(void) {
		ROBO_APP_ASSERT(current_context_ == context::frontend);
		current_context_ = context::backend;
	}

	void system::env::comeback(void) {
		ROBO_APP_ASSERT(current_context_ == context::backend);
		current_context_ = context::frontend;
	}

	time_us_t system::env::time_us(void) {
		return mexo_drive::us_;
	}

	time_us_t system::env::realtime_us(void) {
		return mexo_drive::us_;
	}

	time_ms_t system::env::time_ms(void) {
		return mexo_drive::ms_;
	}

	random_t system::env::rand(random_t _max) {
		return std::rand()% _max;
	}

	void system::env::wakeup(void) {}

	time_us_t system::env::period_us(void) {
		return mexo_drive::period_us_;
	}

	void system::env::sleep(void) {}

	#if ROBO_APP_PRINT_TYPE == ROBO_APP_TYPE_SPECIFIC	
	void system::env::print(cstr  _s) {
		#if ROBO_APP_TERMINAL_ENABLED == 1
		if (system::env::is_frontend())
			::robo::termo::itf::prints(_s);
		#endif
	}
	#endif

	#if ROBO_APP_ALLOC_TYPE == 	ROBO_APP_TYPE_SPECIFIC
	void* system::env::mem_alloc(size_t _size) {
		return malloc(_size);
	}
	void system::env::mem_free(void* _memo) {
		free(_memo);
	}
	#endif
	#endif
}
#include "mexo/mexo.hpp"
uint32_t mexo_drive::current_adc_driver::sence[2] = { 0,0 };

namespace mexo_drive {
	const int MAX_PWM = 3900;

	void pwm_driver::boot_complete(types::discret_t _duty) {
	}

	void pwm_driver::shutdown_begin(void) {
	}

	void pwm_driver::do_run(types::discret_t _duty) {
		if (_duty > 0) {
		}
		else {
		}
	}
		
	perephery_config_s perephery_config =
	{
		{
			{
				{0}
				, 231
				, 10
			}
			,{
				{
					-MAX_PWM
					,MAX_PWM
				}
				,{
					-32767
					,32767
				}
			}
		}
		,{
			{0}
			,{0,1} //index
			,{1, 1} //scale
			,10	// init_count_shift -1024 точек
		}
		,{
			{0}
			, 1
			, 2
		}
		,{
			{0}
			, -1
			, 0
		}
	};

	void  flow_set_addr(uint8_t _addr) {
		//volatile uint32_t tmp = rdk_store_array[0];
		if (_addr > 0 && _addr < 16) {
		}
	}

	class can_port_driver {
	public:
		static inline cstr path = can0_PATH;
		typedef flow_msg_can_id_t id_t;
		enum { suba_count = 16, packet_size = 8, msg_pool_size = 4 };
		static void send(unsigned _id, const uint8_t* _data, size_t _size);
	};

	typedef ::robo::net::flow::port_t<can_port_driver> can0_t;
	can0_t can0;
	void can_port_driver::send(unsigned _id, const uint8_t* _data, size_t _size) {
		if (_size > 0) {
		}
	}

	static volatile uint16_t enco_native_ = 0;

	void as5048_driver::cs_low(void) {
	}
	void as5048_driver::cs_hi(void) {
	}
	void as5048_driver::put(uint16_t& _commamd) {
	}
	void as5048_driver::get(uint16_t& _answer) {
	}

	uint16_t as5040_driver::native(void) {
		return 9;
	}

}
/*	
mexo_drive::can0.on_receive(RxHeader.StdId, RxData, RxHeader.DLC);
*/


namespace mexo {
	machine::slot::simple start(
		machine::slot::kind::start
		, [] {
			mexo_drive::flow_set_addr(0xA);
		}
	);
	::mexo::machine::slot::simple enco_put(
		0
		, &mexo_drive::AS5048A::put
	);

	machine::slot::simple enco_get(
		2
		, &mexo_drive::AS5048A::get
	);
}


