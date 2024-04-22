#include "burst++/burst.hpp"
#include "core/robosd_ring_buf.hpp"
#include "core/robosd_system.hpp"
#include "core/robosd_log.hpp"
#include "core/robosd_ring_safe_buf.hpp"

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
};

#include "mexo/mexo.h"
/*
void mexo_begin(void) {
	mexo::machine::begin();
}
#ifdef ROBO_APP_MEXO_SAMPLE_US
void mexo_start( void ) {
	mexo::machine::start(ROBO_APP_MEXO_SAMPLE_US);
}
#endif
void mexo_start_ps(unsigned int _period_us) {
	mexo::machine::start(_period_us);
}
*/
#include "burst++/burst.h"
#ifdef ROBO_APP_MEXO_SAMPLE_US
void board_begin(void) {
	board::machine::begin(ROBO_APP_MEXO_SAMPLE_US);
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

#if 0


#ifndef BURST_DEV_COUNT
#define BURST_DEV_COUNT 0
#endif

struct burst_s{
	#if BURST_DEV_COUNT == 0
	burst_dev_ref_p * devs;
	#else	
	burst_dev_ref_p devs[BURST_DEV_COUNT];
	#endif
	burst_dev_ref_p * devs_end;
	int dev_count;
	uint32_t panics;
	burst_time_us_t last_panic_us;
};

burst_t burst = BURST_EMPTY_STRUCT;

uint32_t burst_core_panics(void){
	return burst.panics;
}
void burst_core_reset_panics(void){
	burst.panics = 0;
}
void burst_dev_idle_event(burst_dev_ref_p _dev){
		BURST_UNUSED(_dev);
}

void burst_dev_attach(burst_dev_ref_p _ref){
	burst_alarm(burst.dev_count<BURST_DEV_COUNT);
	burst.devs[ burst.dev_count ] = _ref;
	burst.dev_count ++ ;
}

burst_dev_mode_t burst_idle_mode ={
	&burst_dev_idle_event
	,&burst_dev_idle_event
	,&burst_dev_idle_event
	,&burst_dev_idle_event
	,&burst_dev_idle_event
	,&burst_dev_idle_event
	,&burst_dev_idle_event
};

void burst_dev_runA(burst_dev_ref_p _ref){
	_ref->actual_mode->loopA(_ref);
}
void burst_dev_runB(burst_dev_ref_p _ref){
	_ref->actual_mode->loopB(_ref);
}
void burst_dev_runC(burst_dev_ref_p _ref){
	_ref->actual_mode->loopC(_ref);
}
typedef enum { burst_core_unknown=0,burst_core_backend=1,burst_core_frontend=2} burst_core_status;
burst_core_status burst_core_status_ = burst_core_unknown;


#if BURST_PANICS_MASTER_LOST_ENABLED == 1
void burst_master_alive(burst_dev_ref_p _ref){
	if(_ref->config->alive_period_us>0){
		_ref->master_alive_tm = burst_time_us();
		_ref->master_exists = burst_true;
		burst_dev_reset_panic(_ref,burst_panic_dev_master_lost_bit);
	}
}


void burst_master_alive_check(burst_dev_ref_p _ref){
	if(_ref->master_exists){
		burst_time_us_t av = _ref->master_alive_tm;
		if(  burst_time_us() - av > _ref->config->alive_period_us) {
			_ref->master_exists = burst_false;
			burst_dev_raise_panic(_ref,burst_panic_dev_master_lost_bit);
		}
	}
}
#endif

#if BURST_QUEUE_ENABLED == 1
void burst_dev_feedback_query (bust_request_t * _req){
	burst_dev_request_t * r = (burst_dev_request_t *)_req;
	r->owner->update_feedback.on_run(r->owner);
}

void burst_dev_request_confirm (bust_request_t * _req){
	burst_dev_request_t * r = (burst_dev_request_t *)_req;
	r->on_complete(r->owner);
}
#endif

void burst_begin(void){
	burst_sw_begin();
	burst_dev_ref_p * p ;	
	burst.devs_end = burst.devs+BURST_DEV_COUNT;
	
	for( p= burst.devs; p!=burst.devs_end;p++){		
		burst_alarm(*p);
		burst_alarm((*p)->reset);
		burst_alarm((*p)->start);
		burst_alarm((*p)->realtime_loop);
		burst_alarm((*p)->frontend_loop);
		burst_alarm((*p)->config);
		burst_alarm((*p)->action);
		burst_alarm((*p)->feedback);
		burst_alarm((*p)->perform_panic);		
		#if BURST_PROTECTION_ENABLED == 1
		burst_alarm((*p)->realtime_protection);
		burst_alarm((*p)->frontend_protection);
		#endif

		if((*p)->mode_count>0){
			burst_alarm((*p)->modes);
		}	
		#if BURST_QUEUE_ENABLED == 1
		(*p)->update_feedback.request.owner = *p;
		(*p)->update_feedback.request.ref.status = bust_request_status_none;
		(*p)->update_feedback.request.ref.query =  burst_dev_feedback_query;
		(*p)->update_feedback.request.ref.confirm =  burst_dev_request_confirm;
		#endif
		(*p)->actual_mode = &burst_idle_mode;		
		(*p)->modes_end = (*p)->modes+(*p)->mode_count;
		
		for(	burst_dev_mode_p  * pm = (*p)->modes; pm!=(*p)->modes_end; ++pm) {
			burst_dev_mode_p m = *pm;
			if(m){
				burst_alarm(m->applay_action);
				burst_alarm(m->start);
				burst_alarm(m->stop);
				burst_alarm(m->loopA);
				burst_alarm(m->loopB);
				burst_alarm(m->loopC);
				burst_alarm(m->frontend_loop);
			}
		}
		//сброс в начальное состояние
		(*p)->reset((*p));
	}
	
	burst_hw_begin();
	burst_core_status_ = burst_core_frontend;
}
burst_bool_t burst_started_ = burst_false;
void burst_start(void){
	burst_dev_ref_p * p ;
	for( p= burst.devs; p!=burst.devs_end;p++){
		(*p)->start(*p);
	}
	burst_hw_start();
	burst_sw_start();
	burst_started_ = burst_true;
}
void burst_reset(void){
	burst_dev_ref_p * p ;
	for( p= burst.devs; p!=burst.devs_end;p++){
		(*p)->reset(*p);
	}
}

void burst_realtime_loop(void){
	if(burst_started_){
		burst_fall();
		burst_dev_ref_p * p ;
		debug_tp_on(VERB_LOOP);
		debug_tp_on(VERB_REALTIME);
		for( p= burst.devs; p!=burst.devs_end;p++){
			(*p)->realtime_loop(*p);
		}
		burst_hw_realtime_loop();
		burst_sw_realtime_loop();
		#if BURST_PROTECTION_ENABLED == 1
		for( p= burst.devs; p!=burst.devs_end;p++){
			(*p)->realtime_protection(*p);
			if( (*p) -> panic){
				(*p)->perform_panic(*p);				
			}
		}
		#endif
		debug_tp_off(VERB_REALTIME);
	}
}
burst_config_t burst_config_dummy = BURST_CONFIG();
burst_config_t * burst_config = &burst_config_dummy;

#if BURST_PROTECTION_ENABLED == 1
void burst_dev_realtime_protection(burst_dev_ref_p _ref){
	#if BURST_PANICS_BOARD_VOLTAGE_ENABLED 
	int burst_board_voltage = burst_board_voltage_get_pp();
	if( burst_board_voltage >= burst_config->panics.voltage_pp.overhi){
		burst_board_raise_panic(burst_panic_board_overvoltage_bit);
	} else if (burst_board_voltage<=burst_config->panics.voltage_pp.ultralo){
		burst_board_raise_panic(burst_panic_board_lovoltage_bit);
	}
	#endif

	#if BURST_PANICS_BOARD_CURRENT_ENABLED 
	int burst_board_current = burst_board_current_get_pp();
	if( burst_board_current >= burst_config->panics.overcurrent_pp){
		burst_board_raise_panic(burst_panic_board_overcurrent_bit);
	} else if (burst_board_current<=burst_config->panics.locurrent_pp){
		burst_board_raise_panic(burst_panic_board_locurrent_bit);
	}
	#endif
}
void burst_dev_frontend_protection(burst_dev_ref_p _ref){
	#if BURST_PANICS_BOARD_TEMPER_ENABLED == 1
	int burst_board_temper_hi = burst_board_temper_get_hi_pp();
	int burst_board_temper_lo = burst_board_temper_get_lo_pp();
	if( burst_board_temper_hi >= burst_config->panics.temp_pp.overhi){
		burst_board_raise_panic(burst_panic_board_overtemp_bit);
	} else if (burst_board_temper_hi < burst_config->panics.temp_pp.hi){
		if( burst.panics){
			burst_board_reset_panic(burst_panic_board_overtemp_bit);
		}
	}
	
	if (burst_board_temper_lo<=burst_config->panics.temp_pp.ultralo){
		burst_board_raise_panic(burst_panic_board_lotemp_bit);
	} else if(burst_board_temper_lo > burst_config->panics.temp_pp.lo){
		if( burst.panics){
			burst_board_reset_panic(burst_panic_board_lotemp_bit);
		}
	}
	#endif
	if(burst.panics && burst_config->panics.reset_timeout_us){ 
		if( burst_time_us() - burst.last_panic_us > burst_config->panics.reset_timeout_us)
		{
			uint32_t mask = burst.panics;
			mask &= ~(burst_panic_board_overtemp_bit|burst_panic_board_lotemp_bit);
			burst.panics &= ~(mask);
			if(burst.panics == 0){
				burst_dev_ref_p * p ;
				for( p= burst.devs; p!=burst.devs_end;p++){
					if( (*p) -> panic){
						(*p) -> panic &= ~(1<<burst_panic_dev_board_bit);				
					}
				}
			}
		}
	}
}
#endif


void burst_dev_backend_loop(burst_dev_ref_p _ref){
	if(burst_started_){
		//burst_alarm(is_backend__);
		burst_dev_action_p action = _ref->action;
		int action_mode = action->mode;
		burst_dev_mode_p actual_mode = _ref->actual_mode;
		if ( action_mode != _ref->mode) {
			if (actual_mode) {
				actual_mode->stop(_ref);
			}

			if (action_mode == burst_dev_mode_idle) {
				burst_dev_switch_to_idle(_ref);
			}
			else {
				if(action_mode<_ref->mode_count){
					burst_dev_mode_p m = _ref->modes[action_mode];
					if (m == 0) {
						burst_dev_switch_to_idle(_ref);
					}
					else {
						m->start(_ref);
						m->applay_action(_ref);
						_ref->actual_mode = m;
						_ref->mode = action_mode;
					}
				}else{
					burst_dev_switch_to_idle(_ref);
				}
			}
		} else{
			if(action->actual){
				actual_mode->applay_action(_ref);
				action->actual = 0;
			}
		}
		#if BURST_QUEUE_ENABLED == 0
		if(_ref->update_feedback.status == bust_request_status_query){
			_ref->update_feedback.on_run(_ref);
			_ref->update_feedback.status = bust_request_status_confirm;
		}
		#endif
	}
}


typedef void ( *burst_slot_f)(void);

#define BURST_SLOT_D(n) \
void burst_slot_##n(void){ \
	burst_sw_slot_##n(); \
	burst_hw_slot_##n(); \
}

