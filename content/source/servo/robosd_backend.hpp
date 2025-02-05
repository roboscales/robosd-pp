#ifndef robosd_backend_hpp
#define robosd_backend_hpp
#include "servo/robosd_frontend.hpp"
#include "core/robosd_string.hpp"
#include "core/robosd_delegat.hpp"
#include "core/robosd_app.hpp"
#include "core/robosd_tran.h"
#include "core/robosd_tran.h"
#include "net/robosd_net_trafic.hpp"
#include "core/robosd_tran.h"
#include "core/robosd_ini.hpp"
#include "core/robosd_convert.hpp"
#include "core/robosd_system.hpp"
#include "net/robosd_serial.hpp"
namespace robo {
	namespace backend {

		class ROBO_EXPORT task {
		public:
			typedef list::pool < task, time_us_t > pool;
			typedef list::sorted< task, time_us_t > list;
			typedef list::ref ref;
		private:
			cstr name_;
			ref ref_;
		public:
			struct timeout {
				enum { infinite = -1, immediately = 0 };
			};
			enum class state { disable = 0, destroy = 1, sleep = 2, active = 3 };
			enum { state_count = 4 };
		private:
			state state_;
			void step_();
			inline void sleep_timeout_set_(time_us_t _timeout) { ref_.dettach(); ref_.set_key(_timeout); }
		protected:
			inline time_us_t sleep_timeout(void) { return ref_.key(); }
			virtual  result  execute(void) = 0;
		public:

			/** \brief	The name. */
			string name;

			class ROBO_EXPORT machine {
				friend class task;
				pool disabled_;
				pool suspended_;
				list active_;
				pool trash_;
				list timer_;
				pool getup_;
				machine(void) {};
				static machine& instance(void);
				time_us_t time;
				time_us_t time_prev;
				void execute_(void);
			public:
				static inline void execute(void) { instance().execute_(); };
			};
			bool start(bool _suspended);
			bool stop(void);
			task(void);
			virtual ~task(void);
			bool wakeup(void);
			bool destroy(void);
			bool active(void) { return  state_ >= state::sleep; }
		protected:
			bool sleep(time_us_t _timeout = timeout::infinite);
			bool continue_sleep(void);
		};

		class ROBO_EXPORT timer : public task {
		public:
			typedef ::robo::list::unique < timer, time_us_t > list;
			typedef list::ref ref;
		private:
			ref ref_;
			time_us_t  period_;
			timer(time_us_t  _period);
			virtual ~timer(void);
			event on_tick;
			bool started_;
			void restart_(void);
		protected:
			virtual result execute(void);
		public:
			class ROBO_EXPORT core {
				friend class timer;
				timer::list timers_;
				typedef ::robo::list::unique<timer, time_us_t>::ref ref;
				void start_(signal::performer* _performer, time_us_t _period);
				void stop_(signal::performer* _performer, time_us_t _period);
				void restart_(void);
				static core& instance(void);
			public:
				static void start(signal::performer* _performer, time_us_t _period) { instance().start_(_performer, _period); }
				static void stop(signal::performer* _performer, time_us_t _period) { instance().stop_(_performer, _period); }
				static void restart() { instance().restart_(); }
				//static void start(const  ::robo::lambda< void(void) >& _lambda, time_us_t _period) { instance().start_(new robo::signal::temporary::lambda(_lambda), _period); }
			};
		};

		class ROBO_EXPORT queue : public signal {
			void poll_(void);
			void post_(signal::performer* _performer, signal::performer::priority _priority);
			bool execute_(signal::performer* _performer, time_ms_t _timeout);
			bool wait_(time_ms_t _timeout);
			bool ready_(void);
			static queue& instance_(void);
			~queue(void);
		public:
			static void poll(void) { instance_().poll_(); }
			static void post(signal::performer* _performer, signal::performer::priority _priority) { instance_().post_(_performer, _priority); }
			static bool execute(signal::performer* _performer, time_ms_t _timeout) { return instance_().execute_(_performer, _timeout); }
			static bool wait(time_ms_t _timeout) { return instance_().wait_(_timeout); }
			static bool ready(void) { return instance_().ready_(); }
		};

		class ROBO_EXPORT repeater : public signal::performer {
			time_us_t period_;
		public:
			enum { default_period_us = 1000 };
			repeater(time_us_t _period = default_period_us)
				: performer(false)
				, period_(_period) {}
			void start(void) { timer::core::start(this, period_); }
			void start(time_us_t _period) { period_ = _period;  start(); }
			void stop(void) { timer::core::stop(this, period_); }
		};

		#if ROBO_APP_MODULE_ENABLED  == 1

