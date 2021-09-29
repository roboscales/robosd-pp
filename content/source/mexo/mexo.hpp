#ifndef mexo_hpp
#define mexo_hpp
#include "core/robosd_list.hpp"
#include "core/robosd_delegat.hpp"
#include "core/robosd_string.hpp"
#include "core/robosd_system.hpp"
#include "mexo/led.hpp"

#include <initializer_list> 
#ifndef APP_MEXO_SLOT_COUNT
#define APP_MEXO_SLOT_COUNT 16
#endif

#ifndef APP_MEXO_PRIORITY_SLOT_ENABLE
#define APP_MEXO_PRIORITY_SLOT_ENABLE 1
#endif

using namespace robo;
namespace mexo {
	struct tp_verb {
		enum { frontend = 1, backend = 2, priority = 3, loop = 4 };
	};
	#if MEXO_DEBUG_TP1_ENABLED == 1
	class tp_driver {
		//реализацию принудительно делегируем в perephery проекта
	protected:
		void on(void);
		void off(void);
	};
	typedef led_s<tp_driver>  tp;

	#else


	typedef dummy_led  tp;

	#endif 

	class machine {
	public:
		enum { slot_count = APP_MEXO_SLOT_COUNT };
		class slots;
		class slot {
		public:
			enum class kind { begin = slot_count, start, priority, backend, frontend };
			class delegat : public ::robo::delegat::base<void> {
			public:
				typedef list::unsorted<delegat> list;
				typedef list::ref ref;
				virtual ~delegat(void) {}
				void attach(slot::kind _kind);
				static void attach(ref& _ref, int _index);
				static void attach(ref& _ref, slot::kind _kind);
				void attach(int _index);

				void attach(const int* _index, int _count) {
					for (int i = 0; i < _count; ++i, ++_index) {
						attach(*_index);
					}
				}
				template <size_t N> void attach(int(&index)[N]) {
					attach(index, N);
				}
				void attach(std::initializer_list<int> _index) {
					for (const auto x : _index) {
						attach(x);
					}
				}
			};

			class lambda : public ::robo::delegat::lambda<delegat, void> {
			public:
				lambda(const ::robo::lambda< void(void)>& _lambda) : ::robo::delegat::lambda<delegat, void>(_lambda) {}
				lambda(slot::kind _kind, const ::robo::lambda< void(void)>& _lambda) : ::robo::delegat::lambda<delegat, void>(_lambda) {
					attach(_kind);
				}
				lambda(int _index, const  ::robo::lambda< void(void)>& _lambda) : ::robo::delegat::lambda<delegat, void>(_lambda) {
					attach(_index);
				}
				lambda(const int* _index, int _count, const  ::robo::lambda< void(void)>& _lambda) : ::robo::delegat::lambda<delegat, void>(_lambda) {
					attach(_index, _count);
				}
				template <size_t N>  lambda(int(&_index)[N], const  ::robo::lambda< void(void)>& _lambda) : ::robo::delegat::lambda<delegat, void>(_lambda) {
					attach(_index);
				}
				lambda(std::initializer_list<int> _index, const  ::robo::lambda< void(void)>& _lambda) : ::robo::delegat::lambda<delegat, void>(_lambda) {
					attach(_index);
				}
			};

			class simple : public ::robo::delegat::simple<delegat, void> {
			public:
				simple(void (*_lambda)(void)) : ::robo::delegat::simple<delegat, void>(_lambda) {}
				simple(slot::kind _kind, void (*_lambda)(void)) : ::robo::delegat::simple<delegat, void>(_lambda) {
					attach(_kind);
				}
				simple(int _index, void (*_lambda)(void)) : ::robo::delegat::simple<delegat, void>(_lambda) {
					attach(_index);
				}
				simple(const int* _index, int _count, void (*_lambda)(void)) : ::robo::delegat::simple<delegat, void>(_lambda) {
					attach(_index, _count);
				}
				template <size_t N>  simple(int(&_index)[N], void (*_lambda)(void)) : ::robo::delegat::simple<delegat, void>(_lambda) {
					attach(_index);
				}
				simple(std::initializer_list<int> _index, void (*_lambda)(void)) : ::robo::delegat::simple<delegat, void>(_lambda) {
					attach(_index);
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
			#if APP_MEXO_PRIORITY_SLOT_ENABLE == 1
			slot priority;
			#endif
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
		#if APP_MEXO_PRIORITY_SLOT_ENABLE == 1
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
		#if APP_MEXO_PRIORITY_SLOT_ENABLE == 1
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
		node(void);

		static node& root_(void);
		static map& map_(void);
	protected:
		list& childs(void) { return childs_; }
		virtual bool do_reconfig(void) { return true; };
	public:
		node* owner() { return owner_; };
		cstr name(void) { return name_; };

		static bool begin(void);

		bool reconfig(void);

		node(cstr _name, node* _owner = nullptr);

		static node* find(int _key) {
			return map_().find(_key);
		}
		bool enabled(void) { return state_ == state::ready; };
	};

