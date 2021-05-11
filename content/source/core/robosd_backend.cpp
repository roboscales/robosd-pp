#include "core/robosd_backend.hpp"
#include "core/robosd_system.hpp"
namespace robo {
	namespace backend {
		void queue::poll_(void) {
			performer* p = nullptr;
			{
				system::guard l_;
				p = performers.pop();
			}
			if (p != nullptr) {
				(*p)();
			}
		}

		void queue::post_(signal::performer* _performer, signal::performer::priority _priority) {
			system::guard g_;
			_performer->attach_to(this, _priority);
		}

		queue& queue::instance_(void) {
			static queue inst_;
			return inst_;
		}

		bool queue::wait_(time_ms_t _timeout) {
			time_ms_t ms = system::env::time_ms();
			do {
				if (ready()) return true;
				system::env::sleep(); //если есть ос то просто отдаем контекст, чтобы зря не стоять
			} while (system::env::time_ms() - ms < _timeout);
			// такого быть не должно никогда
			return false;
		}

		bool queue::ready_(void) {
			system::guard g_;
			return performers.count() == 0;
		}

		bool queue::execute_(signal::performer* _performer, time_ms_t _timeout) {
			post_(_performer, signal::performer::priority::normal);
			if (!wait_(_timeout)) {
				system::guard g_;
				if (_performer->cancel()) {
					return false;
				}
			}
			return true;
		}

		task::task() : ref_(*this, 0), state_(state::disable) {
			ref_.attach_to(machine::instance().disabled_);
		}
		task::~task() {
		}

		task::machine& task::machine::instance(void) {
			static machine machine__;
			return machine__;
		}
		cstr g_state_names[ task::state_count] = { RT("state::disable"), RT("state::destroy"), RT("state::sleep"), RT("state::active") };

		bool task::wakeup() {
			if (state_ >= state::sleep) {
				ROBO_LBREAKN(ref_.attach_to(machine::instance().getup_));
				state_ = state::active;
				return true;
			}
			else {
#if ROBO_APP_DEBUG_LOG_ENABLED == 1
				cstr _state_name = g_state_names[(int)state_];
#endif
				state_ = state::destroy;
				ref_.attach_to(machine::instance().trash_);
				ROBO_LBREAK_F("task %s wakeup fault (state is incorrect: %s) ", name.c_str(), _state_name);
			}
		}
		
		bool task::destroy() {
			if (state_ == state::disable) {
				ROBO_LBREAKN( ref_.attach_to(machine::instance().trash_) );
				state_ = state::destroy;
				return true;
			}
			else {
#if ROBO_APP_DEBUG_LOG_ENABLED == 1
				cstr _state_name = g_state_names[(int)state_];
#endif
				state_ = state::destroy;
				ref_.attach_to(machine::instance().trash_);
				ROBO_LBREAK_F("task %s state::destroy fault (state is incorrect: %s) ", name.c_str(), _state_name);
			}
		}

		bool task::sleep(time_us_t _timeout) {
			switch (state_) {
			case state::active:
				//if (_timeout != IMMEDIATELY){
				ref_.dettach();
				ref_.set_key(_timeout);
				//}
				return true;
			case state::sleep:
				if (_timeout != timeout::immediately) {
					ref_.set_key(_timeout);
				}
				return true;
			default:
#if ROBO_APP_DEBUG_LOG_ENABLED == 1
				cstr _state_name = g_state_names[(int)state_];
#endif
				state_ = state::destroy;
				ref_.attach_to(machine::instance().trash_);
				ROBO_LBREAK_F("task %s state::sleep fault (state is incorrect: %s) ", name.c_str(), _state_name);
			}
		}

		bool task::continue_sleep() {
			if (state_ == state::active) {
				time_us_t _timeout = sleep_timeout();
				return (_timeout > timeout::immediately) && _timeout != timeout::infinite; //to do иначе спящие никогда не проснуться на weakeup
			}
			else {
#if ROBO_APP_DEBUG_LOG_ENABLED == 1
				cstr _state_name = g_state_names[(int)state_];
#endif
				state_ = state::destroy;
				ref_.attach_to(machine::instance().trash_);
				ROBO_LBREAK_F("task %s continue_state::sleep fault (state is incorrect: %s) ", name.c_str(), _state_name);				
			}
		}

