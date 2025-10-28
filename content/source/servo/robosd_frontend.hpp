/*!
 *  @file robosd_frontend.hpp
 *  @author anyus
 *  @date 2021-09-04
 *  @project robosd++
 *
 *  Declares the robosd frontend.
 */
#ifndef robosd_frontend_hpp
#define robosd_frontend_hpp

#include "core/robosd_common.hpp"
#include "servo/robosd_proto.hpp"

#include "core/robosd_list.hpp"
#include "core/robosd_delegat.hpp"
#include "core/robosd_log.hpp"
#include "core/robosd_delegat.hpp"
#include "core/robosd_autonum.hpp"
#include "core/robosd_app.hpp"
#include "core/robosd_list.hpp"

#include "core/robosd_tree.hpp"
#include "terminal/robosd_termo.hpp"
#include "core/robosd_stateflow.hpp"
#include "net/robosd_net_trafic.hpp"
namespace robo {
	class ROBO_EXPORT dev_id_t {
	public:
		union {
			struct {
				uint8_t address : 8;
				uint8_t bus : 4;
				uint8_t dev : 4;
				uint8_t module : 8;
				uint8_t servo : 7;
			};
			uint32_t value;
		};
		const uint32_t undefined = (uint32_t)(-1);
		dev_id_t(void);
		dev_id_t(uint32_t _id);
		dev_id_t(const dev_id_t& _src);
		dev_id_t(
			uint8_t _servo,
			uint8_t _modul,
			uint8_t _dev,
			uint8_t _bus,
			uint8_t _adress);
		dev_id_t& operator = (const dev_id_t& _src);
		dev_id_t& operator = (const int& _src);
		operator int() const { return (int)value; }
		enum { ALL_INDEX = -1 };
	};

	class ROBO_EXPORT signal {
	public:
		class ROBO_EXPORT  performer  {
		public:
			enum class priority { lo = -1, normal = 0, hi = 1 };
			typedef ::robo::list::sorted <performer, priority> list;
			typedef list::ref ref;
			friend class signal;
		protected:
			ref ref_;
			bool once_;
		public:
			virtual void attach(void) {};
			virtual void dettach(void);
			inline bool once(void) { return once_; }
			inline void set_once(bool _once) { once_ = _once; }
			inline bool cancel(void) {
				if (ref_.attached()) {
					dettach();
					return false;
				}
				else {
					return true;
				}
			}
			virtual  void  operator ()(void) = 0;
			performer(bool _once = false);
			virtual ~performer(void);
			bool attach_to(signal* _signal, priority _priority = priority::lo);
			
			//virtual bool temporary(void) { return false; }
		};

	protected:
		performer::list performers;
	public:
		inline bool used(void) { return performers.count() > 0; }

		signal(void);

		using owned = ::robo::delegat::owned::fabric < performer, void>;
		using autonum = ::robo::delegat::autonum::fabric<performer, void>;
	};

	class ROBO_EXPORT event :public signal {
	public:
		void raise();
	};
	class ROBO_EXPORT quest : protected signal::performer {
	public:
		enum class result { refuse, success, cancel, broke };
		enum class reaction { normal, terminate };
		using request_fabric = ::robo::delegat::autonum_fabric<void, quest*>;
		using answer_fabric = ::robo::delegat::autonum_fabric< reaction, result >;

		using request = request_fabric::ref;
		using answer = answer_fabric::ref;
	private:
		typedef robo::list::unsorted<quest> list;
		typedef list::ref ref;
		int use_ = 0;
		void  happyend_(void);
		void  owned_refuse_(void);
		void  owned_confirm_(void);

		void release(void) {
			ref* r = childs_.last();
			while (r) {
				quest* tmp = &(r->owner());
				r = r->prev();
				tmp->release();
			}

			if (use_ > 0) {
				use_--;
			}
			else {
				delete this;
			}
		}

		class backend_core {
			friend class quest;
			list top_;
			int counter_ = 0;
			/*void inc(void) {
				robo_infolog("\t\tbackend quest ++ (%d)", ++counter_);
			}
			void dec(void) {
				robo_infolog("\t\tbackend quest -- (%d)", --counter_);
			}*/
			void request(void);
			static backend_core& instance_(void);
		};


		class counter {
			friend class quest;
			int counter_ = 0;
			void inc(void) {
				robo_infolog("\t\t++ quest counter ++ (%d)", ++counter_);
			}
			void dec(void) {
				robo_infolog("\t\t-- quest counter -- (%d)", --counter_);
			}
			static counter& instance_(void) {
				static counter  instance__;
				return instance__;
			}
		};
		class frontend_core {
			friend class quest;
			list top_;
			void request(void);
			static frontend_core& instance_(void) {
				static frontend_core  instance__;
				return instance__;
			}
		};
		friend class backend_core;
		friend class frontend_core;

		ref ref_;
		ref sema_ref_;
		ref top_ref_;
		list childs_;
		list owned_;
		quest* owner_;
		request* request_;
		answer* answer_;
		bool isfrontend_;
		enum class status { none, run, confirm, refuse, discarde } status_ = status::none;
		void post_answer_(status _status);
		quest(
			quest* _owner
			, quest* _sema
			, request* _request
			, answer* _answer
		);
	protected:
		virtual  void  operator ()(void);

