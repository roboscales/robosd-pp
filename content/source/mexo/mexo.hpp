#ifndef mexo_hpp
#define mexo_hpp
#include "core/robosd_list.hpp"
#include "core/robosd_delegat.hpp"
#include "core/robosd_string.hpp"
#include "core/robosd_system.hpp"
#include "mexo/led.hpp"
#include "mexo/dev.front.hpp"
#include "mexo/vartree.hpp"
#include <initializer_list> 

#ifndef ROBO_APP_MEXO_SLOT_COUNT
#define ROBO_APP_MEXO_SLOT_COUNT 16
#endif

#ifndef ROBO_APP_MEXO_PRIORITY_SLOT_ENABLE
#define ROBO_APP_MEXO_PRIORITY_SLOT_ENABLE 1
#endif


#ifndef ROBO_APP_MEXO_DEBUG_TP1_ENABLED
#define ROBO_APP_MEXO_DEBUG_TP1_ENABLED 0
#endif

using namespace robo;
namespace mexo {
	
	struct tp_verb {
		enum { frontend = 1, backend = 2, priority = 3, loop = 4 };
	};
	
	#if ROBO_APP_MEXO_DEBUG_TP1_ENABLED == 1
	class tp_driver {
		//реализацию принудительно делегируем в perephery проекта
	protected:
		void on(void);
		void off(void);
	};
	extern led<tp_driver>  tp;

	#else


	extern led<dummy_led>  tp;
//	typedef dummy_led  tp;

	#endif 

	class machine {
	public:
		enum { slot_count = ROBO_APP_MEXO_SLOT_COUNT };
		class slots;
		class slot {
		public:
			enum class kind { begin = slot_count, start, priority, control, backend, frontend };
			class delegat : public ::robo::delegat::base<void> {
			public:
				typedef list::unsorted<delegat> list;
				typedef list::ref ref;
				virtual ~delegat(void) {}
				void attach(slot::kind _kind, delegat * _prev);
				static void attach(ref& _ref, int _index, delegat* _prev);
				static void attach(ref& _ref, slot::kind _kind, delegat* _prev);
				void attach(int _index, delegat* _prev);

				void attach(const int* _index, int _count, delegat* _prev) {
					for (int i = 0; i < _count; ++i, ++_index) {
						attach(*_index, _prev);
					}
				}
				template <size_t N> void attach(int(&index)[N], delegat* _prev) {
					attach(index, N,  _prev);
				}
				void attach(std::initializer_list<int> _index, delegat* _prev) {
					for (const auto x : _index) {
						attach(x, _prev);
					}
				}
			};

			class lambda : public ::robo::delegat::lambda<delegat, void> {
			public:
				lambda(const ::robo::lambda< void(void)>& _lambda) : ::robo::delegat::lambda<delegat, void>(_lambda) {}
				lambda(slot::kind _kind, const ::robo::lambda< void(void)>& _lambda, delegat* _prev = nullptr) : ::robo::delegat::lambda<delegat, void>(_lambda) {
					attach(_kind, _prev);
				}
				lambda(int _index, const  ::robo::lambda< void(void)>& _lambda, delegat* _prev = nullptr) : ::robo::delegat::lambda<delegat, void>(_lambda) {
					attach(_index, _prev);
				}
				lambda(const int* _index, int _count, const  ::robo::lambda< void(void)>& _lambda, delegat* _prev = nullptr) : ::robo::delegat::lambda<delegat, void>(_lambda) {
					attach(_index, _count, _prev);
				}
				template <size_t N>  lambda(int(&_index)[N], const  ::robo::lambda< void(void)>& _lambda, delegat* _prev = nullptr) : ::robo::delegat::lambda<delegat, void>(_lambda) {
					attach(_index, _prev);
				}
				lambda(std::initializer_list<int> _index, const  ::robo::lambda< void(void)>& _lambda, delegat* _prev = nullptr) : ::robo::delegat::lambda<delegat, void>(_lambda) {
					attach(_index, _prev);
				}
			};

			class simple : public ::robo::delegat::simple<delegat, void> {
			public:
				simple(void (*_lambda)(void)) : ::robo::delegat::simple<delegat, void>(_lambda) {}
				simple(slot::kind _kind, void (*_lambda)(void), delegat* _prev = nullptr) : ::robo::delegat::simple<delegat, void>(_lambda) {
					attach(_kind, _prev);
				}
				simple(int _index, void (*_lambda)(void), delegat* _prev = nullptr) : ::robo::delegat::simple<delegat, void>(_lambda) {
					attach(_index, _prev);
				}
				simple(const int* _index, int _count, void (*_lambda)(void), delegat* _prev = nullptr) : ::robo::delegat::simple<delegat, void>(_lambda) {
					attach(_index, _count, _prev);
				}
				template <size_t N>  simple(int(&_index)[N], void (*_lambda)(void), delegat* _prev = nullptr) : ::robo::delegat::simple<delegat, void>(_lambda) {
					attach(_index, _prev);
				}
				simple(std::initializer_list<int> _index, void (*_lambda)(void), delegat* _prev = nullptr) : ::robo::delegat::simple<delegat, void>(_lambda) {
					attach(_index, _prev);
				}
			};


