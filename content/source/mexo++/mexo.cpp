#include "mexo++/mexo.hpp"
#include "core/robosd_log.hpp"
#include "net/robosd_flow.hpp"
namespace mexo {
	#if ROBO_APP_MEXO_DEBUG_TP1_ENABLED == 1
	led_t<tp_driver>  tp;

	#else
	led_t<dummy_led>  tp;
	#endif 
	
	machine machine::instance_;
	machine::machine(void)
		: slots_ref_(slots_())
		, slot_index_(0) {}
	machine::~machine(void) {

	}
	void machine::begin_(void) {
		#if ROBO_APP_SYSTEM_ENABLED == 1
		system::begin();
		#endif
		slots_ref_.begin.execute();
		ROBO_APP_ASSERT(::mexo::node::begin());
		#if ROBO_APP_NET_FLOW_ENABLED == 1
		::robo::net::flow::machine::begin();
		#endif
		#if ROBO_APP_MEXO_VAR_ENABLED == 1
		node::create_vars();
		#endif
	}
	void machine::start_(time_us_t _period_us) {
		#if ROBO_APP_SYSTEM_ENABLED == 1
		system::start(_period_us);
		#endif
		slots_ref_.start.execute();
	}
	#if ROBO_APP_MEXO_REALTIME_SLOT_ENABLE == 1
	void machine::realtime_loop_(void) {
		tp.on(tp_verb::loop);
		tp.on(tp_verb::priority);
		fall__;
		slots_ref_.realtime.execute();
		tp.off(tp_verb::priority);
	}
	#endif