	public:
		static quest* create(
			quest* _owner
			, quest* _sema
			, answer* _answer
			, request* _request = request_fabric::create([](quest* _q) {  _q->confirm();  })
		) {
			if (_owner != nullptr) {
				ROBO_BREAKN(_owner->status_ == status::none, nullptr);
				ROBO_BREAKN(_owner->isfrontend_ == robo::system::env::is_frontend(), nullptr);
			}
			return new quest(
				_owner
				, _sema
				, _request
				, _answer
			);
		}
		static quest* create(
			quest* _owner
			, quest* _sema
			, lambda<reaction(result)> _answer
			, lambda<void(quest*)> _request = [](quest* _q) {  _q->confirm();  }
		) {
			return create(
				_owner
				, _sema
				, answer_fabric::create(_answer)
				, request_fabric::create(_request)
			);
		}

		template <class C> static quest* create(
			quest* _owner
			, quest* _sema
			, C& _instance
			, reaction(C::* _answer) (result)
			, void(C::* _request)(quest*)
		) {
			return create(
				_owner
				, _sema
				, answer_fabric::create(_instance, _answer)
				, request_fabric::create(_instance, _request)
			);
		}
		virtual ~quest(void) {
			ref* r = childs_.last();
			while (r) {
				quest* tmp = &(r->owner());
				r = r->prev();
				tmp->ref_.dettach();
				tmp->top_ref_.dettach();
				tmp->owner_ = nullptr;
			}

			while ((r = owned_.last()) != nullptr) {
				quest* tmp = &(r->owner());
				if (tmp->status_ == status::confirm) {
					tmp->happyend_();
				}
				else {
					tmp->terminate();
				}
			}

			if (answer_) {
				switch (status_) {
				case status::confirm:
					(*answer_)(result::success);
					break;
				case status::refuse:
					(*answer_)(result::refuse);
					break;
				case status::run:
					(*answer_)(result::broke);
					break;
				case status::none:
					(*answer_)(result::cancel);
					break;
				case status::discarde:
					break;
					//	(*answer_)(result::cancel);
						//} else if()
				}
				answer_->dettach();
			}
			if (request_) {
				request_->dettach();
			}
			counter::instance_().dec();
		}
		static void post(void);
		void confirm(void);
		void refuse(void);
		void terminate(void);
	};

	
	namespace frontend {

		class ROBO_EXPORT queue : public signal {
			void poll_(void);
			void post_(signal::performer* _performer, signal::performer::priority _priority);
			static queue& instance_();
		public:
			~queue(void);
			static void post(signal::performer* _performer, signal::performer::priority _priority) { instance_().post_(_performer, _priority); }
			static void poll(void) { instance_().poll_(); }
		};

		class ROBO_EXPORT timer {
			signal::performer * frontend_performer_;
			signal::owned::member<timer> start_delegat_;
			signal::owned::member<timer> stop_delegat_;
			signal::owned::member<timer> execute_delegat_;
			time_us_t period_ = 0;
			signal::performer* backend_performer_;
			void start_(void);
			void stop_(void);
			void execute_(void);
		public:
			void start(time_us_t _period);
			void stop(void);
			timer(signal::performer * _frontend_performer, bool _once = false, signal::performer* _backend_performer = nullptr)
				: frontend_performer_(_frontend_performer)
				, start_delegat_(*this, &timer::start_)
				, stop_delegat_(*this, &timer::stop_)
				, execute_delegat_(*this, &timer::execute_)
				, backend_performer_(_backend_performer) {
				execute_delegat_.set_once(_once);
				if (frontend_performer_) frontend_performer_->attach();
				if (backend_performer_) backend_performer_->attach();
			}
			timer()
				: frontend_performer_(nullptr)
				, start_delegat_(*this, &timer::start_)
				, stop_delegat_(*this, &timer::stop_)
				, execute_delegat_(*this, &timer::execute_)
				, backend_performer_(nullptr) {
			}
			void start(time_us_t _period, signal::performer* _frontend_performer, bool _once = false, signal::performer* _backend_performer = nullptr) {
				if (frontend_performer_) frontend_performer_->dettach();
				if (backend_performer_) backend_performer_->dettach();
				frontend_performer_ = _frontend_performer;
				backend_performer_ = _backend_performer;
				execute_delegat_.set_once(_once);
				if (frontend_performer_) frontend_performer_->attach();
				if (backend_performer_) backend_performer_->attach();
				start(_period);
			}
			virtual ~timer(void) {
				if (frontend_performer_) frontend_performer_->dettach();
				if (backend_performer_) backend_performer_->dettach();
			}
			void pend(void);
		};



		class ROBO_EXPORT repeater : public signal::performer {
			time_us_t period_;
			timer timer_;
		public:
			enum { default_period_us = 1000 };
			repeater(time_us_t _period = default_period_us)
				: performer(false)
				, period_(_period)
				, timer_(this) {}
			void start(void) { timer_.start(period_); }
			void start(time_us_t _period) { period_ = _period;  start(); }
			void stop(void) { timer_.stop(); }
		};
	

