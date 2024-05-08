#include "burst++/burst.hpp"
#include "core/robosd_ring_buf.hpp"
#include "core/robosd_system.hpp"
#include "core/robosd_log.hpp"
#include "core/robosd_ring_safe_buf.hpp"
#if ROBO_APP_NET_FLOW_ENABLED == 1
#include "net/robosd_flow.hpp"
#endif

namespace burst {
	robo::safe_ring_t<guard_t, BURST_BACKEND_QUEUE_SIZE_BITS, request*> backend_queue;
	robo::safe_ring_t<guard_t, BURST_FRONT_QUEUE_SIZE_BITS, request*> frontend_queue;

	void request::post(void) {
		status_ = request::statuses::query;
		if (is_frontend__) {
			ROBO_ASSERT(frontend_queue.try_put(this));
		}
		else {
			ROBO_ASSERT(backend_queue.try_put(this));
		}
	}


	void request::front_perfom_(void) {
		if (status_ == statuses::query) {
			if (query)
				(*query)();
			if (confirm) {
				status_ = statuses::confirm;
				ROBO_ASSERT(backend_queue.try_put(this))
			}
			else {
				status_ = statuses::none;
			}
		}
		else if (status_ == statuses::confirm) {
			(*confirm)();
			status_ = statuses::none;		
		}
	}

	void request::front_perfom(void) {
		request* r = nullptr;
		if (frontend_queue.try_get(&r)) {
			ROBO_ASSERT(r);
			r->front_perfom_();
		}
	}

	void request::backend_perfom_(void) {
		if (status_ == statuses::query) {
			if (query)
				(*query)();
			if (confirm) {
				status_ = statuses::confirm;
				ROBO_ASSERT(frontend_queue.try_put(this))
			}
			else {
				status_ = statuses::none;
			}
		}
		else if (status_ == statuses::confirm) {
			(*confirm)();
			status_ = statuses::none;
		}
	}

	void request::backend_perfom(void) {
		request* r = nullptr;
		if (backend_queue.try_get(&r)) {
			ROBO_ASSERT(r);
			r->backend_perfom_();
		}
	}

	dev::dev(
		int _dev_id
		, config_s& _config
		, present_s& _present
		, action_s& _action
		, feedback_s& _feedback
	) :
		ref_(*this, _dev_id)
		, config_(_config)
		, present_(_present)
		, action_(_action) 
		, feedback_(_feedback)
	{
	}


	dev::mode::mode(int _id, dev & _dev):ref_(*this, _id), dev_(_dev) {
		ref_.attach_to(dev_.modes_);
	}

	void dev::raise_panic(uint32_t _flag) {
		uint32_t mask = (1 << _flag);
		if ((present_.panic & mask) == 0) {
			present_.panic |= mask;
			perform_panic();
		}
	}

	void dev::reset_panic(uint32_t _flag) {
		present_.panic &= ~(1 << _flag);
	}

	#if BURST_PANICS_MASTER_LOST_ENABLED == 1
	void dev::master_alive(void) {
		if (config_.alive_period_us > 0) {
			present_.master_alive_tm = time_us();
			present_.master_exists = true;
			reset_panic(front::panics::dev::bits::master_lost);
		}
	}
	#endif

	board board::instance_;
	board::present_s burst_present;
//	board::config_s burst_config = BURST_CONFIG();
	board::board(void)
		: slots_ref_(slots_())
		, devs_ref_(devs_())
		, slot_index_(0) 
		, present_(burst_present)
		, config_(nullptr)
	{
	}
	board::~board(void) {
	}
	void board::begin_(time_us_t _period_us) {
		#if ROBO_APP_SYSTEM_ENABLED
		::robo::system::begin();
		#endif
		{
			guard__;
			slots_ref_.begin.execute();
			//ROBO_APP_ASSERT(::mexo::node::begin());
			#if ROBO_APP_NET_FLOW_ENABLED == 1
			::robo::net::flow::machine::begin();
			#endif
			#if ROBO_APP_MEXO_VAR_ENABLED == 1
			node::create_vars();
			#endif
			#if ROBO_APP_SYSTEM_ENABLED
			varreg();
			::robo::system::start(_period_us);
			#endif
			ROBO_ASSERT(config_);
			slots_ref_.start.execute();
		}
	}