#define BURST_SLOT(n) \
&burst_slot_##n

#if BURST_SLOT_COUNT > 0
BURST_SLOT_D(0)
#endif

#if BURST_SLOT_COUNT > 1
BURST_SLOT_D(1)
#endif

#if BURST_SLOT_COUNT > 2
BURST_SLOT_D(2)
#endif

#if BURST_SLOT_COUNT > 3
BURST_SLOT_D(3)
#endif

#if BURST_SLOT_COUNT > 4
BURST_SLOT_D(4)
#endif

#if BURST_SLOT_COUNT > 5
BURST_SLOT_D(5)
#endif

#if BURST_SLOT_COUNT > 6
BURST_SLOT_D(6)
#endif

#if BURST_SLOT_COUNT > 7
BURST_SLOT_D(7)
#endif

#if BURST_SLOT_COUNT > 8
BURST_SLOT_D(8)
#endif

#if BURST_SLOT_COUNT > 9
BURST_SLOT_D(9)
#endif

#if BURST_SLOT_COUNT > 10
BURST_SLOT_D(10)
#endif

#if BURST_SLOT_COUNT > 11
BURST_SLOT_D(11)
#endif

#if BURST_SLOT_COUNT > 12
BURST_SLOT_D(12)
#endif

#if BURST_SLOT_COUNT > 13
BURST_SLOT_D(13)
#endif