		class ROBO_EXPORT pulse {
			signal::performer* frontend_performer_;
			signal::owned::member<pulse> start_delegat_;
			signal::owned::member<pulse> stop_delegat_;
			signal::owned::member<pulse> execute_delegat_;
			time_us_t period_ = 0;
			signal::performer* backend_performer_;
			void start_(void);
			void stop_(void);
			void execute_(void);
		public:
			void start(time_us_t _period);
			void stop(void);
			pulse(signal::performer* _frontend_performer, signal::performer* _backend_performer = nullptr)
				: frontend_performer_(_frontend_performer)
				, start_delegat_(*this, &pulse::start_)
				, stop_delegat_(*this, &pulse::stop_)
				, execute_delegat_(*this, &pulse::execute_)
				, backend_performer_(_backend_performer) {
				execute_delegat_.set_once(true);
				if (backend_performer_) {
					backend_performer_->attach();
				}
			}
		};
	
#if 0
		class shared {
		public:
			typedef robo::list::unsorted<shared> list;
			typedef list::ref ref;
			friend class tuple;
		private:
			ref ref_;
			const void* action_addr_begin_;
			const void* action_addr_end_;
			const void* feedback_addr_begin_;
			const void* feedback_addr_end_;

			struct tuple {
				typedef ::robo::list::unsorted<tuple> pool;
				int counter_ = 0;
				signal::performer* on_complete_ = nullptr;
				pool::ref ref_;
				tuple(void) : ref_(*this) {}
				void use_(void) { counter_++; }
				void unuse_(void);
				void try_release_(void);
				static tuple* get(signal::performer* _on_complete);
			};

			struct  action {
				shared* owner_;
				void (shared::* member_) (void);
				signal::owned::member<action> run_;
				tuple* tuple_ = nullptr;
				void execute_(void);
				action(
					shared* _owner
					, void (shared::* _member) (void)
				);

				void attach(tuple* _tuple);
			};

			action apply_action_;
			action update_feedback_;
			action update_feedback_;
			action exchange_;

			struct  core {
				list list_;
				tuple::pool pool_;
				void apply_action_(void* _begin, void* _end, signal::performer* _on_apply_action);
				void update_feedback_(void* _begin, void* _end, signal::performer* _on_update_feedback_);
				void exchange_(void* _begin, void* _end, signal::performer* _on_update_feedback_);
				~core(void);
			};

			static core& core_(void);
			bool is_my_action_(void* _begin, void* _end);
		
			bool is_my_feedback_(void* _begin, void* _end);

		protected:
			virtual void apply_action(void) = 0;
			virtual void uppdate_feedback(void) = 0;
			void exchange(void) {
				apply_action();
				uppdate_feedback();
			}
		protected:
			shared(
				const void* _action_addr_begin
				, const void* _action_addr_end
				, const void* _feedback_addr_begin
				, const void* _feedback_addr_end
			)
				: ref_(*this)
				, action_addr_begin_(_action_addr_begin)
				, action_addr_end_(_action_addr_end)
				, feedback_addr_begin_(_feedback_addr_begin)
				, feedback_addr_end_(_feedback_addr_end)
				, apply_action_(this, &shared::apply_action)
				, update_feedback_(this, &shared::uppdate_feedback)
				, exchange_(this, &shared::exchange) {
				ref_.attach_to(core_().list_);
			}
		public:
			template<typename T> static void apply_action(T& _obj, signal::performer* _on_apply_action = nullptr) {
				void* ptr = (void*)&_obj;
				core_().apply_action_(ptr, (void*)((uint8_t*)ptr + sizeof(T) / sizeof(uint8_t)), _on_apply_action);
			}
			template<typename T> static void exchange(T& _obj, signal::performer* _on_exchange = nullptr) {
				void* ptr = (void*)&_obj;
				core_().exchange_(ptr, (void*)((uint8_t*)ptr + sizeof(T) / sizeof(uint8_t)), _on_exchange);
			}
			template<typename T> static void update_feedback(T& _obj, signal::performer* _on_update_feedback_ = nullptr) {
				void* ptr = (void*)&_obj;
				core_().update_feedback_(ptr, (void*)((uint8_t*)ptr + sizeof(T) / sizeof(uint8_t)), _on_update_feedback_);
			}
			virtual ~shared(void) {}
		};
	
#endif

		namespace varindex {
			
			class node : public ::robo::tree::item {
			public:
				node(::robo::cstr _name, node* _branch) : ::robo::tree::item(_name, _branch) {}
				node* branch(void) { return ::robo::tree::item::pbranch<node>(); }
			};

			class record : public node {
				static node*& current_();
			public:
				const void* addr = nullptr;
				uint32_t size;
				template<typename T>	record(
					const T& _var
					, ::robo::cstr _name
				) : node (_name, current_()){
					size = sizeof(T);
					addr = &_var;
				};
				static node& root();
				static void push(::robo::cstr _name);
				static void pop(void);
			};
		}

		#if ROBO_APP_MODULE_ENABLED  == 1
		class ROBO_EXPORT vartable :public app::node {
		public:

			struct ROBO_EXPORT record {
				cstr name;
				cstr type;
				uint16_t address;
				uint16_t length;
			};


			class ROBO_EXPORT ivar {
				const record& instance_;
				vartable& vartable_;
				bool paranoic_put_ = true;
			public:
				
				/////////////////////////////////////////////////////
				//typedef delegat::ref<void, ivar *, bool>  delegat;

				class ROBO_EXPORT  performer : public ::robo::signal::performer {
				private:
					bool isfrontend_;
					void post_();
				protected:
					ivar* var = nullptr;
					bool result = false;
					performer() : isfrontend_(::robo::system::env::is_frontend()) {}
					public:
					void confirm(ivar * _var) {
						var = _var;
						result = true;
						post_();
					}
					void refuse(ivar* _var) {
						var = _var;
						result = false;
						post_();
					}
				};
				class answer : public performer {
				protected:
					virtual  void  operator ()(void) {
						(*this)(var, result);
					};
					virtual  void  operator ()(ivar*, bool) = 0;
				public:
					using autonum = ::robo::delegat::autonum::fabric<answer, void, ivar*, bool>;
					using owned = ::robo::delegat::owned::fabric<answer, void, ivar*, bool>;
				};