	/*
	void machine::start_(time_us_t _period_us) {
		slots_ref_.start.execute();
		system::start(_period_us);
	}
	*/
	void board::realtime_loop_(void) {
		debug_tp_on(front::tp_verb::loop);
		debug_tp_on(front::tp_verb::realtime);
		fall__;
		slots_ref_.realtime.execute();
		debug_tp_off(front::tp_verb::realtime);
	}

	void board::backend_loop_(void) {
		debug_tp_on(front::tp_verb::backend);
		fall__;
		#if ROBO_APP_BURST_REALTIME_SLOT_ENABLE != 1
		realtime_loop_();
		#endif
		#if ROBO_APP_SYSTEM_ENABLED
		::robo::system::backend_loop();
		#endif
		slots_ref_.periodic[slot_index_].execute();
		//slots_ref_.control.execute();
		slots_ref_.backend.execute();
		slot_index_++;
		if (slot_index_ == slot_count) {
			slot_index_ = 0;
		}
		#if ROBO_APP_NET_FLOW_ENABLED == 1
		::robo::net::flow::machine::backend_poll();
		#endif
		debug_tp_off(front::tp_verb::backend);
		debug_tp_off(front::tp_verb::loop);
	}
	void board::frontend_loop_(void) {
		if(!startuped_){
			slots_ref_.startup.execute();
			startuped_ = slots_ref_.startup.isempty();
		}

		debug_tp_on(front::tp_verb::frontend);
		#if ROBO_APP_SYSTEM_ENABLED
		::robo::system::frontend_loop();
		#endif
		slots_ref_.frontend.execute();
		#if ROBO_APP_NET_FLOW_ENABLED == 1
		::robo::net::flow::machine::frontend_poll();
		#endif
		debug_tp_off(front::tp_verb::frontend);
	}
	void board::raise_fault_(void) {
		slots_ref_.raise_fault.execute();
	}
	board::slots& board::slots_(void) {
		static board::slots slots__;
		return slots__;
	}
	dev::map & board::devs_(void) {
		static dev::map devs__;
		return devs__;
	}
	board::slot& board::slots::operator [] (board::slot::kind _kind) {
		switch (_kind) {
		case slot::kind::begin:
		return begin;

		case slot::kind::start:
		return start;

		case slot::kind::startup:
		return startup;

		case slot::kind::realtime:
		return realtime;

		case slot::kind::backend:
		return backend;

		case slot::kind::frontend:
		return frontend;

		case slot::kind::raise_fault:
		return raise_fault;

		default:
		ROBO_APP_CRASH();
		return dummy;
		}
	}
	void  board::slots::free(void) {
		begin.free();
		start.free();
		startup.free();
		realtime.free();
		//control.free();
		backend.free();
		frontend.free();
		raise_fault.free();
		dummy.free();
		for (int i = 0; i < slot_count; i++) {
			periodic[i].free();
		}
	}

	board::slot& board::slots::operator [] (int _index) {
		if (((_index >= 0) && (_index < slot_count))) {
			return periodic[_index];
		}
		else {
			ROBO_APP_CRASH();
			return dummy;
		}
	}

	board::slot::slot(void) {}

	board::slot::~slot(void) {}

	void board::slot::execute(void) {

		for (delegat::ref* r = delegats_.first(); r; ) {
			auto tmp = r;
			r = r->next();
			tmp->owner()();
		}
	}

	void board::slot::free(void) {
		delegat::ref* r;
		while ((r = delegats_.first()) != nullptr) {
			delete r;
		}
	}

	void board::slot::delegat::attach(slot::kind _kind, delegat* _prev) {
		#if ROBO_APP_ALLOC_TYPE == ROBO_APP_TYPE_NONE
		ROBO_APP_ASSERT(is_frontend__);
		#endif
		auto & r = links_.push(*this);
		guard__;
		r.attach_after(board::slots_()[_kind].delegats_, _prev);
	}