	class dev : public node {
		friend class mode;
	protected:
		virtual void on_idle(void) {};
	public:
		enum { idle_id = 0 };
		struct action_s {
			bool actual;
			int mode;
		};
		struct present_s {
			int mode;
		};

		template <typename S> const S& action_cast(void) {
			return reinterpret_cast <const S&>(action_);
		}

		template <typename P> P& present_cast(void) {
			return reinterpret_cast <P&>(present_);
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
			idle_mode(dev& _dev) : dev::mode(idle_id, RT("idle"), _dev) {};
		};

		idle_mode idle;
		dev(cstr  _name, action_s& _action, present_s& _present);
		void switch_to(int _mode);
	private:
		friend class mode;
		mode::map modes_;
		mode* actual_mode_;
		::robo::delegat::member< mexo::machine::slot::delegat, dev, void> backend_;
		::mexo::machine::slot::delegat::ref  backend_ref_;
		action_s& action_;
		present_s& present_;
		void backend__(void);
	};

	class task : public node, public machine::slot::delegat {
		friend class ihandler;
	public:
	private:
		bool autostart_;
	protected:
		virtual void  do_start(void) = 0;
		virtual void do_stop(void) = 0;
		task(cstr  _name, bool _autostart, node* _owner = nullptr) :node(_name, _owner), autostart_(_autostart) {};
		virtual bool do_reconfig(void);
	public:
		void start(void);
		void stop(void);
	};

	class prioritet_task : public task {
		machine::slot::delegat::ref ref_;
	protected:
		virtual void do_start(void) {
			guard__; machine::slot::delegat::attach(ref_, machine::slot::kind::priority);
		};
		virtual void do_stop(void) { guard__; ref_.dettach(); };
		prioritet_task(cstr  _name, node* _owner = nullptr) : task(_name, false, _owner), ref_(*this) {};
	public:
		prioritet_task(cstr  _name, bool _autostart, node* _owner = nullptr) : task(_name, _autostart, _owner), ref_(*this) {};
	};

	class backend_task : public task {
		machine::slot::delegat::ref ref_;
	protected:
		virtual void do_start(void) { guard__; machine::slot::delegat::attach(ref_, machine::slot::kind::backend); };
		virtual void do_stop(void) { guard__; ref_.dettach(); };
		backend_task(cstr  _name, node* _owner = nullptr) : task(_name, false, _owner), ref_(*this) {};
	public:
		backend_task(cstr  _name, bool _autostart, node* _owner = nullptr) : task(_name, _autostart, _owner), ref_(*this) {};
	};

	class frontend_task : public task {
		machine::slot::delegat::ref ref_;
	protected:
		virtual void do_start(void) { guard__; machine::slot::delegat::attach(ref_, machine::slot::kind::frontend); };
		virtual void do_stop(void) { guard__; ref_.dettach(); };
		frontend_task(cstr  _name, node* _owner = nullptr) : task(_name, false, _owner), ref_(*this) {};
	public:
		frontend_task(cstr  _name, bool _autostart, node* _owner = nullptr) : task(_name, _autostart, _owner), ref_(*this) {};
	};


	class periodic_task : public task {
		machine::slot::delegat::ref** refs_ = nullptr;
		size_t ref_count_ = 0;
		int* index_ = nullptr;
	protected:
		void setup(std::initializer_list<int> _index);
		void clean(void);
		periodic_task(cstr  _name, node* _owner = nullptr);
		virtual void do_start(void);
		virtual void do_stop(void);
	public:
		periodic_task(cstr  _name, bool _autostart, std::initializer_list<int> _index, node* _owner = nullptr);
		virtual ~periodic_task(void);
	};