				/////////////////////////////////////////////////////


				enum class status { disable, clean, ready, put, get, panic };
				enum class hook { free, frontend, backend };
			private:
				status status_ = status::disable;
				hook hook_ = hook::free;
				performer * performer_ = nullptr;
				int repeat_count_ = 3;
				int repeat_current_max_ = 3;
			public:
				const performer *  prf() { return performer_;}
				status st(){ return status_; }
				bool post_a(cstr _s, performer* _answer = nullptr) {
					ROBO_LBREAKN(applay(_s));
					ROBO_LBREAKN(post_a(_answer));
					return true;
				}
				template<typename T> bool post_a(const T & _s, performer* _answer = nullptr) {
					ROBO_LBREAKN(applay(_s));
					ROBO_LBREAKN(post_a(_answer));
					return true;
				}
				bool query_a(performer* _answer = nullptr);

				enum { invalid_value = -1 };
				uint16_t addr(void) const { return  instance_.address; };
				uint16_t length(void) const { return  instance_.length; };
				cstr name(void) const { return  instance_.name; };
				cstr type(void) const { return  instance_.type; };



				bool is_ready(void) { return  (status_ == status::ready) || (status_ == status::panic) || (status_ == status::clean); }
				bool is_success(void) { return  (status_ == status::ready); }
				bool is_busy(void) { return !is_ready(); }
				bool is_paranoic_put(void) { return paranoic_put_;  }
				void set_paranoic_put(bool _paranoic_put) { paranoic_put_ = _paranoic_put; }
				virtual bool put_complete(void) =0;

				typedef ::robo::list::unique<ivar, int> map;
				typedef map::ref map_ref;
				status actual_status(void) const  { return status_; }

				virtual bool applay(cstr _s) = 0;
				virtual bool applay(int8_t _n) = 0;
				virtual bool applay(uint8_t _n) = 0;
				virtual bool applay(int16_t _n) = 0;
				virtual bool applay(uint16_t _n) = 0;
				virtual bool applay(int32_t _n) = 0;
				virtual bool applay(uint32_t _n) = 0;

				virtual bool applay(int64_t _n) = 0;
				virtual bool applay(uint64_t _n) = 0;
				virtual bool applay(float _n) = 0;
				virtual bool applay(double _n) = 0;
				//template <typename T> virtual bool applay(const T& _t) { return false;  }

				template <typename ... Args> bool query(Args...arg) {
					return query_a(answer::autonum::fabric::create(arg...));
				}

				template <typename ... Args> bool post_a(cstr _s, Args...arg) {				
					ROBO_LBREAKN(post_a(_s, answer::autonum::fabric::create (arg...)));
					return true;
				}
				template <typename T, typename ... Args> bool post_a(const T & _t, Args...arg) {
					ROBO_LBREAKN(post_a(_t, answer::autonum::fabric::create(arg...)));
					return true;
				}

				void confirm_(void);
				void refuse_(void);
			protected:

				void reset_delegat(void);
				vartable& vt(void) { return vartable_; }
				void confirm(void);
				void refuse(void);
				ivar(vartable& _vartable, const record& _instance);
				virtual ~ivar(void) {}
				virtual bool rerquest(void) = 0;
				bool post_a(performer* _performer = nullptr);


				/*template <typename ... Args> bool post(Args...arg) {
					return post(answer::autonum::fabric::create(arg...) );
				}*/


				bool begin_hook(void);
				void finish_hook(void);
				hook actual_hook(void) { return hook_; }
				static ivar* create_var(cstr _path, cstr _name);
				void begin(void) { status_ = status::clean; }
				void set_repeat_count(int  _repeat_count) { repeat_count_ = _repeat_count; };


			private:
				map_ref map_ref_;
				bool query_(void);
				bool post_(void);

			};


			template< class B, typename T> class ROBO_EXPORT var_t : public  B {
			protected:
				struct ROBO_EXPORT ifront {
					T& local;
					T& remote;
					ifront(T& _local, T& _remote) : local(_local), remote(_remote) {}
				} front;
			private:
				typedef typename B::performer  performer;
			public:
				/*
				template <typename ... Args> bool post(Args...arg) {
					ROBO_LRET(B::post(B::answer::autonum::fabric::create(arg...)));
				}

				template <typename ... Args> bool post(const T& _value, Args...arg) {
					front.local = _value;
					ROBO_LRET(B::post(B::answer::autonum::fabric::create(arg...)));
				}

				template <typename ... Args> bool try_post(Args...arg) {
					return B::post(B::performer::autonum::fabric(arg...));
				}

				template <typename ... Args>  result try_post(const T& _value, Args...arg) {
					front.local = _value;
					if (front.remote != _value) {
						ROBO_RET(B::post(B::performer::autonum::fabric(arg...)), result::resume, result::panic);
					}
					else {
						return result::complete;
					}
				}
				*/

				static var_t& create_var(cstr _path, cstr _name) {
					var_t* v = dynamic_cast<var_t*>(ivar::create_var(_path, _name));
					ROBO_APP_ASSERT(v != nullptr);
					return (*v);
				}
				const T& value(void) {
					return front.remote;
				}

			protected:
				var_t(vartable& _vartable, const record& _instance, T& _local, T& _remote) : B(_vartable, _instance), front(_local, _remote) {};
			};


