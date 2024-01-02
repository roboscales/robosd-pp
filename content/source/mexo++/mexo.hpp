#ifndef mexo_hpp
#define mexo_hpp
#include "core/robosd_list.hpp"
#include "core/robosd_delegat.hpp"
#include "core/robosd_string.hpp"

#include "core/robosd_system.hpp"

#include "mexo++/common.hpp"

#include "mexo++/dev.front.hpp"

#include "mexo++/led.hpp"

#if ROBO_APP_MEXO_VAR_ENABLED ==1
#include "mexo++/vartree.hpp"
#endif

#include <initializer_list> 



using namespace robo;
namespace mexo {
	
	#if ROBO_APP_MEXO_DEBUG_TP1_ENABLED == 1
	class tp_driver {
		//реализацию принудительно делегируем в perephery проекта
	protected:
		void on(void);
		void off(void);
	};
	extern led_t<tp_driver>  tp;

	#else


	extern led_t<dummy_led>  tp;

	#endif 

	class machine {
	public:
		enum { slot_count = ROBO_APP_MEXO_SLOT_COUNT };
		class slots;
		class slot {
		public:
			enum class kind { begin = slot_count, start, realtime, backend, frontend, raise_fault };
			class delegat : public ::robo::delegat::ref<void> {
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
			slot realtime;
			slot backend;
			slot frontend;
			slot raise_fault;
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
//		void begin_(void);
		void begin_(time_us_t _period_us);
		#if ROBO_APP_MEXO_REALTIME_SLOT_ENABLE == 1
		void realtime_loop_(void);
		#endif
		void backend_loop_(void);
		void frontend_loop_(void);
		void raise_fault_(void);
		static machine instance_;
	public:
		machine(void);
		~machine(void);
		//static void start(time_us_t _period_us) { instance_.start_(_period_us); }
		static void begin(time_us_t _period_us) { instance_.begin_(_period_us); }
		#ifdef ROBO_APP_MEXO_SAMPLE_US
//		static void start(void) { instance_.start_(ROBO_APP_MEXO_SAMPLE_US); }
			static void begin(void) { instance_.begin_(ROBO_APP_MEXO_SAMPLE_US); }
		#endif		
		#if ROBO_APP_MEXO_REALTIME_SLOT_ENABLE == 1
		static void realtime_loop(void) { instance_.realtime_loop_(); }
		#endif
		static void backend_loop(void) { instance_.backend_loop_(); }
		static void frontend_loop(void) { instance_.frontend_loop_(); }
		static int slot_index(void) { return instance_.slot_index_; }
		static void raise_fault(void) { instance_.raise_fault_(); }
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
		#ifdef ROBO_APP_MEXO_SIDE
		#if ROBO_APP_MEXO_VAR_ENABLED ==1
		var::record::list vars;
		#endif
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
	public:
		virtual  ~dev(void){}
			
		typedef front::dev::action_s action_s;
		typedef front::dev::feedback_s feedback_s;

		struct present_s {
			uint8_t mode;
			bool action_actual;
			uint8_t error;
			//robo::time_us_t wachdog_us;
			//bool wachdog_enabled;
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
		bool wait_feedback_ = false;
	protected:
		virtual void do_update_feedback(void);
	public:
		
	void update_feedback(void) {
			#if ROBO_APP_SYSTEM_ENABLED  == 1
			if (::robo::system::env::is_backend()) {
				do_update_feedback();
			}
			else {
				::robo::system::critical c__;
				wait_feedback_ = true;
				while (wait_feedback_ == true) ::system::env::sleep();
			}
			#else
				do_update_feedback();
			#endif
		}
		
		class mode : public node {
			friend class dev;
		public:
			typedef robo::list::unique<mode, int> map;
			typedef map::ref ref;

		private:
			ref ref_;
		protected:
			template<typename T>T& owner_cast(void) { return *(T*)node::owner(); };
			virtual void applay_action(void) = 0;
			virtual void begin(void) = 0;
			virtual void finish(void) = 0;
			virtual void bakend(void) = 0;
			virtual void frontend(void) = 0;
			virtual void manualA(void) = 0;
			virtual void manualB(void) = 0;
		public:
			mode(int _index, cstr  _name, dev& _dev);
		};
		
