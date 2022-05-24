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
			bool attach_to(signal* _signal, priority _priority);
			void dettach(void);
			virtual bool temporary(void) { return false; }
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
			: public  delegat::member < performer, C, void > {
		public:
			member(C* _instance, void (C::* _member) (void))
				: delegat::member < performer, C, void >(_instance, _member) {}
		};

		class temporary : public performer {
			enum class status {
				run, disposal
			} status_ = status::run;
			bool isfrontend_;
			::robo::lambda< void(void) > lambda_;
		protected:
			virtual  void  operator ()(void);
		public:
			temporary(const  ::robo::lambda< void(void) >& _lambda) : lambda_(_lambda) {
				isfrontend_ = system::env::is_frontend();
			}
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
				, start_delegat_(this, &timer::start_)
				, stop_delegat_(this, &timer::stop_)
				, execute_delegat_(this, &timer::execute_)
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
				, start_delegat_(this, &pulse::start_)
				, stop_delegat_(this, &pulse::stop_)
				, execute_delegat_(this, &pulse::execute_)
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
				, execute_delegat_(this, &command::execute_)
				, configure_delegat_(this, &command::configure_)
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
					, run_(this, &action::execute_) {}

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
			template<typename T> static void exchange(T& _obj, signal::performer* _on_apply_action = nullptr) {
				void* ptr = (void*)&_obj;
				core_().exchange_(ptr, (void*)((uint8_t*)ptr + sizeof(T) / sizeof(uint8_t)), _on_apply_action);
			}
			template<typename T> static void update_feedback(T& _obj, signal::performer* _on_update_feedback_ = nullptr) {
				void* ptr = (void*)&_obj;
				core_().update_feedback_(ptr, (void*)((uint8_t*)ptr + sizeof(T) / sizeof(uint8_t)), _on_update_feedback_);
			}
			virtual ~shared(void) {}
		};


		/*template<class D> class devagent_t : public D {
		public:

			typedef typename D::action_s A;			
			typedef typename D::feedback_s F;

			A & action;
			const A & goal;
			const F & feedback;

			devagent_t(
				D * _content
			) : action(_content->action), goal(_content->goal), feedback(_content->feedback) {}

			devagent_t(
				D & _content
			) : action(_content.action), goal(_content.goal), feedback(_content.feedback) {}
			
			devagent_t(
				A& _action
				, const A& _goal
				, const F& _feedback
			) : action(_action), goal(_goal), feedback(_feedback) {}
		};*/


		const struct {
			const cstr u8 = RT("u8");
			const cstr u16 = RT("u16");
			const cstr u32 = RT("u32");
		} type_names;


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
				typedef delegat::base<void, ivar&, bool>  delegat;
				enum class status { disable, clean, ready, put, get, panic };
				enum class hook { free, frontend, backend };
			private:
				status status_ = status::disable;
				hook hook_ = hook::free;
				delegat* static_delegat_ = nullptr;
				delegat* dynamic_delegat_ = nullptr;
				int repeat_count_ = 0;
				int repeat_current_max_ = 30000000;
			public:
				enum { invalid_value = -1 };
				uint16_t addr(void) const { return  instance_.address; };
				uint16_t length(void) const { return  instance_.length; };
				cstr name(void) const { return  instance_.name; };
				cstr type(void) const { return  instance_.type; };

				bool query(void);
				bool query(delegat& _delegat);
				bool query(robo::lambda<void(ivar&, bool)>& _lambda);

				bool is_ready(void) { return  (status_ == status::ready) || (status_ == status::panic) || (status_ == status::clean); }
				bool is_success(void) { return  (status_ == status::ready); }
				bool is_busy(void) { return !is_ready(); }


				typedef ::robo::list::unique<ivar, int> map;
				typedef map::ref map_ref;
				status actual_status(void) const  { return status_; }
			protected:
				void reset_delegat(void);
				vartable& owner(void) { return vartable_; }
				void confirm(void);
				void refuse(void);
				ivar(vartable& _vartable, const record& _instance);
				virtual ~ivar(void) {}
				virtual bool rerquest(void) = 0;
				bool post(void);
				bool post(delegat& _delegat);
				bool post(robo::lambda<void(ivar&, bool)>& _lambda);
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
			public:

				typedef typename B::delegat  delegat;

				bool post(void) {
					ROBO_LRET(B::post());
				}
				bool post(delegat& _delegat) {
					ROBO_LRET(B::post(_delegat));
				}

				bool post(const T& _value) {
					front.local = _value;
					ROBO_LRET(B::post());
				}

				bool post(const T& _value, delegat& _delegat) {
					front.local = _value;
					ROBO_LRET(B::post(_delegat));
				}

				result try_post(const T& _value) {
					front.local = _value;
					if (front.remote != _value) {
						ROBO_RET(B::post(), result::resume, result::panic);
					}
					else {
						return result::complete;
					}
				}

				result try_post(const T& _value, delegat& _delegat) {
					front.local = _value;
					if (front.remote != _value) {
						ROBO_RET(B::post(_delegat), result::resume, result::panic);
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
}
#endif