			class ROBO_EXPORT fabric {
			public:
				typedef ::robo::list::unique<fabric, int> map;
				typedef map::ref ref;
				static fabric::map& fabrics(void);
			private:
				ref ref_;
			public:
				fabric(cstr _type);
				static fabric* find(cstr _type);
				virtual ivar* create(vartable& _vartable, const  record& _record) = 0;
			};

			vartable(node& _owner, const record* const _records, size_t _count);

			const record& find_record_ref(cstr _name);

		protected:
			friend class ivar;
			ivar::map vars;
			const record* const records_ = nullptr;
			size_t count_ = 0;
			const record* find_record(cstr _name);
			ivar* create_var(cstr _name);
		public:
			ivar* find_var(cstr _name);

		};


		class devagent : public app::node {
			friend class servo;
			app::node* backend_ = nullptr;
		public:
			using list = robo::list::unsorted<devagent> ;
			using ref = list:: ref;

		private:
			ref ref_;
			ref index_ref_;
		public:


			typedef robo::devagent::action_s action_s;
			struct common {
				typedef robo::devagent::feedback_s feedback_s;
			};
			using commands = ::robo::devagent::commands;
			using modes = ::robo::devagent::modes;
			
			using statuses = ::robo::devagent::statuses;

			struct feedback_s {
				common::feedback_s dev;
				robo::net::trafic_s trafic;
			};

			typedef robo::devagent::action_s action_s;
			typedef robo::devagent::config_s config_s;

			template <typename F> typename F::feedback_s& feedback(void) {
				return reinterpret_cast <typename F::feedback_s&>(feedback_);
			}

			template <typename A> typename A::action_s& goal(void) {
				return reinterpret_cast <typename A::action_s&> (goal_);
			}

			template <typename A> typename A::action_s& action(void) {
				return reinterpret_cast <typename A::action_s&> (action_);
			}

		private:
			action_s& action_;
			action_s& goal_;
			feedback_s& feedback_;
			statuses::locals status_ = statuses::locals::disabled;
		
		protected:
			::robo::time_us_t  discovery_period_us = 5000000;


			void doTerminate(void) {
				robo_errlog("\n\t\t---------------------%s is termibated and exit from mode %S -------------------- \n", display_alias(), robo::devagent::statuses::locals_names[(int)status_]);
				robo::system::guard g__;
				status_ = statuses::locals::disconnected;
				ref_.attach_to(devagent::disconnected_());
			}
			friend class devnode;
			class devnode;
			devnode* actual_devnode = nullptr;
			controller devcontroller;
			
			class devnode : public controller::node {
			protected:
				devagent& dev;
				devagent::feedback_s& feedback;
				devagent::action_s& action;
				statuses::locals status;

				virtual void onEnter(void) {
					robo_warninglog("\n\t\t---------------------%s switch mode from '%S to '%S'--------------------- \n", dev.display_alias(), robo::devagent::statuses::locals_names[(int)dev.status_], robo::devagent::statuses::locals_names[(int)status]);
					dev.actual_devnode = this;
					dev.status_ = status;
				};
				virtual result doEnter(void) { return result::success; };
				virtual void onExecute(void) {};
				virtual void doExecute(void) {};
				virtual void onLeave(void) {};
				virtual result doLeave(void) { return result::success; }
				virtual void onFinish(void) { dev.actual_devnode = nullptr; };
				virtual void onTerminate(void) { dev.doTerminate(); };
				virtual void onIdle(void) {};
			public:
				devnode( devagent& _devagent, statuses::locals _status) :
					dev(_devagent)
					, status(_status)
					, feedback(_devagent.feedback<devagent>())
					, action(_devagent.action<devagent>())
				{}
				virtual ~devnode(void) {}
			};
			size_t incom_total = 0;
		public:
			statuses::locals status(void) { return status_; };
				
			void exchanhge_lost(void) {
			}
			void discovery_begin(void);
			void poll(void) {
				do_check_command();
				devcontroller.run();

			};

		protected:
			virtual void do_discovery_begin(void) {
				incom_total = feedback_.trafic.incom.success.bytes.total;
				robo::system::guard g__;
				ref_.attach_to(devagent::slow_());

			}
			virtual void do_discovery_complete(void);
			virtual bool do_discovery_check(void) {
				return feedback_.trafic.incom.success.bytes.total - incom_total > 0;
			};
			virtual void do_discovery_refuse(void) {
				devcontroller.terminate();
			}


			virtual void do_configure_start(void) {
			}
			virtual bool do_configure_started(void) {
				return feedback_.dev.status == statuses::remotes::configure;
			}
			virtual robo::quest* configure_expansion_create(robo::quest* _quest) {
				return _quest;
			}

			virtual void on_configure_execute(void) {
				//configure_quest_ = 
				//::robo::quest::create(
				configure_expansion_create(
					::robo::quest::create(
						nullptr
						, nullptr
						, [this](::robo::quest::result _r)->robo::quest::reaction {
							if (_r == robo::quest::result::success) {
								//robo_detaillog(6, robo::log::mask::disabled, "\t\tquest: 'load startup var's ' for %s/%s  - success", display_alias(), _sect);
								configute_confirm();
								return robo::quest::reaction::normal;
							}
							else {
								//robo_errlog("\t\tquest: 'load startup var's' for  %s/%s  - refused, canceled or termibated (%d) ", display_alias(), _sect, (int)_r);
								configure_refuse();
								return robo::quest::reaction::terminate;
							}
						}
					)
				);
				//, nullptr
				//, nullptr
				//);
				quest::post();
				//if (configure_quest_) {
					//configure_quest_->confirm();
				//}
			}
			virtual devnode::result do_configure_execute(void) {
				return  configure_status_;
			}
			virtual void do_configure_complete(void);
			void configute_confirm(void) {
				configure_status_ = devnode::result::success;
			}
			virtual void configure_refuse(void) {
				devcontroller.terminate();
			}