#if BURST_SLOT_COUNT > 14
BURST_SLOT_D(14)
#endif

#if BURST_SLOT_COUNT > 15
BURST_SLOT_D(15)
#endif

burst_slot_f burst_slots[BURST_SLOT_COUNT]={
	#if BURST_SLOT_COUNT > 0
	BURST_SLOT(0)
	#endif
	#if BURST_SLOT_COUNT > 1
	,BURST_SLOT(1)
	#endif

	#if BURST_SLOT_COUNT > 2
	,BURST_SLOT(2)
	#endif

	#if BURST_SLOT_COUNT > 3
	,BURST_SLOT(3)
	#endif

	#if BURST_SLOT_COUNT > 4
	,BURST_SLOT(4)
	#endif

	#if BURST_SLOT_COUNT > 5
	,BURST_SLOT(5)
	#endif

	#if BURST_SLOT_COUNT > 6
	,BURST_SLOT(6)
	#endif

	#if BURST_SLOT_COUNT > 7
	,BURST_SLOT(7)
	#endif

	#if BURST_SLOT_COUNT > 8
	,BURST_SLOT(8)
	#endif

	#if BURST_SLOT_COUNT > 9
	,BURST_SLOT(9)
	#endif

	#if BURST_SLOT_COUNT > 10
	,BURST_SLOT(10)
	#endif

	#if BURST_SLOT_COUNT > 11
	,BURST_SLOT(11)
	#endif

	#if BURST_SLOT_COUNT > 12
	,BURST_SLOT(12)
	#endif

	#if BURST_SLOT_COUNT > 13
	,BURST_SLOT(13)
	#endif

	#if BURST_SLOT_COUNT > 14
	,BURST_SLOT(14)
	#endif

	#if BURST_SLOT_COUNT > 15
	,BURST_SLOT(15)
	#endif
};
burst_slot_f * burst_slot = burst_slots;
burst_slot_f * burst_slots_end = burst_slots+BURST_SLOT_COUNT;