		class boardagent;

		class ROBO_EXPORT router : public app::node {
		public:
			struct record {
				typedef uint8_t address_t;
				typedef uint8_t suba_t;
				robo_tran_header_t tran_header;
				int bus_id;
				suba_t request_suba;
				suba_t answer_suba;
			};
			enum class mode { table, dummy } mode_ = mode::table;
		private:
			size_t table_size_ = 0;
			record* table_ = nullptr;
		protected:
			virtual bool do_load(void);
			virtual void do_clean(void);
		public:
			mode actual_mode(void) { return mode_; }
			virtual record* resolve(int _bus_id, robo_tran_header_p  _tran_header);
			router(cstr _name, app::module& _owner);
		};

		/**/


		class devagent : public app::node {
		public:
			typedef  ::robo::list::sorted<devagent, int> bus_index;
			typedef  bus_index::ref bus_ref;
			
			class ROBO_EXPORT stream : public app::node {
			public:

				enum class query_result {
					none = 0
					, success
					, repeat
				};

				class ROBO_EXPORT msg {
				protected:
					stream* stream_;
					friend class stream;
				public:
					typedef  ::robo::list::unsorted<msg> list;
					list::ref ref;
					robo_tran_t tran = {};
					void confirm(void);
					msg(void);
					virtual bool prepare(void);
					inline devagent * own_agent(void) { return stream_ == nullptr ? nullptr : &stream_->own_agent(); }
				};

				typedef signal::performer::priority priority;
				typedef  ::robo::list::sorted<stream, priority> list;
				typedef  list::ref ref;
			private:
				ref ref_;
				devagent& agent_;
			protected:
				virtual bool do_load(void);
				virtual void do_clean(void);
			public:
				inline devagent& own_agent(void) { return agent_; }
				stream(cstr _name, devagent& _agent, priority _priority);
				virtual ~stream(void);
				virtual bool exchange_need(void) = 0;
				virtual query_result query(robo_tran_t& _tran) = 0;
				virtual void confirm(const robo_tran_t& _tran) = 0;
				query_result query(msg* _msg);
			};

			class tunnel : public stream {
			protected:
				robo::net::iserial* port_ = nullptr;
				robo::string port_name_;
			protected:
				virtual bool do_load(void);
				virtual bool do_start(void);
				virtual void do_stop(void);
			public:
				tunnel(cstr _name, devagent& _agent, priority _priority);
				virtual ~tunnel(void);
			};

			class ROBO_EXPORT datamap : public stream {
			protected:
				uint8_t * data_;
				uint8_t* buffer_;
				size_t size_;
				uint8_t page_;
				enum class state { stopped, startup, get, complete, panic } state_ = state::stopped;
				robo_tran_command_id_t dev_command_;
				friend class wakeup;
				class ROBO_EXPORT  wakeup : public signal::performer {
				public:
					virtual  void  operator ()() {
						if (datamap_->state_ == state::complete || datamap_->state_ == state::panic) {
							datamap_->state_ = state::startup;
						}
					};
					datamap* datamap_;
					wakeup(datamap* _datamap) : signal::performer(false), datamap_(_datamap) {}
				} wakeup_;				
			public:
				struct {
					event on_start;
					event on_complete;
					event on_panic;
				} events;
				const uint8_t * raw() { return data_; } //to do осмыслить
				size_t size() { return size_; }
				time_us_t period_us=0;
				datamap(cstr _name, devagent& _agent, priority _priority, uint8_t * _data, uint8_t* _buffer, size_t _size, robo_tran_command_id_t _dev_command);
				virtual ~datamap();
				virtual query_result query(robo_tran_t& _tran);
				virtual void confirm(const robo_tran_t& _tran);
				virtual query_result first_query(robo_tran_t& _tran);

				virtual bool exchange_need(void) { return !(state_ == state::stopped || state_ == state::complete); }
				virtual void on_complete(void) {}