			template<typename C> class member : public ::robo::delegat::rmember<delegat, C, void> {
				typedef ::robo::delegat::rmember<delegat, C, void> A;
			public:
				member(C & _instance, void (C::* _member) (void)) : A(_instance, _member) {}
				member(slot::kind _kind, C& _instance, void(C::* _member)(void), delegat* _prev = nullptr): A(_instance, _member) {
					A::attach(_kind, _prev);
				}
				member(int _index, C& _instance, void (C::* _member) (void), delegat* _prev = nullptr) : A(_instance, _member) {
					A::attach(_index, _prev);
				}
				member(const int& _index, int _count, C* _instance, void (C::* _member) (void), delegat* _prev = nullptr) : A(_instance, _member) {
					A::attach(_index, _count, _prev);
				}
				template <size_t N>  member(int(&_index)[N], C& _instance, void (C::* _member) (void), delegat* _prev = nullptr) : A(_instance, _member) {
					A::attach(_index, _prev);
				}
				member(std::initializer_list<int> _index, C& _instance, void (C::* _member) (void), delegat* _prev = nullptr) : A(_instance, _member) {
					A::attach(_index, _prev);
				}
			};


		private:
			friend class machine;
			friend class slots;
			delegat::list delegats_;
			slot(void);
			~slot(void);
			void free(void);
			void execute(void);
		};
		class slots {
			friend class slot;
			friend class machine;
			slot begin;
			slot start;
			slot priority;
			slot control;
			slot backend;
			slot frontend;
			slot periodic[slot_count];
			slot dummy;
			slot& operator [] (slot::kind _kind);
			slot& operator [] (int _index);
			void free(void);
		};

	private:
		slots& slots_ref_;
		int slot_index_;
		static slots& slots_(void);
		void begin_(void);
		void start_(void);
		#if ROBO_APP_MEXO_PRIORITY_SLOT_ENABLE == 1
		void priority_loop_(void);
		#endif
		void backend_loop_(void);
		void frontend_loop_(void);
		static machine instance_;
	public:
		machine(void);
		~machine(void);
		static void begin(void) { instance_.begin_(); }
		static void start(void) { instance_.start_(); }
		#if ROBO_APP_MEXO_PRIORITY_SLOT_ENABLE == 1
		static void priority_loop(void) { instance_.priority_loop_(); }
		#endif
		static void backend_loop(void) { instance_.backend_loop_(); }
		static void frontend_loop(void) { instance_.frontend_loop_(); }
		static int slot_index(void) { return instance_.slot_index_; }
	};
	class node {
	public:
		typedef robo::list::unique<node, int> map;
		typedef map::ref map_ref;
		typedef robo::list::unsorted<node> list;
		typedef list::ref ref;
		enum class state { configure, ready, fault };
	private:
		ref ref_;
		list childs_;
		map_ref map_ref_;

		cstr name_;
		node* owner_;
		//bool auto_enabled_ = false;
		state state_ = state::configure;
		size_t path_offset_ = 0;
		node(void);

		static map& map_(void);
		#if ROBO_APP_MEXO_VAR_ENABLED == 1
		void create_vars_(void);
		void create_vars_index_(void);
		int var_count_(void);
		public:
		static void create_vars(void);
		#endif
	protected:
		list& childs(void) { return childs_; }
		virtual bool do_reconfig(void) { return true; };
		virtual void do_create_vars(void) {};
	public:
		virtual ~node(void){}
		#if ROBO_APP_MEXO_VAR_ENABLED == 1
		var::record::list vars;
		#endif
		static node& root(void);
		node* owner(void) { return owner_; };
		cstr name(void) { return name_; };
		int key(void) { return map_ref_.key(); }

		static bool begin(void);
		bool reconfig(void);

		node(cstr _name, node* _owner = nullptr);

		static node* find(int _key) {
			return map_().find(_key);
		}

		bool enabled(void) { return state_ == state::ready; };

		node* first_on_path(char_t*& _path, size_t& _len);
		node* next_on_path(char_t*& _path, size_t& _len);

	};

	class dev : public node {
		friend class mode;
	protected:
		virtual void on_idle(void) {};
	public:
		virtual  ~dev(void){}
			
		typedef front::dev::action_s action_s;
		typedef front::dev::feedback_s feedback_s;

		struct present_s {
			uint8_t mode;
			bool action_actual;
			uint8_t error;
			//template<typename T> operator T& () { return reinterpret_cast<T&>(*this)}
		};
		struct config_s {
			int tag;
		};

		template <typename T> typename T::action_s& action(void) {
			return reinterpret_cast <typename T::action_s&>(action_);
		}

		template <typename T> typename T::present_s& present(void) {
			return reinterpret_cast <typename T::present_s&>(present_);
		}

		template <typename T> typename T::config_s& config(void) {
			return reinterpret_cast <typename T::config_s&>(config_);
		}
		template <typename T> typename T::feedback_s& feedback(void) {
			return reinterpret_cast <typename T::feedback_s& >(feedback_);
		}
	private:
		enum class wait { none = 0, wait = 1, complete = 2 } wait_feedback_ = wait::none;
	protected:
		virtual void do_update_feedback(void);
	public:
		bool check_update_feedback(void) {
			if (::robo::system::env::is_backend()) {
				do_update_feedback();
				return true;
			}
			else {
				switch (wait_feedback_) {
				case wait::none:
					wait_feedback_ = wait::wait;
					return false;
				case wait::wait:
					return false;
				case wait::complete:
					wait_feedback_ = wait::none;
					return true;
				}
				return false;
			}
		}
		class mode : public node {
			friend class dev;
		public:
			typedef robo::list::unique<mode, int> map;
			typedef map::ref ref;