		class idle_mode : public mode {
		protected:
			virtual void applay_action(void) {};
			virtual void begin(void) {};
			virtual void finish(void) {};
			virtual void bakend(void) {};
			virtual void frontend(void) {};
			virtual void manualA(void) {};
			virtual void manualB(void) {};

		public:
			idle_mode(dev& _dev) : dev::mode(front::dev::mode::idle, RT("idle"), _dev) {};
		};

		idle_mode idle;
		
		dev(cstr  _name, action_s& _action, feedback_s& _feedback, present_s& _present, config_s& _config);
		
		void switch_to(int _mode);
		void action_auto_apply_enable(void) { action_auto_apply_ = true;  }
		void action_auto_apply_disable(void) { action_auto_apply_ = false; }
		void action_update(void) { present_.action_actual = true; }

	protected:
		#if ROBO_APP_MEXO_VAR_ENABLED == 1
		virtual void do_create_vars(void);
		#endif
	private:
		
		//если false то  при переключении режима самостоятельно интерпретирует  action
		bool action_auto_apply_ = true;

		friend class mode;
	
		mode::map modes_;
		mode* actual_mode_;
		
		::robo::delegat::owned::fabric < mexo::machine::slot::delegat, void>::member<dev> backend_;
		::mexo::machine::slot::delegat::ref  backend_ref_;
		action_s& action_;
		feedback_s& feedback_;
		present_s& present_;
		config_s& config_;
		void backend__(void);
		void frontend_(void);
		void manuaA_(void);
		void manuaB_(void);
		protected:
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
				prepare = 1,
				startup = 3,
				execute = 4,
				shutdown = 5,
				relax = 6
			} ;
			state state_ = state::stopped;
			void start_(void) { command_ = command::start; };
		protected:
			bool run(void);
			virtual void onPrepare(void) {}
			virtual void onStartup(void) {}
			virtual void onExecute(void) {}
			virtual void onFinish(void) {}
			virtual void onShutdown(void) {}
			virtual void onRelax(void) {}

			virtual result doPrepare(void) { return result::success; }
			virtual result doStartup(void) { return result::success; }
			virtual result doExecute(void) { return result::success; }
			virtual result doShutdown(void) { return result::success; }
			virtual result doRelax(void) { return result::success; }
			virtual void doIdle(void) {};
			virtual void doTerminate(void) {};
		protected:
			process(void) {};
			void stop(void) { command_ = command::stop; };
			void restart(void) { if(command_ == command::stop && state_ == state::relax ) state_ = state::stopped; };
			void terminate(void);
			virtual ~process(void) {};
		};
		class independed : public process{
			protected:
				independed(){}
			public:
				bool run(void){
					return process::run();
				}
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
	

	template< class H, class S> class timer_t :  public H{
	public:
		using tm = ::robo::time_us_t;
	private:
		tm last_= tm(0);
		tm period_ = tm(0);
		bool once_;
		bool started_;
	protected:
		virtual void operator ()(void){
			::robo::time_us_t now = S::time_us();
			if(now - last_ > period_){
				last_ = now;
				H::operator () ();
				if(once_){
				 H::stop();
				}
			}
		}
	public:
		timer_t(void)
		{
		}
		void start(tm _period, bool _once = false){
			period_ = _period;			
			once_ = _once;
			last_ = S::time_us();
			H::start();
			started_ = true;
		}
		void stop(void ){
			H::stop();
			started_ = false;
		}
		bool started(void){ return started_; }
	};

	template< class H, typename ... Args> class timer_delegat_t :  public H{
	public:
		void stop(void){}
		void start(void){}
		timer_delegat_t(Args ... args): H(args...)
		{
		}
	};

	typedef ::mexo::timer_t< timer_delegat_t<::robo::delegat::owned_fabric<void>::simple, void(*)(void)>, void(*)(void) > repeat_t;

	
	class stateflow {
	public:
		class node {
			friend class stateflow;
		protected:
			virtual void onStartup(void) = 0;
			virtual void doExecute(void) = 0;
			virtual void onFinish(void) = 0;
		};
	private:
		node * runned_ = 0;
	public:
		void switchto(node * _node);
		void run(void);
	};	
}
#endif