				void start() {
					state_ = state::startup;
					timer::core::start(&wakeup_, period_us);
				};
				void stop() {
					state_ = state::stopped;
					wakeup_.dettach();
				}
				virtual bool do_load(void)
				{
					ROBO_LBREAKN(stream::do_load());
					ROBO_LBREAKN(ini::load(current_path(),defaults_path(),RT("datamap_period_us"),period_us))
					return true;
				}
			};
			class ROBO_EXPORT exchange : public datamap {
				robo_tran_command_id_t out_command_;
			protected:
				virtual size_t encode(uint8_t* _data, size_t _max_size) = 0;
			public:
				exchange(
					cstr _name
					, devagent& _agent
					, priority _priority
					, uint8_t* _out_data, uint8_t* _out_buffer, size_t _out_size
					, robo_tran_command_id_t _out_dev_command
					, robo_tran_command_id_t _in_dev_command
				);
				virtual ~exchange(void);
				virtual query_result first_query(robo_tran_t& _tran);
			};
			typedef common::devagent::state_s state_s;
			typedef common::devagent::commands commands;
			typedef common::devagent::statuses statuses;
			typedef common::devagent::action_s action_s;
			typedef common::devagent::feedback_s feedback_s;
			typedef common::devagent::config_s config_s;
			template <typename F> typename F::feedback_s & feedback(void) {
				return reinterpret_cast <typename F::feedback_s&>(feedback_);
			}
			template <typename A> typename A::action_s& goal(void) {
				return reinterpret_cast <typename A::action_s& > (goal_);
			}

		
		private:
			boardagent& boardagent_;
			dev_id_t dev_id_;
			bus_ref bus_ref_;
			int bus_order_ = 0;
			string bus_alias_;
			string router_alias_;
			router* router_ = nullptr;
			//state_s actual_state_;
			stream::list streams_;
			//commands actual_command_ = commands::stop;
			action_s& goal_;
			feedback_s& feedback_;
		protected:
			virtual  ::robo::quest* do_create_discovery_quest(void) { return nullptr; }
			virtual bool agent_apply_action(commands _command) {
				if (_command == commands::sw2dirrect) {
					return true;
				}
				else {
					goal_.command = _command;
					return false;
				}
			}

			virtual void agent_uppdate_feedback(void) {
				feedback_.status = actual_status(goal_.command);
			}

			bool exchabge_enabled(void);
			bool configure_complete(void);

			virtual bool do_load(void);
			virtual void do_clean(void);
			virtual bool do_start(void);
			/*
			#if ROBO_APP_MEXO_VAR_ENABLED == 1
			quest* var_query_quest(quest* _owner, ::robo::cstr _var) {

				::robo::string* sv = new ::robo::string(_var);

				return ::robo::quest::create(
					_owner,

					[this, sv](::robo::quest* _quest) {

						robo_detaillog(6, robo::log::mask::disabled, "\t\tquest: %s/%s - start query", this->alias(), sv->c_str());

						if (!vars.query(sv->c_str(), [this, _quest, sv](varindex::ivar* _var, bool _result) {
							if (_result) {
								_quest->confirm();
							}
							else {
								_quest->refuse();
							}
										})
							) {
							_quest->refuse();
						};
					}
					, [this, sv](::robo::quest::result r)->robo::quest::reaction {

						if (r == robo::quest::result::success) {
							robo_detaillog(6, robo::log::mask::disabled, "\t\tquest: %s/%s  - success", this->alias(), sv->c_str());
							delete sv;
							return robo::quest::reaction::normal;
						}
						else {
							robo_errlog("\t\tquest: %s/%s  - refused, canceled or termibated (%d) ", this->alias(), sv->c_str(), (int)r);
							delete sv;
							return robo::quest::reaction::terminate;
						}
					}
					);
			}
			#endif*/
		public:
			//статистика  трафика
			itrafic trafic;
			
			//идентификатор устройства
			const dev_id_t& dev_id(void) { return dev_id_; };

			//функция для диспетчера, который опрашивает агент что и когда посылать
			stream::query_result query(stream::msg* _msg);

			//функция для диспетчера, определяющая запись в таблице маршрутизации
			router::record* resolve(int _bus_id, robo_tran_header_p  _tran_header);

			devagent(cstr _name, boardagent& _boardagent, action_s& _goal, feedback_s& _feedback);

			//тенкущий (вычисляемый) статус
			statuses actual_status(commands _command);
			//тенкущая команда
			commands actual_command(void) { return goal_.command; };
			const state_s & actual_state(void) { return  feedback_.state; };
			//void dev_set_id(uint8_t _addr) { dev_id_.address = _addr; };
			virtual void on_configute_complete(void) {};
			virtual void on_configute_refuse(void) {};
			virtual void on_discovery_complete(void) {
			};
			virtual void on_discovery_refuse(void) {
			};

			::robo::quest* create_discovery_quest(::robo::quest* _owner) {
				return ::robo::quest::create( 
					_owner,
					do_create_discovery_quest(),
					::robo::quest::answer_fabric::create([this](robo::quest::result r)->robo::quest::reaction {
						if (r == robo::quest::result::success) {
							robo_infolog("====================================================================================\n\t\tquest: '%s discovery' success finished (A response was received) \n====================================================================================", this->display_alias());
							feedback_.state.local = state_s::locals::configure;
							on_discovery_complete();
							return robo::quest::reaction::normal;
						}
						else {
							robo_errlog("====================================================================================\n\t\tquest: '%s discovery' terminated (object isn't found) \n====================================================================================", this->display_alias());
							feedback_.state.local = state_s::locals::disabled;
							on_discovery_refuse();
							return robo::quest::reaction::terminate;
						}
					}														 
				));

			}