		private:
			ref ref_;
		protected:
			template<typename T>T& owner_cast(void) { return *((T*)node::owner()); };
			virtual void applay_action(void) = 0;
			virtual void do_start(void) = 0;
			virtual void do_stop(void) = 0;
		public:
			mode(int _index, cstr  _name, dev& _dev);
		};
		
		class idle_mode : public mode {
		protected:
			virtual void applay_action(void) {};
			virtual void do_start(void) {};
			virtual void do_stop(void) {};
			virtual	void do_execute(void) {};

		public:
			virtual	void operator()(void) {};
			idle_mode(dev& _dev) : dev::mode(front::dev::mode::idle, RT("idle"), _dev) {};
		};

		idle_mode idle;
		dev(cstr  _name, action_s& _action, feedback_s& _feedback, present_s& _present, config_s& _config);
		void switch_to(int _mode);
		void action_enable(void) { action_enabled_ = true;  }
		void action_disable(void) { action_enabled_ = false; }
		void action_update(void) { present_.action_actual = true; }

	protected:
		#if ROBO_APP_MEXO_VAR_ENABLED == 1
		virtual void do_create_vars(void);
		#endif
	private:
		
		//если false то  при переключении режима самостоятельно интерпретирует cnhernehe action
		bool action_enabled_ = true;

		friend class mode;
		mode::map modes_;
		mode* actual_mode_;
		
		::robo::delegat::member< mexo::machine::slot::delegat, dev, void> backend_;
		::mexo::machine::slot::delegat::ref  backend_ref_;
		action_s& action_;
		feedback_s& feedback_;
		present_s& present_;
		config_s& config_;
		void backend__(void);
		protected:
	};

	class task : public node, public machine::slot::delegat {
		friend class ihandler;
	public:
	private:
		bool autostart_;
	protected:
		virtual void  do_start(delegat* _prev = nullptr) = 0;
		virtual void do_stop(void) = 0;
		task(cstr  _name, bool _autostart, node* _owner = nullptr) :node(_name, _owner), autostart_(_autostart) {};
		task(cstr  _name, bool _autostart, task* _prev) :node(_name, _prev?_prev->owner():nullptr ), autostart_(_autostart) {};
		virtual bool do_reconfig(void);
	public:
		void start(void);
		void stop(void);
	};

	class prioritet_task : public task {
		machine::slot::delegat::ref ref_;
	protected:
		virtual void do_start(delegat* _prev = nullptr) {
			guard__; machine::slot::delegat::attach(ref_, machine::slot::kind::priority, _prev);
		};
		virtual void do_stop(void) { guard__; ref_.dettach(); };
		prioritet_task(cstr  _name, node* _owner) : task(_name, false, _owner), ref_(*this) {};
		prioritet_task(cstr  _name, prioritet_task* _prev) : task(_name, false, _prev), ref_(*this) {};
	public:
		prioritet_task(cstr  _name, bool _autostart, node* _owner = nullptr) : task(_name, _autostart, _owner), ref_(*this) {};
		prioritet_task(cstr  _name, bool _autostart, prioritet_task* _prev) : task(_name, _autostart, _prev), ref_(*this) {};
	};


	class control_task : public task {
		machine::slot::delegat::ref ref_;
	protected:
		virtual void do_start(delegat* _prev = nullptr) { guard__; machine::slot::delegat::attach(ref_, machine::slot::kind::control, _prev); };
		virtual void do_stop(void) { guard__; ref_.dettach(); };
		control_task(cstr  _name, node* _owner) : task(_name, false, _owner), ref_(*this) {};
		control_task(cstr  _name, prioritet_task* _prev) : task(_name, false, _prev), ref_(*this) {};
	public:
		control_task(cstr  _name, bool _autostart, node* _owner = nullptr) : task(_name, _autostart, _owner), ref_(*this) {};
		control_task(cstr  _name, bool _autostart, prioritet_task* _prev) : task(_name, _autostart, _prev), ref_(*this) {};
	};

	class backend_task : public task {
		machine::slot::delegat::ref ref_;
	protected:
		virtual void do_start(delegat* _prev = nullptr) { guard__; machine::slot::delegat::attach(ref_, machine::slot::kind::backend, _prev); };
		virtual void do_stop(void) { guard__; ref_.dettach(); };
		backend_task(cstr  _name, node* _owner) : task(_name, false, _owner), ref_(*this) {};
		backend_task(cstr  _name, prioritet_task* _prev) : task(_name, false, _prev), ref_(*this) {};
	public:
		backend_task(cstr  _name, bool _autostart, node* _owner = nullptr) : task(_name, _autostart, _owner), ref_(*this) {};
		backend_task(cstr  _name, bool _autostart, backend_task* _prev) : task(_name, _autostart, _prev), ref_(*this) {};
	};