	void machine::backend_loop_(void) {
		tp.on(tp_verb::backend);
		fall__;
		#if ROBO_APP_MEXO_REALTIME_SLOT_ENABLE != 1
		slots_ref_.realtime.execute();
		#endif
		#if ROBO_APP_SYSTEM_ENABLED == 1
		system::backend_loop();
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
		tp.off(tp_verb::backend);
		tp.off(tp_verb::loop);
	}
	void machine::frontend_loop_(void) {
		tp.on(tp_verb::frontend);
		#if ROBO_APP_SYSTEM_ENABLED == 1
		system::frontend_loop();
		#endif
		slots_ref_.frontend.execute();
		#if ROBO_APP_NET_FLOW_ENABLED == 1
		::robo::net::flow::machine::frontend_poll();
		#endif
		tp.off(tp_verb::frontend);
	}
	void machine::raise_fault_(void){
		slots_ref_.raise_fault.execute();
	}
	machine::slots& machine::slots_(void) {
		static machine::slots slots__;
		return slots__;
	}
	machine::slot& machine::slots::operator [] (machine::slot::kind _kind) {
		switch (_kind) {
		case slot::kind::begin:
		return begin;
		
		case slot::kind::start:
		return start;
		
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
	void  machine::slots::free(void) {
		begin.free();
		start.free();
		realtime.free();
		backend.free();
		frontend.free();
		raise_fault.free();
		dummy.free();
		for (int i = 0; i < slot_count; i++) {
			periodic[i].free();
		}
	}

	machine::slot& machine::slots::operator [] (int _index) {
		if (((_index >= 0) && (_index < slot_count))) {
			return periodic[_index];
		}
		else {
			ROBO_APP_CRASH();
			return dummy;
		}
	}

	machine::slot::slot(void) {}

	machine::slot::~slot(void) {}

	void machine::slot::execute(void) {
		for (delegat::ref* r = delegats_.first(); r; r = r->next()) {
			r->owner()();
		}
	}

	void machine::slot::free(void) {
		delegat::ref* r;
		while ((r = delegats_.first()) != nullptr) {
			delete r;
		}
	}

	void machine::slot::delegat::attach(slot::kind _kind, delegat* _prev) {
		ROBO_APP_ASSERT(is_frontend__);
		ref* r = new ref(*this);
		guard__;
		r->attach_after(machine::slots_()[_kind].delegats_, _prev );
	}

	void machine::slot::delegat::attach(int _index, delegat* _prev) {
		ROBO_APP_ASSERT(is_frontend__);
		ref* r = new ref(*this);
		guard__;
		r->attach_after(machine::slots_()[_index].delegats_, _prev);
	}

	void machine::slot::delegat::attach(ref& _ref, int _index, delegat* _prev) {
		guard__;
		_ref.attach_after(machine::slots_()[_index].delegats_, _prev);
	}
	

	void machine::slot::delegat::attach(ref& _ref, slot::kind _kind, delegat* _prev) {
		guard__;
		_ref.attach_after(machine::slots_()[_kind].delegats_, _prev);
	}


	node::node(void) : ref_(*this), map_ref_(*this, 0), name_(RT("root")), owner_(nullptr)/*, auto_enabled_(true)*/ {
		ROBO_APP_ASSERT(is_frontend__);
	}

	node& node::root(void) {
		static node root__;
		return root__;
	}

	node::map& node::map_(void) {
		static map map__;
		return map__;
	}

	bool node::reconfig(void) {
		state_ = state::fault;
		for (ref* r = childs_.first(); r; r = r->next()) {
			ROBO_LBREAKN(r->owner().reconfig())
		}
		ROBO_LBREAKN(do_reconfig());
		state_ = state::ready;
		return true;
	}

	node::node(cstr _name, node* _owner) : ref_(*this), map_ref_(*this, 0)
		, name_( _name)
		, owner_(_owner == nullptr ? &root() : _owner) {
		int key;
		if (_owner) {
			key = owner_->map_ref_.key();
			key = hash(RT("."), key);
		}
		else {
			key = 0;
		}
		if(name_ &&  name_[0]!=0){
			key = hash(name_, key);
		} else{			
			key^=owner_->childs_.count();
		}
		map_ref_.set_key(key);
		ROBO_APP_ASSERT(map_ref_.attach_to(map_()));
		ref_.attach_to(owner_->childs_);
	}

	#if ROBO_APP_MEXO_VAR_ENABLED == 1		
	void node::create_vars_index_(void) {
		for (var::record::ref* r = vars.first(); r; r = r->next()) {
			var::machine::reg(*r);
		}
		for (ref* r = childs_.first(); r; r = r->next()) {
			r->owner().create_vars_index_();
		}
	}

	void node::create_vars_(void) {
		do_create_vars();
		for (ref* r = childs_.first(); r; r = r->next()) {
			r->owner().create_vars_();
		}
	}

	int node::var_count_(void) {
		int tmp = 0;
		for (ref* r = childs_.first(); r; r = r->next()) {
			tmp += r->owner().var_count_();
		}
		return tmp + vars.count();
	}

	void node::create_vars(void) {
		root().create_vars_();
		var::machine::begin(root().var_count_());
		root().create_vars_index_();
	}
	#endif
	bool node::begin(void) {
		return  root().reconfig();
	}

	node* node::first_on_path(char_t*& _path, size_t& _len) {
		if(_path==nullptr || _len==0) return nullptr;
		if (this != &node::root()) {
//			path_offset_ = ::robo::system::sprintf(_path, _len, RT("%s."), name_);
			path_offset_=0;
			cstr s = name_;
			while ( *s && _len--){				
				*_path++ = *s++;
				path_offset_++;
			}
			if(_len<=2){
				_path -= path_offset_;
				_len += path_offset_;
				return nullptr;
			}
			*_path++ = '.';
			*_path = 0;
			_len -= 1;
			path_offset_ += 1;
			//_path += path_offset_;
			//_len -= path_offset_;
		}
		else {
			path_offset_ = 0;
		}
		if (childs_.first()) {
			return  childs_.first()->owner().first_on_path(_path, _len);
		}
		else {
			return this;
		}
	};
	node* node::next_on_path(char_t*& _path, size_t& _len) {
		_path -= path_offset_;
		_len += path_offset_;
		*_path = (char_t)0;
		ref* r = ref_.next();
		if (r) {
			return r->owner().first_on_path(_path, _len);
		}
		else {
			if (owner_) {
				return owner_;// ->next_on_path(_path, _len);
			}
			else {
				return nullptr;
			}
		}
	};

	
	dev::dev(cstr  _name, action_s& _action, feedback_s& _feedback, present_s& _present, config_s& _config)
		: node(_name, nullptr)
		, idle(*this)
		, actual_mode_(&idle)
		, backend_(*this, &dev::backend__)
		, backend_ref_(backend_)
		, action_(_action)
		, feedback_(_feedback)
		, present_(_present)
		, config_(_config) {
		mexo::machine::slot::delegat::attach(backend_ref_, mexo::machine::slot::kind::backend, nullptr);
		present_.mode = front::dev::mode::idle;
		#if ROBO_APP_MEXO_EXTERNAL_CONFIGURE_NEED == 1
		present_.error = front::dev::error::startup;
		#endif
	}

	void dev::switch_to(int _mode_id) {
		ROBO_APP_ASSERT(is_backend__);
		if (_mode_id != present_.mode) {
			actual_mode_->finish();

			if (_mode_id == front::dev::mode::idle) {
				actual_mode_ = &idle;
				present_.mode = front::dev::mode::idle;
			}
			else {
				mode* m = modes_.find(_mode_id);
				if (m == nullptr || !m->enabled()) {
					actual_mode_ = &idle;
					present_.mode = front::dev::mode::idle;
				}
				else {
					actual_mode_ = m;
					present_.mode = _mode_id;
					if (action_auto_apply_) {
						m->applay_action();
					}
					m->begin();
				}
			}
		}
	}
	void dev::backend__(void) {
		ROBO_APP_ASSERT(is_backend__);
		if (action_.mode != present_.mode) {
			guard__;
			if( present_.error != front::dev::error::none ){
				action_.mode = front::dev::mode::idle;
				switch_to(front::dev::mode::idle);
			} else{
				switch_to(action_.mode);
			}
		}
		if (present_.action_actual) {
			guard__;
			present_.action_actual = false;
			if (action_auto_apply_) {
				actual_mode_->applay_action();
			}
		}
		if(wait_feedback_){
			do_update_feedback();
			wait_feedback_ = false;
		}
	}
	void dev::do_update_feedback(void) {
		feedback_.mode = present_.mode;
		feedback_.fault = present_.error != 0;
	}

	dev::mode::mode(int _index, cstr  _name, dev& _dev)
		: node(_name, &_dev)
		, ref_(*this, _index) {
		ref_.attach_to(_dev.modes_);
	};

	#if ROBO_APP_MEXO_VAR_ENABLED == 1
	void dev::do_create_vars(void) {
		var::record::create(var::uint8, present_.error, RT("dev.error"), key(), vars);
		if (var::machine::actual_mode() >= var::machine::mode::full) {
			var::record::create( var::const_uint8, present_.mode, RT("dev.mode"), key() , vars );
		}
		if (var::machine::actual_mode() >= var::machine::mode::config) {
			var::record::create(var::uint8, action_.mode, RT("act.dev.mode"), key(), vars);
			var::record::create(var::uint8, present_.action_actual, RT("act.dev.actual"), key(), vars);
		}
	}
	#endif
	bool controller::process::run(void) {
		if (command_ == command::start) {
			switch (state_) {
			case state::stopped:
			onPrepare();
			state_ = state::prepare;
			case state::prepare:
			if (doPrepare() == result::success) {
				state_ = state::startup;
				onStartup();
			}
			else
				break;
			case  state::startup:
			if (doStartup() == result::success) {
				state_ = state::execute;
				onExecute();
			}
			else
				break;
			case state::execute:
			if (doExecute() == result::success) {
				onShutdown();
				stop();
			}
			else
				break;
			case state::shutdown:
			if (doShutdown() == result::success) {
				state_ = state::relax;
				onRelax();
			}
			else
				break;
			case state::relax:
			if (doRelax() == result::success) {
				onFinish();
				state_ = state::stopped;
			}
			else
				break;
			}
		}
		else {
			switch (state_) {
			case state::stopped:
				doIdle();
			break;
			case state::prepare:
				//onPrepare(); -- ? это зачем? onPrepare уже было!
			//break;
			case state::startup:
			case state::execute :
				state_ = state::shutdown;
			onShutdown();
			case state::shutdown :
			if (doShutdown() == result::success) {
				state_ = state::relax;
				onRelax();
			}
			else
				break;
			case state::relax:
			if (doRelax() == result::success) {
				onFinish();
				state_ = state::stopped;
				return true;
			}
			else
				break;
			}
		}
		return false;
	}

	void controller::process::terminate(void) {
		doTerminate();
		state_ = state::stopped;
		command_ = command::stop;
	}

	void controller::switchto(controller::process* _task) {
		if (selected_ == 0 && runned_ == _task) {
			if (runned_)
				runned_->start_();
		}
		else {
			selected_ = _task;
			if (runned_) runned_->stop();
		}
	}

	void controller::run(void) {
		if (runned_ == 0) {
			if (selected_ != 0) {
				runned_ = selected_;
				selected_ = 0;
				runned_->start_();
			}
			else {
				return;
			}
		}
		if (runned_->run()) {
			runned_ = 0;
		}
	}
	void controller::stop() {
		selected_ = 0;
		if (runned_) runned_->stop();
	}
	void controller::terminate() {
		selected_ = 0;
		if (runned_) runned_->terminate();
		runned_ = 0;
		doTerminate();
	}

	
	void stateflow::switchto(node * _node){
		if(runned_){
			runned_->onFinish();
		}		
		runned_ = _node;		
		if(runned_){
			runned_->onStartup();
		}		
	}
	
	void stateflow::run(void){
		if(runned_) runned_->doExecute();
	}

}

#include "mexo/mexo.h"
void mexo_begin(void) {
	mexo::machine::begin();
}

#ifdef ROBO_APP_MEXO_SAMPLE_US
void mexo_start( void ) {
	mexo::machine::start(ROBO_APP_MEXO_SAMPLE_US);
}
#else
void mexo_start( void ) {
	mexo::machine::start(0);
}
#endif

void mexo_start_ps(unsigned int _period_us) {
	mexo::machine::start(_period_us);
}
void mexo_realtime_loop(void) {
	mexo::machine::realtime_loop();
}
void mexo_backend_loop(void) {
	mexo::machine::backend_loop();
}
void mexo_frontend_loop() {
	mexo::machine::frontend_loop();
}
void mexo_raise_fault() {
	mexo::machine::raise_fault();
}
