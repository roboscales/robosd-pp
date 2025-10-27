#include "servo/robosd_frontend.hpp"
#include "servo/robosd_backend.hpp"
#include "core/robosd_log.hpp"
#include "core/robosd_system.hpp"
#include <cstring>
namespace robo {
	dev_id_t::dev_id_t(
		uint8_t _servo,
		uint8_t _module,
		uint8_t _dev,
		uint8_t _bus,
		uint8_t _adress)
		: address((uint8_t)_adress)
		, bus((uint8_t)_bus)
		, dev((uint8_t)_dev)
		, module((uint8_t)_module)
		, servo((uint8_t)_servo) {

	}
	dev_id_t::dev_id_t(void)
		: value(0) {};

	dev_id_t::dev_id_t(const dev_id_t& _src)
		: value(_src.value) {};
	dev_id_t& dev_id_t::operator = (const dev_id_t& _src) {
		value = _src.value;  return *this;
	}
	dev_id_t& dev_id_t::operator = (const int& _src) {
		value = (unsigned int)_src;  return *this;
	}

	signal::performer::performer(bool _once)
		: ref_(*this, priority::normal)
		, once_(_once) {}

	signal::performer::~performer(void) {}

	bool signal::performer::attach_to(signal* _signal, priority _priority) {
		ref_.set_key(_priority);
		if (!ref_.attached()) {
			ROBO_LBREAKN(ref_.attach_to(_signal->performers))
			attach();
		}
		return true;
	}

	void signal::performer::dettach(void) {
		ref_.dettach();
	}

	signal::signal(void) {}

	void event::raise(void) {
		event::performer::ref* _ref = performers.first();
		while (_ref) {
			performer::ref* tmp = _ref;
			performer* p = &(tmp->owner());
			_ref = _ref->next();
			(*p)();
			if (p->once()) {
				p->dettach();
			}
		}

	}

	namespace frontend {
		queue::~queue(void) {
			performer* p = nullptr;

			do {
				{
					system::guard g_;
					p = performers.pop();
				}
				if (p != nullptr) {
					p->dettach();
				}
				else {
					break;
				}
			} while (true);

		}
		void queue::poll_(void) {
			performer* p = nullptr;
			{
				system::guard g_;
				p = performers.pop();
			}
			if (p != nullptr) {
				p->dettach();
				(*p)();
			}
		}

		void queue::post_(signal::performer* _performer, signal::performer::priority _priority) {
			system::guard l__;
			_performer->attach_to(this, _priority);
		}
		queue& queue::instance_() {
			static queue inst_;
			return inst_;
		}



		void timer::start_(void) {
			::robo::backend::timer::core::start(&(execute_delegat_), period_);
		}

		void timer::stop_(void) {
			::robo::backend::timer::core::stop(&(execute_delegat_), period_);
		}

		void timer::execute_(void) {
			if (backend_performer_)(*(backend_performer_))();
			queue::post(frontend_performer_, signal::performer::priority::hi);
		}

		void timer::start(time_us_t _period) {
			period_ = _period;
			backend::queue::post(&start_delegat_, signal::performer::priority::hi);
		}

		void timer::stop(void) {
			backend::queue::post(&stop_delegat_, signal::performer::priority::hi);
		}



		void pulse::start_(void) {
			::robo::backend::timer::core::start(&(execute_delegat_), period_);
		}

		void pulse::stop_(void) {
			::robo::backend::timer::core::stop(&(execute_delegat_), period_);
		}

		void pulse::execute_(void) {
			if (backend_performer_) {
				backend_performer_->dettach();
				(*(backend_performer_))();
			}
			backend_performer_ = nullptr;
			queue::post(frontend_performer_, signal::performer::priority::hi);
			frontend_performer_ = nullptr;
		}

		void pulse::start(time_us_t _period) {
			period_ = _period;
			backend::queue::post(&start_delegat_, signal::performer::priority::hi);
		}