	class frontend_task : public task {
		machine::slot::delegat::ref ref_;
	protected:
		virtual void do_start(delegat* _prev = nullptr) { guard__; machine::slot::delegat::attach(ref_, machine::slot::kind::frontend, _prev); };
		virtual void do_stop(void) { guard__; ref_.dettach(); };
		frontend_task(cstr  _name, node* _owner ) : task(_name, false, _owner), ref_(*this) {};
		frontend_task(cstr  _name, prioritet_task* _prev) : task(_name, false, _prev), ref_(*this) {};
	public:
		frontend_task(cstr  _name, bool _autostart, node* _owner = nullptr) : task(_name, _autostart, _owner), ref_(*this) {};
		frontend_task(cstr  _name, bool _autostart, frontend_task* _prev) : task(_name, _autostart, _prev), ref_(*this) {};
	};


	class periodic_task : public task {
		machine::slot::delegat::ref** refs_ = nullptr;
		size_t ref_count_ = 0;
		int* index_ = nullptr;
	protected:
		void clean(void);
		periodic_task(cstr  _name, node* _owner);
		periodic_task(cstr  _name, periodic_task* _prev);
		virtual void do_start(delegat* _prev = nullptr);
		virtual void do_stop(void);
	public:
		void setup(std::initializer_list<int> _index);
		void setup(int _ix);
		periodic_task(cstr  _name, bool _autostart, std::initializer_list<int> _index, node* _owner = nullptr);
		periodic_task(cstr  _name, bool _autostart, std::initializer_list<int> _index, periodic_task * _prev);
		virtual ~periodic_task(void);
	};

	template<int ... Nums> class periodic_task_t : public periodic_task {
	public:
		periodic_task_t(cstr  _name, node* _owner = nullptr) : periodic_task(_name, _owner) {
			periodic_task::setup({ Nums ... });
		}
		periodic_task_t(cstr  _name, periodic_task* _prev) : periodic_task(_name, _prev) {
			periodic_task::setup({ Nums ... });
		}
	};

	enum class satstate_t { none, both, low, up };

	class subsystem;
	class subsystem_handler : public node {
	public:
		typedef robo::list::unsorted<subsystem_handler> list;
		typedef list::ref ref;
	private:
		ref ref_;
		subsystem* subsystem_ = nullptr;
	public:
	protected:
		friend class subsystem;
		subsystem_handler(cstr  _name, subsystem* _subsystem);
		subsystem_handler(cstr  _name, subsystem_handler* _prev);
		virtual void operator ()(void) = 0;
	};

	//делегируем метод от произвольного объекта в подсистему
	template < typename C > class member_block_t : public subsystem_handler {
		C& instance_;
		void (C::* member_) (void);
	public:
		member_block_t(
			cstr  _name
			, subsystem* _subsystem
			, C& _instance
			, void (C::* _member) (void)
		) 
			:subsystem_handler(_name, _subsystem)
			, instance_(_instance)
			, member_(_member) 
		{
		};
		member_block_t(
			cstr  _name
			, subsystem_handler* _prev
			, C& _instance
			, void (C::* _member) (void)
		) :subsystem_handler(_name, _prev) 
		, instance_(_instance)
		, member_(_member) 
		{
		};
		virtual void operator ()(void) {
			(instance_.*member_)();
		}
	};

	//делегируем статический метод в подсистему
	template < typename C > class simple_block_t : public subsystem_handler {
		void (*delegat_) (void);
	public:
		simple_block_t(
			cstr  _name
			, subsystem* _subsystem
			, void (* _delegat) (void)
		)
			:subsystem_handler(_name, _subsystem)
			, delegat_(_delegat) {};

		simple_block_t(
			cstr  _name
			, subsystem_handler* _prev
			, void (*_delegat) (void)
		) :subsystem_handler(_name, _prev)
			, delegat_(_delegat) {};

		virtual void operator ()(void) {
			delegat_();
		}
	};
	//делегируем лямбду в подсистему
	template < typename C > class lambda_block_t : public subsystem_handler {
		::robo::lambda< void(void) > lambda_;
	public:
		lambda_block_t(
			cstr  _name
			, subsystem* _subsystem
			, const ::robo::lambda< void (void) >& _lambda
		)
			:subsystem_handler(_name, _subsystem)
			, lambda_(_lambda) {};

		lambda_block_t(
			cstr  _name
			, subsystem_handler* _prev
			, const ::robo::lambda< void(void) >& _lambda
		) 
			:subsystem_handler(_name, _prev)
			, lambda_(_lambda) {};

		virtual void operator ()(void) {
			 lambda_();
		}
	};


	class subsystem {
	protected:
		friend class subsystem_handler;
		virtual node* owned_node(void) = 0;
		subsystem_handler::list handlers;
		void run(void) {
			for (subsystem_handler::ref* r = handlers.first(); r; r = r->next()) {
				(r->owner())();
			}
		}
	};

	class prioritet_subsystem : public prioritet_task, public subsystem {
	protected:
		virtual node* owned_node(void) { return this; };
		prioritet_subsystem(cstr  _name, node* _owner) : prioritet_task(_name, false, _owner) {};
		prioritet_subsystem(cstr  _name, prioritet_subsystem* _prev) : prioritet_task(_name, false, _prev) {};
	public:
		virtual void operator ()(void) { subsystem::run(); };
		prioritet_subsystem(cstr  _name, bool _autostart, node* _owner = nullptr) : prioritet_task(_name, _autostart, _owner) {};
		prioritet_subsystem(cstr  _name, bool _autostart, prioritet_subsystem* _prev) : prioritet_task(_name, _autostart, _prev) {};
	};

