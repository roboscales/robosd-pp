#include "mexo/mexo.hpp"
#include "core/robosd_log.hpp"
#include "net/robosd_flow.hpp"
namespace mexo {
	#if ROBO_APP_MEXO_DEBUG_TP1_ENABLED == 1
	led<tp_driver>  tp;

	#else

	led<dummy_led>  tp;
//	typedef dummy_led  tp;

	#endif 
	
	machine machine::instance_;
	machine::machine(void)
		: slots_ref_(slots_())
		, slot_index_(0) {}
	machine::~machine(void) {

	}
	void machine::begin_(void) {
		system::begin();
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
		system::start(_period_us);
		slots_ref_.start.execute();
	}
	#if ROBO_APP_MEXO_PRIORITY_SLOT_ENABLE == 1
	void machine::priority_loop_(void) {
		tp.on(tp_verb::loop);
		tp.on(tp_verb::priority);
		fall__;
		slots_ref_.priority.execute();
		tp.off(tp_verb::priority);
	}
	#endif

	void machine::backend_loop_(void) {
		tp.on(tp_verb::backend);
		fall__;
		system::backend_loop();
		#if ROBO_APP_MEXO_PRIORITY_SLOT_ENABLE != 1
		slots_ref_.priority.execute();
		#endif
		slots_ref_.periodic[slot_index_].execute();
		slots_ref_.control.execute();
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
		system::frontend_loop();
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
		
		case slot::kind::priority:
		return priority;
		
		case slot::kind::control:
		return control;

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
		priority.free();
		control.free();
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
		, name_(_name)
		, owner_(_owner == nullptr ? &root() : _owner) {
		int key;
		if (_owner) {
			key = owner_->map_ref_.key();
			key = hash(RT("."), key);
		}
		else {
			key = 0;
		}
		key = hash(name_, key);
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

	bool task::do_reconfig(void) {
		ROBO_LBREAKN(node::do_reconfig());
		if (autostart_) start();
		return true;
	}

	void task::start(void) {
		do_start();
	}

	void task::stop(void) {
		do_stop();
	}

	void periodic_task::do_start(delegat* _prev) {
		machine::slot::delegat::ref** pref = refs_;
		int* ix = index_;
		for (size_t n = 0; n < ref_count_; ++n, ++pref, ++ix) {
			machine::slot::delegat::attach(**pref, *ix, _prev);
		}
	}

	void periodic_task::do_stop(void) {
		machine::slot::delegat::ref** pref = refs_;
		for (size_t n = 0; n < ref_count_; ++n, ++pref) {
			(*pref)->dettach();
		}
	}

	void periodic_task::clean(void) {
		if (ref_count_) {
			machine::slot::delegat::ref** pref = refs_;
			for (size_t n = 0; n < ref_count_; ++n, ++pref) {
				delete* pref;
			}
			delete[] refs_;
			delete[] index_;
			ref_count_ = 0;
			refs_ = nullptr;
			index_ = nullptr;
		}
	}
	void periodic_task::setup(std::initializer_list<int> _index) {
		ref_count_ = _index.end() - _index.begin();
		if (ref_count_ > 0) {
			refs_ = new machine::slot::delegat::ref * [ref_count_];
			index_ = new int[ref_count_];
			machine::slot::delegat::ref** pref = refs_;
			const int* src = _index.begin();
			int* dst = index_;
			for (size_t n = 0; n < ref_count_; ++n, ++pref, ++src, ++dst) {
				*pref = new machine::slot::delegat::ref(*this);
				*dst = *src;
			}
		}
	}

	void periodic_task::setup(int _index) {
		ref_count_ = 1;
		refs_ = new machine::slot::delegat::ref * [ref_count_];
		index_ = new int[ref_count_];
		machine::slot::delegat::ref** pref = refs_;
		refs_[0] = new machine::slot::delegat::ref(*this);
		index_[0] = _index;
	}

	periodic_task::periodic_task(cstr  _name, bool _autostart, std::initializer_list<int> _index, node* _owner)
		: task(_name, _autostart, _owner) {
		setup(_index);
	}
	periodic_task::periodic_task(cstr  _name, bool _autostart, std::initializer_list<int> _index, periodic_task* _prev)
		: task(_name, _autostart, _prev) {
		setup(_index);
	}
	periodic_task::periodic_task(cstr  _name, node* _owner)
		: task(_name, false, _owner) {}
	periodic_task::periodic_task(cstr  _name, periodic_task* _prev)
		: task(_name, false, _prev) {}

	periodic_task::~periodic_task(void) {
		if (refs_ != nullptr) {
			machine::slot::delegat::ref** pref = refs_;
			for (size_t n = 0; n < ref_count_; ++n, ++pref) {
				delete* pref;
			}
			delete[] refs_;
		}
	}

	subsystem_handler::subsystem_handler(cstr  _name, subsystem* _subsystem)
		: node(_name, _subsystem ? _subsystem->owned_node() : nullptr)
		, ref_(*this) {
		if (_subsystem) {
			subsystem_ = _subsystem;
			ref_.attach_to(_subsystem->handlers);
		}
		else {
			subsystem_ = nullptr;
		}
	}
	subsystem_handler::subsystem_handler(cstr  _name, subsystem_handler* _prev)
		: node(_name, _prev ? _prev->owner() : nullptr)
		, ref_(*this) {
		if (_prev  ) {
			subsystem_ = _prev->subsystem_;
			ref_.attach_after(subsystem_->handlers, _prev);
		}
		else {
			subsystem_ = nullptr;
		}
	}

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
			if (present_.mode != front::dev::mode::idle) {
				actual_mode_->do_stop();
			}

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
					m->do_start();
					if (action_enabled_) {
						m->applay_action();
					}
					actual_mode_ = m;
					present_.mode = _mode_id;
				}
			}
			if (actual_mode_ == &idle) {
				on_idle();
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
			if (action_enabled_) {
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
	bool controller::process::run_(void) {
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
				state_ = state::prepare;
			onPrepare();
			break;
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
		if (runned_->run_()) {
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

}

#include "mexo/mexo.h"
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
void mexo_priority_loop(void) {
	mexo::machine::priority_loop();
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
