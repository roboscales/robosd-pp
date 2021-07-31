#include "core/robosd_frontend.hpp"
#include "core/robosd_backend.hpp"
#include "core/robosd_log.hpp"
#include "core/robosd_system.hpp"
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
		, servo((uint8_t)_servo)
	{

	}
	dev_id_t::dev_id_t(void)
		: value(0) {
	};

	dev_id_t::dev_id_t(const dev_id_t& _src)
		: value(_src.value) {
	};
	dev_id_t& dev_id_t::operator = (const dev_id_t& _src) {
		value = _src.value;  return *this;
	}
	dev_id_t& dev_id_t::operator = (const int& _src) {
		value = (unsigned int)_src;  return *this;
	}

	signal::performer::performer(bool _once)
		: ref_(*this,priority::normal)
		, once_(_once)
	{
	}

	signal::performer::~performer(void) {
	}

	bool signal::performer::attach_to(signal* _signal, priority _priority) {
		ref_.set_key(_priority);
		ROBO_LRET( ref_.attach_to(_signal->performers) )
	}

	void signal::performer::dettach(void) {
		ref_.dettach();
	}

	signal::signal(void)  {
	}

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
		void queue::poll_(void) {
			performer* p = nullptr;
			{
				system::guard g_;
				p = performers.pop();
			}
			if (p != nullptr) {
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
			::robo::backend::timer::core::start( &(execute_delegat_), period_);
		}

		void timer::stop_(void) {
			::robo::backend::timer::core::stop(&(execute_delegat_), period_);
		}

		void timer::execute_(void) {
			if (backend_performer_)(*(backend_performer_))();
			queue::post(&frontend_performer_, signal::performer::priority::hi);
		}

		void timer::start(time_us_t _period) {
			period_ = _period;
			queue::post(&start_delegat_, signal::performer::priority::hi);
		}

		void timer::stop(void) {
			backend::queue::post(&stop_delegat_, signal::performer::priority::hi);
		}		

		void command::execute_(void) {
			if (performer_ == nullptr) {
				configure_();
				ROBO_VBREAKN(performer_ != nullptr );
			};
			(*performer_)(*this);
		}

		void command::configure_(void) {
			{
				system::guard g__;
				performer_ = performer::map_().find(id_);
			}
			ROBO_ALARMN_F(performer_ != nullptr, "command performer '%s' is't found", name_);
		}

		void command::configure(void) {
			queue::post(&configure_delegat_, signal::performer::priority::hi);
		}

		void command::execute(void) {
			queue::post(&execute_delegat_, signal::performer::priority::hi);
		}

		command::performer::map& command::performer::map_(void) {
			static map instance_;
			return instance_;
		}

#if		ROBO_APP_MODULE_ENABLED == 1
		bool contrltable::ivar::query_(void) {
			ROBO_LBREAKN(begin_hook());
			status_ = status::get;
			ROBO_LRET(rerquest() );
		}

		bool contrltable::ivar::post_(void) {
			ROBO_LBREAKN(begin_hook());
			status_ = status::put;
			ROBO_LRET(rerquest());
		}

		bool contrltable::ivar::query(void) {
			reset_delegat();
			ROBO_LRET( query_() );
		}

		bool contrltable::ivar::post(void) {
			reset_delegat();
			ROBO_LRET(post_());
		}

		bool contrltable::ivar::query(delegat& _delegat) {
			reset_delegat();
			static_delegat_ = &_delegat;
			dynamic_delegat_ = nullptr;
			ROBO_LRET(query_());
		}
		bool contrltable::ivar::post(delegat& _delegat) {
			reset_delegat();
			static_delegat_ = &_delegat;
			dynamic_delegat_ = nullptr;
			ROBO_LRET(post_());
		}
		void contrltable::ivar::reset_delegat(void){
			if (dynamic_delegat_) {
				delete dynamic_delegat_;
				dynamic_delegat_ = nullptr;
			}
			static_delegat_ = nullptr;
			set_repeat_count(repeat_current_max_);
		}
		bool contrltable::ivar::query(robo::lambda<void(ivar&, bool)>& _lambda) {
			reset_delegat();
			dynamic_delegat_ = new ::robo::delegat::slambda<void, ivar&, bool>(_lambda) ;
			static_delegat_ = dynamic_delegat_;
			ROBO_LRET(query_());
		}
		bool contrltable::ivar::post(robo::lambda<void(ivar&, bool)>& _lambda) {
			reset_delegat();
			dynamic_delegat_ = new ::robo::delegat::slambda<void, ivar&, bool>(_lambda);
			static_delegat_ = dynamic_delegat_;
			ROBO_LRET(post_());
		}


		contrltable::ivar::ivar(contrltable& _contrltable, const record& _instance) 
			: contrltable_(_contrltable)
			, map_ref_(*this, 0)
			, instance_(_instance)
		{
			map_ref_.set_key( fast_hash( instance_.name,0));
			ROBO_ALARMN_F(map_ref_.attach_to(contrltable_.vars) , "eroor attach var '%s' to map ", instance_.name);
		}

		void contrltable::ivar::confirm(void) {
			switch (status_) {
				case status::put:
					status_ = status::ready;
					finish_hook();
					query_();
					break;
				case status::get:
					status_ = status::ready;
					finish_hook();					
					if (static_delegat_) (*static_delegat_) (*this, true);
					break;
				default:
					ROBO_ALARM_F("error state for var '%s'", name());
			}
		}

		void contrltable::ivar::refuse(void) {
			status tmp = status_;
			status_ = status::panic;
			if (static_delegat_) (*static_delegat_) (*this, false);
			if( repeat_count_ > 0 ) {
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
			} else {
				finish_hook();
			}
		}

		bool contrltable::ivar::begin_hook(void) {
			ROBO_LBREAKN_F(hook_ == hook::free, "error hook state for var '%s'", name());
			if ( system::env::is_frontend() ) {
				hook_ = hook::frontend;
			}
			else {
				hook_ = hook::backend;
			}
			return true;
		}

		void contrltable::ivar::finish_hook(void) {
			if (system::env::is_frontend()) {
				if(hook_ != hook::frontend){
					robo_errlog("error hook state for var '%s'", name());					
				}
			}
			else {
				if(hook_ != hook::backend){
					robo_errlog("error hook state for var '%s'", name());
				}
			}
			hook_ = hook::free;
		}

		contrltable::contrltable(node& _owner, const record * _records, size_t _count) 
			: node( RT("ct"), &_owner)
			, records_(_records)
			, count_(_count){
		}

		contrltable::ivar * contrltable::ivar::create_var(cstr _path, cstr _name) {
			app::node * _node = app::node::find(_path);
			ROBO_BREAKN_F(_node,nullptr, "invalid path: '%s' ", _path);
			contrltable* ct = dynamic_cast<contrltable *>(_node);
			ROBO_BREAKN_F(ct, nullptr, "invalid object '%s' ", _path);
			return ct->create_var(_name);
		}


		const contrltable::record* contrltable::find_record(cstr _name) {
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

		const contrltable::record & contrltable::find_record_ref(cstr _name) {
			const record* r = find_record(_name);
			ROBO_APP_ASSERT(r != nullptr);
			return *r;
		}

		contrltable::ivar* contrltable::create_var(cstr _name) {
			ivar* v = find_var( _name );
			if (v) {
				return v;
			}
			else {
				const record* _record = find_record(_name);

				ROBO_BREAKN_F(_record, nullptr, "invalid record: '%s' ", _record->name);

				fabric* f = fabric::find(_record->type);

				ROBO_BREAKN_F(_record, nullptr, "invalid fabric: '%s' ", _record->type);

				v = f->create(*this, *_record);

				ROBO_BREAKN_F(v, nullptr, "error create var '%s' with type '%s' ", _record->name, _record->type );

				return v;

			}
		}

		contrltable::fabric::map& contrltable::fabric::fabrics(void) {
			static map fabrics_;
			return fabrics_;;
		}

		contrltable::fabric::fabric(cstr _type) : ref_(*this,0) {
			ref_.set_key( fast_hash(_type) );
			ref_.attach_to( fabrics() );
		}

		contrltable::fabric* contrltable::fabric::find(cstr _type) {
			return fabrics().find(fast_hash(_type));
		}
		
		contrltable::ivar * contrltable::find_var(cstr _name){
			return vars.find( fast_hash(_name) );
		}
#endif

	}
}