	class backend_subsystem : public backend_task, public subsystem {
	protected:
		virtual node* owned_node(void) { return this; };
		backend_subsystem(cstr  _name, node* _owner) : backend_task(_name, false, _owner) {};
		backend_subsystem(cstr  _name, backend_subsystem* _prev) : backend_task(_name, false, _prev) {};
	public:
		virtual void operator ()(void) { subsystem::run(); };
		backend_subsystem(cstr  _name, bool _autostart, node* _owner = nullptr) : backend_task(_name, _autostart, _owner) {};
		backend_subsystem(cstr  _name, bool _autostart, backend_subsystem* _prev) : backend_task(_name, _autostart, _prev) {};
	};

	class control_subsystem : public control_task, public subsystem {
	protected:
		virtual node* owned_node(void) { return this; };
		control_subsystem(cstr  _name, node* _owner) : control_task(_name, false, _owner) {};
		control_subsystem(cstr  _name, control_subsystem* _prev) : control_task(_name, false, _prev) {};
	public:
		virtual void operator ()(void) { subsystem::run(); };
		control_subsystem(cstr  _name, bool _autostart, node* _owner = nullptr) : control_task(_name, _autostart, _owner) {};
		control_subsystem(cstr  _name, bool _autostart, control_subsystem* _prev) : control_task(_name, _autostart, _prev) {};
	};

	class frontend_subsystem : public frontend_task, public subsystem {
	protected:
		virtual node* owned_node(void) { return this; };
		frontend_subsystem(cstr  _name, node* _owner) : frontend_task(_name, false, _owner) {};
		frontend_subsystem(cstr  _name, frontend_subsystem* _prev) : frontend_task(_name, false, _prev) {};
	public:
		virtual void operator ()(void) { subsystem::run(); };
		frontend_subsystem(cstr  _name, bool _autostart, node* _owner = nullptr) : frontend_task(_name, _autostart, _owner) {};
		frontend_subsystem(cstr  _name, bool _autostart, frontend_subsystem* _prev) : frontend_task(_name, _autostart, _prev) {};
	};

	class periodic_subsystem : public periodic_task, public subsystem {
	protected:
		virtual node* owned_node(void) { return this; };
		periodic_subsystem(cstr  _name, node* _owner = nullptr) : periodic_task(_name, _owner) {};
		periodic_subsystem(cstr  _name, periodic_subsystem * _prev) : periodic_task(_name, _prev) {};
	public:
		virtual void operator ()(void) { subsystem::run(); };

		periodic_subsystem(cstr  _name, bool _autostart, std::initializer_list<int> _index, node* _owner = nullptr) : periodic_task(_name, _autostart, _index, _owner) {};
		periodic_subsystem(cstr  _name, bool _autostart, std::initializer_list<int> _index, periodic_subsystem* _prev) : periodic_task(_name, _autostart, _index, _prev) {};
	};

	template <typename A> struct range_s {
		A low;
		A hi;
	};

	template< typename T, typename R, typename S, typename P, typename ... Args > class handler_t : public T, public  R {
	public:
		typedef typename R::config_s config_s;
		typedef typename R::present_s present_s;
		virtual void operator ()(void) {
			R::execute();
		}
	protected:
		virtual bool do_reconfig(void) {
			ROBO_LBREAKN(T::do_reconfig());
			ROBO_LBREAKN(R::do_handler_reconfig());
			return true;
		}
		#if ROBO_APP_MEXO_VAR_ENABLED == 1
		virtual void	do_create_vars(void) {
			T::do_create_vars();
			R::do_handler_create_vars(T::vars, T::key());
		}
		#endif

	public:
		handler_t(cstr  _name, S* _owner, const config_s& _config, present_s& _present, Args ... args)
			: T(_name, _owner)
			, R(_config, _present, args...) {}
		handler_t(cstr  _name, P* _prev, const config_s& _config, present_s& _present, Args ... args)
			: T(_name,  _prev)
			, R(_config, _present, args...) {}
				present_s& present_ref(void) { return   T::template present_cast<present_s>(); };
		config_s& config_ref(void) { return  T::template  config_cast<config_s>(); };

	};


	class handler {
	public:
		struct config_s {
			int tag;
		};
		struct present_s {
			int tag;
		};
	private:
		const config_s& config_;
		present_s& present_;
	protected:
		virtual bool do_handler_reconfig(void) { return true; };
		virtual void do_handler_adjust(void) {};
		#if ROBO_APP_MEXO_VAR_ENABLED == 1
		virtual void do_handler_create_vars(var::record::list&/* _vars*/, int /*_master_key*/) {};
		#endif
	public:
		template <typename P>  const  typename P::config_s& config(void) { return reinterpret_cast <const  typename P::config_s&>(config_); }
		template <typename P>  typename P::present_s& present(void) { return reinterpret_cast <typename P::present_s&>(present_); }
		handler(const config_s& _config, present_s& _present) :config_(_config), present_(_present) {}
	};

	//==================================
	 