	void board::slot::delegat::dettach(slot::kind _kind) {
		#if ROBO_APP_ALLOC_TYPE == ROBO_APP_TYPE_NONE
		ROBO_APP_ASSERT(is_frontend__);
		#endif
		guard__;
		ref* r = board::slots_()[_kind].locate(this);
		if (r) {
			links_.remove(r);
		}
	}


	void board::slot::delegat::attach(int _index, delegat* _prev) {
		#if ROBO_APP_ALLOC_TYPE == ROBO_APP_TYPE_NONE
		ROBO_APP_ASSERT(is_frontend__);
		#endif
		auto & r = links_.push( *this);
		guard__;
		r.attach_after(board::slots_()[_index].delegats_, _prev);
	}

	void board::slot::delegat::dettach(int _index) {
		#if ROBO_APP_ALLOC_TYPE == ROBO_APP_TYPE_NONE
		ROBO_APP_ASSERT(is_frontend__);
		#endif
		guard__;
		ref* r = board::slots_()[_index].locate(this);
		if (r) {
			links_.remove(r);
		}
	}

	void board::slot::delegat::attach(ref& _ref, int _index, delegat* _prev) {
		guard__;
		_ref.attach_after(board::slots_()[_index].delegats_, _prev);
	}


	void board::slot::delegat::attach(ref& _ref, slot::kind _kind, delegat* _prev) {
		guard__;
		_ref.attach_after(board::slots_()[_kind].delegats_, _prev);
	}

	void board::reset_(void) {
		for (dev::ref* p = devs_ref_.first(); p; p = p->next()) {
			p->owner().reset();
		}
	}
	
	void dev::switch_to_idle_(void) {
		actual_mode_ = nullptr;
		present_.mode = front::dev::modes::idle;
	}

	void dev::perform_panic(void) {
		if (present_.mode != front::dev::modes::idle) {
			actual_mode_->stop();
			switch_to_idle_();
			on_perform_panic();
		}
		action_.mode = front::dev::modes::idle;
	}
	/*
	void board::perform_panic(void) {
		for (dev::ref* p = devs_ref_.first(); p; p = p->next()) {
			p->owner().perform_panic();
		}
	}
	*/
	void board::raise_panic(uint32_t _flag) {
		uint32_t mask = (1 << _flag);
		if ((present_.panics & mask) == 0) {
			present_.last_panic_us = time_us();
			present_.panics |= mask;
			for (dev::ref* p = devs_ref_.first(); p; p = p->next()) {
				p->owner().raise_panic(front::panics::dev::bits::board);
			}
		}
	}

	void board::reset_panic(uint32_t _flag) {
		uint32_t mask = (1 << _flag);
		if ((present_.panics & mask) == 0) {
			present_.last_panic_us = time_us();
			present_.panics |= mask;
		}
		if (present_.panics == 0) {
			for (dev::ref* p = devs_ref_.first(); p; p = p->next()) {
				p->owner().reset_panic(front::panics::dev::bits::board);
			}
		}
	}

	uint32_t board::panics(void) {
		return present_.panics;
	}

	void board::reset_panics(void) {
		present_.panics = 0;
		for (dev::ref* p = devs_ref_.first(); p; p = p->next()) {
			p->owner().present_.panic = 0;
		}
	}
#if BURST_PROTECTION_ENABLED
	void board::realtime_protection(void) {
		#if BURST_PANICS_BOARD_VOLTAGE_ENABLED 
		int burst_board_voltage = voltage_get_pp();
		if (burst_board_voltage >= config_->panics.overvoltage_pp) {
			raise_panic(front::panics::board::bits::overvoltage);
		}
		else if (burst_board_voltage <= config_->panics.lovoltage_pp) {
			raise_panic(front::panics::board::bits::lovoltage);
		}
		#endif

		#if BURST_PANICS_BOARD_CURRENT_ENABLED 
		int burst_board_current = current_get_pp();
		if (burst_board_current >= config_->panics.overcurrent_pp) {
			raise_panic(front::panics::board::bits::overcurrent);
		}
		else if (burst_board_current <= config_->panics.locurrent_pp) {
			raise_panic(front::panics::board::bits::locurrent);
		}
		#endif	
	}