		void pulse::stop(void) {
			backend::queue::post(&stop_delegat_, signal::performer::priority::hi);
		}


		#if		ROBO_APP_MODULE_ENABLED == 1
		bool vartable::ivar::query_(void) {
			ROBO_LBREAKN(begin_hook());
			status_ = status::get;
			ROBO_LRET(rerquest());
		}

		bool vartable::ivar::post_(void) {
			ROBO_LBREAKN(begin_hook());
			status_ = status::put;
			ROBO_LRET(rerquest());
		}


		bool vartable::ivar::query_a(performer * _performer) {
			performer_ = _performer;
			ROBO_LRET(query_());
		}

		bool vartable::ivar::post_a(performer* _performer) {
			performer_ = _performer;
			ROBO_LRET(post_());
		}

		void vartable::ivar::reset_delegat(void) {
			performer_ = nullptr;
			set_repeat_count(repeat_current_max_);
		}

		vartable::ivar::ivar(vartable& _vartable, const record& _instance)
			: vartable_(_vartable)
			, map_ref_(*this, 0)
			, instance_(_instance) {
			map_ref_.set_key(hash(instance_.name, 0));
			system::guard g__;
			ROBO_ALARMN_F(map_ref_.attach_to(vartable_.vars), "eroor attach var '%s' to map ", instance_.name);
		}
		void vartable::ivar::confirm_(void)
		{
			if (performer_) performer_->confirm(this);
		}
		void vartable::ivar::refuse_(void)
		{
			if (performer_) performer_->refuse(this);	
		}

		void vartable::ivar::confirm(void) {
			switch (status_) {
			case status::put:
			status_ = status::ready;
			finish_hook();
			if (paranoic_put_) {
				query_();
				return;
			}
			else {
				confirm_();
			}
			break;
			case status::get:
			status_ = status::ready;
			finish_hook();
			/*if (paranoic_put_)
			{
				if (put_complete() )
				{					
					confirm_();
				}
				else
				{
					refuse_();										
				}				
			}
			else
			{
				confirm_();
			}*/
			confirm_();
			break;
			default:
				refuse_();
				ROBO_ALARM_F("error state for var '%s'", name());
			}
			reset_delegat();
		}

		void vartable::ivar::refuse(void) {
			status tmp = status_;
			status_ = status::panic;
			if (repeat_count_ > 0) {
				repeat_count_--;
				finish_hook();
				switch (tmp) {
				case status::put:
				robo_infolog("var '%s' sent again", name());
				post_();
				break;
				case status::get:
				robo_infolog("var '%s' query again", name());
				query_();
				break;
				default:
				ROBO_ALARM_F("error state for var '%s'", name());
				}
			}
			else {
				if (performer_) performer_->refuse(this);
				finish_hook();
				reset_delegat();
			}
		}

		bool vartable::ivar::begin_hook(void) {
			ROBO_LBREAKN_F(hook_ == hook::free, "error hook state for var '%s'", name());
			if (system::env::is_frontend()) {
				hook_ = hook::frontend;
			}
			else {
				hook_ = hook::backend;
			}
			return true;
		}

		void vartable::ivar::finish_hook(void) {
			if (system::env::is_frontend()) {
				if (hook_ != hook::frontend) {
					robo_errlog("error hook state for var '%s'", name());
				}
			}
			else {
				if (hook_ != hook::backend) {
					robo_errlog("error hook state for var '%s'", name());
				}
			}
			hook_ = hook::free;
		}

		vartable::vartable(node& _owner, const record* _records, size_t _count)
			: node(RT("vt"), &_owner)
			, records_(_records)
			, count_(_count) {}

		vartable::ivar* vartable::ivar::create_var(cstr _path, cstr _name) {
			app::node* _node = app::node::find(_path);
			ROBO_BREAKN_F(_node, nullptr, "invalid path: '%s' ", _path);
			vartable* ct = dynamic_cast<vartable*>(_node);
			ROBO_BREAKN_F(ct, nullptr, "invalid object '%s' ", _path);
			return ct->create_var(_name);
		}