			::robo::quest* quest_configure(::robo::quest* _owner,::robo::quest* _sema=nullptr) {
				return ::robo::quest::create(
					_owner
					, _sema
					, ::robo::quest::answer_fabric::create( [this](robo::quest::result r)->robo::quest::reaction {
						if (r == robo::quest::result::success) {
							robo_infolog("====================================================================================\n\t\tquest: '%s configure' success finished\n====================================================================================", this->display_alias());
							feedback_.state.local = state_s::locals::ready;
							on_configute_complete();
							return robo::quest::reaction::normal;
						}
						else {
							robo_errlog("====================================================================================\n\t\tquest: '%s configure' terminated\n====================================================================================", this->display_alias());
							feedback_.state.local = state_s::locals::disabled;
							on_configute_refuse();
							return robo::quest::reaction::terminate;
						}
						}
						)					
					);
			}

		};

		class ROBO_EXPORT bus : public app::node {
		public:
			typedef ::robo::list::unique<bus, int> index;
			typedef index::ref index_ref;
		private:
			index_ref index_ref_;
		public:
			class ROBO_EXPORT msg : public devagent::stream::msg {
			public:
				router::record::address_t address;
				router::record::suba_t suba;
				bus* ownbus;
				msg(void) : devagent::stream::msg(), ownbus(0) {}
				virtual ~msg() {}
				bool  prepare(void);
			} message_;

		private:
			friend class devagent;
			devagent::bus_ref* current_agent_ref_ = nullptr;
			devagent::bus_index agents_;
			void refuse__(void);
			friend class api;
			bool request_(void);
			void perform_(void);
			time_us_t  request_begin_us_;
			time_us_t  timeout_us_;
			time_us_t  default_timeout_us_;
			void tick1sec_(void);
		protected:
			virtual void post(void) = 0;
			virtual void cancel(void) = 0;
			virtual void reset(void) = 0;
			virtual bool ready(void) = 0;
			virtual bool do_load(void);
			virtual void do_clean(void);
		public:
			itrafic trafic;
			int id(void) { return index_ref_.key(); }
			bus(cstr _name, app::module* _owner);
			virtual ~bus(void);
			void confirm(robo_tran_status_t _result);
			static void perform(void);
			static void tick1sec(void);
		};

		template<class D> class protdevagent_b : public D, public ::robo::frontend::shared {
		public:
			typedef typename D::action_s action_s;
			typedef typename D::feedback_s feedback_s;
			typedef typename D::config_s config_s;
			template <typename A> const A& action_cast(void) {
				return reinterpret_cast <const A&>(front_.action);
			}
			config_s & config(void) {
				return front_.config;
			}
		private:
			struct front_s {
				const action_s& action;
				action_s& goal;
				feedback_s& feedback;
				config_s & config;
				front_s(
					const action_s& _action
					, action_s& _goal
					, feedback_s& _feedback
					,config_s& config
				) : action(_action), goal(_goal), feedback(_feedback), config(config) {}
			} front_;
			action_s goal_;
			feedback_s feedback_;
		protected:
			virtual void apply_action(void) {
				if (D::agent_apply_action( action_cast<devagent::action_s>().command) ) {
					goal_ = front_.action;
				}
			}

			virtual void uppdate_feedback(void) {
				D::agent_uppdate_feedback();
				front_.feedback = feedback_;
				front_.goal = goal_;
			}
		public:
			protdevagent_b(cstr _name, boardagent& _boardagent, const action_s& _action, action_s& _goal, feedback_s& _feedback, config_s& _config)
				: D(_name, _boardagent, goal_, feedback_)
				, front_(_action, _goal, _feedback,_config)
				, ::robo::frontend::shared(
					(void*)(&_action)
					, (void*)((uint8_t*)(&_action) + sizeof(action_s) / sizeof(uint8_t))
					, (void*)(&_feedback)
					, (void*)((uint8_t*)((&_feedback)) + sizeof(feedback_s) / sizeof(uint8_t))
				) {}
		};
		template<class D > class protdevagent_t : public protdevagent_b<D>{
		public:
			typedef typename D::action_s action_s;
			typedef typename D::feedback_s feedback_s;
			typedef typename D::config_s config_s;
			typedef typename D::content_s content_s;
			protdevagent_t(cstr _name, boardagent& _boardagent, content_s& _content) :
				protdevagent_b<D>(_name, _boardagent, _content.action, _content.goal, _content.feedback, _content.config) {}
			/*			devagent_t(cstr _name, boardagent& _boardagent, content_s* _content, Args ... _args) :
				devagent_b<D, Args...>(_name, _boardagent, _content->action, _content->goal, _content->feedback, _content->config, _args ...)	{}
			devagent_t(cstr _name, boardagent& _boardagent, const action_s & _action, action_s& _goal, feedback_s& _feedback, config_s & _config, Args ... _args) :
				devagent_b<D, Args...>(_name, _boardagent, _action, _goal, _feedback, _config, _args ...){}*/
		};

