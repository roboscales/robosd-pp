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
			virtual ~timer(void) {
				if (frontend_performer_) frontend_performer_->dettach();
				if (backend_performer_) backend_performer_->dettach();
			}
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


		class ROBO_EXPORT command {
		public:
			class ROBO_EXPORT  performer {
				friend class command;
			public:
				typedef list::unique <performer, int> map;
				typedef map::ref ref;
			private:
				ref ref_;
				static map& map_(void);
			public:
				virtual  void  operator ()(command& _command) = 0;
				performer(cstr _name) : ref_(*this, hash(_name)) {
					ROBO_ALARMN(ref_.attach_to(map_()));;
				}
				virtual ~performer(void) {};
			};

			template <class C> class ROBO_EXPORT member
				: public  delegat::member < performer, C, void > {
			public:
				member(C* _instance, void (C::* _member) (void))
					: delegat::member < performer, C, void, command&  >(_instance, _member) {}
			};

		private:
			performer* performer_ = nullptr;
			signal::performer* confirm_;
			signal::owned::member<command> execute_delegat_;
			signal::owned::member<command> configure_delegat_;
			void execute_(void);
			void configure_(void);
			int id_;
			cstr name_;
		public:
			void configure(void);
			void execute(void);
			command(cstr _name, signal::performer* _confirm = nullptr)
				: confirm_(_confirm)
				, execute_delegat_(*this, &command::execute_)
				, configure_delegat_(*this, &command::configure_)
				, id_(hash(_name))
				, name_(_name) {}
		};

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
				)
					: owner_(_owner)
					, member_(_member)
					, run_(*this, &action::execute_) {}

				void attach(tuple* _tuple);
			};

			action apply_action_;
			action update_feedback_;
			action exchange_;

			struct  core {
				list list_;
				tuple::pool pool_;
				void apply_action_(void* _begin, void* _end, signal::performer* _on_apply_action);
				void update_feedback_(void* _begin, void* _end, signal::performer* _on_update_feedback_);
				void exchange_(void* _begin, void* _end, signal::performer* _on_update_feedback_);
				~core(void) {
					tuple* tmp;
					while ((tmp = pool_.pop()) != nullptr)
						delete tmp;
				}

			};

			static core& core_(void);
			bool is_my_action_(void* _begin, void* _end) {
				return _begin <= action_addr_begin_ && action_addr_end_ <= _end;
			}
		
			bool is_my_feedback_(void* _begin, void* _end) {
				return _begin <= feedback_addr_begin_ && feedback_addr_end_ <= _end;
			}

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




		/*const struct {
			const cstr u8 = RT("u8");
			const cstr u16 = RT("u16");
			const cstr u32 = RT("u32");
		} type_names;*/


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

				virtual bool applay(cstr _s) {
					return false;
				}


				template <typename ... Args> bool query(Args...arg) {
					return query_a(answer::autonum::fabric::create(arg...));
				}

				template <typename ... Args> bool post_a(cstr _s, Args...arg) {				
					ROBO_LBREAKN(post_a(_s, answer::autonum::fabric::create (arg...)));
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


		#endif

		class ROBO_EXPORT servo : public robo::app::node {
		public:
			servo(robo::cstr _name, robo::app::module& _module)
				: robo::app::node(_name, &_module) {}
		};

	}
	//todo 
	class ROBO_EXPORT quest :  protected signal::performer {
	public:
		enum class result { refuse, success, cancel, broke };
		enum class reaction { normal, terminate};
		using request_fabric = ::robo::delegat::autonum_fabric<void, quest*>;
		using answer_fabric = ::robo::delegat::autonum_fabric< reaction, result >;

		using request = request_fabric::ref ;
		using answer = answer_fabric::ref ;
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
			int counter_=0;
			void inc(void) {
				robo_infolog("\t\tbackend quest ++ (%d)", ++counter_);
			}
			void dec(void) {
				robo_infolog("\t\tbackend quest -- (%d)", --counter_);
			}
			void request(void);
			static backend_core& instance_(void) {
				static backend_core  instance__;
				return instance__;
			}
		};

		class counter {
			friend class quest;
			int counter_ = 0;
			void inc(void) {
				robo_infolog("\t\t++ backend quest ++ (%d)", ++counter_);
			}
			void dec(void) {
				robo_infolog("\t\t-- backend quest -- (%d)", --counter_);
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
			, request* _request =  request_fabric::create( [](quest* _q) {  _q->confirm();  } )
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
			, C & _instance
			, reaction (C::* _answer) (result)
			, void(C::* _request)(quest *)
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

			while ( (r = owned_.last())!=nullptr ) {
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
}
#endif