		const vartable::record* vartable::find_record(cstr _name) {
			const record* r = records_;
			for (size_t i = 0; i < count_; ++i, ++r) {
				#if ROBO_UNICODE_ENABLED ==1
				if (std::wcscmp(r->name, _name) == 0) {
					return r;
				}
				#else 
				if (std::strcmp(r->name, _name) == 0) {
					return r;
				}
				#endif
			}
			return nullptr;
		}

		const vartable::record& vartable::find_record_ref(cstr _name) {
			const record* r = find_record(_name);
			ROBO_APP_ASSERT(r != nullptr);
			return *r;
		}

		vartable::ivar* vartable::create_var(cstr _name) {
			ivar* v = find_var(_name);
			if (v) {
				return v;
			}
			else {
				const record* _record = find_record(_name);
				ROBO_BREAKN_F(_record != nullptr, nullptr, "invalid record: '%s' ", _name);

				fabric* f = fabric::find(_record->type);

				ROBO_BREAKN_F(f, nullptr, "invalid fabric: '%s' ", _record->type);

				{
					v = f->create(*this, *_record);
				}

				ROBO_BREAKN_F(v, nullptr, "error create var '%s' with type '%s' ", _record->name, _record->type);

				return v;

			}
		}

		void vartable::ivar::performer::post_(void) {
			if (isfrontend_) {
				::robo::frontend::queue::post(this, priority::lo);
			} else {
				::robo::backend::queue::post(this, priority::lo);
			}
		}

		vartable::fabric::map& vartable::fabric::fabrics(void) {
			static map fabrics_;
			return fabrics_;
		}

		vartable::fabric::fabric(cstr _type) : ref_(*this, 0) {
			ref_.set_key(hash(_type));
			system::guard g__;
			ref_.attach_to(fabrics());
		}

		vartable::fabric* vartable::fabric::find(cstr _type) {
			system::guard g__;
			return fabrics().find(hash(_type));
		}

		vartable::ivar* vartable::find_var(cstr _name) {
			system::guard g__;
			return vars.find(hash(_name));
		}
		#endif


		void shared::action::execute_(void) {
			(owner_->*member_)();
			{
				system::guard g__;
				tuple_->unuse_();
				tuple_ = nullptr;
			}
		}
		shared::action::action(
			shared* _owner
			, void (shared::* _member) (void)
		)
			: owner_(_owner)
			, member_(_member)
			, run_(*this, &action::execute_) {
		}

		void shared::action::attach(tuple* _tuple) {
			bool need = false;
			{
				system::guard g__;
				if (tuple_ == nullptr) {
					tuple_ = _tuple;
					_tuple->use_();
					need = true;
				}
			}
			if (need) backend::queue::post(&run_, signal::performer::priority::hi);
		}

		void shared::tuple::unuse_(void) {
			if (counter_ > 0) {
				counter_--;
				if (counter_ == 0) {
					try_release_();
				}
			}
		}

		void shared::tuple::try_release_(void) {
			if (counter_ == 0) {
				if (on_complete_) {
					frontend::queue::post(on_complete_, signal::performer::priority::hi);
				}
				on_complete_ = nullptr;
				{
					ref_.attach_to(shared::core_().pool_);
				}
			}
		}


		/*		void shared::core::complete_(tuple* _tuple) {
					if (_tuple->complete_())
						delete _tuple;
				}*/