		class servo_s : public  robo::app::node {
		public:
			servo_s(robo::cstr _name, robo::app::module& _module)
				: robo::app::node(_name, &_module) {}
		};

		class boardagent : public app::node {
			friend class devagent;
			time_us_t request_pause_us_ = 0;
			time_us_t last_request_us_ = 0;
		protected:
			virtual bool do_load(void);
			virtual void do_clean(void);
		public:
			servo_s& servo;
			boardagent(cstr _name, servo_s& _servo)
				: app::node(_name, &_servo) 
				, servo(_servo)
			{};
			boardagent(cstr _name, servo_s& _servo, app::node & _owner)
				: app::node(_name, &_owner)
				, servo(_servo)
			{};
		};

		class ROBO_EXPORT vartable : public devagent::stream, public frontend::vartable {
		public:
			class ROBO_EXPORT ivar : public frontend::vartable::ivar {
				friend class vartable;
				typedef ::robo::list::unsorted<ivar> queue;
				typedef queue::ref ref;
				
				ref ref_;
			protected:
				ivar(frontend::vartable& _vartable, const record& _instance);
				virtual bool rerquest(void);

			public:
				virtual bool encode(uint8_t* _dst) = 0;
				virtual bool decode(uint8_t* _dst) = 0;
			public:
				vartable& vt(void) { return (vartable&)frontend::vartable::ivar::vt(); }

				class ROBO_EXPORT answer : public performer {
				protected:
					virtual  void  operator ()(void) {
						(*this)((ivar *)var, result);
					};
					virtual  void  operator ()(ivar*, bool) = 0;
				public:
					using autonum = ::robo::delegat::autonum::fabric<answer, void, ivar*, bool>;
					using owned = ::robo::delegat::owned::fabric<answer, void, ivar*, bool>;
				};
				
				template <typename ... Args> bool query(Args...arg) {
					performer* p = answer::autonum::fabric::create(arg...);
					return query_a( p );
				}
				template <typename ... Args> bool post(cstr _s, Args...arg) {
					performer* p = answer::autonum::fabric::create(arg...);
					ROBO_LBREAKN(post_a(_s,p ));
					return true;
				}
			};


			template<  typename T> class ROBO_EXPORT var_t : public frontend::vartable::var_t< ivar, T> {
				typedef frontend::vartable::var_t< ivar, T> C;
			public:
				typedef frontend::vartable::ivar::performer performer;
			protected:
				struct ROBO_EXPORT iactual {
					T& local;
					T& remote;
					iactual(T& _local, T& _remote) : local(_local), remote(_remote) {}
				} actual;
				virtual bool applay(cstr _s) {
					if (is_frontend__) {
						return string::to_number(_s, C::front.local);
					}
					else {
						return string::to_number(_s, actual.local);
					}
				}
			public:
				virtual bool put_complete(void) {
					return actual.remote == actual.local;
				}



				/*bool post(performer* _performer) {
					ROBO_LRET(C::post(_performer));
				}

				bool post(const T& _value, performer* _performer=nullptr) {
					actual.local = _value;
					ROBO_LRET(C::post(_performer));
				}

				result try_post(const T& _value, performer* _performer=ullptr) {
					actual.local = _value;
					if (actual.remote != _value) {
						ROBO_RET(C::post(_performer), result::resume, result::panic);
					}
					else {
						return result::complete;
					}
				}
				*/
				bool query(performer* _performer=nullptr) {
					ROBO_LRET(C::query(_performer));
				}

				template <typename ... Args> bool query(Args...arg) {
					ROBO_LRET(C::query(arg...));
				}


				static var_t& create_var(cstr _path, cstr _name) {
					var_t* v = dynamic_cast<var_t*>(ivar::create_var(_path, _name));
					ROBO_APP_ASSERT(v != nullptr);
					return *v;
				}

				const T& value(void) {
					if (system::env::is_backend()) {
						return actual.remote;
					}
					else {
						return C::front.remote;
					}
				}

				operator const T& (void) {
					return actual.remote;
				}

				var_t(frontend::vartable& _vartable, cstr _name, T& _front_local, T& _front_remote, T& _actual_local, T& _actual_remote)
					: frontend::vartable::var_t< ivar, T>(
						_vartable
						, _vartable.find_record_ref(_name)
						, _front_local
						, _front_remote
						)
					, actual(_actual_local, _actual_remote) {
					ROBO_VBREAKN_F(sizeof(T) == C::length(), "error typecast for var '%s' ", ivar::name());
					C::begin();
				};
				var_t(frontend::vartable& _vartable, const  robo::frontend::vartable::record& _record, T& _front_local, T& _front_remote, T& _actual_local, T& _actual_remote)
					: frontend::vartable::var_t< ivar, T>(
						_vartable
						, _record
						, _front_local
						, _front_remote
						)
					, actual(_actual_local, _actual_remote) {
					ROBO_VBREAKN_F(sizeof(T) == C::length(), "error typecast for var '%s' ", ivar::name());
					C::begin();
				};

			public:

				virtual bool encode(uint8_t* _dst) {
					{
						system::guard g__;
						if (C::actual_hook() == C::hook::frontend) {
							actual.local = C::front.local;
						}
						else {
							C::front.local = actual.local;
						}
					}
					std::copy_n((uint8_t*)(&actual.local), C::length(), _dst);
					return true;
				}

				virtual bool decode(uint8_t* _src) {
					std::copy_n(_src, C::length(), (uint8_t*)(&actual.remote));
					{
						system::guard g__;
						C::front.remote = actual.remote;
					}
					return true;
				}

			};

