#ifndef robosd_backend_hpp
#define robosd_backend_hpp
#include "core/robosd_frontend.hpp"
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

		class ROBO_EXPORT repeater : public signal::performer {
			time_us_t period_;
		public:
			enum { default_period_us = 1000 };
			repeater(time_us_t _period = default_period_us)
				: performer(false)
				, period_(_period)
			{}
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
			mode actual_mode(void) { return mode_;  }
			virtual record* resolve(int _bus_id, robo_tran_header_p  _tran_header);
			router(cstr _name, app::module & _owner);
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
			typedef frontend::idevagent::istate state;
			typedef frontend::idevagent::icommand command;
			typedef frontend::idevagent::istatus status;

		private:
			boardagent& boardagent_;
			dev_id_t dev_id_;
			bus_ref bus_ref_;
			int bus_order_ = 0;
			router* router_ = nullptr;
			state actual_state_;
			stream::list streams_;
			command actual_command_ = command::stop;
		protected:
			void perform_command(command _command) {

			}
			virtual void apply_action(void) {};
			virtual void uppdate_feedback(void) {};

			bool exchabge_enabled(void) { return actual_state_.local > state::ilocal::disabled; }
			bool configure_complete(void) { 
				ROBO_LBREAKN(actual_state_.local == state::ilocal::configure);
				actual_state_.local = state::ilocal::ready;
				return true;
			}
			virtual bool do_load(void);
			virtual void do_clean(void);
			virtual bool do_node_start(void);

		public:
			itrafic trafic;

			const dev_id_t& dev_id(void) { return dev_id_; };
			void dev_set_id(uint8_t _addr) { dev_id_.address =_addr; };

			stream::query_result query(stream::msg* _msg);

			idevagent(cstr _name, boardagent& _boardagent);

			router::record* resolve(int _bus_id, robo_tran_header_p  _tran_header);

			status actual_status(void) {
				static const status tb[] =
				{
					//icommand
					//stop = 0,			sw2service = 1,		raise_fault = 2,	sw2independed = 3,	sw2dirrect = 4,	reset_fault = 5
					//unknown
					status::unknown,	status::unknown,	status::unknown,	status::unknown,	status::unknown,	status::unknown,
					//stopped
					status::stopped,	status::stopped,	status::stopped,	status::stopped,	status::stopped,	status::stopped,
					//fault
					status::fault,		status::fault,		status::fault,		status::fault,		status::fault,		status::busy,
					//run
					status::busy,		status::service,	status::busy,		status::independed,status::dirrect,	status::busy,
					//broke
					status::broke,		status::broke,		status::broke,		status::broke,		status::broke,		status::broke
				};
				switch (actual_state_.local) {
				case state::ilocal::unknown:
					return status::unknown;
				case state::ilocal::disabled:
					return status::disabled;
				case state::ilocal::configure:
					return status::busy;
				case state::ilocal::ready:
					return tb[((int)actual_state_.remote * 5) + (int)actual_command_];
				default:
					return status::unknown;
				}
			}

			command actual_command(void) { return actual_command_;  };
			state::ilocal local_state(void) { return  actual_state_.local; };
			state::iremote remote_state(void) { return  actual_state_.remote; };

		};

		class ROBO_EXPORT bus : public app::node {
		public:
			typedef ::robo::list::unique<bus, int> index;
			typedef index::ref index_ref;
		private:
			index_ref index_ref_;
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
			virtual bool do_load(void);
			virtual void do_clean(void);
		public:
			//короткий id
			int id(void) { return index_ref_.key(); }
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

		template<class D> class devagent : public ::robo::frontend::devagent<D>, public ::robo::backend::idevagent {
		public:
			typedef typename ::robo::frontend::devagent<D>::iaction iaction;
			typedef typename ::robo::frontend::devagent<D>::ifeedback ifeedback;
			iaction action;
			ifeedback feedback;

			virtual void apply_action(void) {
				perform_command( ::robo::frontend::devagent<D>::front.action.command ) ;
				feedback.status = actual_status();
				action.deseired = ::robo::frontend::devagent<D>::front.action.deseired;
				if (feedback.status == status::dirrect ) {
					action.required = action.deseired;
				} 
			}

			virtual void uppdate_feedback(void) {
				feedback.status = actual_status();
				::robo::frontend::devagent<D>::front.feedback = feedback;
				::robo::frontend::devagent<D>::front.action.required = action.required;
			}

			devagent(cstr _name, boardagent& _boardagent, iaction& _action, ifeedback& _feedback) :
				::robo::frontend::devagent<D>(_action, _feedback)
				, ::robo::backend::idevagent(_name, _boardagent) {}
		};

		class boardagent : public app::node {
			friend class idevagent;
			time_us_t request_pause_us_ = 0;
			time_us_t last_request_us_ = 0;
		protected:
			virtual bool do_load(void);
			virtual void do_clean(void);
		public:
			boardagent(cstr _name, app::module& _owner) :app::node(_name, &_owner) {};
		};

		class contrltable : public idevagent:: stream, frontend::contrltable{
		private:
			int command_;
		public:
			class ivar : public frontend ::contrltable :: ivar {
				friend class contrltable;
				typedef ::robo::list::unsorted<ivar> queue;
				typedef queue::ref ref;
				ref ref_;
			protected:
				ivar(frontend::contrltable& _contrltable, const record& _instance);
				virtual bool rerquest(void);
				virtual bool encode(uint8_t* _dst) = 0;
				virtual bool decode(uint8_t* _dst) = 0;
			public:
				contrltable& owner(void) { return (contrltable&)frontend::contrltable::ivar::owner(); }
			};

			
			template<  typename T> class var : public frontend::contrltable::var< ivar, T>  {
			public:
				typedef frontend::contrltable::var< ivar, T> B;
				typedef frontend::contrltable::ivar::delegat delegat; 
			protected:
				struct {
					T local;
					T remote;
				} actual;
			public:

				bool post(void) {
					ROBO_LRET(B::post());
				}
				bool post(delegat& _delegat) {
					ROBO_LRET(B::post(_delegat));
				}			

				bool post(const T& _value) {
					actual.local = _value;
					//robo::system::printf (RT("convert '%s/%s' %f\n\r"), B::owner().own_agent().alias(), B::name(), actual.local);
					ROBO_LRET(B::post());
				}
				bool post(const T& _value, delegat& _delegat) {
					actual.local = _value;
					ROBO_LRET(B::post(_delegat));
				}			
				
				result try_post(const T& _value) {
					actual.local  = _value;
					if(  actual.remote != _value  ){
						ROBO_RET(B::post(),result::resume,result::panic);
					} else {
						return result::complete;
					}
				}

				result try_post(const T& _value, delegat& _delegat) {
					actual.local  = _value;
					if(  actual.remote != _value  ){
						ROBO_RET(B::post(_delegat),result::resume,result::panic);
					} else {
						return result::complete;
					}				
				}

				
				bool query(void) {
					ROBO_LRET( B::query() );
				}
				
				bool query(delegat& _delegat) {
					ROBO_LRET(B::query(_delegat) );
				}				

				static var & create_var( cstr _path, cstr _name ) {
					var * v = dynamic_cast<var *>(ivar::create_var(_path, _name));
					ROBO_APP_ASSERT(v!=nullptr)
					return *v;
				}

				const T & value(void) { 
					if(system::env::is_backend()){
						return actual.remote;
					} else {
						return B::front.remote;
					}
				}

				operator const T & (void) { 
						return actual.remote;
				}

				var(contrltable& _contrltable, cstr _name) 
					: frontend::contrltable::var< ivar, T>(
						_contrltable
						, _contrltable.find_record_ref(_name)
					) {
					ROBO_VBREAKN_F(sizeof(T) == B::length(), "error typecast for var '%s/%s' ", B::owner().alias(), ivar::name());
					B::begin();
				};

			protected:

				virtual bool encode(uint8_t* _dst) {
					{
						system::guard g__;
						if (B::actual_hook() == B::hook::frontend) {
							actual.local = B::front.local;
						}
						else {
							B::front.local = actual.local ;
						}
					}
					std::copy_n((uint8_t*)(&actual.local), B::length() , _dst);
					return true;
				}
				
				virtual bool decode(uint8_t* _src) {
					std::copy_n(_src, B::length(), (uint8_t*)(&actual.remote));
					{
						system::guard g__;
						B::front.remote = actual.remote;
					}
					return true;
				}
				
		};
			
		template<  typename T> class fvar : public var<T>  {
			converter * converter_  = nullptr;
			public:
				typedef var<T> B;
				bool set_converter(cstr _name){
					converter_ = dynamic_cast<converter *>( app::node::find(_name));
					return converter_ != nullptr;
				}

				bool set_converter(converter * _converter){					
					converter_ = _converter;
					return converter_ != nullptr;
				}

				fvar(contrltable& _contrltable, cstr _name, cstr _converter = nullptr) 
					: var< T>(
						_contrltable
						, _name
					) 
				{
					if(_converter != nullptr) {
						set_converter(_converter);
					}
				};
				

				result try_load(cstr _section, cstr _key) {
					ROBO_BREAKN(ini::load(_section, _key, B::actual.local), result::panic);			
					if( B::actual.local != B::actual.remote )	{
						ROBO_RET(B::post(),result::resume,result::panic);
					} else {
						return result::complete;
					}
				}
				
				typedef frontend::contrltable::ivar::delegat delegat; 
				result try_load( cstr _section, cstr _key, delegat& _delegat) {
					ROBO_BREAKN(ini::load(_section, _key, B::actual.local), result::panic);			
					if( B::actual.local != B::actual.remote )	{
						ROBO_RET(B::post(_delegat),result::resume,result::panic);
					} else {
						return result::complete;
					}
				}
				
				template <typename U> float  to_float( U _src){  return converter_==nullptr? 0.f  : converter_->to_float(_src); }
				result try_post_min(void){
					if(converter_){
						if( fabs( B::acual.remote - converter_->min() ) > converter_->eps() ){
							B::acual.local  = converter_->min();
							ROBO_RET(B::post(),result::resume,result::panic);
						}
					}
					return result::complete;
				}

				result try_post_max(void){
					if(converter_){
						if( fabs( B::actual.remote - converter_->max()) > converter_->eps() ){
							B::actual.local  = converter_->max();
							ROBO_RET(B::post(),result::resume,result::panic);
						}
					}
					return result::complete;
				}

				result try_post_min(delegat& _delegat){
					if(converter_){
						if( fabs( B::actual.remote - converter_->min() ) > converter_->eps() ){
							B::actual.local  = converter_->min();
							ROBO_RET(B::post(_delegat),result::resume,result::panic);
						}
					}
					return result::complete;
				}
				converter * conv(void){ return converter_; }
				result try_post_max(delegat& _delegat){
					if(converter_){
						if( fabs( B::actual.remote - converter_->max() ) > converter_->eps() ){
							B::actual.local  = converter_->max();
							ROBO_RET(B::post(_delegat),result::resume,result::panic);
						}
					}
					return result::complete;
				}


				result try_post(const T& _value) {
					if(converter_){
						B::actual.local  = _value;
						if( fabs( B::actual.remote - _value ) > converter_->eps() ){
							ROBO_RET(B::post(),result::resume,result::panic);
						} else {
							return result::complete;
						}
					}else{
							return B::try_post(_value);;
					}
				}

				result try_post(const T& _value, delegat& _delegat) {
					if(converter_){
						B::actual.local  = _value;
						if( fabs( B::actual.remote - _value ) > converter_->eps() ){
							ROBO_RET(B::post(_delegat),result::resume,result::panic);
						} else {
							return result::complete;
						}
					}else{
							return B::try_post(_value,_delegat);;
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
							B::front.local = B::actual.local ;
						}
					}
					if(converter_){
						//robo::system::printf (RT("convert '%s/%s' %f\n\r"), B::owner().own_agent().alias(), B::name(), B::actual.local);
						switch(B::length()){
							case 1:
							{
								uint8_t tmp = converter_->to_u8( B::actual.local);
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
					} else {
						switch(B::length()){
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
								std::copy_n((uint8_t*)(&tmp), 4 , _dst);
							}
								break;
							default:
								std::copy_n((uint8_t*)(&B::actual.local), B::length() , _dst);
								
						}
					}
					return true;
				}
				
				virtual bool decode(uint8_t* _src) {
					//std::copy_n(_src, B::length(), (uint8_t*)(&actual.remote));
					if(converter_){
						switch(B::length()){
							case 1:
								B::actual.remote= (T)converter_->to_float( *(uint8_t *)_src);
								break;
							case 2:
								B::actual.remote= (T)converter_->to_float( *(uint16_t *)_src);
								break;
							case 4:
								B::actual.remote= (T)converter_->to_float( *(uint32_t *)_src);
								break;								
							default:
								return false;
						}
					} else {						
						std::copy_n( _src, B::length() , (uint8_t*)(&B::actual.remote));
					}
					
					{
						system::guard g__;
						B::front.remote = B::actual.remote;
					}
					return true;
				}
			};

			virtual query_result query(robo_tran_p _tran);
			virtual void confirm(robo_tran_p _tran);
			virtual bool exchange_need() { system::guard g__;  return queue_.count() > 0; }
			contrltable(idevagent& _agent, priority _priority, int command_, const record* const _records, size_t _count);
			bool query(void);
			bool query(frontend::contrltable::ivar::delegat & _delegat);
			bool ready(void);
		private:
			ivar::queue queue_;
			ivar* current_ = nullptr;
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


		}
#endif
	}
}
#endif