		shared::tuple* shared::tuple::get(signal::performer* _on_complete) {
			ROBO_APP_ASSERT(system::env::is_frontend());
			tuple* ret;
			{
				system::guard g__;
				ret = shared::core_().pool_.pop();
			}
			if (ret == nullptr) {
				ret = new  tuple;
			}
			ret->on_complete_ = _on_complete;
			return ret;
		}
		void shared::core::apply_action_(void* _begin, void* _end, signal::performer* _on_apply_action) {
			tuple* t = tuple::get(_on_apply_action);
			for (ref* r = shared::core_().list_.first(); r; r = r->next()) {
				if (r->owner().is_my_action_(_begin, _end)) {
					r->owner().apply_action_.attach(t);
				}
			}
			{
				system::guard g__;
				t->try_release_();
			}
		}

		void shared::core::exchange_(void* _begin, void* _end, signal::performer* _on_exchange) {
			tuple* t = tuple::get(_on_exchange);
			for (ref* r = shared::core_().list_.first(); r; r = r->next()) {
				if (r->owner().is_my_action_(_begin, _end)) {
					r->owner().exchange_.attach(t);
				}
			}
			{
				system::guard g__;
				t->try_release_();
			}
		}


		void shared::core::update_feedback_(void* _begin, void* _end, signal::performer* _on_update_feedback_) {
			tuple* t = tuple::get(_on_update_feedback_);
			for (ref* r = list_.first(); r; r = r->next()) {
				if (r->owner().is_my_feedback_(_begin, _end)) {
					r->owner().update_feedback_.attach(t);
				}
			}
			{
				system::guard g__;
				t->try_release_();
			}
		}

		shared::core& shared::core_(void) {
			static core core__;
			return core__;
		}
		shared::core::~core(void) {
			tuple* tmp;
			while ((tmp = pool_.pop()) != nullptr)
				delete tmp;
		}
		bool shared::is_my_action_(void* _begin, void* _end) {
			return _begin <= action_addr_begin_ && action_addr_end_ <= _end;
		}

		bool shared::is_my_feedback_(void* _begin, void* _end) {
			return _begin <= feedback_addr_begin_ && feedback_addr_end_ <= _end;
		}
	}

	quest::quest(
		quest* _owner
		, quest* _sema
		, request* _request
		, answer* _answer
	)
		: signal::performer(true)
		, ref_(*this)
		, sema_ref_(*this)
		, top_ref_(*this)
		, isfrontend_ ( system::env::is_frontend() )
		, request_(_request)
		, answer_(_answer)
	{
		if (_owner != nullptr) {
			owner_ = _owner;
			ref_.attach_to(_owner->childs_);
			owner_->top_ref_.dettach();
		}
		else {
			owner_ = nullptr;
		}
		if (_request != nullptr) {
			_request->attach();
			if (isfrontend_) {
				system::critical c_;
				top_ref_.attach_to(frontend_core::instance_().top_);
			}
			else {
				top_ref_.attach_to(backend_core::instance_().top_);
			}
		}
		if (answer_ != nullptr) {
			answer_->attach();
		}
		counter::instance_().inc();
		if (_sema != nullptr  ) {
			ROBO_VBREAKN( (_sema != this) && (_sema != owner_) );
			quest * r = new quest(this, nullptr, nullptr, nullptr);
			ROBO_VBREAKN( r != nullptr );
			r->sema_ref_.attach_to(_sema->owned_);
		}
	}
	quest::backend_core& quest::backend_core::instance_(void) {
		static backend_core  instance__;
		return instance__;
	}

	void quest::post(void) {
		if (system::env::is_frontend()) {
			system::critical c_;
			frontend_core::instance_().request();
		} else {
			backend_core::instance_().request();
		}
	}

	void quest::backend_core::request(void) {
		ref* r = top_.last();
		while (r) {
			ref* tmp = r;
			r = r->prev();
			ROBO_JAMPN_F(tmp->owner().status_ == status::none, nxt, "error status %d", (int)tmp->owner().status_);
			tmp->owner().status_ = status::run;
			ROBO_JAMPN_F( tmp->owner().isfrontend_ == false, nxt, "backend required");
			frontend::queue::post(&(tmp->owner()), priority::lo);
			tmp->dettach();
			nxt:;
		}
	}