			virtual void do_stop_request(void) {
			}
			virtual bool is_stopped(void) {
				return feedback_.dev.status == statuses::remotes::ready;
			}
			virtual void do_stop_refuse(void) {
				devcontroller.terminate();
			}
			virtual void do_stop_success(void) {
				robo::system::guard g__;
				ref_.attach_to(devagent::slow_());
			}
			void confirm_command(void) {

			}
			virtual void do_check_command(void) {
				if (status_ == statuses::locals::disabled) return;
				if (feedback_.dev.status == statuses::remotes::panic ) {
					if (status_ != statuses::locals::panic  && status_ != statuses::locals::reset_panic) {
						devcontroller.switchto(&devpanic_);
						status_ = statuses::locals::panic;
						return;
					}
				}
				if (action_.command != commands::none) {
					switch (action_.command) {
					case commands::stop:
						devcontroller.switchto(&devstopped_);
						break;
					case commands::reset_panic:
						if (status_ == statuses::locals::panic) {
							devcontroller.switchto(&devreset_panic_);
						}
						break;
					case commands::sw2dirrect:
						if (status_ == statuses::locals::stopped){
							devcontroller.switchto(&devdirrect_);
						}
						else {
							robo_errlog("\n\t\t---------------------%s is't stopped (%S) and don't switch to mode 'dirrect' -------------------- \n", display_alias(), robo::devagent::statuses::locals_names[(int)status_]);
						}
						break;
					case commands::sw2independed:
						if (status_ == statuses::locals::stopped) {
							devcontroller.switchto(&devindepended_);
						}
						else {
							robo_errlog("\n\t\t---------------------%s is't stopped (%S) and don't switch to mode 'independed' -------------------- \n", display_alias(), robo::devagent::statuses::locals_names[(int)status_]);
						}
						break;
					case commands::sw2service:
						if (status_ == statuses::locals::stopped)
							devcontroller.switchto(&devservice_);
						else 
							robo_errlog("\n\t\t---------------------%s is't stopped (%S) and don't switch to mode 'service' -------------------- \n", display_alias(), robo::devagent::statuses::locals_names[(int)status_]);

						break;

					case commands::halt:
						devcontroller.switchto(&devconfigure_);
						break;
					case commands::discovery:
						devcontroller.switchto(&devdiscovery_);
						break;
					default:
						break;
						//common::devagent::commands::locals::none;
					}
					action_.command = commands::none;
				}
			}
		public:
			void setup_backend(app::node* _backend) { backend_ = _backend; };
		protected:
			app::node * backend(void) {
				return backend_	;
			}
		private:
			
			devnode::result configure_status_ = devnode::result::wait;
			friend class discovery_s;
			class discovery_s : public devnode {
				robo::time_us_t us;
			protected:
				virtual void onExecute(void) {
					dev.do_discovery_begin();
					us = robo::system::time_us();
					robo_infolog("\n\t\t---------------------%s discovery start -------------------- \n", dev.display_alias());
				};
				virtual void doExecute(void) {
					if (dev.do_discovery_check()) {
						robo_infolog("\n\t\t---------------------%s discovery compleete -------------------- \n", dev.display_alias());
						dev.do_discovery_complete();
					}
					else {
						if (::robo::system::time_us() - us > dev.discovery_period_us) {
							robo_warninglog("\n\t\t---------------------%s discovery refuse -------------------- \n", dev.display_alias());
							dev.do_discovery_refuse();
						}
					}
				}

			public:
				discovery_s(devagent& _devagent) : devnode(_devagent, statuses::locals::discovery) {}
			} devdiscovery_;
			friend class devconfigure_s;
			class devconfigure_s : public devnode {
				robo::time_us_t us;
			public:
				virtual void onEnter(void) {
					devnode::onEnter();
					dev.do_configure_start();
					robo_infolog("\n\t\t---------------------%s configure request -------------------- \n", dev.display_alias());
					us = robo::system::time_us();
				};
				virtual result doEnter(void) {
					if (dev.do_configure_started()) {
						us = ::robo::system::time_us();
						return  result::success;
					}
					else {
						if (::robo::system::time_us() - us > dev.discovery_period_us) {
							dev.configure_refuse();
							robo_warninglog("\n\t\t---------------------%s configure refuse -------------------- \n", dev.display_alias());
						}
						return result::wait;
					}
				};
				virtual void onExecute(void) {
					dev.on_configure_execute();
					dev.configure_status_ = devnode::result::wait;
					robo_infolog("\n\t\t---------------------%s configure start -------------------- \n", dev.display_alias());

				};

				virtual void doExecute(void) {
					if (dev.do_configure_execute() == result::success) {
						dev.do_configure_complete();
						robo_infolog("\n\t\t---------------------%s configure compleete -------------------- \n", dev.display_alias());
					}
				}
				devconfigure_s(devagent& _devagent) : devnode(_devagent, statuses::locals::configure) {}
			} devconfigure_;