			template< typename T> class ROBO_EXPORT var : public  var_t<T> {
			private:
				struct {
					T local;
					T remote;
				} front_;
				struct {
					T local;
					T remote;
				} actual_;
			public:
				var(frontend::vartable& _vartable, cstr _name)
					: var_t<T>(_vartable, _name, front_.local, front_.remote, actual_.local, actual_.remote) {};
				var(frontend::vartable& _vartable, const  robo::frontend::vartable::record& _record)
					: var_t<T>(_vartable, _record, front_.local, front_.remote, actual_.local, actual_.remote) {};
			};

			template < typename T> class  ROBO_EXPORT fabric_t: public robo::frontend::vartable::fabric {
			public:
				fabric_t(cstr _type_name) : robo::frontend::vartable::fabric(_type_name) {}
				virtual robo::frontend::vartable::ivar* create(robo::frontend::vartable& _vartable, const  robo::frontend::vartable::record& _record) {

						return new robo::backend::vartable::var<T>(_vartable, _record);
				}
			};

			template<  typename T> class fvar_t : public var_t<T> {
				converter* converter_ = nullptr;
			public:
				fvar_t * find( cstr _path, cstr _name) {
					app::node* _node = app::node::find(_path);
					ROBO_BREAKN_F(_node, nullptr, "invalid path: '%s' ", _path);
					vartable* ct = dynamic_cast<vartable*>(_node);
					ROBO_BREAKN_F(ct, nullptr, "invalid object '%s' ", _path);
					vartable::ivar * v = ct->find_var(_name);
					ROBO_BREAKN_F(v, nullptr, "var is't found '%s.%s' ", _path, _name);
					fvar_t* fv = dynamic_cast<fvar_t*>(v);
					ROBO_BREAKN_F(fv, nullptr, "var is't found '%s.%s' ", _path, _name);
					return fv;
				}
				fvar_t* find(vartable* _vt, cstr _name) {
					vartable::ivar* v = _vt->find_var(_name);
					ROBO_BREAKN_F(v, nullptr, "var is't found '%s.%s' ", _vt->own_agent().alias(), _name);
					fvar_t* fv = dynamic_cast<fvar_t*>(v);
					ROBO_BREAKN_F(fv, nullptr, "var is't found '%s.%s' ", _vt->own_agent().alias(), _name);
					return fv;
				}

				typedef var_t<T> B;
				bool set_converter(cstr _name) {
					converter_ = dynamic_cast<converter*>(app::node::find(_name));
					return converter_ != nullptr;
				}

				bool set_converter(converter* _converter) {
					converter_ = _converter;
					return converter_ != nullptr;
				}

				fvar_t(
					vartable& _vartable
					, cstr _name
					, cstr _converter
					, T& _front_local
					, T& _front_remote
					, T& _actual_local
					, T& _actual_remote
				) : var_t< T>(
					_vartable
					, _name
					, _front_local
					, _front_remote
					, _actual_local
					, _actual_remote
					) {
					if (_converter != nullptr) {
						set_converter(_converter);
					}
				};


				result try_load(cstr _section, cstr _key) {
					ROBO_BREAKN(ini::load(_section, _key, B::actual.local), result::panic);
					if (B::actual.local != B::actual.remote) {
						ROBO_RET(B::post(), result::resume, result::panic);
					}
					else {
						return result::complete;
					}
				}

				typedef  typename T::performer* performer;

				result try_load(cstr _section, cstr _key, performer * _performer) {
					ROBO_BREAKN(ini::load(_section, _key, B::actual.local), result::panic);
					if (B::actual.local != B::actual.remote) {
						ROBO_RET(B::post(_performer), result::resume, result::panic);
					}
					else {
						return result::complete;
					}
				}

				template <typename U> float  to_float(U _src) { return converter_ == nullptr ? 0.f : converter_->to_float(_src); }
				result try_post_min(void) {
					if (converter_) {
						if (fabs(B::acual.remote - converter_->min()) > converter_->eps()) {
							B::acual.local = converter_->min();
							ROBO_RET(B::post(), result::resume, result::panic);
						}
					}
					return result::complete;
				}

				result try_post_max(void) {
					if (converter_) {
						if (fabs(B::actual.remote - converter_->max()) > converter_->eps()) {
							B::actual.local = converter_->max();
							ROBO_RET(B::post(), result::resume, result::panic);
						}
					}
					return result::complete;
				}

				result try_post_min(performer * _performer) {
					if (converter_) {
						if (fabs(B::actual.remote - converter_->min()) > converter_->eps()) {
							B::actual.local = converter_->min();
							ROBO_RET(B::post(_performer), result::resume, result::panic);
						}
					}
					return result::complete;
				}
				converter* conv(void) { return converter_; }
				result try_post_max(performer* _performer) {
					if (converter_) {
						if (fabs(B::actual.remote - converter_->max()) > converter_->eps()) {
							B::actual.local = converter_->max();
							ROBO_RET(B::post(_performer), result::resume, result::panic);
						}
					}
					return result::complete;
				}


				result try_post(const T& _value) {
					if (converter_) {
						if (fabs(B::actual.remote - _value) > converter_->eps()) {
							B::actual.local = _value;
							ROBO_RET(B::post(), result::resume, result::panic);
						}
						else {
							return result::complete;
						}
					}
					else {
						if (B::actual.remote != _value)
						{
							B::actual.local = _value;
							ROBO_RET(B::post(), result::resume, result::panic);
						}
						else
						{
							return result::complete;							
						}
					}
				}

				result try_post(const T& _value, performer* _performer) {
					if (converter_) {
						if (fabs(B::actual.remote - _value) > converter_->eps()) {
							B::actual.local = _value;
							ROBO_RET(B::post(_performer), result::resume, result::panic);
						}
						else {
							return result::complete;
						}
					}
					else {
						return B::try_post(_value, _performer);;
					}
				}

			protected:
				virtual bool encode(uint8_t* _dst) {
					{
						system::guard g__;
						if (B::actual_hook() == B::hook::frontend) {
							B::actual.local = B::front.local;
						}
						else {
							B::front.local = B::actual.local;
						}
					}
					if (converter_) {
						//robo::system::printf (RT("convert '%s/%s' %f\n\r"), B::owner().own_agent().alias(), B::name(), B::actual.local);
						switch (B::length()) {
						case 1:
						{
							uint8_t tmp = converter_->to_u8(B::actual.local);
							std::copy_n(&tmp, 1, _dst);
						}
						break;
						case 2:
						{
							uint16_t tmp = converter_->to_u16(B::actual.local);
							std::copy_n((uint8_t*)(&tmp), 2, _dst);
						}
						break;
						case 4:
						{
							uint32_t tmp = converter_->to_u32(B::actual.local);
							std::copy_n((uint8_t*)(&tmp), 4, _dst);
						}
						break;
						default:
						return false;
						}
					}
					else {
						switch (B::length()) {
						case 1:
						{
							uint8_t tmp = (uint8_t)B::actual.local;
							std::copy_n(&tmp, 1, _dst);
						}
						break;
						case 2:
						{
							uint16_t tmp = (uint16_t)(B::actual.local);
							std::copy_n((uint8_t*)(&tmp), 2, _dst);
						}
						break;
						case 4:
						{
							uint32_t tmp = (uint32_t)(B::actual.local);
							std::copy_n((uint8_t*)(&tmp), 4, _dst);
						}
						break;
						default:
						std::copy_n((uint8_t*)(&B::actual.local), B::length(), _dst);

						}
					}
					return true;
				}

				virtual bool decode(uint8_t* _src) {
					//std::copy_n(_src, B::length(), (uint8_t*)(&actual.remote));
					if (converter_) {
						switch (B::length()) {
						case 1:
						B::actual.remote = (T)converter_->to_float(*(uint8_t*)_src);
						break;
						case 2:
						B::actual.remote = (T)converter_->to_float(*(uint16_t*)_src);
						break;
						case 4:
						B::actual.remote = (T)converter_->to_float(*(uint32_t*)_src);
						break;
						default:
						return false;
						}
					}
					else {
						std::copy_n(_src, B::length(), (uint8_t*)(&B::actual.remote));
					}

					{
						system::guard g__;
						B::front.remote = B::actual.remote;
					}
					return true;
				}
			};