	template < typename B, typename S, typename ... Args> class handler_block_t
	: public handler_t<subsystem_handler, B, S, subsystem_handler, Args...>{
		typedef handler_t<subsystem_handler, B, S, subsystem_handler, Args...> BB;
	public:
	typedef typename BB::config_s config_s;
	typedef typename BB::present_s present_s;
	
	handler_block_t(
		cstr  _name
		, S* _owner
		, const config_s& _config
		, present_s& _present
		, Args ...args
	) : BB(_name,_owner, _config, _present, args...){}
	handler_block_t(
		cstr  _name
		, subsystem_handler* _prev
		, const config_s& _config
		, present_s& _present
		, Args ... args
	)
		: BB(_name, _prev, _config, _present, args...) {}
	};

	template <  typename B, typename S, typename ... Args> class handler_task_t
		: public handler_t<S, B, node, task, Args...> {
		typedef  handler_t<S, B, node, task, Args...> BB;
	public:
		typedef typename BB::config_s config_s;
		typedef typename BB::present_s present_s;
		handler_task_t(
			cstr  _name
			, node* _owner
			, const config_s& _config
			, present_s& _present
			, Args ... args
		)
			: BB(_name, _owner, _config, _present, args...) {}

		handler_task_t(
			cstr  _name
			, task* _prev
			, const typename BB::config_s& _config
			, typename BB::present_s& _present
			, Args ... args
		)
			: BB(_name, _prev, _config, _present, args...) {}
	};

	

	template < typename I, typename O> class controller_handler : public handler {
		I dummy_input_ = (I)0;
		O dummy_output_ = (O)0;
	protected:
		I* deseired;
		O* output;
		//connection_point<O>* connection_point_;
	public:
		const range_s<O>& range;
		const satstate_t& master_satstate;
		const  O   actual_output(void) { return  *output; }
		const satstate_t& actual_satstate(void) { return present_cast<present_s>().satstate.actual; }

		typedef I input_t;
		typedef O output_t;
		struct present_s {
			handler::present_s ref;
			struct {
				satstate_t actual;
				satstate_t local;
			}satstate;
		};
		controller_handler(
			const config_s& _config
			, present_s& _present
			, const range_s<O>& _range
			, const satstate_t& _master_satstate
		)
			: handler(_config, _present.ref)
			, range(_range)
			, master_satstate(_master_satstate)
			, deseired(&dummy_input_)
			, output(&dummy_output_) {}
		void set_output(O* _output) {
			if (_output) {
				output = _output;
			}
			else {
				output = &dummy_output_;
			}
		}
		void set_input(I* _input) {
			if (_input) {
				deseired = _input;
			}
			else {
				deseired = &dummy_input_;
			}
		}

	protected:
		void update_satstate(void) {
			present_s& prsnt = present<controller_handler>();
			satstate_t remote = master_satstate;
			if (remote == satstate_t::none) {
				prsnt.satstate.actual = prsnt.satstate.local;
			}
			else {
				prsnt.satstate.actual = remote;
			}
		}
		virtual bool do_handler_reconfig(void) {
			ROBO_LBREAKN(handler::do_handler_reconfig());
			ROBO_LBREAKN(range.low <= range.hi);
			do_handler_adjust();
			return true;
		}
		#if ROBO_APP_MEXO_VAR_ENABLED == 1
		virtual void do_handler_create_vars(var::record::list& _vars, int _master_key) {
			present_s& prsnt = present<controller_handler>();
			if (var::machine::actual_mode() == var::machine::mode::full) {
				var::record::create(var::const_uint8, prsnt.satstate.actual, RT("ss.ac"), _master_key, _vars);
				var::record::create(var::const_uint8, prsnt.satstate.local, RT("ss.loc"), _master_key, _vars);
			}
		};
		#endif
	};
	
	template < typename B, typename S, typename ... Args> class controller_block_t
		: public handler_t<subsystem_handler, B, S, subsystem_handler, const range_s<typename B::output_t>&, const satstate_t&, Args...> {
		typedef  handler_t<subsystem_handler, B, S, subsystem_handler, const range_s<typename B::output_t>&, const satstate_t&, Args...> BB;
	public:
		typedef typename BB::config_s config_s;
		typedef typename BB::present_s present_s;

		controller_block_t(
			cstr  _name
			, S* _owner
			, const config_s& _config
			, present_s& _present
			, const range_s<typename B::output_t>& _range
			, const satstate_t& _master_satstate
			, Args ... args
		)
			: BB(_name, _owner,  _config, _present, _range, _master_satstate, args...) {}

		controller_block_t(
			cstr  _name
			, subsystem_handler* _prev
			, const config_s& _config
			, present_s& _present
			, const range_s<typename B::output_t>& _range
			, const satstate_t& _master_satstate
			, Args ... args
		)
			: BB(_name, _prev, _config, _present, _range, _master_satstate, args...) {}
	};

