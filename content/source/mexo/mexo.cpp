#include "mexo/mexo.hpp"
#include "core/robosd_log.hpp"
namespace mexo {
	machine machine::instance_;
	machine::machine(void) 
		: slots_ref_(slots_())
		, slot_index_(0){
	}
	machine::~machine(void) {

	}
	void machine::begin_(void) {
		slots_ref_.begin.execute();
	}
	void machine::start_(void) {
		slots_ref_.start.execute();
		ROBO_APP_ASSERT(::mexo::node::begin());

	}
	void machine::priority_loop_(void) {
		system::fall f__;
		slots_ref_.priority.execute();
	}
	void machine::backend_loop_(void) {
		system::fall f__;
		slots_ref_.backend.execute();
		slots_ref_.periodic[slot_index_]. execute();
		slot_index_++;
		if (slot_index_ == slot_count) {
			slot_index_ = 0;
		}
	}
	void machine::frontend_loop_(void) {
		slots_ref_.frontend.execute();
	}
	machine::slots& machine::slots_(void) {
		static machine::slots slots__;
		return slots__;
	}
	machine::slot& machine::slots::operator [] (machine::slot::kind _kind) {
		switch (_kind){
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

	machine::slot::slot(void) {
	}

	machine::slot::~slot(void) {
	}

	void machine::slot::execute(void) {
		for (delegat::ref* r = delegats_.first(); r; r = r->next()) {
			r->owner()();
		}
	}

	void machine::slot::free(void) {
		delegat::ref* r;
		while ( ( r = delegats_.first() ) != nullptr ){
			delete r;
		}
	}

	void machine::slot::delegat::attach(slot::kind _kind) {
		ROBO_APP_ASSERT(system::env::is_frontend());
		ref* r = new ref(*this);
		robo::system::guard g__;
		r->attach_to(machine::slots_()[_kind].delegats_);
	}

	void machine::slot::delegat::attach(int _index) {
		ROBO_APP_ASSERT(system::env::is_frontend());
		ref* r = new ref(*this);
		robo::system::guard g__;
		r->attach_to(machine::slots_()[_index].delegats_);
	}

	void machine::slot::delegat::attach(ref & _ref, int _index) {
		robo::system::guard g__;
		_ref.attach_to(machine::slots_()[_index].delegats_);
	}
	void machine::slot::delegat::attach(ref& _ref, slot::kind _kind) {
		robo::system::guard g__;
		_ref.attach_to(machine::slots_()[_kind].delegats_);
	}


	node::node(void) : ref_(*this), map_ref_(*this, 0), name_(RT("root")), owner_(nullptr)/*, auto_enabled_(true)*/ {
		ROBO_APP_ASSERT(system::env::is_frontend());
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
	
	/*void node::enable(bool _hand) {
		if (auto_enabled_ || _hand) {
			for (ref* r = childs_.first(); r; r = r->next()) {
				r->owner().enable();
			}
			do_enable();
			enabled_ = true;
		}
	}
	void node::disable(void) {
		for (ref* r = childs_.first(); r; r = r->next()) {
			r->owner().disable();
		}
		do_disable();
		enabled_ = false;
	}*/

	node::node(cstr _name, /*bool _auto_enabled,*/ node* _owner) : ref_(*this), map_ref_(*this, 0)
		, name_(_name)
		, owner_(_owner == nullptr ? &root_() : _owner)
	//	, auto_enabled_(_auto_enabled) 
	{
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
		//root_().enable();
		return  root_().reconfig();
	}

	isubsystem::isubsystem(cstr  _name, bool _autostart, node* _owner) : node(_name, _owner), ref_(*this), autostart_(_autostart)  {
		isubsystem* m = dynamic_cast<isubsystem*>(_owner);
		if (m != nullptr) {
			ref_.attach_to(m->childs_);			
		}
	}

	void isubsystem::execute(void) {
		for (iblock::ref* r = blocks_.first(); r; r = r->next()) {
			r->owner().execute();
		}
		for (isubsystem::ref* r = childs_.first(); r; r = r->next()) {
			r->owner().execute();
		}
	}

	void isubsystem::start(void) {
		for (isubsystem::ref* r = childs_.first(); r; r = r->next()) {
			r->owner().start();
		}
		do_start();
	}

	void isubsystem::stop(void) {
		for (isubsystem::ref* r = childs_.first(); r; r = r->next()) {
			r->owner().stop();
		}
		do_stop();
	}

	iblock::iblock(isubsystem& _subsystem, cstr  _name, config_s& _config)
		: node(_name, &_subsystem)
		, ref_(*this)
		, config(_config) {
			ref_.attach_to(_subsystem.blocks_);
	};

	subsystem::subsystem(cstr  _name, bool _autostart, node* _owner) : isubsystem(_name, _autostart, _owner) {
	};
	
	void subsystem::operator()(void) {
		isubsystem::execute();
	}



	dev::mode::mode(int _index, cstr  _name, dev& _dev)
		: isubsystem(_name, false, &_dev)
		, ref_(*this, _index) {
		ref_.attach_to(_dev.modes_);
	};

	/*dev::control::command dev::mode::start_(void) {
		start();
		return do_reset();
	}*/

	dev::dev(cstr  _name, action& _action, snapshot& _snapshot)
		: node(_name, nullptr)
		, idle(*this)
		, actual_mode_(&idle)
		, actual_mode_id_(idle_id)
		, backend_(this, &dev::backend__)
		, backend_ref_(backend_)
		, action_(_action)
		, snapshot_(_snapshot)
	{
		mexo::machine::slot::delegat::attach(backend_ref_, mexo::machine::slot::kind::backend);
	}

	void dev::switch_to(int _mode_id) {
		ROBO_APP_ASSERT(system::env::is_backend());
		if (_mode_id != actual_mode_id_) {
			if (actual_mode_id_ != idle_id) {
				actual_mode_->stop();
			}

			if (_mode_id == idle_id) {
				actual_mode_ = &idle;
				actual_mode_id_ = idle_id;
			}
			else {
				mode* m = modes_.find(_mode_id);
				if (m == nullptr || ! m->enabled() ) {
					actual_mode_ = &idle;
					actual_mode_id_ = idle_id;
				}
				else {
					m->start();
					m->applay_action();
					actual_mode_ = m;
					actual_mode_id_ = _mode_id;
				}
			}
		}
	}
	void dev::backend__(void) {
		ROBO_APP_ASSERT(system::env::is_backend());

		robo::system::guard g__;

		if (action_.mode != actual_mode_id_) {
			switch_to(action_.mode);
		}

		if (action_.actual) {
			action_.actual = false;
			actual_mode_->applay_action();
		}

	}

}
