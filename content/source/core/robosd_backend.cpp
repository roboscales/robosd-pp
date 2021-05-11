#include "core/robosd_backend.hpp"
#include "core/robosd_system.hpp"
#include "core/robosd_ini.hpp"
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
		cstr g_state_names[task::state_count] = { RT("state::disable"), RT("state::destroy"), RT("state::sleep"), RT("state::active") };

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
				ROBO_LBREAKN(ref_.attach_to(machine::instance().trash_));
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
				ROBO_ALARMN(t->execute() == result::panic);
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

		timer::core& timer::core::instance(void) {
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
				sleep(timeout::infinite);
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
		void timer::core::stop_(signal::performer* _performer, time_us_t _period) {
			_performer->cancel();
			timer* _timer = timers_.find(_period);
			if (_timer != nullptr) {
				if (!_timer->on_tick.used()) {
					delete _timer;
				}
			}
		}
		void  timer::core::restart_() {
			for (ref* _ref = timers_.first(); _ref; _ref = _ref->next()) {
				_ref->owner().restart_();
			}
		}

		timer::timer(time_us_t _period) :task(), ref_(*this, _period), period_(_period), started_(false) {
			name.format(RT("timer-%d"), _period);
			ref_.attach_to(core::instance().timers_);
			start(true);
		}
		timer::~timer() {
		}

		idevagent::idevagent(cstr _name, boardagent& _boardagent) 
			: app::node(_name, _boardagent)
			, boardagent_(_boardagent)
			, bus_ref_(*this, 0){
		}

		router::record* idevagent::resolve(int _bus_id, robo_tran_header_p  _tran_header) {
			router::record* rec = router_->resolve(_bus_id, _tran_header);
			if (rec == 0) {
				ROBO_ALARM_F("tran header is't resolved: agent: agent '%s' 0x%x, bus 0x%x, dev 0x%x, command 0x%x",alias(), dev_id().value, _bus_id, _tran_header->dev_id, _tran_header->command);
			}
			return rec;
		}

		idevagent::stream::query_result idevagent::query(idevagent::stream::msg* _msg) {
			if (exchabge_enabled()) {
				time_us_t tm = system::env::time_us();
				if (boardagent_.request_pause_us_ < tm - boardagent_.last_request_us_) {
					stream::query_result ret;
					for (stream::ref* _ref = streams_.first(); _ref; _ref = _ref->next()) {
						ret = _ref->owner().query(_msg);
						if (ret == stream::query_result::none) {
							continue;
						}
						else {
							_msg->tran_->header.dev_id = dev_id_.dev;
							if (!_msg->prepare()) {
								ROBO_ALARM();
								return stream::query_result::none;
							}
							boardagent_.last_request_us_ = tm;
							return ret;
						}
					}
				}
			}
			return stream::query_result::none;
		}

		bool bus::request_(msg* _msg) {
			system::guard g__;
			if (current_msg_ == 0) {
				bool proto_res;
				switch (_msg->tran.request) {
				case ROBO_TRAN_REQUEST_GET:
					request_begin_us_ = system::env::time_us();
					timeout_us_ = default_timeout_us_;
					proto_res = post(_msg);
					break;
				case ROBO_TRAN_REQUEST_PUT:
					request_begin_us_ = system::env::time_us();
					timeout_us_ = default_timeout_us_;
					proto_res = post(_msg);
					break;
				}
				if (proto_res) {
					current_msg_ = _msg;
					current_msg_->tran.status = ROBO_TRAN_EXECUTE_START;
					return true;
				}
			}
			_msg->tran.status = ROBO_TRAN_REFUSE;
			_msg->confirm();
			return false;
		}

		bool bus::node_load(void) {
			int bus_id;
			ROBO_LBREAKN(ini::load(name(), RT("BUS_ID"), bus_id));
			ROBO_LBREAKN(setup_(bus_id));
			ROBO_LBREAKN( ini::load( name(), RT("DEFAULT_TIMEOUT_US"), default_timeout_us_) )
			return true;
		}
		
		void   bus::node_clean(void) {
			ref_.dettach();
		}

		void bus::confirm(robo_tran_status_t _result) {
			robo::system::guard __g;
			if (current_msg_ != 0) {
				current_msg_->tran.status = _result;
				robo_detaillog(7, 0, "tran result - %d", (int)current_msg_->tran.status);
				system::env::wakeup();
			}
			else {
				robo_errlog("confirm without msg!");
			}
		}

		static bus::index & bus_indext(void) {
			static bus::index bus_indext_;
			return bus_indext_;
		};

		bool bus::setup_(int _id) {
			ref_.set_key(_id);
			ROBO_LRET( ref_.attach_to(bus_indext()));
		}

		bus::bus(cstr _name, app::module* _owner)
			: app::node(_name, _owner), ref_(*this,0)
		{
		}
		bus::~bus() {
			while (agents_.count()) agents_.pop();
			ref_.dettach();
		}
		bus* _get_bus(int _id) {
			bus* _bus = bus_indext().find(_id);
			if (_bus == 0) {
				ROBO_ALARM_F("bus with id %d is't found", _id);
				return 0;
			}
			else {
				return _bus;
			}
		}
		bool bus::msg::prepare() {
			idevagent::stream::msg::prepare();
			idevagent& owner = stream_->own_agent();
			const dev_id_t & id_ = owner.dev_id();
			address = id_.address;
			//на будущее- агент может работать сразу на нескольких шинах
			//			router::record * rec = owner->resolve(, );
			router::record* rec = owner.resolve(ownbus->id(), &(tran_->header));
			ROBO_LBREAKN(rec!=nullptr);
			if (tran.request == ROBO_TRAN_REQUEST_GET) {
				suba = rec->answer_suba;
			}
			else {
				suba = rec->request_suba;
			}
			return true;
		}

		void bus::perform(void) {
#if APP_BUSMARSHAL_ENABLED == 1
			::busmarshal::begin_tran();
#endif
			for (ref* p = bus_indext().first(); p; p = p->next()) {
				p->owner().perform_();
			}
#if ROBO_APP_BUSMARSHAL_ENABLED == 1
			::busmarshal::commit_tran();
			//todo 
			::busmarshal::private_loop();
#endif
		}

		void bus::tick1sec_(void) {
			trafic.tick1sec();
			for ( idevagent::bus_ref * _ref = agents_.first(); _ref; _ref = _ref->next()) {
				_ref->owner().trafic.tick1sec();
			}
		}

		void bus::tick1sec(void) {
			for (ref* p = bus_indext().first(); p; p = p->next()) {
				p->owner().tick1sec_();
			}
		}

		bool bus::ready_() {
			system::guard g__;
			return current_msg_ == 0 && ready();
		}
		bus::msg* bus::pop_incom_msg_() {
			msg* ret = 0;
			idevagent* broken_obj = nullptr;
			time_us_t now = system::env::time_us();
			{
				system::guard g__;
				if (current_msg_) {
					if (now - request_begin_us_ >= timeout_us_) {
						ret = current_msg_;
						ret->tran.status = ROBO_TRAN_REFUSE;
						broken_obj = &current_msg_->own_agent();
						current_msg_ = 0;
					}
					else {
						if ((current_msg_->tran.status & ROBO_TRAN_EXECUTE) != ROBO_TRAN_EXECUTE) {
							ret = current_msg_;
							current_msg_ = 0;
						}
					}
				}
			}
			if (broken_obj) {
				cancel();
				ROBO_ALARM_F("bus %s refuse current message by timeout %u %u %u by object %s 0x%x", alias(), now - request_begin_us_, now, request_begin_us_, broken_obj->alias(), broken_obj->dev_id().value);
			}
			return ret;
		}


		void bus::perform_() {
			msg* m = pop_incom_msg_();
			if (m) {
				if (m->tran.status == ROBO_TRAN_COMPLETE) {
					if (m->tran.request == ROBO_TRAN_REQUEST_GET) {
						trafic.incom.success.inc(m->tran.size_actual);
						m->own_agent().trafic.incom.success.inc(m->tran.size_actual);
					}
					else {
						trafic.outcom.success.inc(m->tran.size_actual);
						m->own_agent().trafic.outcom.success.inc(m->tran.size_actual);
					}
				}
				else {
					if (m->tran.request == ROBO_TRAN_REQUEST_GET) {
						trafic.incom.refuse.inc(m->tran.size_actual);
						m->own_agent().trafic.incom.refuse.inc(m->tran.size_actual);
					}
					else {
						trafic.outcom.refuse.inc(m->tran.size_actual);
						m->own_agent().trafic.outcom.refuse.inc(m->tran.size_actual);
					}
				}

				m->confirm();

				release_msg(m);
			}
			bool rd = ready_();
			if (rd && agents_.count()) {
				if (current_agent_ref_ == 0) {
					current_agent_ref_ = agents_.first();
				}
				idevagent::bus_ref * first_agent_ref_ = current_agent_ref_;

				if (current_agent_ref_) {
					msg* _msg = get_msg();
					if (_msg) {
						_msg->ownbus = this;
						do {
							idevagent::stream::query_result res = current_agent_ref_->owner().query(_msg);
							switch (res) {
							case idevagent::stream::query_result::success:
								if (request_(_msg)) {
									current_agent_ref_ = current_agent_ref_->next();
									if (current_agent_ref_ == 0) {
										current_agent_ref_ = agents_.first();
									}
									return;
								}
								break;
							case idevagent::stream::query_result::repeat:
								if ( request_(_msg) ) {
									return;
								}
								break;
							case idevagent::stream::query_result::none:
								break;
							}
							current_agent_ref_ = current_agent_ref_->next();
							if (current_agent_ref_ == 0) {
								current_agent_ref_ = agents_.first();
							}
						} while (current_agent_ref_ != first_agent_ref_);
						release_msg(_msg);
					}
				}
			}
		}


		bool router::node_load(void) {
			ROBO_LBREAKN( ini::load(name(), RT("ROUT_TABLE_SIZE"), table_size_) )
			if (table_size_ > 0) {
				table_ = new record[table_size_];
				ROBO_LBREAKN(table_!=nullptr);
				if (table_) {
					string key;
					record* rec = table_;
					for (size_t i = 0; i < table_size_; i++, rec++) {
						key.format(RT("RT_%d"), i + 1);
						int tmp[5];
						ROBO_LBREAKN(ini::load_arr(name(), key, tmp, 5));
						
						rec->bus_id = (int)tmp[0];
						rec->tran_header.dev_id = (robo_tran_dev_id_t)tmp[1];
						rec->tran_header.command = (robo_tran_command_id_t)tmp[2];
						rec->request_suba = (record::suba_t)tmp[3];
						rec->answer_suba = (record::suba_t)tmp[4];
					}

				}
			}
			return true;
		}

		void router::node_clean(void) {
			if (table_) {
				delete[] table_;
				table_ = 0;
			}
		}
		router::record* router::resolve(int _bus_id, robo_tran_header_p  _tran_header) {
			record* rec = table_;
			for (size_t i = 0; i < table_size_; ++i, ++rec) {
				if ((rec->tran_header.hash == _tran_header->hash) && (rec->bus_id == _bus_id)) {
					return rec;
				}
			}
			return 0;
		}
		router::router(cstr _name, app::module* _owner): app::node(_name,_owner){
		}

		idevagent::stream::msg::msg(robo_tran_p _tran)
			: stream_(0)
			, ref_(*this)
			, tran_(_tran)
		{
		}

		void idevagent::stream::msg::confirm() {
			if (stream_) {
				stream_->confirm(tran_);
			}
		}

		bool idevagent::stream::msg::prepare() {
			tran_->status = ROBO_TRAN_NONE;
			return true;
		}

		idevagent::stream::stream(idevagent & _agent, idevagent::stream::priority _priority) :
			ref_(*this, _priority)
			, agent_(_agent)
		{
			ref_.attach_to(agent_.streams_);
		}
		idevagent::stream::~stream() {
		}

		idevagent::stream::query_result idevagent::stream::query(idevagent::stream::msg* _msg) {
			stream::query_result ret = query(_msg->tran_);
			if (ret != query_result::none ) {
				_msg->stream_ = this;
			}
			return ret;
		};

		bool boardagent::node_load(void) {
			ROBO_LBREAKN(ini::load(name(), RT("REQUEST_PAUSE_US"), request_pause_us_));
			return true;
		}
		void boardagent::node_clean(void) {
		}

		bool idevagent::node_load(void) {
			uint8_t tmp;
//			ROBO_LBREAKN(ini::load(name(), RT("BUS_ID"), tmp));
//			dev_id_.bus = tmp;
			ROBO_LBREAKN(ini::load(name(), RT("BOARD_DEV_ID"), tmp));
			dev_id_.dev = tmp;
			ROBO_LBREAKN(ini::load(name(), RT("BOARD_ADDRESS"), tmp));
			dev_id_.address = tmp;
			return true;
		}
		
		bool idevagent::node_start(void) {
			string bus_name;
			bus_name.load(name(), RT("BUS_NAME"));
			bus* b = dynamic_cast<bus*>(find(bus_name));
			bus_ref_.set_key(dev_id_.value);
			if (b) {
				ROBO_LBREAKN(bus_ref_.attach_to(b->agents_));
				dev_id_.bus = b->id();
				robo_infolog("agent '%s' sucsess loaded with id (0x%x)", alias(), dev_id_.value);
				return true;
			}
			else {
				ROBO_LBREAK_F("bus is't found by name '%s' for  object '%s' (0x%x)", bus_name.c_str(), alias(), dev_id_.value)
			}
		}

		void idevagent::node_clean(void) {
		}
	}
}