	void board::frontend_protection(void) {
		#if BURST_PANICS_BOARD_TEMPER_ENABLED == 1
		int burst_board_temper_hi = temper_get_hi_pp();
		int burst_board_temper_lo = temper_get_lo_pp();
		if (burst_board_temper_hi >= config_->panics.temp_pp.overhi) {
			raise_panic(front::panics::board::bits::overtemp);
		}
		else if (burst_board_temper_hi < config_->panics.temp_pp.hi) {
			if (present_.panics) {
				reset_panic(front::panics::board::bits::overtemp);
			}
		}

		if (burst_board_temper_lo <= config_->panics.temp_pp.ultralo) {
			raise_panic(front::panics::board::bits::lotemp);
		}
		else if (burst_board_temper_lo > config_->panics.temp_pp.lo) {
			if (present_.panics) {
				reset_panic(front::panics::board::bits::lotemp);
			}
		}
		#endif
		if (present_.panics && config_->panics.reset_timeout_us) {
			if (time_us() - present_.last_panic_us > config_->panics.reset_timeout_us) {
				uint32_t mask = present_.panics;
				mask &= ~(front::panics::board::masks::overtemp | front::panics::board::masks::lotemp);
				present_.panics &= ~(mask);
				if (present_.panics == 0) {
					for (dev::ref* p = devs_ref_.first(); p; p = p->next()) {
						p->owner().reset_panic(front::panics::dev::bits::board);
					}
				}
			}
		}
	}
#endif
	#if BURST_VAR_ENABLED == 1
	void board::varreg(void) {
		using namespace burst::var;
		push("board");
		push("cfg");
		reg(types::int32, (instance_.config_->vercion), "ver");
		push("panics");

		#if BURST_PROTECTION_ENABLED == 1
		reg(types::uint32, (instance_.config_->panics.reset_timeout_us), "reset_tm_us");
		#if BURST_PANICS_BOARD_TEMPER_ENABLED == 1 
		push("temper");
		reg(types::int16, (instance_.config_->panics.temp_pp.overhi), "overhi");
		reg(types::int16, (instance_.config_->panics.temp_pp.hi), "hi");
		reg(types::int16, (instance_.config_->panics.temp_pp.lo), "lo");
		reg(types::int16, (instance_.config_->panics.temp_pp.ultralo), "ultralo");
		pop();
		#endif
		#if BURST_PANICS_BOARD_VOLTAGE_ENABLED == 1 
		reg(types::int16, (instance_.config_->panics.overvoltage_pp), "overvolt");
		reg(types::int16, (instance_.config_->panics.lovoltage_pp), "lovolt");
		#endif
		#endif
		#if BURST_PANICS_BOARD_CURRENT_ENABLED == 1 
		reg(types::int16, (instance_.config_->panics.overcurrent_pp), "overcur");
		reg(types::int16, (instance_.config_->panics.locurrent_pp), "locur");
		#endif

		pop();
		pop();
		pop();

		for (dev::ref* p = instance_.devs_ref_.first(); p; p = p->next()) {
			p->owner().varreg();
		}
	}
	#endif

}

#include "burst++/burst.h"
#ifdef ROBO_APP_BURST_SAMPLE_US
void burst_begin(void) {
	burst::board::begin(ROBO_APP_BURST_SAMPLE_US);
}
#endif
void burst_begin_ps(unsigned int _period_us) {
	burst::board::begin(_period_us);
}
void burst_realtime_loop(void) {
	burst::board::realtime_loop();
}
void burst_backend_loop(void) {
	burst::board::backend_loop();
}
void burst_frontend_loop() {
	burst::board::frontend_loop();
}
void burst_raise_fault() {
	burst::board::raise_fault();
}

void robo::crash(char const* _fun, char const* _file, int _line) {
	#if ROBO_APP_DEBUG_LOG_ENABLED
	::robo::log::print(robo::log::verb::error, robo::log::mask::disabled, RT(" abort \r\n\t%s\r\n\t%s - %d"), _fun, _file, _line);
	#endif
	burst_raise_fault();
	abort();
}



