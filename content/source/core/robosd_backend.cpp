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
				system::env::sleep();
			} while (system::env::time_ms() - ms < _timeout);
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
		task::~task() {}

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
				return (_timeout > timeout::immediately) && _timeout != timeout::infinite; //to do
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

			timer_.inc_key(-_period);
			active_.inc_key(-_period);

			task::ref* ref = timer_.first();
			while (ref && ((int)ref->key()) <= 0) {
				task* t = &(ref->owner());
				ref = ref->next();
				t->ref_.dettach();
				t->ref_.set_key(task::timeout::immediately);
				t->wakeup();
			}

			ref = active_.first();
			if (ref == 0) {
				ref = getup_.first();
			}
			while (ref) {
				task* t = &(ref->owner());
				ref = ref->next();
				ROBO_ALARMN(t->execute() != result::panic);
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
		timer::~timer() {}
		#if ROBO_APP_MODULE_ENABLED  == 1

		devagent::devagent(cstr _name, boardagent& _boardagent, action_s& _goal, feedback_s& _feedback)
			: app::node(_name, &_boardagent)
			, boardagent_(_boardagent)
			, bus_ref_(*this, 0)
			, goal(_goal)
			, feedback(_feedback)	{
		}

		router::record* devagent::resolve(int _bus_id, robo_tran_header_p  _tran_header) {
			if (router_->actual_mode() == router::mode::dummy) {
				return nullptr;
			}
			else {
				router::record* rec = router_->resolve(_bus_id, _tran_header);
				if (rec == 0) {
					ROBO_ALARM_F("tran header is't resolved: agent: agent '%s' 0x%x, bus 0x%x, dev 0x%x, command 0x%x", alias(), dev_id().value, _bus_id, _tran_header->dev_id, _tran_header->command);
				}
				return rec;
			}
		}

		devagent::statuses devagent::actual_status(commands _command) {
			static const statuses tb[] =
			{
				//icommand
				//stop = 0,			sw2service = 1,		raise_fault = 2,	sw2independed = 3,	sw2dirrect = 4,		reset_fault = 5
				//unknown
				statuses::unknown,	statuses::unknown,	statuses::unknown,	statuses::unknown,	statuses::unknown,	statuses::unknown,
				//fault
				statuses::fault,	statuses::fault,	statuses::fault,	statuses::fault,	statuses::fault,	statuses::busy,
				//ready
				statuses::idle,		statuses::service,	statuses::busy,		statuses::independed,statuses::dirrect,	statuses::busy
			};
			switch (feedback.state.local) {
			case state_s::locals::unknown:
			return statuses::unknown;
			case state_s::locals::disabled:
			return statuses::disabled;
			case state_s::locals::configure:
			return statuses::busy;
			case state_s::locals::ready:
			return tb[((int)feedback.state.remote.status * 5) + (int)_command];
			default:
			return statuses::unknown;
			}
		}


		devagent::stream::query_result devagent::query(devagent::stream::msg* _msg) {
			if (exchabge_enabled()) {
				time_us_t tm = system::env::time_us();
				if (boardagent_.request_pause_us_ < tm - boardagent_.last_request_us_) {
					stream::query_result ret;
					for (stream::ref* _ref = streams_.first(); _ref; _ref = _ref->next()) {
						if (_ref->owner().exchange_need()) {
							ret = _ref->owner().query(_msg);
							if (ret == stream::query_result::none) {
								continue;
							}
							else {
								_msg->tran.header.dev_id = dev_id_.dev;
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
			}
			return stream::query_result::none;
		}

		bool bus::request_(void) {
			{
				system::guard g__;
				bool proto_res = false;
				switch (message_.tran.request) {
				case ROBO_TRAN_REQUEST_GET:
				case ROBO_TRAN_EXCANGE:
				case ROBO_TRAN_REBOOT_ME:
				case ROBO_TRAN_REQUEST_PUT:
					request_begin_us_ = system::env::time_us();
					timeout_us_ = default_timeout_us_;
					proto_res = post();
					break;
				}
				if (proto_res) {
					message_.tran.status = ROBO_TRAN_EXECUTE_START;
					return true;
				}
			}
			message_.tran.status = ROBO_TRAN_REFUSE;
			message_.confirm();
			return false;
		}

		static bus::index& bus_index(void) {
			static bus::index bus_indext_;
			return bus_indext_;
		};

		bool bus::do_load(void) {
			ROBO_LBREAKN(app::node::do_load());
			index_ref_.set_key(hash(alias()));
			ROBO_LRET(index_ref_.attach_to(bus_index()));
			ROBO_LBREAKN(ini::load(current_path(), RT("DEFAULT_TIMEOUT_US"), default_timeout_us_));
			return true;
		}

		void   bus::do_clean(void) {
			index_ref_.dettach();
			app::node::do_clean();
		}

		void bus::confirm(robo_tran_status_t _result) {
			robo::system::guard __g;
			if (message_.tran.status != ROBO_TRAN_NONE) {
				message_.tran.status = _result;
				robo_detaillog(7,0,"tran result - %d", ((int)message_.tran.status) );
				system::env::wakeup();
			}
			else {
				robo_errlog("confirm without msg!");
			}
		}



		bus::bus(cstr _name, app::module* _owner)
			: app::node(_name, _owner), index_ref_(*this, 0) {}
		bus::~bus() {
			while (agents_.count()) agents_.pop();
			index_ref_.dettach();
		}
		bus* _get_bus(int _id) {
			bus* _bus = bus_index().find(_id);
			if (_bus == 0) {
				ROBO_ALARM_F("bus with id %d is't found", _id);
				return 0;
			}
			else {
				return _bus;
			}
		}
		bool bus::msg::prepare() {
			devagent::stream::msg::prepare();
			devagent& owner = stream_->own_agent();
			const dev_id_t& id_ = owner.dev_id();
			address = id_.address;
			router::record* rec = owner.resolve(ownbus->id(), &(tran.header));
			if (rec != nullptr) {
				if (tran.request == ROBO_TRAN_REQUEST_GET) {
					suba = rec->answer_suba;
				}
				else {
					suba = rec->request_suba;
				}
			}
			else {
				suba = tran.header.command;
			}

			return true;
		}

		void bus::perform(void) {
			#if APP_BUSMARSHAL_ENABLED == 1
			::busmarshal::begin_tran();
			#endif
			for (index_ref* p = bus_index().first(); p; p = p->next()) {
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
			for (devagent::bus_ref* _ref = agents_.first(); _ref; _ref = _ref->next()) {
				_ref->owner().trafic.tick1sec();
			}
		}

		void bus::tick1sec(void) {
			for (index_ref* p = bus_index().first(); p; p = p->next()) {
				p->owner().tick1sec_();
			}
		}

		void bus::perform_(void) {
			time_us_t now = system::env::time_us();
			switch (message_.tran.status) {
			case	ROBO_TRAN_EXECUTE:
			case	ROBO_TRAN_EXECUTE_START:
			case	ROBO_TRAN_EXECUTE_PHY:
				if (now - request_begin_us_ >= timeout_us_) {
					devagent* broken_obj = message_.own_agent();
					if (broken_obj) {
						ROBO_ALARM_F("bus %s refuse current message by timeout %u %u %u by object %s 0x%x", alias(), now - request_begin_us_, now, request_begin_us_, broken_obj->alias(), broken_obj->dev_id().value);
					}
					else {
						ROBO_ALARM_F("bus %s refuse current message by timeout %u %u %u by object 'unknown'", alias(), now - request_begin_us_, now, request_begin_us_);
					}
					cancel();
				}
			break;
			case	ROBO_TRAN_REFUSE:
				if (message_.tran.request == ROBO_TRAN_REQUEST_GET) {
					trafic.incom.refuse.inc(message_.tran.size_actual);
				}
				else {
					trafic.outcom.refuse.inc(message_.tran.size_actual);
				}
				message_.confirm();
			break;
			case	ROBO_TRAN_COMPLETE:
				if (message_.tran.request == ROBO_TRAN_REQUEST_GET) {
					trafic.incom.success.inc(message_.tran.size_actual);
				}
				else {
					trafic.outcom.success.inc(message_.tran.size_actual);
				}
				message_.confirm();
			break;
			}
			if ( message_.tran.status == ROBO_TRAN_NONE ) {
				if (ready() && agents_.count()) {
					if (current_agent_ref_ == nullptr) {
						current_agent_ref_ = agents_.first();
					}
					devagent::bus_ref* first_agent_ref_ = current_agent_ref_;

					if (current_agent_ref_) {
						message_.ownbus = this;
						reset();
						do {
							devagent::stream::query_result res = current_agent_ref_->owner().query(&message_);
							switch (res) {
							case devagent::stream::query_result::success:
							if (request_()) {
								current_agent_ref_ = current_agent_ref_->next();
								if (current_agent_ref_ == 0) {
									current_agent_ref_ = agents_.first();
								}
								return;
							}
							break;
							case devagent::stream::query_result::repeat:
							if (request_()) {
								return;
							}
							break;
							case devagent::stream::query_result::none:
							break;
							}
							current_agent_ref_ = current_agent_ref_->next();
							if (current_agent_ref_ == 0) {
								current_agent_ref_ = agents_.first();
							}
						} while (current_agent_ref_ != first_agent_ref_);
					}
				}
			}
		}


		bool router::do_load(void) {
			ROBO_LBREAKN(app::node::do_load());
			ROBO_LBREAKN(ini::load(current_path(), RT("ROUT_TABLE_SIZE"), table_size_))
			if (table_size_ > 0) {
				table_ = new record[table_size_];
				ROBO_LBREAKN(table_ != nullptr);
				if (table_) {
					string key;
					string recs;
					record* rec = table_;
					for (size_t i = 0; i < table_size_; i++, rec++) {
						key.format(RT("RT_%d"), i + 1);

						ROBO_LBREAKN(recs.load(current_path(), key));
						cstr vs = recs.c_str();
						enum class wait { skip, begin, end} wt = wait::skip;
						if (vs) {
							const char_t * c = vs;
							while (*c) {
								switch (wt) {
								case wait::skip:
								if (*c == ' ') {
									vs++;
									wt = wait::begin;
									break;
								}
								case wait::begin:
								if (*c == '"') {
									wt = wait::end;
									break;
								}
								else {
									ROBO_LBREAK_F("error load key from array  for  router '%s'", alias())
								}
								case wait::end:
								if (*c == '"') {
									int bus_id = robo::hash(vs+1, c - 1, 0);
									ROBO_LBREAKN_F(find<bus>(bus_id) != nullptr, "bus is't found by string '%s' for  router '%s'", vs, alias());
									int tmp[4];
									ROBO_LBREAKN(string::to_number_array(c+1, tmp, 4));
									rec->bus_id = bus_id;
									rec->tran_header.dev_id = (robo_tran_dev_id_t)tmp[0];
									rec->tran_header.command = (robo_tran_command_id_t)tmp[1];
									rec->request_suba = (record::suba_t)tmp[2];
									rec->answer_suba = (record::suba_t)tmp[3];
									mode_ = mode::table;
									return true;
								}
								}
								c++;
							}
						}
					}
				}
			}
			else {
				mode_ = mode::dummy;
			}
			return true;
		}

		void router::do_clean(void) {
			if (table_) {
				delete[] table_;
				table_ = 0;
			}
			app::node::do_clean();
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
		router::router(cstr _name, app::module& _owner) : app::node(_name, &_owner) {}

		devagent::stream::msg::msg(void)
			: stream_(0)
			, ref(*this)
			{}

		void devagent::stream::msg::confirm() {
			if (stream_) {
				stream_->confirm(tran);
				if (tran.status == ROBO_TRAN_REFUSE) {
					if (tran.request == ROBO_TRAN_REQUEST_GET) {
						stream_->own_agent().trafic.incom.refuse.inc(tran.size_actual);
					}
					else {
						stream_->own_agent().trafic.outcom.refuse.inc(tran.size_actual);
					}
				}
				else {
					if (tran.request == ROBO_TRAN_REQUEST_GET) {
						stream_->own_agent().trafic.incom.success.inc(tran.size_actual);
					}
					else {
						stream_->own_agent().trafic.outcom.success.inc(tran.size_actual);
					}
				}
			}
			tran.status = ROBO_TRAN_NONE;
		}

		bool devagent::stream::msg::prepare() {
			tran.status = ROBO_TRAN_NONE;
			return true;
		}

		devagent::stream::stream(devagent& _agent, devagent::stream::priority _priority) :
			ref_(*this, _priority)
			, agent_(_agent) {
			ref_.attach_to(agent_.streams_);
		}
		devagent::stream::~stream() {}

		devagent::stream::query_result devagent::stream::query(devagent::stream::msg* _msg) {
			stream::query_result ret = query(_msg->tran);
			if (ret != query_result::none) {
				_msg->stream_ = this;
			}
			return ret;
		};

		bool boardagent::do_load(void) {
			ROBO_LBREAKN(app::node::do_load());
			ROBO_LBREAKN(ini::load( current_path(), RT("REQUEST_PAUSE_US"), request_pause_us_));
			return true;
		}
		void boardagent::do_clean(void) {
			app::node::do_clean();
		}

		bool devagent::exchabge_enabled(void) {
			return feedback.state.local > state_s::locals::disabled; 
		}
		bool devagent::configure_complete(void) {
			ROBO_LBREAKN(feedback.state.local == state_s::locals::configure);
			feedback.state.local = state_s::locals::ready;
			return true;
		}


		bool devagent::do_load(void) {
			ROBO_LBREAKN(app::node::do_load());
			uint8_t tmp;
			//			ROBO_LBREAKN(ini::load(name(), RT("BUS_ID"), tmp));
			//			dev_id_.bus = tmp;
			ROBO_LBREAKN(ini::load( current_path(), RT("BOARD_DEV_ID"), tmp));
			dev_id_.dev = tmp;
			ROBO_LBREAKN(ini::load(current_path(), RT("BOARD_ADDRESS"), tmp));
			dev_id_.address = tmp;

			ROBO_LBREAKN(ini::load(current_path(), RT("ENABLED"), tmp));

			if (tmp) {
				feedback.state.local = state_s::locals::configure;
				ROBO_LBREAKN(bus_alias_.load(current_path(), RT("BUS_ALIAS")));
				ROBO_LBREAKN(router_alias_.load(current_path(), RT("ROUTER_ALIAS")));
			}
			else {
				feedback.state.local = state_s::locals::disabled;
			}
			return true;
		}

		bool devagent::do_start(void) {
			ROBO_LBREAKN(app::node::do_start());
			bus* b = find<bus>(bus_alias_);
			bus_ref_.set_key(dev_id_.value);
//			robo::system::printf(RT("%s - bus: %s - %p "), alias(), bus_alias_.c_str(), (void*)b);
			if (b) {
				ROBO_LBREAKN(bus_ref_.attach_to(b->agents_));
				dev_id_.bus = b->id();
			}
			else {
				ROBO_LBREAK_F("bus is't found by name '%s' for  object '%s' (0x%x)", bus_alias_.c_str(), alias(), dev_id_.value)
			}

			router_ = find<router>(router_alias_);
			ROBO_LBREAKN_F(router_ != nullptr, "router is't found by name '%s' for  object '%s' (0x%x)", router_alias_.c_str(), alias(), dev_id_.value);
			robo_infolog("agent '%s' sucsess loaded with id (0x%x)", alias(), dev_id_.value);
			return true;
		}

		void devagent::do_clean(void) {
			dev_id_.value = (uint32_t)-1;
			bus_ref_.dettach();
			app::node::do_clean();
		}

		contrltable::contrltable(devagent& _agent, priority _priority, int command_, const record* const _records, size_t _count) :
			frontend::contrltable(_agent, _records, _count), stream(_agent, _priority) {

		}

		contrltable::ivar::ivar(frontend::contrltable& _contrltable, const record& _instance)
			: frontend::contrltable::ivar(_contrltable, _instance), ref_(*this) {};

		bool contrltable::ivar::rerquest(void) {
			system::guard g__;
			ROBO_LBREAKN_F(!ref_.attached(), "var '%s' is busy and rwef is used ", name());
			ref_.attach_to(owner().queue_);
			return true;
		}


		contrltable::query_result contrltable::query(robo_tran_t & _tran) {
			if (current_ != nullptr) {
				ROBO_ALARM_F("invalid proto for %s/%S", own_agent().alias(), current_->name());
				current_->refuse();
				current_ = nullptr;
				return query_result::none;
			}
			{
				system::guard g__;
				current_ = queue_.pop();
			}

			if (current_ == nullptr) {
				return query_result::none;
			}

			if (current_->actual_status() == ivar::status::put) {
				if (_tran.size_max > current_->length()) {
					_tran.header.command = (robo_tran_command_id_t)current_->addr();
					_tran.size_actual = current_->length();
					if (current_->encode(_tran.data)) {
						_tran.request = ROBO_TRAN_REQUEST_PUT;
						return query_result::success;
					}
				}
				ROBO_ALARM_F("var oversize or format error for %s/%s", own_agent().alias(), current_->name());
				current_->refuse();
				current_ = nullptr;
				return query_result::none;
			}
			else {
				if (current_->actual_status() == ivar::status::get) {
					_tran.header.command = (robo_tran_command_id_t)current_->addr();
					_tran.size_actual = current_->length();
					_tran.request = ROBO_TRAN_REQUEST_GET;
					return query_result::success;
				}
				else {
					ROBO_ALARM_F("var oversize for %s/%S", own_agent().alias(), current_->name());
					current_->refuse();
					current_ = nullptr;
					return query_result::none;
				}
			}
			ROBO_ALARM_F("invalid proto for %s/%S", own_agent().alias(), current_->name());
			current_->refuse();
			current_ = nullptr;
			return query_result::none;
		}

		void contrltable::confirm(const robo_tran_t & _tran) {
			if (current_ == nullptr) {
				ROBO_ALARM_F("invalid proto for %s/%S", own_agent().alias(), current_->name());
			}
			else {
				if (_tran.status == ROBO_TRAN_COMPLETE) {
					if (_tran.size_actual > current_->length()) {
						current_->refuse();
					}
					else {
						if (current_->decode(_tran.data)) {
							current_->confirm();
						}
						else {
							ROBO_ALARM_F("var oversize or format error for %s/%s", own_agent().alias(), current_->name());
							current_->refuse();
						}
					}
				}
				else {
					current_->refuse();
				}
			}
			current_ = nullptr;
		}

		bool contrltable::query(void) {
			for (frontend::contrltable::ivar::map_ref* r = vars.first(); r; r = r->next()) {
				ROBO_LBREAKN(r->owner().query());
			}
			return true;
		}

		bool contrltable::query(frontend::contrltable::ivar::delegat& _delegat) {
			for (frontend::contrltable::ivar::map_ref* r = vars.first(); r; r = r->next()) {
				ROBO_LBREAKN(r->owner().query(_delegat));
			}
			return true;
		}

		bool contrltable::ready(void) {
			for (frontend::contrltable::ivar::map_ref* r = vars.first(); r; r = r->next()) {
				if (!r->owner().is_ready()) {
					return false;
				}
			}
			return true;
		}
		#endif
	}
}

