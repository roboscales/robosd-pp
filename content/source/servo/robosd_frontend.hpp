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
		class ROBO_EXPORT  performer {
		public:
			enum class priority { lo = -1, normal = 0, hi = 1 };
			typedef list::sorted <performer, priority> list;
			typedef list::ref ref;
			friend class signal;
		protected:
			ref ref_;
			bool once_;
		public:
			inline bool once(void) { return once_; }
			inline void set_once(bool _once) { once_ = _once; }
			inline bool cancel(void) {
				if (ref_.attached()) {
					ref_.dettach();
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
			void dettach(void);
			//virtual bool temporary(void) { return false; }
		};

	protected:
		performer::list performers;
	public:
		inline bool used(void) { return performers.count() > 0; }

		signal(void);

		typedef delegat::lambda< performer, void >  lambda;
		typedef delegat::uni< performer, void>  uni;
		typedef delegat::simple< performer, void>  simple;
		template <class C> class ROBO_EXPORT member
			: public  delegat::rmember < performer, C, void > {
		public:
			member(C & _instance, void (C::* _member) (void))
				: delegat::rmember < performer, C, void >(_instance, _member){}
		};

		class temporary {
		public:
			static void post(bool _isfrontend, ::robo::signal::performer *);
			template< typename T> class performer : public T{
				enum class status {
					run, disposal
				} status_ = status::run;
				bool isfrontend_;
				virtual ~performer() {}
			protected:
				virtual  void  operator ()(void) {
					if (status_ == status::run) {
						this->T::operator()();
						status_ = status::disposal;
						post(isfrontend_, this);
					}
					else {
						delete this;
					}

				}
			public:
				template<typename ... Args > performer(Args... args):  T(args...) {
					isfrontend_ = system::env::is_frontend();
				}
				void raise(void) {
					post(false, this);
				}
			};

			typedef performer< ::robo::signal::lambda > lambda;
			typedef performer< ::robo::signal::uni >  uni;
			typedef performer < ::robo::signal::simple >  simple;
			template <class C> class ROBO_EXPORT member
				: public  ::robo::signal::member <C>  {
			public:
				member(C & _instance, void (C::* _member) (void))
					: ::robo::signal::member <C>(_instance, _member) {}
			};
		};
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
			static void post(signal::performer* _performer, signal::performer::priority _priority) { instance_().post_(_performer, _priority); }
			static void poll(void) { instance_().poll_(); }
		};

		class ROBO_EXPORT timer {
			signal::performer& frontend_performer_;
			signal::member<timer> start_delegat_;
			signal::member<timer> stop_delegat_;
			signal::member<timer> execute_delegat_;
			time_us_t period_ = 0;
			signal::performer* backend_performer_;
			void start_(void);
			void stop_(void);
			void execute_(void);
		public:
			void start(time_us_t _period);
			void stop(void);
			timer(signal::performer& _frontend_performer, bool _once = false, signal::performer* _backend_performer = nullptr)
				: frontend_performer_(_frontend_performer)
				, start_delegat_(*this, &timer::start_)
				, stop_delegat_(*this, &timer::stop_)
				, execute_delegat_(*this, &timer::execute_)
				, backend_performer_(_backend_performer) {
				execute_delegat_.set_once(_once);
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
				, timer_(*this) {}
			void start(void) { timer_.start(period_); }
			void start(time_us_t _period) { period_ = _period;  start(); }
			void stop(void) { timer_.stop(); }
		};


		class ROBO_EXPORT pulse {
			signal::performer* frontend_performer_;
			signal::member<pulse> start_delegat_;
			signal::member<pulse> stop_delegat_;
			signal::member<pulse> execute_delegat_;
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
			signal::member<command> execute_delegat_;
			signal::member<command> configure_delegat_;
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
				signal::member<action> run_;
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
		class vartable :public app::node {
		public:

			struct record {
				cstr name;
				cstr type;
				uint16_t address;
				uint16_t length;
			};


			class ivar {
				const record& instance_;
				vartable& vartable_;
			public:
				
				/////////////////////////////////////////////////////
				//typedef delegat::base<void, ivar *, bool>  delegat;

				class ROBO_EXPORT  performer : public ::robo::signal::performer {
				public:
					typedef ::robo::delegat::base<void, ivar*, bool>  delegat;
				private:
					delegat& delegat_;
					ivar* var_ = nullptr;
					bool result_ =false;
					bool isfrontend_;
					void post_();
				public:
					virtual  void  operator ()(void) {
						delegat_(var_, result_);
					};
				protected:
					performer(delegat& _delegat) : delegat_(_delegat), isfrontend_(::robo::system::env::is_frontend()) {}
					public:
					void confirm(ivar * _var) {
						var_ = _var;
						result_ = true;
						post_();
						/*if (isfrontend_) {
							::robo::frontend::queue::post(this, priority::lo);
						}
						else {
							::robo::backend::queue::post(this, priority::lo);
						}*/
					}
					void refuse(ivar* _var) {
						var_ = _var;
						result_ = false;
						post_();
					}
				public:
					static performer* create(performer* _performer) {
						return _performer;
					}
					static performer* create(const ::robo::lambda<void(ivar*, bool)>& _lambda) {
						return new temporary::lambda(_lambda);
					}
					enum class support { simple};
					static performer* create(void(*_simple)(ivar*, bool), support /*_support*/ ) {
						return new temporary::simple(_simple);
					}
					static performer* create(void* _instance, void(*_uni)(void*, ivar*, bool)) {
						return new temporary::uni(_instance, _uni);
					}
					template <typename C> static performer* create(C& _instance, void (C::* _member) (ivar*, bool)) {
						return new temporary::member<C>(_instance, _member);
					}

				};
				class lambda : public performer {
					::robo::delegat::slambda< void, ivar*, bool> delegat;
				public:
					lambda(const ::robo::lambda<void(ivar*, bool)>& _lambda)
						: performer(delegat)
						, delegat(_lambda) {}
				};

				class simple : public performer {
					::robo::delegat::ssimple< void, ivar*, bool> delegat;
				public:
					simple( void( * _simple)(ivar*, bool))
						: performer(delegat)
						, delegat(_simple) {}
				};

				class uni : public performer {
					::robo::delegat::suni< void, ivar*, bool> delegat;
				public:
					uni(void * _instance, void(*_uni)(void* , ivar*, bool))
						: performer(delegat)
						, delegat(_instance, _uni) {}
				};

				template <class C> class ROBO_EXPORT member
					: public performer {
					::robo::delegat::srmember< C, void, ivar*, bool> delegat;
				public:
					member(C & _instance, void (C::* _member) (ivar*, bool))
						: delegat::smember < void, ivar*, bool >(_instance, _member) {}
				};

				class temporary {
				public:

					typedef ::robo::signal::temporary::performer< ::robo::frontend::vartable::ivar::lambda > lambda;
					typedef ::robo::signal::temporary::performer< ::robo::frontend::vartable::ivar::uni > uni;
					typedef ::robo::signal::temporary::performer< ::robo::frontend::vartable::ivar::simple > simple;

					template <class C> class ROBO_EXPORT member
						: public   ::robo::frontend::vartable::ivar::member <C> {
					public:
						member(C & _instance, void (C::* _member) (ivar*, bool))
							: ::robo::frontend::vartable::ivar::member <C>(_instance, _member) {}
					};
				};

				/////////////////////////////////////////////////////


				enum class status { disable, clean, ready, put, get, panic };
				enum class hook { free, frontend, backend };
			private:
				status status_ = status::disable;
				hook hook_ = hook::free;
				performer * performer_ = nullptr;
				int repeat_count_ = 0;
				int repeat_current_max_ = 30000000;
			public:
				enum { invalid_value = -1 };
				uint16_t addr(void) const { return  instance_.address; };
				uint16_t length(void) const { return  instance_.length; };
				cstr name(void) const { return  instance_.name; };
				cstr type(void) const { return  instance_.type; };

				bool query(performer *  _performer = nullptr);
				template <typename ... Args> bool query(Args...arg) {
					return query(performer::create(arg...));
				}


				bool is_ready(void) { return  (status_ == status::ready) || (status_ == status::panic) || (status_ == status::clean); }
				bool is_success(void) { return  (status_ == status::ready); }
				bool is_busy(void) { return !is_ready(); }


				typedef ::robo::list::unique<ivar, int> map;
				typedef map::ref map_ref;
				status actual_status(void) const  { return status_; }
			protected:

				void reset_delegat(void);
				vartable& vt(void) { return vartable_; }
				void confirm(void);
				void refuse(void);
				ivar(vartable& _vartable, const record& _instance);
				virtual ~ivar(void) {}
				virtual bool rerquest(void) = 0;
				bool post(performer* _performer = nullptr);


				template <typename ... Args> bool post(Args...arg) {
					return post( create(arg...) );
				}


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


			template< class B, typename T> class var_t : public  B {
			protected:
				struct ifront {
					T& local;
					T& remote;
					ifront(T& _local, T& _remote) : local(_local), remote(_remote) {}
				} front;
			private:
				typedef typename B::performer  performer;
			public:

				template <typename ... Args> bool post(Args...arg) {
					ROBO_LRET(B::post(create(arg...)));
				}

				template <typename ... Args> bool post(const T& _value, Args...arg) {
					front.local = _value;
					ROBO_LRET(B::post(create(arg...)));
				}

				template <typename ... Args> bool try_post(Args...arg) {
					return B::post(create(arg...));
				}

				template <typename ... Args>  result try_post(const T& _value, Args...arg) {
					front.local = _value;
					if (front.remote != _value) {
						ROBO_RET(B::post(create(arg...)), result::resume, result::panic);
					}
					else {
						return result::complete;
					}
				}


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


			class fabric {
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
			ivar* find_var(cstr _name);
		};


		#endif

		class servo : public robo::app::node {
		public:
			servo(robo::cstr _name, robo::app::module& _module)
				: robo::app::node(_name, &_module) {}
		};

	}

	class quest :  protected signal::performer {
	public:
		enum class result { refuse, success, cancel, broke };
		enum class reaction { normal, terminate };
		typedef ::robo::delegat::base<void, quest *> request;
		typedef ::robo::delegat::base<reaction, result> answer;
	private:
		typedef robo::list::unsorted<quest> list;
		typedef list::ref ref;
		int use_ = 0;
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
		ref top_ref_;
		list childs_;
		quest* owner_;
		request* request_;
		answer* answer_;
		bool isfrontend_;
		enum class status { none, run, confirm, refuse, discarde } status_ = status::none;
		void post_answer_(status _status);
		quest(
			quest* _owner
			, request* _request
			, answer* _answer
		);
	protected:
		virtual  void  operator ()(void);
	public:

		static quest* create(
			quest* _owner
			, ::robo::lambda< void(quest *) > _request
			, ::robo::lambda< reaction(result) > _confirm
		) {
			if (_owner != nullptr) {
				ROBO_BREAKN(_owner->status_ == status::none, nullptr);
				ROBO_BREAKN(_owner->isfrontend_ == robo::system::env::is_frontend(), nullptr);
			}
			return new quest (
				_owner
				, new robo::delegat::slambda<void, quest*>(_request)
				, new robo::delegat::slambda<reaction,result>(_confirm)
			);
		}

		static quest* simple_create(
			quest* _owner
			, void(*_request )(quest*)
			, reaction(*_confirm)(result)
		) {
			if (_owner != nullptr) {
				ROBO_BREAKN(_owner->status_ == status::none, nullptr);
				ROBO_BREAKN(_owner->isfrontend_ == robo::system::env::is_frontend(),nullptr);
			}
			return new quest(
				_owner
				, new robo::delegat::ssimple<void, quest *>(_request)
				, new robo::delegat::ssimple<reaction, result>(_confirm)
			);
		}

		static quest* create(
			quest* _owner
			, void* _instance
			, void(*_request)(void *, quest*)
			, reaction(*_confirm)(void* ,result)
		) {
			if (_owner != nullptr) {
				ROBO_BREAKN(_owner->status_ == status::none, nullptr);
				ROBO_BREAKN(_owner->isfrontend_ == robo::system::env::is_frontend(),nullptr);
			}
			return new quest(
				_owner
				, new robo::delegat::suni<void, quest*>(_instance,_request )
				, new robo::delegat::suni<reaction, result>(_instance, _confirm)
			);
		}

		template <typename C> static  quest* create(
			quest* _owner
			, C * _instance
			, result(C::*_request)(quest*)
			, reaction(C::*_confirm)(result)
		) {
			if (_owner != nullptr) {
				ROBO_BREAKN(_owner->status_ == status::none, nullptr);
				ROBO_BREAKN(_owner->isfrontend_ == robo::system::env::is_frontend(), nullptr);
			}
			return new quest(
				_owner
				, new robo::delegat::smember<C,result, quest*>(_instance, _request)
				, new robo::delegat::smember<C, reaction, result>(_instance, _confirm)
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
 				delete answer_;
			}
			if (request_) delete request_;
			counter::instance_().dec();
		}
		static void post(void);
		void confirm(void);
		void refuse(void);
		void terminate(void);
	};
}
#endif