			friend class devstopped_s;
			class devstopped_s : public devnode {
				robo::time_us_t us;
			public:
				virtual void onEnter(void) {
					devnode::onEnter();
					dev.do_stop_request();
					robo_infolog("\n\t\t---------------------%s stopped request -------------------- \n", dev.display_alias());
					us = robo::system::time_us();
				};
				virtual result doEnter(void) {
					if (dev.is_stopped()) {
						robo_infolog("\n\t\t---------------------%s is stopped  -------------------- \n", dev.display_alias());
						return  result::success;
					}
					else {
						if (::robo::system::time_us() - us > dev.discovery_period_us) {
							dev.do_stop_refuse();
							robo_warninglog("\n\t\t---------------------%s stopped refuse -------------------- \n", dev.display_alias());
							return result::success;
						}
						else {
							return result::wait;
						}
					}
				};
				virtual void onExecute(void) {
					dev.do_stop_success();
				};

				virtual void doExecute(void) {
				}
				devstopped_s(devagent& _devagent) : devnode(_devagent, statuses::locals::stopped) {}
			} devstopped_;


			class devpanic : public devnode {
			protected:
				virtual void onExecute(void) {					
					robo_warninglog ("\n\t\t---------------------%s fail to panic! -------------------- \n", dev.display_alias());
				};
			public:
				devpanic(devagent& _devagent) : devnode(_devagent, statuses::locals::panic) {}
			} devpanic_;

			class devreset_panic_s : public devnode {
				robo::time_us_t us;
			public:
				virtual void onEnter(void) {
					devnode::onEnter();
					dev.do_stop_request();
					robo_infolog("\n\t\t---------------------%s reset panic request -------------------- \n", dev.display_alias());
					us = robo::system::time_us();
				};
				virtual result doEnter(void) {
					if (dev.is_stopped()) {
						robo_infolog("\n\t\t---------------------%s switched to reset panic mode  -------------------- \n", dev.display_alias());
						return  result::success;
					}
					else {
						if (::robo::system::time_us() - us > dev.discovery_period_us) {
							dev.do_stop_refuse();
							robo_warninglog("\n\t\t---------------------%s reset panic refuse -------------------- \n", dev.display_alias());
							return result::success;
						}
						else {
							return result::wait;
						}
					}
				};
				devreset_panic_s(devagent& _devagent) : devnode(_devagent, statuses::locals::reset_panic) {}
			} devreset_panic_;

			class devdirrect : public devnode {
				robo::time_us_t us;
			public:
				virtual void onEnter(void) {
					devnode::onEnter();
					dev.do_stop_request();
					robo_infolog("\n\t\t---------------------%s dirrect request -------------------- \n", dev.display_alias());
					us = robo::system::time_us();
				};
				virtual result doEnter(void) {
					if (dev.is_stopped()) {
						robo_infolog("\n\t\t---------------------%s switched to dirrect mode  -------------------- \n", dev.display_alias());
						return  result::success;
					}
					else {
						if (::robo::system::time_us() - us > dev.discovery_period_us) {
							dev.do_stop_refuse();
							robo_warninglog("\n\t\t---------------------%s dirrect refuse -------------------- \n", dev.display_alias());
							return result::success;
						}
						else {
							return result::wait;
						}
					}
				};
			public:
				devdirrect(devagent& _devagent) : devnode(_devagent, statuses::locals::dirrect) {}
			} devdirrect_;



			class devindepended : public devnode {
				robo::time_us_t us;
			public:
				virtual void onEnter(void) {
					devnode::onEnter();
					dev.do_stop_request();
					robo_infolog("\n\t\t---------------------%s independed request -------------------- \n", dev.display_alias());
					us = robo::system::time_us();
				};
				virtual result doEnter(void) {
					if (dev.is_stopped()) {
						robo_infolog("\n\t\t---------------------%s switched to independed mode  -------------------- \n", dev.display_alias());
						return  result::success;
					}
					else {
						if (::robo::system::time_us() - us > dev.discovery_period_us) {
							dev.do_stop_refuse();
							robo_warninglog("\n\t\t---------------------%s independed refuse -------------------- \n", dev.display_alias());
							return result::success;
						}
						else {
							return result::wait;
						}
					}
				};

			public:
				devindepended(devagent& _devagent) : devnode(_devagent, statuses::locals::dirrect) {}
			} devindepended_;

			class devservice : public devnode {
				robo::time_us_t us;
			public:
				virtual void onEnter(void) {
					devnode::onEnter();
					dev.do_stop_request();
					robo_infolog("\n\t\t---------------------%s service request -------------------- \n", dev.display_alias());
					us = robo::system::time_us();
				};
				virtual result doEnter(void) {
					if (dev.is_stopped()) {
						robo_infolog("\n\t\t---------------------%s switched to service mode  -------------------- \n", dev.display_alias());
						return  result::success;
					}
					else {
						if (::robo::system::time_us() - us > dev.discovery_period_us) {
							dev.do_stop_refuse();
							robo_warninglog("\n\t\t---------------------%s service refuse -------------------- \n", dev.display_alias());
							return result::success;
						}
						else {
							return result::wait;
						}
					}
				};

			public:
				devservice(devagent& _devagent) : devnode(_devagent, statuses::locals::service) {}
			} devservice_;
		protected:

			virtual bool do_load(void);
			virtual void do_clean(void);
			virtual bool do_start(void);

		public:

			devagent(robo::cstr _name, robo::app::node & _owner, action_s& _action,  action_s& _goal, feedback_s& _feedback);