	void quest::frontend_core::request(void) {
		ref* r = top_.last();
		while (r) {
			ref* tmp = r;
			r = r->prev();
			ROBO_JAMPN_F(tmp->owner().status_ == status::none,nxt,"error status %d",(int)tmp->owner().status_);
			tmp->owner().status_ = status::run;
			ROBO_JAMPN_F(tmp->owner().isfrontend_ == true , nxt, "frontend required");
			backend::queue::post(&(tmp->owner()), priority::lo);
			tmp->dettach();
			nxt:;
		}
	}
	void  quest::happyend_(void) {
		owned_confirm_();
		quest* own = owner_;
		release();
		if (own) {
			if (own->childs_.count() == 0) {
				ROBO_VBREAKN(own->status_ == status::none);
				ROBO_VBREAKN(own->isfrontend_ == robo::system::env::is_frontend());
				own->status_ = status::run;
				if (own->isfrontend_) {
					backend::queue::post(own, priority::lo);
				}
				else {
					frontend::queue::post(own, priority::lo);
				}
			}
		}
	}

	void quest::owned_refuse_(void) {
		ref* r = owned_.last();
		while (r) {
			quest* tmp = &(r->owner());
			r = r->prev();
			tmp->status_ = status::refuse;
		}
	}
	void quest::owned_confirm_(void) {
		ref* r = owned_.last();
		while (r) {
			quest* tmp = &(r->owner());
			r = r->prev();
			tmp->status_ = status::confirm;
		}
	}

	void  quest::operator ()(void) {
		switch (status_) {
		case status::run:
			ROBO_VBREAKN(system::env::is_frontend() != isfrontend_);
			if (request_ == nullptr ) {
				refuse();
			}
			else {
				use_++;
				(*request_)(this);
			}
			return;
		case status::confirm:		
			if (use_ == 0) {
				release();
				return;
			}
			else {
				release();
			}
			status_ = status::discarde;
			ROBO_VBREAKN(system::env::is_frontend() == isfrontend_);
			if (answer_) {
				switch  ( (*answer_)(result::success) ) 
				{
				case reaction::terminate:
					terminate();
					return;
				case reaction::normal:
					break;
				}
			}
			else {
				owned_confirm_();
			}


			break;
		case status::refuse:
			if (use_ == 0) {
				release();
				return;
			}
			else {
				release();
			}
			status_ = status::discarde;
			ROBO_VBREAKN(system::env::is_frontend() == isfrontend_);
			if (answer_) {
				switch ((*answer_)(result::refuse)) {
				case reaction::terminate:
				terminate();
				return;
				case reaction::normal:
				break;
				}
			}
			else {
				terminate();
				return;
			}
			break;
		}
		happyend_();
	}

	void quest::post_answer_(status _status) {
		ROBO_VBREAKN(status_==status::run);
		status_ = _status;
		if (isfrontend_) {
			ROBO_VBREAKN( system::env::is_frontend() == false);
			frontend::queue::post(this, priority::lo);
		} else {
			ROBO_VBREAKN(system::env::is_frontend() == true);
			backend::queue::post(this, priority::lo);
		}
	}
	void quest::confirm(void) {
		post_answer_(status::confirm);
		ref* r = owned_.last();
		while (r) {
			quest* tmp = &(r->owner());
			r = r->prev();
			tmp->status_ = status::confirm;
//			tmp-> status_ = status::run;
//			tmp->confirm();
		}
	}
	void quest::refuse(void) {
		post_answer_(status::refuse);
	}
	void quest::terminate(void) {
		owned_refuse_();

		quest* tmp = this;
		while (tmp->owner_ != nullptr) {
			tmp = tmp->owner_;
		}
		tmp->release();
	}