void burst_backend_loop(void){
	//todo!!! govnocod
	if(burst_started_){
		debug_tp_on(VERB_BACKEND);
		#if BURST_TIMER_ENABLED == 1
		burst_timer_poll();
		#endif
		#if BURST_SLOT_COUNT > 0
		(*burst_slot)();
		burst_slot++;
		if(burst_slot==burst_slots_end) 
			burst_slot = burst_slots;
		#endif
		{
			burst_dev_ref_p * p ;
			for( p= burst.devs; p!=burst.devs_end;p++){
				burst_dev_backend_loop((*p));
			}
		}
		burst_hw_backend_loop();
		burst_sw_backend_loop();
		#if BURST_QUEUE_ENABLED == 1
		bust_backend_request_perform();
		#endif
		debug_tp_off(VERB_BACKEND);
		debug_tp_off(VERB_LOOP);

		burst_comeback();
	}

}



void burst_frontend_loop(void){
	debug_tp_on(VERB_FRONTEND);
	#if BURST_PROTECTION_ENABLED == 1
	{
		burst_dev_ref_p * p ;
		for( p= burst.devs; p!=burst.devs_end;p++){
			(*p)->frontend_protection(*p);
		}
	}
	#endif
	#if BURST_TIMER_ENABLED == 1
	burst_timer_poll();
	#endif
	burst_sw_frontend_loop();
	burst_hw_frontend_loop();
	{
		burst_dev_ref_p * p ;
		for( p= burst.devs; p!=burst.devs_end;p++){
			burst_dev_ref_p s = *p;
			#if BURST_PANICS_MASTER_LOST_ENABLED == 1
			burst_master_alive_check( s );
			#endif
			s->frontend_loop(s);
			s->actual_mode->frontend_loop(s);
			#if BURST_QUEUE_ENABLED == 0
			if(s->update_feedback.status == bust_request_status_confirm){
				if(s->update_feedback.on_complete){
					s->update_feedback.on_complete(s);
				}
				s->update_feedback.status = bust_request_status_none;
			}
			#endif
		}
	}
	
	#if BURST_BUTTON_ENABLED == 1
	burst_btn_poll();
	#endif
	#if BURST_QUEUE_ENABLED == 1
	bust_front_request_perform();
	#endif
	debug_tp_off(VERB_FRONTEND);
}


