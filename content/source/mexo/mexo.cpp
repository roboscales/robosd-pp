#include "mexo/mexo.hpp"
#include "core/robosd_log.hpp"
#include "net/robosd_flow.hpp"
namespace mexo {

	machine machine::instance_;
	machine::machine(void)
		: slots_ref_(slots_())
		, slot_index_(0) {}
	machine::~machine(void) {

	}
	void machine::begin_(void) {
		slots_ref_.begin.execute();
		ROBO_APP_ASSERT(::mexo::node::begin());
		#if ROBO_APP_NET_FLOW_ENABLED == 1
		::robo::net::flow::machine::begin();
		#endif
	}
	void machine::start_(void) {
		slots_ref_.start.execute();
	}
	void machine::priority_loop_(void) {
		tp::on(tp_verb::loop);
		tp::on(tp_verb::priority);
		fall__;
		slots_ref_.priority.execute();
		tp::off(tp_verb::priority);
	}
	void machine::backend_loop_(void) {
		tp::on(tp_verb::backend);
		fall__;
		slots_ref_.backend.execute();
		slots_ref_.periodic[slot_index_].execute();
		slot_index_++;
		if (slot_index_ == slot_count) {
			slot_index_ = 0;
		}
		#if ROBO_APP_NET_FLOW_ENABLED == 1
		::robo::net::flow::machine::backend_poll();
		#endif
		tp::off(tp_verb::backend);
		tp::off(tp_verb::loop);
	}
	void machine::frontend_loop_(void) {
		tp::on(tp_verb::frontend);
		slots_ref_.frontend.execute();
		#if ROBO_APP_NET_FLOW_ENABLED == 1
		::robo::net::flow::machine::frontend_poll();
		#endif
		tp::off(tp_verb::frontend);
	}
	machine::slots& machine::slots_(void) {
		static machine::slots slots__;
		return slots__;
	}
	machine::slot& machine::slots::operator [] (machine::slot::kind _kind) {
		switch (_kind) {
		case slot::kind::begin:
		return begin;
		break;
		case slot::kind::start:
		return start;
		break;
		#if APP_MEXO_PRIORITY_SLOT_ENABLE == 1
		case slot::kind::priority:
		return priority;
		break;
		#endif
		case slot::kind::backend:
		return backend;
		break;
		case slot::kind::frontend:
		return frontend;
		break;
		default:
		ROBO_APP_CRASH();
		return dummy;
		}
	}
	void  machine::slots::free(void) {
		begin.free();
		start.free();
		#if APP_MEXO_PRIORITY_SLOT_ENABLE == 1
		priority.free();
		#endif
		backend.free();
		frontend.free();
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

	void machine::slot::delegat::attach(slot::kind _kind) {
		ROBO_APP_ASSERT(is_frontend__);
		ref* r = new ref(*this);
		guard__;
		r->attach_to(machine::slots_()[_kind].delegats_);
	}

	void machine::slot::delegat::attach(int _index) {
		ROBO_APP_ASSERT(is_frontend__);
		ref* r = new ref(*this);
		guard__;
		r->attach_to(machine::slots_()[_index].delegats_);
	}

	void machine::slot::delegat::attach(ref& _ref, int _index) {
		guard__;
		_ref.attach_to(machine::slots_()[_index].delegats_);
	}
	void machine::slot::delegat::attach(ref& _ref, slot::kind _kind) {
		guard__;
		_ref.attach_to(machine::slots_()[_kind].delegats_);
	}


	node::node(void) : ref_(*this), map_ref_(*this, 0), name_(RT("root")), owner_(nullptr)/*, auto_enabled_(true)*/ {
		ROBO_APP_ASSERT(is_frontend__);
	}

	node& node::root_(void) {
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
		, owner_(_owner == nullptr ? &root_() : _owner) {
		int key;
		if (_owner) {
			key = owner_->map_ref_.key();
			key = fast_hash(RT("."), key);
		}
		else {
			key = 0;
		}
		key = fast_hash(name_, key);
		map_ref_.set_key(key);
		ROBO_APP_ASSERT(map_ref_.attach_to(map_()));
		ref_.attach_to(owner_->childs_);
	}

	bool node::begin(void) {
		return  root_().reconfig();
	}

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

	void periodic_task::do_start(void) {
		machine::slot::delegat::ref** pref = refs_;
		int* ix = index_;
		for (size_t n = 0; n < ref_count_; ++n, ++pref, ++ix) {
			machine::slot::delegat::attach(**pref, *ix);
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
	periodic_task::periodic_task(cstr  _name, node* _owner)
		: task(_name, false, _owner) {}

	periodic_task::~periodic_task(void) {
		machine::slot::delegat::ref** pref = refs_;
		for (size_t n = 0; n < ref_count_; ++n, ++pref) {
			delete* pref;
		}
		delete[] refs_;
	}

	subsystem_handler::subsystem_handler(cstr  _name, subsystem* _subsystem)
		: node(_name, _subsystem ? _subsystem->owned_node() : nullptr)
		, ref_(*this) {
		if (_subsystem) {
			ref_.attach_to(_subsystem->handlers);
		}
	}

	dev::dev(cstr  _name, action_s& _action, present_s& _present)
		: node(_name, nullptr)
		, idle(*this)
		, actual_mode_(&idle)
		, backend_(this, &dev::backend__)
		, backend_ref_(backend_)
		, action_(_action)
		, present_(_present) {
		mexo::machine::slot::delegat::attach(backend_ref_, mexo::machine::slot::kind::backend);
		present_.mode = idle_id;
	}

	void dev::switch_to(int _mode_id) {
		ROBO_APP_ASSERT(is_backend__);
		if (_mode_id != present_.mode) {
			if (present_.mode != idle_id) {
				actual_mode_->do_stop();
			}

			if (_mode_id == idle_id) {
				actual_mode_ = &idle;
				present_.mode = idle_id;
			}
			else {
				mode* m = modes_.find(_mode_id);
				if (m == nullptr || !m->enabled()) {
					actual_mode_ = &idle;
					present_.mode = idle_id;
				}
				else {
					m->do_start();
					m->applay_action();
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
			switch_to(action_.mode);
		}
		if (action_.actual) {
			guard__;
			action_.actual = false;
			actual_mode_->applay_action();
		}
	}

	dev::mode::mode(int _index, cstr  _name, dev& _dev)
		: node(_name, &_dev)
		, ref_(*this, _index) {
		ref_.attach_to(_dev.modes_);
	};
}

#include "mexo/mexo.h"
void mexo_begin(void) {
	mexo::machine::begin();
}
void mexo_start(void) {
	mexo::machine::start();
}
void mexo_priority_loop(void) {
	mexo::machine::priority_loop();
}
void mexo_backend_loop(void) {
	mexo::machine::backend_loop();
	#if ROBO_APP_NET_FLOW_ENABLED==1	
	#endif
}
void mexo_frontend_loop() {
	mexo::machine::frontend_loop();
	#if ROBO_APP_NET_FLOW_ENABLED==1	
	#endif
}