	frontend::varindex::node& frontend::varindex::record::root() {
		static node root__(RT(""), nullptr);
		return root__;
	}
	frontend::varindex::node*& frontend::varindex::record::current_() {
		static node* current__ = &root();
		return current__;
	}
	void frontend::varindex::record::push(::robo::cstr _name) {
		current_() = new node(_name, current_());
	}
	void frontend::varindex::record::pop(void) {
		ROBO_VBREAKN_F(current_()->branch(), RT("the current node has no parent "))
		current_() = current_()->branch();
	}

	frontend::servo::servo(robo::cstr _name, robo::app::module& _module)
		: robo::app::node(_name, &_module), reboot_termo_cmd_(*this){
	}

	bool frontend::servo::do_load(void) {
		ROBO_LBREAKN(robo::app::node::do_load())
		
		#if ROBO_APP_TERMINAL_ENABLED
		termoserial_name_.tryload(current_path(), RT("termo_serial"));
		#endif
		return true;
	}

	

	bool frontend::servo::do_start(void) {
		ROBO_LBREAKN(robo::app::node::do_start())

#if ROBO_APP_TERMINAL_ENABLED
		termoserial_ = robo::net::iserial::query<robo::net::iserial>(termoserial_name_.c_str());
		if (termoserial_) {
			robo::termo::itf::connect(termoserial_);

			robo::termo::itf::set_prompt(robo::string(RT("%s>"), alias() ).ascii() );
			robo::termo::itf::exec("w\n\r");
			robo::termo::itf::new_line();
		}
#endif
		return true;
	}
#if ROBO_APP_TERMINAL_ENABLED
	frontend::servo::root_termo_cmd::root_termo_cmd(void)
		: ::robo::termo::node(
			RT8("servo")
			, RT8("servo commands")
			, RT8("servo <CR>")
			, robo::termo::itf::root()
		) {
	}
	bool frontend::servo::reboot_termo_cmd_s::begin(void) {
		return servo_.reconfig_command(nullptr,nullptr);
	}
	bool frontend::servo::reboot_termo_cmd_s::parse_long_arg(const char* _arg, const char* _val) {
		return true;
	}
	bool frontend::servo::reboot_termo_cmd_s::parse_arg(char _arg, const char* _val) {
		return true;
	}
	frontend::servo::reboot_termo_cmd_s::reboot_termo_cmd_s(servo& _servo) 
		: ::robo::termo::node(
			"reconfig"
			, " devagent reconfig  "//const char * note; 
			, "reconfig <CR>" //const char * usage;  
			, &_servo.root_termo_cmd_
		)
		, servo_(_servo) {
	}
#endif
	frontend::devagent::devagent(robo::cstr _name, robo::app::node& _owner, action_s& _goal, feedback_s& _feedback)
		: app::node(_name, &_owner)
		, goal_(_goal)
		, feedback_(_feedback)
		, devdiscovery_(*this)
		, devstopped_(*this)
		, devconfigure_(*this)
	{
	}
	bool frontend::devagent::do_load(void) {
		ROBO_LBREAKN(robo::app::node::do_load());
		bool tmp;
		ROBO_LBREAKN(ini::load(current_path(), defaults_path(), RT("ENABLED"), tmp));

		if (tmp) {
			//feedback_.status.connection = statuses::connections::discovery;
			discovery_begin();
			devcontroller.run();
		}
		return true;
	}
	void frontend::devagent::do_clean(void) {
		robo::app::node::do_clean();
	}
	bool frontend::devagent::do_start(void) {
		ROBO_LBREAKN(robo::app::node::do_start());
		return true;
	}
	void   frontend::devagent::discovery_begin(void) {
		robo::system::guard g__;
		devcontroller.switchto(&devdiscovery_);
		//incom_total = trafic.incom.success.bytes.total;
	}
	void   frontend::devagent::do_discovery_complete(void) {
		devcontroller.switchto(&devconfigure_);
	};
	void  frontend::devagent::do_configure_complete(void) {
		//configure_quest_ = nullptr;
		devcontroller.switchto(&devstopped_);
	}

}