	template <  typename B, typename S, typename ... Args> class controller_task_t
		: public handler_t<S, B, node, task, const range_s<typename B::output_t>&, const satstate_t&, Args...> {
		typedef  handler_t<S, B, node, task, const range_s<typename B::output_t>&, const  satstate_t&, Args...> BB;
	public:
		typedef typename BB::config_s config_s;
		typedef typename BB::present_s present_s;
		controller_task_t(
			cstr  _name
			, node* _owner
			, const config_s& _config
			, present_s& _present
			, const range_s<typename B::output_t>& _range
			, const satstate_t& _master_satstate
			, Args ... args
		)
			: BB(_name, _owner, _config, _present, _range, _master_satstate, args...) {}
		controller_task_t(
			cstr  _name
			, task* _prev
			, const config_s& _config
			, present_s& _present
			, const range_s<typename B::output_t>& _range
			, const satstate_t& _master_satstate
			, Args ... args
		)
			: BB(_name, _prev, _config, _present, _range, _master_satstate, args...) {}
	};

	
/*	template < typename O, typename D = O> class sence_handler : public handler {
	protected:
	public:
		struct present_s {
			handler::present_s ref;
			O output;
			D delta;
		};
		sence_handler(const config_s& _config, present_s& _present)
			: handler(_config, _present.ref) {}
		const O& output(void) { const present_s& present = present<dev_t>();  return present.output; }
		const D& delta(void) { const present_s& present = present<dev_t>();  return present.delta; }
	};

	template< typename R, typename S > class sence_block_t
		: public 	handler_t <	subsystem_handler, R, S	> {
		typedef handler_t <	subsystem_handler, R, S	> BB;
	public:
		sence_block_t(cstr _name, S* _owner, const typename BB::config_s& _config, typename  BB::present_s& _present)
			: BB(_name, _owner, _config, _present) {}
	};

	template< typename R, typename S> class sence_task_t
		: public handler_t <S, R, ::mexo::node> {
		typedef handler_t <S, R, ::mexo::node> BB;
	public:
		sence_task_t(cstr _name, ::mexo::node* _owner, const typename BB::config_s& _config, typename  BB::present_s& _present)
			: BB(_name, _owner, _config, _present) {}
	};
	*/
	template < typename I, typename O> class function_handler : public handler {
	protected:
		I& input;
	public:
		typedef I input_t;
		typedef O output_t;
		struct present_s {
			handler::present_s ref;
			output_t output;
		};
		output_t& output(void) {
			return present_cast<present_s>().output;
		}
		function_handler(
			const config_s& _config
			, present_s& _present
			, input_t& _input
		)
			: handler(_config, _present.ref)
			, input(_input) {}
	};

	template < typename R, typename S, typename ... Args> class function_block_t
		: public handler_t<subsystem_handler, R, S, subsystem_handler, typename R::input_t&, Args...> {
		typedef  handler_t<subsystem_handler, R, S, subsystem_handler, typename R::input_t&, Args...> BB;
	public:
		typedef typename BB::config_s config_s;
		typedef typename BB::present_s present_s;
		function_block_t(
			cstr  _name
			, S* _owner
			, const config_s& _config
			, present_s& _present
			, typename R::input_t& _input
			, Args ... args
		)
			: BB(_name, _owner, _config, _present, _input, args...) {}
		function_block_t(
			cstr  _name
			, subsystem_handler* _prev
			, const config_s& _config
			, present_s& _present
			, typename R::input_t& _input
			, Args ... args
		)
			: BB(_name, _prev, _config, _present, _input, args...) {}
	};
	template <  typename R, typename S, typename ... Args> class function_task_t
		: public handler_t<S, R, node, task, const typename R::input_t&, Args...> {
		typedef  handler_t<S, R, node, task, const typename R::input_t&, Args...> BB;
	public:
		typedef typename BB::config_s config_s;
		typedef typename BB::present_s present_s;
		function_task_t(
			cstr  _name
			, node* _owner
			, const config_s& _config
			, present_s& _present
			, const typename R::input_t& _input
			, Args ... args
		)
			: BB(_name, _owner, _config, _present, _input, args...) {}
		function_task_t(
			cstr  _name
			, task* _prev
			, const config_s& _config
			, present_s& _present
			, const typename R::input_t& _input
			, Args ... args
		)
			: BB(_name, _prev, _config, _present, _input, args...) {}
	};
	/*
	template < typename I> class scope_handler : public handler {
	protected:
		const I& input;
		handler::present_s present_;
	public:
		scope_handler(
			config_s& _config
			, const I& _input
		)
			: handler(_config, present_)
			, input(_input) {}
	};*/

	template < typename I> class finall_controller_handler : public handler {
		I dummy_input_;
	protected:
		I* deseired;
	public:
		const satstate_t& actual_satstate(void) { return present<finall_controller_handler>().actual_satstate; }

		typedef I input_t;

		struct present_s {
			handler::present_s ref;
			satstate_t actual_satstate;
		};
		finall_controller_handler(
			const config_s& _config
			, present_s& _present
		)
			: handler(_config, _present.ref)
			, dummy_input_(0)
			, deseired(&dummy_input_) {
		}
		void set_input(I* _input) {
			if (_input) {
				deseired = _input;
			}
			else {
				deseired = &dummy_input_;
			}
		}

	protected:
		virtual bool do_handler_reconfig(void) {
			ROBO_LBREAKN(handler::do_handler_reconfig());
			do_handler_adjust();
			return true;
		}
		#if ROBO_APP_MEXO_VAR_ENABLED == 1
		virtual void do_handler_create_vars(var::record::list& _vars, int _master_key) {
			if (var::machine::actual_mode() == var::machine::mode::full) {
				var::record::create(var::const_uint8, present<finall_controller_handler>().actual_satstate, RT("ss"), _master_key, _vars);
			}
		};
		#endif
	};