		bool task::start(bool _suspended) {
			if (state_ == state::disable) {
				ref_.dettach();
				if (_suspended) {
					ref_.set_key(timeout::infinite);
					ROBO_LBREAKN(ref_.attach_to(machine::instance().suspended_));
					state_ = state::sleep;
				}
				else {
					ref_.set_key(timeout::immediately);
					ROBO_LBREAKN(ref_.attach_to(machine::instance().getup_));
					state_ = state::active;
				}
				return true;
			}
			else {
#if ROBO_APP_DEBUG_LOG_ENABLED == 1
				cstr _state_name = g_state_names[(int)state_];
#endif
				ROBO_LBREAK_F("task %s start fault (state is incorrect: %s) ", name.c_str(), _state_name);
			}
		}

		bool task::stop() {
			if (state_ >= state::sleep) {
				ROBO_LBREAKN(ref_.attach_to(machine::instance().disabled_));
				state_ = state::disable;
				return true;
			}
			else {
#if ROBO_APP_DEBUG_LOG_ENABLED == 1
				cstr _state_name = g_state_names[(int)state_];
#endif
				state_ = state::destroy;
				ref_.attach_to(machine::instance().trash_);
				ROBO_LBREAK_F("task %s stop fault (state is incorrect: %s) ", name.c_str(), _state_name);
			}
		}
		
		void task::machine::execute_() {
			time = system::env::time_us();
			int _period = time - time_prev;
			time_prev = time;

			//таймауты в сортированном списке фигурируют как id
			//декрементируем таймауты ожидающих таймер  task
			timer_.inc_key(-_period);
			//таймауты активныех  task тоже декрементируем - малоли они вернуться ко сну
			active_.inc_key(-_period);

			//будим спящих
			task::ref* ref = timer_.first();
			while (ref && ((int)ref->key()) <= 0) {
				task* t = &(ref->owner());
				ref = ref->next();
				t->ref_.dettach();
				t->ref_.set_key(task::timeout::immediately);
				t->wakeup();
			}

			//по очереди работают сначала все активные task, потом только что  проснувшиеся
			//также task может разбудить другие таски напрямую wakeup или посредством signal, mutex и так далее
			ref = active_.first();
			if (ref == 0) {
				ref = getup_.first();
			}
			while (ref) {
				task* t = &(ref->owner());
				ref = ref->next();
				ROBO_ALARMN( t->execute()== result::panic );
				if (t->state_ == state::active) {
					time_us_t _timeout = t->sleep_timeout();
					if (_timeout != timeout::immediately) {
						if (_timeout == timeout::infinite) {
							ROBO_ALARMN(t->ref_.attach_to(suspended_));
						}
						else {
							ROBO_ALARMN(t->ref_.attach_to(timer_));
							t->state_ = state::sleep;
						}
					}
					else {
						ROBO_ALARMN(t->ref_.attach_to(active_));
					}
				}
				if (ref == 0) {
					//продолжаем работать с только что разбуженными 
					//to do выстрел себе в ногу? если таски начнут будить друг друга, то это плохо закончиться
					//поставить счетчик?
					ref = getup_.first();
				}
			}
		}

		timer::core & timer::core::instance(void) {
			static core core_;
			return core_;
		}

		void timer::restart_() {
			started_ = false;
			if (on_tick.used()) {
				wakeup();
			}
		}

		result timer::execute() {
			if (on_tick.used()) {
				if (!started_) {
					sleep(period_);
					started_ = true;
				}
				else {
					if (!continue_sleep()) {
						on_tick.raise();
						sleep(period_);
					}
				}
			}
			else {
				sleep( timeout::infinite);
			}
			return result::resume;
		}

		void  timer::core::start_(signal::performer* _performer, time_us_t _period) {
			timer* _timer = timers_.find(_period);
			if (_timer == 0) {
				_timer = new timer(_period);
			}
			_performer->attach_to(&(_timer->on_tick), signal::performer::priority::normal);
			_timer->wakeup();
		}
		void timer::core::stop_( signal::performer* _performer, time_us_t _period ) {
			_performer->cancel();
			timer* _timer = timers_.find(_period);
			if (_timer != nullptr) {
				if (!_timer->on_tick.used()) {
					delete _timer;
				}
			}
		}
		void  timer::core::restart_() {
			for ( ref* _ref = timers_.first(); _ref; _ref = _ref->next()) {
				_ref->owner().restart_();
			}
		}

		timer::timer(time_us_t _period) :task(), ref_(*this, _period), period_(_period), started_(false) {
			name.format(RT("timer-%d"), _period);
			ref_.attach_to( core::instance().timers_ );
			start(true);
		}
		timer::~timer() {
		}

		idevagent::idevagent(cstr _name, boardagent& _boardagent) :app::node(_name, _boardagent), boardagent_(_boardagent) {}

	}
}
