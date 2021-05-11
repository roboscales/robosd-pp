#ifndef robosd_backend_hpp
#define robosd_backend_hpp

#include "core/robosd_frontend.hpp"
#include "core/robosd_string.hpp"
#include "core/robosd_delegat.hpp"
#include "core/robosd_app.hpp"
#include "core/robosd_tran.h"
#include "core/robosd_tran.h"
#include "net/robosd_net_trafic.hpp"
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
			enum { state_count=4};
		private:
			state state_;
			void step_();
			inline void sleep_timeout_set_(time_us_t _timeout) { ref_.dettach(); ref_.set_key(_timeout); }
		protected:
			inline time_us_t sleep_timeout(void) { return ref_.key(); }
			virtual  result  execute(void) = 0;
		public:
			string name;
			class ROBO_EXPORT machine {
				friend class task;
				pool disabled_;
				pool suspended_;
				list active_;
				pool trash_;
				list timer_;
				pool getup_;
				machine(void){};
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
			bool sleep( time_us_t _timeout = timeout::infinite );
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
				void start_( signal::performer* _performer, time_us_t _period);
				void stop_( signal::performer* _performer, time_us_t _period);
				void restart_(void);
				static core& instance(void);
			public:
				static void start( signal::performer* _performer, time_us_t _period) { instance().start_(_performer, _period); }
				static void stop( signal::performer* _performer, time_us_t _period) { instance().stop_(_performer, _period); }
				static void restart() { instance().restart_(); }
			};
		};

		class ROBO_EXPORT queue : public signal {
			void poll_(void);
			void post_(signal::performer* _performer, signal::performer::priority _priority);
			bool execute_(signal::performer* _performer, time_ms_t _timeout);
			bool wait_(time_ms_t _timeout);
			bool ready_(void);
			static queue& instance_(void);
		public:
			static void poll(void) { instance_().poll_(); }
			static void post(signal::performer* _performer, signal::performer::priority _priority) { instance_().post_(_performer, _priority); }
			static bool execute(signal::performer* _performer, time_ms_t _timeout) { return instance_().execute_(_performer, _timeout); }
			static bool wait(time_ms_t _timeout) { return instance_().wait_(_timeout); }
			static bool ready(void) { return instance_().ready_(); }
		};

		class ROBO_EXPORT repeater: public signal::performer {
			time_us_t period_;
		public:
			enum { default_period_us = 1000 };
			repeater( signal::performer& _performer, time_us_t _period = default_period_us)
				: performer(false)
				, period_(_period)
			{}
			void start(void) { timer::core::start(this, period_); }
			void start( time_us_t _period) { period_ = _period;  start(); }
			void stop(void) { timer::core::stop(this, period_); }
		};

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
		private:
			size_t table_size_=0;
			record* table_ = nullptr;
		public:
			virtual bool node_load(void);
			virtual void node_clean(void);
			record* resolve(int _bus_id, robo_tran_header_p  _tran_header);
			router(cstr _name, app::module* _owner);
		};


		class idevagent : public app::node {
		public:
			typedef  ::robo::list::sorted<idevagent, int> bus_index;
			typedef  bus_index::ref bus_ref;
			class ROBO_EXPORT  stream {
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
					typedef  list::ref ref;
					ref ref_;
					robo_tran_p tran_;
					void confirm();
					msg(robo_tran_p _tran);
					virtual bool prepare();
					inline idevagent& own_agent() { return stream_->own_agent(); }
				};

				typedef signal::performer::priority priority;
				typedef  ::robo::list::sorted<stream, priority> list;
				typedef  list::ref ref;
			private:
				ref ref_;
				idevagent& agent_;
			public:
				inline idevagent& own_agent() { return agent_; }
				stream(idevagent& _agent, priority _priority);
				virtual ~stream();
				virtual bool exchange_need() = 0;
				virtual query_result query(robo_tran_p _tran) = 0;
				virtual void confirm(robo_tran_p _tran) = 0;
				query_result query(msg* _msg);
			};

		private:
			boardagent& boardagent_;
			dev_id_t dev_id_;
			bus_ref bus_ref_;
			int bus_order_ = 0;
			router* router_ = nullptr;
			typedef frontend::idevagent::istate state;
			state actual_state_ = state::unknown;
			stream::list streams_;
		protected:
			virtual void apply_action(void) = 0;
			virtual void uppdate_feedback(void) = 0;
			bool exchabge_enabled(void) { return actual_state_ > state::disabled; }
			virtual bool node_load(void);
			virtual void node_clean(void);
			virtual bool node_start(void);

		public:
			itrafic trafic;

			const dev_id_t& dev_id(void) { return dev_id_; };
			
			stream::query_result query(stream::msg* _msg);

			idevagent(cstr _name, boardagent& _boardagent);

			router::record* resolve(int _bus_id, robo_tran_header_p  _tran_header);
		};

		class ROBO_EXPORT bus : public app::node {
		public:
			typedef ::robo::list::unique<bus, int> index;
			typedef index::ref ref;
		private:
			ref ref_;
		public:
			class ROBO_EXPORT msg : public idevagent::stream::msg {
			public:
				robo_tran_t tran;
				router::record::address_t address;
				router::record::suba_t suba;
				bus* ownbus;
				msg(void) : idevagent::stream::msg(&tran), ownbus(0) {}
				virtual ~msg() {}
				virtual bool  prepare(void);
				virtual void release(void) {}
			};

		private:
			friend class idevagent;
			idevagent::bus_ref* current_agent_ref_ = nullptr;
			idevagent::bus_index agents_;
			msg* current_msg_ = nullptr;
			void refuse__(msg* _msg);
			friend class api;
			bool request_(msg* _msg);
			void perform_(void);
			bool ready_(void);
			msg* pop_incom_msg_(void);
			time_us_t  request_begin_us_;
			time_us_t  timeout_us_;
			time_us_t  default_timeout_us_;
			void tick1sec_(void);
			bool setup_(int _id);
		protected:
			virtual bool post(msg* _msg) = 0;
			virtual void cancel(void) = 0;
			virtual bool ready(void) = 0;
			virtual bool node_load(void);
			virtual void node_clean(void);
		public:
			//короткий id
			bool id(void) { return ref_.key(); }
			//todo подпорка для busmarshal
			msg* current_msg(void) { return current_msg_; };
			itrafic trafic;
			virtual msg* get_msg(void) = 0;
			virtual void  release_msg(msg*) = 0;
			bus(cstr _name, app::module* _owner);
			virtual ~bus(void);
			void confirm(robo_tran_status_t _result);
			static void perform(void);
			static void tick1sec(void);
		};





		template<class D> class devagent : public ::robo::frontend::devagent<D> , public ::robo::backend::idevagent {
		public:
			struct {
				typename D::irequired required;
				typename D::istatus status;
				typename D::iaction action;
				typename D::ifeedback feedback;
			} actual;
			virtual void apply_action(void){
				if ((istate)actual.status.state == istate::external) {
					actual.action = front.action;
				}
				if ((istate)actual.status.state == istate::stopped) {
					actual.required = front.required;
				}
			}

			virtual void uppdate_feedback(void) {
				if ( (istate)actual.status.state == istate::external) {
					front.action = actual.action;
				}
				front.feedback = actual.feedback;
			
			}
			devagent(cstr _name, boardagent& _boardagent) : ::robo::backend::idevagent(_name, _boardagent) {}
		};

		class boardagent : public app::node {
			friend class idevagent;
			time_us_t request_pause_us_ = 0;
			time_us_t last_request_us_ = 0;
		protected:
			virtual bool node_load(void);
			virtual void node_clean(void);
		public:
			boardagent(cstr _name, app::module* _owner) :app::node(_name,_owner) {};

		};


		




		namespace process {
			class base;
			class ROBO_EXPORT controller : public task {
				base* selected_ = nullptr;
				base* runned_ = nullptr;
				time_us_t period_us_;
				idevagent** owned_ = nullptr;
				size_t owned_count_ = 0;
				bool attached_ = false;
				void begin_process_(base* _runned);
				void finish_process_(void);
			protected:
				virtual  result  execute(void);
			public:
				void switchto(base* _process);
				controller(void);
				controller(idevagent** _owned, size_t _owned_count);
				void  setup(idevagent** _owned, size_t _owned_count);
				virtual ~controller(void);
				void agents_set_stop_mode(void);
				void agents_raise_fault(void);
				bool agents_stopped(void);
				bool agents_set_work_mode(void);
				bool agents_active(void);
				//				void forall_exec( delegat::base<void, devagent> & _f);
				//				bool all_set(robo::lambda <bool(agent*)>  _f);
				//				bool any_set(robo::lambda <bool(agent*)>  _f);
				bool continue_sleep(void) { return  task::continue_sleep(); }
			};

			class ROBO_EXPORT base: public app::node {
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


			/*
			class ROBO_EXPORT repeater : public base {
				front::signal::performer& performer_;
			protected:
				virtual bool do_start(void) { return true; };
				virtual void do_stop(void) {};
				virtual result run(void);
				repeater(const robo::string& _name, front::signal::performer& _performer, robo_time_us_t _period_us);
			};

			class ROBO_EXPORT repeat_simple : public repeater {
				front::signal::simple simple_;
			public:
				repeat_simple(const robo::string& _name, void(*_simple)(void), robo_time_us_t _period, bool _once = false)
					: repeater(_name, simple_, _period), simple_(_simple, _once) {}
			};

			class ROBO_EXPORT repeat_uni : public repeater {
				front::signal::uni uni_;
			public:
				repeat_uni(const robo::string& _name, void(*_uni)(void*), void* _context, robo_time_us_t _period, bool _once = false)
					: repeater(_name, uni_, _period), uni_(_uni, _context, _once) {}
			};

			class ROBO_EXPORT repeat_lambda : public repeater {
				front::signal::lambda lambda_;
			public:
				repeat_lambda(const robo::string& _name, lambda< void(void) >  _lambda, robo_time_us_t _period, bool _once = false)
					:repeater(_name, lambda_, _period), lambda_(_lambda, _once) {}
			};



			class ROBO_EXPORT unimachin : public base {
			public:
			private:
				enum class command { start, stop } command_;
				enum class state { stopped, startup, run, shutdown, panic };
				state state_;
				void panic_(void);
			protected:
				virtual status do_startup(void) = 0;
				virtual status do_run(void) = 0;
				virtual status do_shutdown(void) = 0;
				virtual void do_panic(void) = 0;
				virtual bool do_start(void) {
					command_ = command::start;
					return true;
				}
				virtual void do_stop(void) {
					command_ = command::stop;
				}
				virtual result run(void);
			public:
				unimachin(const robo::string& _name)
					: base(_name)
					, state_(state::stopped)
				{
				}
			};


			class ROBO_EXPORT  machine_lambda : public unimachin {
				robo::lambda<status(void)>  do_startup_;
				robo::lambda<status(void)>  do_run_;
				robo::lambda<status(void)>  do_shutdown_;
				robo::lambda<void(void)>  do_panic_;
			protected:
				virtual status do_startup(void) {
					return do_startup_();
				}
				virtual status do_run(void) {
					return do_run_();
				}
				virtual status do_shutdown(void) {
					return do_shutdown_();
				}
				virtual void do_panic(void) {
					do_panic_();
				}

			public:
				machine_lambda(
					const robo::string& _name
					, robo::lambda<status(void)>  _do_startup
					, robo::lambda<status(void)>  _do_run
					, robo::lambda<status(void)>  _do_shutdown
					, robo::lambda<void(void)>  _do_panic
				)
					: unimachin(_name)
					, do_startup_(_do_startup)
					, do_run_(_do_run)
					, do_shutdown_(_do_shutdown)
					, do_panic_(_do_panic)
				{
				}
			};
			*/
		}
	}
}
#endif