			template<typename T> class fvar : public  fvar_t< T> {
			private:
				struct {
					T local;
					T remote;
				} front_;
				struct {
					T local;
					T remote;
				} actual_;
			public:
				/*
				fvar(vartable& _vartable, const record& _instance)
					: fvar_t<T>(_vartable, _instance, front_.local, front_.remote, actual_.local, actual_.remote) {
				}
				*/
				fvar(vartable& _vartable, cstr _name, cstr _converter = nullptr)
					: fvar_t<T>(_vartable, _name, _converter, front_.local, front_.remote, actual_.local, actual_.remote) {}
			};



			class proto {
			public:
				enum class result { success, repeat, fail};
				virtual result request(robo_tran_t& _tran, ivar * _var) = 0;
				virtual result confirm( const robo_tran_t& _tran, ivar* _var) = 0;
			};
			virtual query_result query(robo_tran_t& _tran);
			virtual void confirm(const robo_tran_t& _tran);
			virtual bool exchange_need(void);
			vartable(devagent& _agent, proto& _proto, priority _priority, const record* const _records, size_t _count);
			bool query(frontend::vartable::ivar::performer* _performer);
			bool ready(void);
			template<class T> T& proto_cast(void) { return reinterpret_cast<T&>(proto_); }
		protected:
			virtual bool do_ready(void) { return true;  }
			virtual bool do_exchange_need(void) { return false; }



