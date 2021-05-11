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
	}
}