#ifdef BURST_WEAK

BURST_WEAK void burst_sw_begin(void){}
BURST_WEAK void burst_hw_begin(void){}

BURST_WEAK void burst_sw_start(void){}
BURST_WEAK void burst_hw_start(void){}

BURST_WEAK void burst_sw_backend_loop(void){}
BURST_WEAK void burst_hw_backend_loop(void){}

BURST_WEAK void burst_sw_realtime_loop(void){}
BURST_WEAK void burst_hw_realtime_loop(void){}

BURST_WEAK void burst_sw_frontend_loop(void){}
BURST_WEAK void burst_hw_frontend_loop(void){}

#if BURST_SLOT_COUNT > 0
BURST_WEAK void burst_sw_slot_0(void){}
BURST_WEAK void burst_hw_slot_0(void){}
#endif

#if BURST_SLOT_COUNT > 1
BURST_WEAK  void burst_sw_slot_1(void){}
BURST_WEAK  void burst_hw_slot_1(void){}
#endif

#if BURST_SLOT_COUNT > 2
BURST_WEAK  void burst_sw_slot_2(void){}
BURST_WEAK  void burst_hw_slot_2(void){}
#endif

#if BURST_SLOT_COUNT > 3
BURST_WEAK  void burst_sw_slot_3(void){}
BURST_WEAK  void burst_hw_slot_3(void){}
#endif

#if BURST_SLOT_COUNT > 4
BURST_WEAK  void burst_sw_slot_4(void){}
BURST_WEAK  void burst_hw_slot_4(void){}
#endif

#if BURST_SLOT_COUNT > 5
BURST_WEAK  void burst_sw_slot_5(void){}
BURST_WEAK  void burst_hw_slot_5(void){}
#endif

#if BURST_SLOT_COUNT > 6
BURST_WEAK  void burst_sw_slot_6(void){}
BURST_WEAK  void burst_hw_slot_6(void){}
#endif

#if BURST_SLOT_COUNT > 7
BURST_WEAK  void burst_sw_slot_7(void){}
BURST_WEAK  void burst_hw_slot_7(void){}
#endif


#if BURST_SLOT_COUNT > 8
BURST_WEAK  void burst_sw_slot_8(void){}
BURST_WEAK  void burst_hw_slot_8(void){}
#endif

#if BURST_SLOT_COUNT > 9
BURST_WEAK  void burst_sw_slot_9(void){}
BURST_WEAK  void burst_hw_slot_9(void){}
#endif

#if BURST_SLOT_COUNT > 10
BURST_WEAK  void burst_sw_slot_10(void){}
BURST_WEAK  void burst_hw_slot_10(void){}
#endif

#if BURST_SLOT_COUNT > 11
BURST_WEAK  void burst_sw_slot_11(void){}
BURST_WEAK  void burst_hw_slot_11(void){}
#endif