			virtual bool check_configure_complete(void) {
				switch (feedback_.dev.status) {
				case statuses::remotes::ready:
				case statuses::remotes::run:
					return true;
				default:
					return false;
				}
			}

			class command  {
			public:
				using feedback_performer = robo::delegat::ref<void, devagent::list&>;
			private:
				devagent::list* list_;
				using performer = signal::owned::member<command>;
				performer query_feedback_;
				performer perform_feedback_;
				performer apply_action_;
				void perform_feedback__(void);
				void query_feedback__(void);
				void apply_action__(void);
				timer t_;
				feedback_performer* feedback_performer_ = nullptr;
			protected:
				virtual void query_feedback(devagent&) = 0;
				virtual void perform_feedback(devagent::list& _list) {
					if (feedback_performer_) {
						(*feedback_performer_)(_list);
					}
				}
				virtual void apply_action(devagent&) = 0;
			public:
				command() :
					query_feedback_(*this,&command::query_feedback__)
					, perform_feedback_(*this, &command::perform_feedback__)
					, apply_action_(*this, &command::apply_action__)
					, t_(&perform_feedback_,false, &query_feedback_)
				{

				}
				void start(robo::time_us_t _time_us, devagent::list& _list, feedback_performer * _feedback_performer) {
					list_ = &_list;
					feedback_performer_ = _feedback_performer;
					t_.start(_time_us);
				}
				void stop(void) {
					t_.stop();
				}
			};
/*
			class unicommand : command {
			private:
				devagent::list* list_;
				performer* front_;
				performer* back_;
			protected:
				virtual void front(devagent& _dev) { if (front_)  (*front_)(_dev); };
				virtual void back(devagent& _dev) { if (back_)  (*back_)(_dev);  };
			public:
				void execute(devagent::list& _list, performer* _back, performer* _front, priority _priority);
			};*/
		private:
			void apply_action_(void);
			void query_feedback_(void);
		private:

			friend class exchange;
			class exchange : public command {
				timer t_;
			public:
				virtual void query_feedback(devagent& _dev) {
					_dev.query_feedback_();
				}
				virtual void apply_action(devagent& _dev) {
					_dev.apply_action_();
				}

			};
			static list & fast_(void);
			static list& slow_(void);
			static list& disabled_(void);
			static list& disconnected_(void);
		};


		class ROBO_EXPORT servo : public robo::app::node {
			virtual bool reconfig_command(void);
			virtual bool discovery_command(void);
			virtual bool service_command(void);
			virtual bool stop_command(void);
			virtual bool dirrect_command(void);
			virtual bool independed_command(void);
			virtual bool reset_panic_command(void);
#if ROBO_APP_TERMINAL_ENABLED
			robo::string termoserial_name_;
			robo::net::iserial* termoserial_ = nullptr;
			class root_termo_cmd : public ::robo::termo::node {
			public:
				root_termo_cmd(void);
			} root_termo_cmd_;

			class reconfig_termo_cmd_s : public ::robo::termo::node {
				servo& servo_;
			protected:
				virtual bool begin(void);
			public:
				reconfig_termo_cmd_s(servo& _servo);
			} reconfig_termo_cmd_;

			class discovery_termo_cmd_s : public ::robo::termo::node {
				servo& servo_;
			protected:
				virtual bool begin(void);
			public:
				discovery_termo_cmd_s(servo& _servo);
			} discovery_termo_cmd_;

			class service_termo_cmd_s : public ::robo::termo::node {
				servo& servo_;
			protected:
				virtual bool begin(void);
			public:
				service_termo_cmd_s(servo& _servo);
			} service_termo_cmd_;

			class stop_termo_cmd_s : public ::robo::termo::node {
				servo& servo_;
			protected:
				virtual bool begin(void);
			public:
				stop_termo_cmd_s(servo& _servo);
			} stop_termo_cmd_;

			class dirrect_termo_cmd_s : public ::robo::termo::node {
				servo& servo_;
			protected:
				virtual bool begin(void);
			public:
				dirrect_termo_cmd_s(servo& _servo);
			} dirrect_termo_cmd_;

			class independed_termo_cmd_s : public ::robo::termo::node {
				servo& servo_;
			protected:
				virtual bool begin(void);
			public:
				independed_termo_cmd_s(servo& _servo);
			} independed_termo_cmd_;

			class reset_panic_termo_cmd_s : public ::robo::termo::node {
				servo& servo_;
			protected:
				virtual bool begin(void);
			public:
				reset_panic_termo_cmd_s(servo& _servo);
			}reset_panic_termo_cmd_;
#endif
			robo::time_us_t slow_exchange_period_us_;
		public:
			servo(robo::cstr _name, robo::app::module& _module);
		protected:
			virtual bool do_load(void);
			virtual bool do_start(void);
			void on_slow_exchange__(devagent::list&);
			virtual void on_slow_exchange(devagent::list&);
			delegat::owned_fabric<void, devagent::list & > ::member<servo> on_slow_exchange_;
		private:
			devagent::exchange fast_exchange_;
			devagent::exchange slow_exchange_;

			void fast_exchange_start(robo::time_us_t _time_us, devagent::exchange::feedback_performer* _feedback_performer);
			void fast_exchange_stop(void) { fast_exchange_.stop(); };

			void slow_exchange_start(void);
			void slow_exchange_stop(void) { slow_exchange_.stop(); };
		public:
		
		};

#endif
	}
	//todo 
}
#endif