	template < typename B, typename S, typename ... Args> class finall_controller_block_t
		: public handler_t<subsystem_handler, B, S, subsystem_handler, Args...> {
		typedef  handler_t<subsystem_handler, B, S, subsystem_handler, Args...> BB;
	public:
		typedef typename BB::config_s config_s;
		typedef typename BB::present_s present_s;

		finall_controller_block_t(
			cstr  _name
			, S* _owner
			, const config_s& _config
			, present_s& _present
			, Args ... args
		)
			: BB(_name, _owner,  _config, _present, args...) {}
		finall_controller_block_t(
			cstr  _name
			, subsystem_handler* _prev
			, const config_s& _config
			, present_s& _present
			, Args ... args
		)
			: BB(_name, _prev, _config, _present, args...) {}
	};

	template <  typename B, typename S, typename ... Args> class finall_controller_task_t
		: public handler_t<S, B, node, task, Args...> {
		typedef  handler_t<S, B, node, task, Args...> BB;
	public:
		typedef typename BB::config_s config_s;
		typedef typename BB::present_s present_s;
		finall_controller_task_t(
			cstr  _name
			, node* _owner
			, task * _prev
			, const config_s& _config
			, present_s& _present
			, Args ... args
		)
			: BB(_name, _owner, _prev, _config, _present, args...) {}
		finall_controller_task_t(
			cstr  _name
			, task* _prev
			, const config_s& _config
			, present_s& _present
			, Args ... args
		)
			: BB(_name, _prev, _config, _present, args...) {}
	};

	template <typename T, int SZ > class ixvar {
		void (*on_set_)(const T &) ;
		T (*on_get_)(void);
		enum { size = SZ };
		static ixvar * * pool_(void) {
			static ixvar * pool__[size] = {};
			return pool__;
		}
	public:
		ixvar(unsigned int _ix, void (*_on_set)(const T & ), T(*_on_get)(void)) : on_set_(_on_set), on_get_(_on_get) {
			if (_ix >= 0 && _ix < size) {
				on_set_ = _on_set;
				on_get_ = _on_get;
				ixvar** p = pool_() + _ix;
				(*p) = this;
			}
		}
		static T get(int _ix) {
			if (_ix >= 0 && _ix < size) {
				ixvar** p = pool_() + _ix;
				if (*p) {
					return (*p)->on_get_();
				}
			}
			return (T)0;
		}
		static void set(int _ix, const T & _src) {
			if (_ix >= 0 && _ix < size) {
				ixvar** p = pool_() + _ix;
				if (*p) {
					(*p)->on_set_(_src);
				}
			}
		}
		static void set(int _ix, const T * _src) {
			if (_ix >= 0 && _ix < size) {
				ixvar** p = pool_() + _ix;
				if (*p) {
					(*p)->on_set_(*_src);
				}
			}
		}
		static void set(int _ix, const void * _src) {
			if (_ix >= 0 && _ix < size) {
				ixvar** p = pool_() + _ix;
				if (*p) {
					(*p)->on_set_( *((T*)_src) );
				}
			}
		}
	};


	class controller {
	public:
		class process {
		protected:
			enum class result {
				wait = false,
				success = true
			};
		private:
			friend class controller;

			enum class command {
				stop = 0,
				start = 1
			};
			command command_ = command::stop;
			enum class state {
				stopped = 0,
				begin = 1,
				startup = 3,
				execute = 4,
				shutdown = 5,
				reset = 6
			} ;
			state state_;
			void start_(void) { command_ = command::start; };
			bool run_(void);
		protected:

			virtual void onBegin(void) {}
			virtual void onStartup(void) {}
			virtual void onExecute(void) {}
			virtual void onFinish(void) {}
			virtual void onShutdown(void) {}
			virtual void onReset(void) {}

			virtual bool doBegin(void) { return true; }
			virtual bool doStartup(void) { return true; }
			virtual bool doExecute(void) { return true; }
			virtual bool doShutdown(void) { return true; }
			virtual bool doReset(void) { return true; }
			virtual void doIdle(void) {};
			virtual void doTerminate(void) {};
		protected:
			process(void) {};
			void stop(void) { command_ = command::stop; };
			void terminate(void);
			virtual ~process(void) {};
		};
	private:
		process* selected_ = 0;
		process* runned_ = 0;
	protected:
		virtual void doTerminate() {};
	public:
		void switchto(process* _task);
		void stop(void);
		void run(void);
		void terminate(void);
	};	

}
#endif

/*
		template< typename T> class embd_mode :public mode {
		public:
			typedef (typename T::* delegat) (void) ;
		private:
			T& dev_;
			delegat do_applay_;
			delegat do_start_;
			delegat do_stop_;
		protected:
			virtual void applay_action(void) {
				(dev_.*do_applay_)();
			}

			virtual void do_start(void) {
				(dev_.*do_start_)();
			}

			virtual void do_stop(void) {
				(dev_.*do_stop_)();
			}

		public:
			embd_mode(
				int _index
				, cstr _name
				, T& _owner
				, delegat _do_applay
				, delegat _do_start
				, delegat _do_stop
			) :
				mode(
					_index
					, _name
					, _owner
				)
				, do_applay_(_do_applay)
				, do_start_(_do_start)
				, do_stop_(_do_stop) {}
		};
		*/