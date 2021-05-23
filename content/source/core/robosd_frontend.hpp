#ifndef robosd_frontend_hpp
#define robosd_frontend_hpp

#include "core/robosd_common.hpp"
#include "core/robosd_list.hpp"
#include "core/robosd_delegat.hpp"
#include "core/robosd_log.hpp"
#include "core/robosd_delegat.hpp"
#include "core/robosd_app.hpp"

namespace robo {

	struct ROBO_EXPORT dev_id_t {
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
				: delegat::member < performer, C, void >(_instance, _member) {
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
				performer(cstr _name) : ref_(*this,fast_hash(_name)) {
					ROBO_ALARMN(ref_.attach_to(map_()));;
				}
				virtual ~performer(void) {};
			};

			template <class C> class ROBO_EXPORT member
				: public  delegat::member < performer, C, void > {
			public:
				member(C* _instance, void (C::* _member) (void))
					: delegat::member < performer, C, void, command&  >(_instance, _member) {
				}
			};

		private:
			performer* performer_ = nullptr;
			signal::performer * confirm_;
			signal::member<command> execute_delegat_;
			signal::member<command> configure_delegat_;
			void execute_(void);
			void configure_(void);
			int id_;
			cstr name_;
		public:
			void configure( void );
			void execute(void);
			command(cstr _name, signal::performer* _confirm = nullptr)
				: confirm_(_confirm)
				, execute_delegat_(this, &command::execute_)
				, configure_delegat_(this, &command::configure_)
				, id_(fast_hash(_name))
				, name_(_name)
			{
			}
		};

		class idevagent {
		public:
			enum class icommand { external, service, stopped, fault, reset,none };
			enum class istate { unknown, disabled, external, independed, service, stopped, fault, configure };

/*			struct iaction {
			};
			struct ifeedback {
			};
			*/
			struct irequired {
				icommand command = icommand::none;
			};
			struct istatus {
				istate state = istate::unknown;
			};
		};
		
		template<class D> class devagent : public D {
		public:
			struct {
				typename D::irequired required;
				typename D::istatus status;
				typename D::iaction action;
				typename D::ifeedback feedback;
			} front;
		};
		
		const struct{
			const cstr u8 = RT("u8");
			const cstr u16 = RT("u16");
			const cstr u32 = RT("u32");
		} type_names;
		
		class contrltable:public app::node {
		public:

			struct record {
				cstr name;
				cstr type;
				uint16_t address;
				uint16_t length;				
			};

			class ivar {
				const record & instance_ ;
				contrltable & contrltable_;
			public:
				typedef delegat::base<void, ivar&, bool>  delegat;
				enum class status { disable, clean, ready, put, get, panic };
				enum class hook { free, frontend, backend };
			private:
				status status_ = status::disable;
				hook hook_ = hook::free;
				delegat* static_delegat_ = nullptr;
				delegat* dynamic_delegat_ = nullptr;
				int repeat_count_=0;
				int repeat_current_max_ = 30;
			public:
				enum {invalid_value = -1};
				uint16_t addr(void) { return  instance_.address; };
				uint16_t length(void) { return  instance_.length; };
				cstr name(void) { return  instance_.name; };
				cstr type(void) { return  instance_.type; };

				bool query(void);
				bool query(delegat & _delegat);
				bool query( robo::lambda<void(ivar &, bool)> & _lambda );

				bool is_ready(void) { return  (status_ == status::ready) || (status_ == status::panic) || (status_ == status::clean);  }
				bool is_success(void) { return  (status_ == status::ready) ; }
				bool is_busy(void) { return !is_ready(); }


				typedef ::robo::list::unique<ivar, int> map;
				typedef map::ref map_ref;
				status actual_status(void) { return status_; }
			protected:
				void reset_delegat(void);
				contrltable& owner(void) { return contrltable_;  }
				void confirm(void);
				void refuse(void);
				ivar(contrltable& _contrltable, const record& _instance);
				virtual ~ivar(void) {}
				virtual bool rerquest(void) = 0;
				bool post(void);
				bool post(delegat& _delegat);
				bool post(robo::lambda<void(ivar&, bool)>& _lambda);
				bool begin_hook(void);
				void finish_hook(void);
				hook actual_hook(void) { return hook_; }
				static ivar * create_var(cstr _path, cstr _name);
				void begin(void) { status_ = status::clean ; }
				void set_repeat_count( int  _repeat_count ){ repeat_count_ = _repeat_count;};
			private:
				map_ref map_ref_;
				bool query_(void);
				bool post_(void);

			};


			template< class B, typename T > class var : public  B {
			protected:
				struct {
					T local;
					T remote;
				} front;
			public:

				typedef typename B::delegat  delegat;

				bool post(void) {
					ROBO_LRET(B::post());	
				}
				bool post(delegat& _delegat) {
					ROBO_LRET(B::post(_delegat));	
				}
				
				bool post(const T & _value) {
					front.local = _value;
					ROBO_LRET(B::post());
				}

				bool post(const T& _value, delegat& _delegat) {
					front.local = _value;
					ROBO_LRET(B::post(_delegat ));
				}
				static var & create_var(cstr _path, cstr _name) {
					var * v =  dynamic_cast<var * >( ivar::create_var(_path, _name));
					ROBO_APP_ASSERT(v!=nullptr);
					return (*v);
				}
				const T & value(void) { 
					return front.remote;
				}
				
			protected:
				var( contrltable & _contrltable, const record& _instance) : B(_contrltable, _instance) {};
			};

			class fabric {
			public:
				typedef ::robo::list::unique<fabric, int> map;
				typedef map::ref ref;
				static fabric::map & fabrics(void);
			private:
				ref ref_;
			public:
				fabric(cstr _type);
				static fabric * find(cstr _type);
				virtual ivar* create(contrltable & _contrltable, const  record & _record ) = 0;
			};

			contrltable(node & _owner, const record  *  const _records, size_t _count);

			const record& find_record_ref(cstr _name);

		protected:
			friend class ivar;
			ivar::map vars;
			const record * const records_ = nullptr;
			size_t count_ = 0;
			const record * find_record( cstr _name );
			ivar * create_var(cstr _name);
			ivar* find_var(cstr _name);
		};


	}
}
#endif