	template<int ... Nums> class periodic_task_t : public periodic_task {
	public:
		periodic_task_t(cstr  _name, node* _owner = nullptr) : periodic_task(_name, _owner) {
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
	public:
	protected:
		friend class subsystem;
		subsystem_handler(cstr  _name, subsystem* _subsystem);
		virtual void operator ()(void) = 0;
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
	public:
		virtual void operator ()(void) { subsystem::run(); };
		prioritet_subsystem(cstr  _name, bool _autostart, node* _owner = nullptr) : prioritet_task(_name, _autostart, _owner) {};
	};
	class backend_subsystem : public backend_task, public subsystem {
	protected:
		virtual node* owned_node(void) { return this; };
	public:
		virtual void operator ()(void) { subsystem::run(); };
		backend_subsystem(cstr  _name, bool _autostart, node* _owner = nullptr) : backend_task(_name, _autostart, _owner) {};
	};
	class frontend_subsystem : public frontend_task, public subsystem {
	protected:
		virtual node* owned_node(void) { return this; };
	public:
		virtual void operator ()(void) { subsystem::run(); };
		frontend_subsystem(cstr  _name, bool _autostart, node* _owner = nullptr) : frontend_task(_name, _autostart, _owner) {};
	};

	class periodic_subsystem : public periodic_task, public subsystem {
	protected:
		virtual node* owned_node(void) { return this; };
	public:
		virtual void operator ()(void) { subsystem::run(); };

		periodic_subsystem(cstr  _name, bool _autostart, std::initializer_list<int> _index, node* _owner = nullptr) : periodic_task(_name, _autostart, _index, _owner) {};
	};

	template <typename A> struct range_s {
		A low;
		A hi;
	};

	template< typename T, typename R, typename S, typename ... Args > class handler_t : public T, public  R {
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
	public:
		handler_t(cstr  _name, S* _owner, const config_s& _config, present_s& _present, Args ... args)
			: T(_name, _owner)
			, R(_config, _present, args...) {}
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
	public:
		template <typename S>const  S& config_cast() { return reinterpret_cast <const  S&>(config_); }
		template <typename P>  P& present_cast() { return reinterpret_cast <P&>(present_); }
		handler(const config_s& _config, present_s& _present) :config_(_config), present_(_present) {}
	};

	/*template < typename T> class connection_point {
		static T& dummy_() {
			static T dummy__ = (T)0;
			return dummy__;
		}
		T& value_;
		T* input_;
		T* output_;
	public:
		connection_point(T& _value) :value_(_value) {
			input_ = &dummy_();
			output_ = &dummy_();
		}
		void connect(T* _input, T* _output) {
			if (_input == nullptr) {
				input_ = &dummy_();
			}
			else {
				input_ = _input;
			}
			if (_output == nullptr) {
				output_ = &dummy_();
			}
			else {
				output_ = _output;
			}
		}
		void update() {
			*output_ = value_ = *input_;
		}
	};
	*/


	template < typename I, typename O> class controller_handler : public handler {
		I dummy_input_ = (I)0;
		O dummy_output_ = (O)0;
	protected:
		const range_s<O>& range;
		const satstate_t& master_satstate;
		I* deseired;
		O* output;
		//connection_point<O>* connection_point_;
	public:
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
		void set_output(I* _output) {
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
			present_s& present = present_cast<present_s>();
			satstate_t remote = master_satstate;
			if (remote == satstate_t::none) {
				present.satstate.actual = present.satstate.local;
			}
			else {
				present.satstate.actual = remote;
			}
		}
		virtual bool do_handler_reconfig(void) {
			ROBO_LBREAKN(handler::do_handler_reconfig());
			ROBO_LBREAKN(range.low <= range.hi);
			do_handler_adjust();
			return true;
		}
	};
	template < typename T, typename B, typename S, typename ... Args> class controller_t
		: public handler_t<T, B, S, const range_s<typename B::output_t>&, satstate_t, Args...> {
	public:
		typedef  handler_t<T, B, S, const range_s<typename B::output_t>&, satstate_t, Args...> BB;
		controller_t(
			cstr  _name
			, S* _owner
			, const typename BB::config_s& _config
			, typename BB::present_s& _present
			, const range_s<typename B::output_t>& _range
			, satstate_t& _master_satstate
			, Args ... args
		)
			: BB(_name, _owner, _config, _present, _range, _master_satstate, args...) {}
	};
	template < typename B, typename S, typename ... Args> class controller_block_t
		: public handler_t<subsystem_handler, B, S, const range_s<typename B::output_t>&, const satstate_t&, Args...> {
	public:
		typedef  handler_t<subsystem_handler, B, S, const range_s<typename B::output_t>&, const satstate_t&, Args...> BB;
		controller_block_t(
			cstr  _name
			, S* _owner
			, const typename BB::config_s& _config
			, typename BB::present_s& _present
			, const range_s<typename B::output_t>& _range
			, const satstate_t& _master_satstate
			, Args ... args
		)
			: BB(_name, _owner, _config, _present, _range, _master_satstate, args...) {}
	};
	template <  typename B, typename S, typename ... Args> class controller_task_t
		: public handler_t<S, B, node, const range_s<typename B::output_t>&, const satstate_t&, Args...> {
	public:
		typedef  handler_t<S, B, node, const range_s<typename B::output_t>&, const  satstate_t&, Args...> BB;
		controller_task_t(
			cstr  _name
			, node* _owner
			, const typename BB::config_s& _config
			, typename BB::present_s& _present
			, const range_s<typename B::output_t>& _range
			, const satstate_t& _master_satstate
			, Args ... args
		)
			: BB(_name, _owner, _config, _present, _range, _master_satstate, args...) {}
	};


	template < typename O> class sence_handler : public handler {
	protected:
	public:
		struct present_s {
			handler::present_s ref;
			O output;
		};
		sence_handler(const config_s& _config, present_s& _present)
			: handler(_config, _present.ref) {}
	};

	template< typename R, typename S > class sence_block_t : public handler_t <
		::mexo::subsystem_handler
		, R
		, S
	> {
	public:
		typedef handler_t <
			::mexo::subsystem_handler
			, R
			, S
		> A;
		sence_block_t(cstr _name, S* _owner, const typename A::config_s& _config, typename  A::present_s& _present)
			: A(_name, _owner, _config, _present) {}
	};

	template< typename R, typename S> class sence_task_t : public handler_t <
			S
		, R
		, ::mexo::node
	> {
	public:
		typedef handler_t <
				S
			, R
			, ::mexo::node
		> A;
		sence_task_t(cstr _name, ::mexo::node* _owner, const typename A::config_s& _config, typename  A::present_s& _present)
			: A(_name, _owner, _config, _present) {}
	};

	template < typename I, typename O> class function_handler : public handler {
	protected:
		const I & input;
	public:
		typedef I input_t;
		typedef O output_t;
		struct present_s {
			handler::present_s ref;
			output_t output;
		};
		function_handler(
			const config_s& _config
			, present_s& _present
			, const input_t& _input
		)
			: handler(_config, _present.ref)
			, input(_input){}
	};

	template < typename T, typename R, typename S, typename ... Args> class function_t
		: public handler_t<T, R, S, const typename R::input_t&, Args...> {
		typedef  handler_t<T, R, S, const typename R::input_t&, Args...> BB;
	public:
		function_t(
			cstr  _name
			, S* _owner
			, const typename BB::config_s& _config
			, typename BB::present_s& _present
			, const typename R::input_t & _input
			, Args ... args
		)
			: BB(_name, _owner, _config, _present,_input, args...) {}
	};
	template < typename R, typename S, typename ... Args> class function_block_t
		: public handler_t<subsystem_handler, R, S, const typename R::input_t&, Args...> {
		typedef  handler_t<subsystem_handler, R, S, const typename R::input_t&, Args...> BB;
	public:
		function_block_t(
			cstr  _name
			, S* _owner
			, const typename BB::config_s& _config
			, typename BB::present_s& _present
			, const typename R::input_t& _input
			, Args ... args
		)
			: BB(_name, _owner, _config, _present, _input, args...) {}
	};
	template <  typename R, typename S, typename ... Args> class function_task_t
		: public handler_t<S, R, node, const typename R::input_t&, Args...> {
		typedef  handler_t<S, R, node, const typename R::input_t&, Args...> BB;
	public:
		function_task_t(
			cstr  _name
			, node* _owner
			, const typename BB::config_s& _config
			, typename BB::present_s& _present
			, const typename R::input_t& _input
			, Args ... args
		)
			: BB(_name, _owner, _config, _present, _input, args...) {}
	};

	template < typename I> class scope_handler : public handler {
	protected:
		const I& input;
		handler::present_s present_;
	public:
		scope_handler(
			config_s& _config
			, const I & _input
		)
			: handler(_config, present_)
			, input(_input) {}
	};
}
#endif