#if BURST_SLOT_COUNT > 12
BURST_WEAK  void burst_sw_slot_12(void){}
BURST_WEAK  void burst_hw_slot_12(void){}
#endif

#if BURST_SLOT_COUNT > 13
BURST_WEAK  void burst_sw_slot_13(void){}
BURST_WEAK  void burst_hw_slot_13(void){}
#endif

#if BURST_SLOT_COUNT > 14
BURST_WEAK  void burst_sw_slot_14(void){}
BURST_WEAK  void burst_hw_slot_14(void){}
#endif

#if BURST_SLOT_COUNT > 15
BURST_WEAK  void burst_sw_slot_15(void){}
BURST_WEAK  void burst_hw_slot_15(void){}
#endif

BURST_WEAK  void burst_sw_on_crash(const char * _file, const char * _function, int _line){
	BURST_UNUSED(_file);
	BURST_UNUSED(_function);
	BURST_UNUSED(_line);
}
BURST_WEAK  void burst_hw_on_crash(void){
}
BURST_WEAK  void burst_hw_fall(void){}
BURST_WEAK  void burst_hw_comeback(void){}

BURST_WEAK  burst_guard_op_t burst_hw_critical_enter(void){
	return burst_guard_op_skip;
}
BURST_WEAK  void burst_hw_critical_leave(void){
}

BURST_WEAK  burst_guard_op_t burst_hw_guard_enter(void){
	return burst_guard_op_skip;
}
BURST_WEAK  void burst_hw_guard_leave(void){
}

BURST_WEAK  burst_guard_op_t burst_hw_guard_lock(void){ 
return burst_guard_op_skip;
}
BURST_WEAK  void burst_hw_guard_unlock(void){ 
}
#endif

BURST_WEAK  void burst_hw_reboot(void){}

	burst_bool_t burst_is_frontend(void){
	return (burst_bool_t) (burst_core_status_== burst_core_frontend );
}
burst_bool_t burst_is_backend(void){
	return (burst_bool_t) (burst_core_status_== burst_core_backend);
}

void burst_fall(void){
	BURST_CORE_ALARM( burst_is_frontend() );
	burst_hw_fall();
	burst_core_status_= burst_core_backend;
}
void burst_comeback(void){
	BURST_CORE_ALARM(burst_is_backend());
	burst_hw_comeback();
	burst_core_status_= burst_core_frontend;
}
int32_t guest_count_ = 0;
int32_t lock_count_ = 0;
typedef union {
	uint32_t value;
	struct{
		uint8_t critical_op;
		uint8_t guard_op;
		#if BURST_CORE_DEBUG == 1
		uint8_t guad_context;
		#endif
	};
} burst_guard_context_t;

uint32_t burst_guard_enter(void){
	burst_guard_context_t ctx;
	if (burst_is_frontend()) {
		ctx.critical_op = (uint8_t)burst_hw_critical_enter();
		#if BURST_CORE_DEBUG == 1
		guest_count_++;			
		ctx.guad_context = (uint8_t)burst_core_frontend;
		#endif
		ctx.guard_op = (uint8_t)burst_hw_guard_enter();
	}	else {
		#if BURST_CORE_DEBUG == 1
		lock_count_++;
		ctx.guad_context = (uint8_t)burst_core_backend;
		#endif
		ctx.guard_op = (uint8_t)burst_hw_guard_lock();
	}
	return ctx.value;
}

void burst_guard_leave(uint32_t _context){
	burst_guard_context_t ctx;
	ctx.value	= _context; 
	if (burst_is_frontend()) {
		#if BURST_CORE_DEBUG == 1
		guest_count_--;
		burst_alarm(ctx.guad_context == burst_core_frontend )
		burst_alarm(guest_count_ >= 0);
		#endif

		if ( (burst_guard_op_t)ctx.guard_op == burst_guard_op_run) {
			burst_hw_guard_leave();
		}
		if ( (burst_guard_op_t)ctx.critical_op == burst_guard_op_run) 
			burst_hw_critical_leave();
	}
	else {
		#if BURST_CORE_DEBUG == 1
		lock_count_--;
		burst_alarm(lock_count_ >= 0);
		burst_alarm(ctx.guad_context == burst_core_backend);
		#endif
		if ((burst_guard_op_t)ctx.guard_op == burst_guard_op_run)
			burst_hw_guard_unlock();
	}
}
	