		private:
			proto& proto_;
			ivar::queue queue_;
			ivar* current_ = nullptr;
		};
		/*
		namespace process {
			class base;
			class ROBO_EXPORT controller : public task {
				base* selected_ = nullptr;
				base* runned_ = nullptr;
				time_us_t period_us_;
				devagent** owned_ = nullptr;
				size_t owned_count_ = 0;
				bool attached_ = false;
				void begin_process_(base* _runned);
				void finish_process_(void);
			protected:
				virtual  result  execute(void);
			public:
				void switchto(base* _process);
				controller(void);
				controller(devagent** _owned, size_t _owned_count);
				void  setup(devagent** _owned, size_t _owned_count);
				virtual ~controller(void);
				void agents_set_stop_mode(void);
				void agents_raise_fault(void);
				bool agents_stopped(void);
				bool agents_set_work_mode(void);
				bool agents_active(void);
				//				void forall_exec( delegat::ref<void, devagent> & _f);
				//				bool all_set(robo::lambda <bool(agent*)>  _f);
				//				bool any_set(robo::lambda <bool(agent*)>  _f);
				bool continue_sleep(void) { return  task::continue_sleep(); }
			};

			class ROBO_EXPORT base : public app::node {
			private:
				friend class controller;
				time_us_t period_min_us_;
				time_us_t period_max_us_;
				time_us_t period_us_;
				controller* controller_;
				bool active_;
			protected:
				bool active(void) { return  active_; }
				bool start(void);
				void stop(void);
				virtual bool do_start(void) = 0;
				virtual void do_stop(void) = 0;
				virtual result run(void) = 0;
				base(cstr& _name);

				virtual bool node_load(void);

				void agents_set_stop_mode(void);
				void agents_raise_fault(void);
				bool agents_stopped(void);
				bool agents_set_work_mode(void);
				bool agents_active(void);
			public:
				void set_period(time_us_t _period_us);
				void set_range(time_us_t _period_min_us, time_us_t _period_max_us);
				void scale_period(float _score_pp);

				inline time_us_t period() { return period_us_; }
				inline time_us_t period_max() { return period_max_us_; }
				inline void run_fast(void) { period_us_ = period_min_us_; };
				inline void run_slow(void) { period_us_ = period_max_us_; };
				inline time_us_t period_min() { return period_min_us_; }
				//static base* find(int _index) { return index_().find(_index); }
				static bool load(void);
				static void clean(void);

			};
		}
		*/
		#endif
	}
}
#endif

