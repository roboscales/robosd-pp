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
		, const config_s& _config
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
		ref_.attach_to(board::devs_());
	}


	dev::mode::mode(int _id, dev & _dev):ref_(*this, _id), dev_(_dev) {
		ROBO_ASSERT(ref_.attach_to(dev_.modes_));
	}

	void dev::raise_panic(uint8_t _flag) {
		uint32_t mask = (1 << _flag);
		if ((present_.panic & mask) == 0) {
			present_.panic |= mask;
			perform_panic();
			board::instance_. present_.last_panic_us = time_us();				
		}
	}

	void dev::reset_panic(uint8_t _flag) {
		present_.panic &= ~(1 << _flag);
	}
	
	
	void dev::reset_panics(uint32_t _mask) {
		present_.panic &= ~(_mask);
	}

	#if BURST_PANICS_MASTER_LOST_ENABLED == 1
	void dev::master_alive(void) {
		if (config_.alive_period_us > 0) {
			present_.master_alive_tm = time_us();
			present_.master_exists = true;
			reset_panic(front::dev::panics::bits::master_lost);
		}
	}
	#endif

	#if BURST_PROTECTION_ENABLED == 1
	void dev::frontend_protection() {
		#if BURST_PANICS_MASTER_LOST_ENABLED == 1
		if (present_.master_exists) {
			time_us_t av = present_.master_alive_tm;
			if (time_us() - av > config_.alive_period_us) {
				present_.master_exists = false;
				raise_panic(front::dev::panics::bits::master_lost);
			}
		}
		#endif
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
	#if BURST_DEBUG_TP_ENABLED == 1
	tp_t tp(burst_present.tp_verb);
	#endif
	
	board::~board(void) {
	}
	void board::begin_(time_us_t _period_us) {
		#if ROBO_APP_SYSTEM_ENABLED
		::robo::system::begin();		
		#endif
		{
			guard__;
			burst_present.status= front::board::statuses::startuped;
			slots_ref_.begin.execute();
			ROBO_ASSERT(config_);
			//ROBO_APP_ASSERT(::mexo::node::begin());
			#if ROBO_APP_NET_FLOW_ENABLED == 1
			::robo::net::flow::machine::begin();
			#endif
			
			#if ROBO_APP_BURST_VARTREE_ENABLED
			regvar_conf();
			#endif
			#if ROBO_APP_SYSTEM_ENABLED
			::robo::system::start(_period_us);
			#endif
			raise_panic_(front::board::panics::bits::config);
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
		#if ROBO_APP_BURST_REALTIME_SLOT_ENABLE != 1
		realtime_loop_();
		#endif
		fall__;
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
		debug_tp_on(front::tp_verb::frontend);
		switch ( present_.status ){
			case front::board::statuses::startuped:
			{
				slots_ref_.startup.execute();
				if(slots_ref_.startup.isempty()){
					if(config_->wait_config_on_startup){
						present_.status = front::board::statuses::configure ;
						present_.command = front::board::commands::configure ;
					} else{
						reset_panic_(front::board::panics::bits::config);
						present_.status = front::board::statuses::ready ;
						slots_ref_.ready.execute();
					}
				} 
				break;
			}
			case front::board::statuses::restart:
			{
				slots_ref_.startup.execute();
				if(slots_ref_.startup.isempty()){
					reset_panic_(front::board::panics::bits::config);
					present_.status = front::board::statuses::ready ;
					slots_ref_.ready.execute();
				} 
				break;
			}
			
			case front::board::statuses::configure:
				if(present_.command != front::board::commands::configure){
					if(reconfig_query_count_>0){
						reconfig_query_count_ = 0;
						slots_ref_.reconfig.execute();
						while(slots_ref_.startup_holder.delegats_.count()){
							auto * tmp = slots_ref_.startup_holder.delegats_.pop();
							ROBO_APP_ASSERT(tmp);
							tmp->links_.first()->value().attach_to(slots_().startup_holder.delegats_);
						}
						present_.status = front::board::statuses::restart ;						
					} else {
						reset_panic_(front::board::panics::bits::config);
						present_.status = front::board::statuses::ready ;
						slots_ref_.ready.execute();
					}
				}
				break;
			case front::board::statuses::ready:
				if(present_.command == front::board::commands::configure){
					raise_panic_(front::board::panics::bits::config);
					present_.status = front::board::statuses::configure ;
				}
				break;
			case front::board::statuses::unknown:
				ROBO_APP_CRASH();							
		}
		#if ROBO_APP_SYSTEM_ENABLED
		::robo::system::frontend_loop();
		#endif
		slots_ref_.frontend.execute();
		#if ROBO_APP_NET_FLOW_ENABLED == 1
		::robo::net::flow::machine::frontend_poll();
		#endif
		#if BURST_PROTECTION_ENABLED == 1
		frontend_protection_();
		#endif
		debug_tp_off(front::tp_verb::frontend);
	}
	

	void board::handle_panic_(void) {
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

		case slot::kind::ready:
		return ready;

		case slot::kind::reconfig:
		return reconfig;

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
		ready.free();
		reconfig.free();
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
	/*
	void board::reset_(void) {
		for (dev::ref* p = devs_ref_.first(); p; p = p->next()) {
			p->owner().reset();
		}
	}
	*/
	void dev::switch_to_idle_(void) {
		switch_to_mode(front::dev::modes::idle);
	}
	void board::slot::delegat::finish_(slot::delegat & _delegat){
		board::finish_(_delegat);
	}

	void board::finish_(slot::delegat & _delegat){
		if(_delegat.links_.count() == 1){
			if(board::instance_.slots_ref_.startup.delegats_.contains(_delegat.links_.first()->value().owner()) ){
				_delegat.links_.first()->value().attach_to(slots_().startup_holder.delegats_);
			} else {
				_delegat.dettach();
			}
		}
	}


	void dev::switch_to_mode(int _mode_id) {
		ROBO_APP_ASSERT(is_backend__);
		if (_mode_id != present_.mode) {
			if (present_.mode != front::dev::modes::idle) {
				actual_mode_->stop();
			}

			if (_mode_id == front::dev::modes::idle) {
				actual_mode_ = nullptr;
				present_.mode = front::dev::modes::idle;
			}
			else {
				mode* m = modes_.find(_mode_id);
				if (m == nullptr ) {
					actual_mode_ = nullptr;
					present_.mode = front::dev::modes::idle;
				}
				else {
					actual_mode_ = m;
					present_.mode = _mode_id;
					m->start();
					if (action_enabled_) {
						m->applay_action();
					}
					action_.action_actual = false;
				}
			}
		}
	}

	void dev::perform_panic(void) {
		if (present_.mode != front::dev::modes::idle) {
			guard__;
			if(is_backend__){
				switch_to_idle_(); 
			}
		}
		action_.mode = front::dev::modes::idle;
	}

	void dev::loopA(void) {

		ROBO_APP_ASSERT(is_backend__);

		if (action_.mode != present_.mode) {
			guard__;
			if (present_.panic != 0) {
				action_.mode = front::dev::modes::idle;
				switch_to_idle_();
			}
			else {
				switch_to_mode(action_.mode);
			}
		}
		if (action_enabled_) {
			if (action_.action_actual) {
				guard__;
				action_.action_actual = false;
				if( actual_mode_ ){
					actual_mode_->applay_action();
				}
			}
		}
		#if 0
		if (wait_feedback_) {
			do_update_feedback();
			wait_feedback_ = false;
		}
		#endif
		
		if (actual_mode_) {
			actual_mode_->loopA();
		}
	}
	void dev::loopB(void) {
		if (actual_mode_) {
			actual_mode_->loopB();
		}
	}
	void dev::loopC(void) {
		if (actual_mode_) {
			actual_mode_->loopC();
		}
	}
	void dev::frontend_loop(void) {
		if (actual_mode_) {
			actual_mode_->frontend_loop();
		}
	}

	/*
	void board::perform_panic(void) {
		for (dev::ref* p = devs_ref_.first(); p; p = p->next()) {
			p->owner().perform_panic();
		}
	}
	*/
	void board::raise_panic_(uint32_t _flag) {
		uint32_t mask = (1 << _flag);
		if ((present_.panics & mask) == 0) {
			present_.last_panic_us = time_us();
			present_.panics |= mask;
			for (dev::ref* p = devs_ref_.first(); p; p = p->next()) {
				p->owner().raise_panic(front::dev::panics::bits::board);
			}
		}
	}

	void board::reset_panic_(uint32_t _flag) {
		uint32_t mask = (1 << _flag);
		if ((present_.panics & mask) != 0) {
			present_.last_panic_us = time_us();
			present_.panics &= ~mask;
		}
		if (present_.panics == 0) {
			for (dev::ref* p = devs_ref_.first(); p; p = p->next()) {
				p->owner().reset_panic(front::dev::panics::bits::board);
			}
		}
	}

	uint32_t board::panics(void) {
		return present_.panics;
	}

	void board::reset_panics_(void) {
		present_.panics = 0;
		for (dev::ref* p = devs_ref_.first(); p; p = p->next()) {
			p->owner().present_.panic = 0;
		}
	}
#if BURST_PROTECTION_ENABLED
	void board::realtime_protection_(void) {
		#if BURST_PANICS_BOARD_VOLTAGE_ENABLED 
		int burst_board_voltage = voltage_get_pp();
		if (burst_board_voltage >= config_->panics.overvoltage_pp) {
			raise_panic(front::board::panics::bits::overvoltage);
		}
		else if (burst_board_voltage <= config_->panics.lovoltage_pp) {
			raise_panic(front::board::panics::bits::lovoltage);
		}
		#endif

		#if BURST_PANICS_BOARD_CURRENT_ENABLED 
		int burst_board_current = current_get_pp();
		if (burst_board_current >= config_->panics.overcurrent) {
			raise_panic(front::board::panics::bits::overcurrent);
		}
		else if (burst_board_current <= config_->panics.locurrent_pp) {
			raise_panic(front::board::panics::bits::locurrent);
		}
		#endif	
		
	}

	void board::frontend_protection_(void) {
		#if BURST_PANICS_BOARD_TEMPER_ENABLED == 1
		int burst_board_temper_hi = temper_get_hi_pp();
		int burst_board_temper_lo = temper_get_lo_pp();
		if (burst_board_temper_hi >= config_->panics.temp_pp.overhi) {
			raise_panic_(front::board::panics::bits::overtemp);
		}
		else if (burst_board_temper_hi < config_->panics.temp_pp.hi) {
			if (present_.panics) {
				reset_panic_(front::board::panics::bits::overtemp);
			}
		}

		if (burst_board_temper_lo <= config_->panics.temp_pp.ultralo) {
			raise_panic_(front::board::panics::bits::lotemp);
		}
		else if (burst_board_temper_lo > config_->panics.temp_pp.lo) {
			if (present_.panics) {
				reset_panic_(front::board::panics::bits::lotemp);
			}
		}
		#endif
		if ( config_->panics.reset_timeout_us) {
			if (time_us() - present_.last_panic_us > config_->panics.reset_timeout_us) {
				if(present_.panics){
					uint32_t mask = present_.panics;
					mask &= ~( front::board::panics::masks::overtemp | front::board::panics::masks::lotemp | front::board::panics::masks::config );
					present_.panics &= ~(mask);
					if (present_.panics == 0) {
						for (dev::ref* p = devs_ref_.first(); p; p = p->next()) {
							p->owner().reset_panic(front::dev::panics::bits::board);
						}
					}
				}
				for (dev::ref* p = devs_ref_.first(); p; p = p->next()) {
					auto & d = p->owner();
					uint32_t mask = d.present_.panic;
					if(mask){
						mask &= ~(d.noreset_panic_mask);
						d.reset_panics(mask);
					}
				}			
			}						
		}
		for (dev::ref* p = devs_ref_.first(); p; p = p->next()) {
			p->owner().frontend_protection();
		}
	}
#endif
	#if ROBO_APP_BURST_VARTREE_ENABLED == 1
	void board::regvar_conf(void) {
		using namespace burst::var;
		if (actual_mode >= mode::tuning) {
			push(RT("board"));
			reg(types::uint8, (burst_present.command), RT("cmd"));
			reg(types::uint8, (burst_present.status), RT("status"));
			push(RT("cfg"));
			reg(types::int32, (instance_.config_->vercion), RT("ver"));
			push(RT("panics"));

			#if BURST_PROTECTION_ENABLED == 1
			reg(types::time_us, instance_.config_->panics.reset_timeout_us, RT("reset_tm_us"));
			#if BURST_PANICS_BOARD_TEMPER_ENABLED == 1 
			push(RT("temper"));
			reg(types::int16, (instance_.config_->panics.temp_pp.overhi), RT("overhi"));
			reg(types::int16, (instance_.config_->panics.temp_pp.hi), RT("hi"));
			reg(types::int16, (instance_.config_->panics.temp_pp.lo), RT("lo"));
			reg(types::int16, (instance_.config_->panics.temp_pp.ultralo), RT("ultralo"));
			pop();
			#endif
			#if BURST_PANICS_BOARD_VOLTAGE_ENABLED == 1 
			reg(types::int16, (instance_.config_->panics.overvoltage_pp), RT("overvolt"));
			reg(types::int16, (instance_.config_->panics.lovoltage_pp), RT("lovolt"));
			#endif
			#endif
			#if BURST_PANICS_BOARD_CURRENT_ENABLED == 1 
			reg(types::int16, (instance_.config_->panics.overcurrent), RT("overcur"));
			reg(types::int16, (instance_.config_->panics.locurrent_pp), RT("locur"));
			#endif

			pop();
			pop();
			pop();
		}

	}
	#endif
	#if ROBO_APP_BURST_VARTREE_ENABLED
	void dev::regvar_present(robo::cstr _name) {
		using namespace burst::var;
		DEV_PRESENT_S(p);
		if (actual_mode >= burst::var::mode::action) {
			push(_name);
			{
				if (actual_mode >= burst::var::mode::full) {
					reg(types::const_uint32, p.mode, RT("mode"));
					reg(types::uint32, p.panic, RT("panic"));
					#if BURST_PANICS_MASTER_LOST_ENABLED == 1
					reg(types::const_time_us, p.master_alive_tm, RT("master_alive_tm"));
					reg(types::const_uint8, p.master_exists, RT("master_exists"));
					#endif
				}
			} pop();
		}
	}
	void dev::regvar_action(robo::cstr _name) {
		using namespace burst::var;
		DEV_ACTION_S(a);
		if (actual_mode >= burst::var::mode::action) {
			push(_name);
			{
				reg(types::uint8, a.action_actual, RT("action_actual"));
				reg(types::const_uint32, a.mode, RT("mode"));
			} pop();
		}
	}
	void dev::regvar_conf(robo::cstr _name) {
		DEV_CONFIG_S(c);
		using namespace burst::var;
		if (actual_mode >= burst::var::mode::tuning) {
			push(_name);
			{
				#if BURST_PANICS_MASTER_LOST_ENABLED == 1
				reg(types::time_us, c.alive_period_us, RT("alive_period_us"));
				#endif
			} pop();
		}
	}
	#endif

}

#include "burst++/burst.h"
#ifdef ROBO_APP_BURST_SAMPLE_US
void burst_begin(void) {
	burst::board::begin();
}
#else
void burst_begin_ps(unsigned int _period_us) {
	burst::board::begin(_period_us);
}
#endif
#if ROBO_APP_BURST_REALTIME_SLOT_ENABLE
void burst_realtime_loop(void) {
	burst::board::realtime_loop();
}
#endif
void burst_backend_loop(void) {
	burst::board::backend_loop();
}
void burst_frontend_loop() {
	burst::board::frontend_loop();
}

#if ROBO_APP_SYSTEM_ENABLED
void robo::system::env::abort(void) {
	
	burst::board::instance_.handle_panic_();
	burst::board::abort();
}
#endif

extern "C" void burst_core_crash_(const char * _function, const char * _file, int _line){
	robo::crash(_function,_file,_line);
}