uint32_t burst_critical_enter(void){
	//быстрым процессам сдесь делать нечего - это разборки между потоками "фронткнд"
	#if BURST_CORE_DEBUG == 1
	burst_alarm(burst_is_frontend());
	#endif
	return (uint32_t) burst_hw_critical_enter();
}
void burst_critical_leave(uint32_t _context){
	#if BURST_CORE_DEBUG == 1
	burst_alarm(burst_is_frontend());
	#endif
	if ((burst_guard_op_t)_context == burst_guard_op_run)
		burst_hw_critical_leave();
}

#if BURST_DEBUG_TP_ENABLED == 1
#define CLCH_NAME burst_tp
#include "burst/cliche/tp.h"
#endif

void burst_event_perform_panic(burst_dev_ref_p _dev){	
	if(_dev->mode!=burst_dev_mode_idle ){
		burst_dev_mode_p actual_mode = _dev->actual_mode;
		if (actual_mode) {
			actual_mode->stop(_dev);
		}
		
		burst_dev_switch_to_idle(_dev);
	}
	_dev->action->mode = burst_dev_mode_idle;
}

void burst_board_raise_panic(uint32_t _flag){
	uint32_t mask = ( 1<< _flag );
	if( (burst.panics & mask)  == 0){
		burst.last_panic_us = burst_time_us();
		burst.panics |= mask;
	}
	burst_dev_ref_p * p ;
	for( p= burst.devs; p!=burst.devs_end;p++){
		burst_dev_raise_panic(*p, burst_panic_dev_board_bit);
	}	
}

void burst_board_reset_panic(uint32_t _flag){
	uint32_t mask = ( 1<< _flag );
	if( (burst.panics & mask)  != 0){
		burst.panics &= ~mask;
		if(burst.panics == 0){
			burst_dev_ref_p * p ;
			for( p= burst.devs; p!=burst.devs_end;p++){
				burst_dev_reset_panic(*p, burst_panic_dev_board_bit);
			}
		}	
	}
}

void burst_dev_raise_panic(burst_dev_ref_p _dev, uint32_t _flag){
	uint32_t mask = ( 1<< _flag );
	if( (_dev->panic & mask)  == 0){
		_dev->panic |= mask;
		burst_event_perform_panic(_dev);
	}
}
void burst_dev_reset_panic(burst_dev_ref_p _dev, uint32_t _flag){
	uint32_t mask = ( 1<< _flag );
	_dev->panic &= ~mask;
}
void burst_config_set( burst_config_t * _config){
	 burst_config = _config;
}


void burst_query_feedback(burst_dev_ref_p _ref, burst_dev_mode_event _on_complete){
	#if BURST_QUEUE_ENABLED == 0
	/*if(burst_is_frontend()){
		_ref->update_feedback.on_run(_ref);
		if(_ref->update_feedback.on_complete){
			_ref->update_feedback.on_complete(_ref);
		}
	} else{
		_ref->update_feedback.flag.query = burst_true;
	}*/
	if(_ref->update_feedback.status == bust_request_status_none  || _ref->update_feedback.status == bust_request_status_panic ){
		_ref->update_feedback.status = bust_request_status_query;
	}
	#else
	if(_ref->update_feedback.request.ref.status == bust_request_status_none){
		_ref->update_feedback.request.on_complete = _on_complete;
		bust_post(&(_ref->update_feedback.request.ref));
	}
	#endif
}

#if BURST_STRING_HASH_ENABLED
int32_t burst_string_hash(const char * _str, int _start){
	int32_t x = _start;
	const char * c;
	for (c = _str; *c; ++c) {
		x += 0x990C9AB5 * (*c);
		x = x ^  (x >> 16);
	}
	return x;
}

#endif

#endif
