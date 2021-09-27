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
		friend class iatom;
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
	class subsystem_atom : public node {
	public:
		typedef robo::list::unsorted<subsystem_atom> list;
		typedef list::ref ref;
	private:
		ref ref_;
	public:
	protected:
		friend class subsystem;
		subsystem_atom(cstr  _name, subsystem* _subsystem);
		virtual void operator ()(void) = 0;
	};

	class subsystem {
	protected:
		friend class subsystem_atom;
		virtual node* owned_node(void) = 0;
		subsystem_atom::list atoms;
		void run(void) {
			for (subsystem_atom::ref* r = atoms.first(); r; r = r->next()) {
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



	/*template< typename D, typename S, typename ... Args > class subsys_atom_t : public atom_t<::mexo::subsystem_atom, D, S> {
	public:
		typedef atom_t<::mexo::subsystem_atom, D, S> A;
		typedef typename A::config_s config_s;
		typedef typename A::present_s present_s;
		subsys_atom_t(cstr  _name, S* _owner, const config_s& _config, present_s& _present, Args ... args)
			: A(_name, _owner, _config, _present, args...) {}
	};*/

	template <typename A> struct range_s {
		A low;
		A hi;
	};

	template< typename T, typename R, typename S, typename ... Args > class atom_t : public T, public  R {
	public:
		typedef typename R::config_s config_s;
		typedef typename R::present_s present_s;
		virtual void operator ()(void) {
			R::execute();
		}
		virtual bool do_reconfig(void) {
			return R::do_reconfig();
		}
		atom_t(cstr  _name, S* _owner, const config_s& _config, present_s& _present, Args ... args)
			: T(_name, _owner)
			, R(_config, _present, args...) {}
	};

	template < typename T, typename B, typename S, typename ... Args> class controller_t
		: public atom_t<T, B, S, const typename B::input_t&, const range_s<typename B::output_t>&, satstate_t, Args...> {
	public:
		typedef  atom_t<T, B, S, const typename B::input_t&, const range_s<typename B::output_t>&, satstate_t, Args...> BB;
		controller_t(
			cstr  _name
			, S* _owner
			, typename const BB::config_s& _config
			, typename BB::present_s& _present
			, const typename B::input_t& _deseired
			, const range_s<typename B::output_t>& _range
			, satstate_t& _master_satstate
			, Args ... args
		)
			: BB(_name, _owner, _config, _present, _deseired, _range, _master_satstate, args...) {}
	};
	template < typename B, typename S, typename ... Args> class controller_block_t
		: public atom_t<subsystem_atom, B, S, const typename B::input_t&, const range_s<typename B::output_t>&, satstate_t, Args...> {
	public:
		typedef  atom_t<subsystem_atom, B, S, const typename B::input_t&, const range_s<typename B::output_t>&, satstate_t, Args...> BB;
		controller_block_t(
			cstr  _name
			, S* _owner
			, typename const BB::config_s& _config
			, typename BB::present_s& _present
			, const typename B::input_t& _deseired
			, const range_s<typename B::output_t>& _range
			, satstate_t& _master_satstate
			, Args ... args
		)
			: BB(_name, _owner, _config, _deseired, _range, _master_satstate, _output, _satstate, args...) {}
	};
	template <  typename B, typename S, typename ... Args> class controller_task_t
		: public atom_t<S, B, node, const typename B::input_t&, const range_s<typename B::output_t>&, satstate_t, Args...> {
	public:
		typedef  atom_t<S, B, node, const typename B::input_t&, const range_s<typename B::output_t>&, satstate_t, Args...> BB;
		controller_task_t(
			cstr  _name
			, node* _owner
			, typename const BB::config_s& _config
			, typename BB::present_s& _present
			, const typename B::input_t& _deseired
			, const range_s<typename B::output_t>& _range
			, satstate_t& _master_satstate
			, Args ... args
		)
			: BB(_name, _owner, _config, _present, _deseired, _range, _master_satstate, args...) {}
	};


	class atom {
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
	public:
		template <typename S>const  S& config_cast() { return reinterpret_cast <const  S&>(config_); }
		template <typename P>  P& present_cast() { return reinterpret_cast <P&>(present_); }
		atom(const config_s& _config, present_s& _present) :config_(_config), present_(_present) {}
	};

	template < typename I, typename O> class controller_atom : public atom {
	public:
	protected:
		const I& deseired;
		const range_s<O>& range;
		satstate_t& master_satstate;
	public:
		struct present_s {
			atom::present_s ref;
			O output;
			struct {
				satstate_t actual;
				satstate_t local;
			}satstate;
		};
		controller_atom(
			const config_s& _config
			, present_s& _present
			, const I& _deseired
			, const range_s<O>& _range
			, satstate_t& _master_satstate
		)
			: atom(_config, _present.ref)
			, deseired(_deseired)
			, range(_range)
			, master_satstate(_master_satstate) {}

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
		virtual bool do_reconfig(void) {
			ROBO_LBREAKN(range.low <= range.hi);
			return true;
		}
	};
	template < typename O> class sence_atom : public atom {
	protected:
	public:
		struct present_s {
			atom::present_s ref;
			O output;
		};
		sence_atom(const config_s& _config, present_s& _present)
			: atom(_config, _present.ref) {}
	};

	template< typename R, typename S > class sence_block_t : public atom_t <
		::mexo::subsystem_atom
		, R
		, S
	> {
	public:
		typedef atom_t <
			::mexo::subsystem_atom
			, R
			, S
		> A;
		sence_block_t(cstr _name, S* _owner, const typename A::config_s& _config, typename  A::present_s& _present)
			: A(_name, _owner, _config, _present) {}
	};

	template< typename R, typename S> class sence_task_t : public atom_t <
		, S
		, R
		, ::mexo::node
	> {
	public:
		typedef atom_t <
			, S
			, R
			, ::mexo::node
		> A;
		sence_task_t(cstr _name, ::mexo::node* _owner, const typename A::config_s& _config, typename  A::present_s& _present)
			: A(_name, _owner, _config, _present) {}
	};

	/*	template < typename T, typename B, typename S, typename I, typename O> class function_atom_t
			: public atom_t<
			T
			, B
			, S
			, const I&
			, O&
			> {
		public:
			typedef  atom_t<
				T
				, B
				, S
				, const I&
				, O&
			> BB;
			function_atom_t(
				cstr  _name
				, S& _owner
				, const I& _deseired
				, O& _output
			)
				: BB(_name, _owner, _deseired, _output) {}
		};*/

	template < typename I, typename O> class function_atom : public atom {
	public:
		const I& input;
		struct present_s {
			atom::present_s ref;
			O output;
		};
		function_atom(
			config_s& _config
			, const I& _input
			, present_s& _present
		)
			: atom(_config, _present)
			, input(_input) {}
	};
	/*
	template < typename T, typename B, typename S, typename I, typename O> class scope_atom_t
		: public atom_t<
		T
		, B
		, S
		, const I&
		> {
	public:
		typedef  atom_t<
			T
			, B
			, S
			, const I&
		> BB;
		scope_atom_t(
			cstr  _name
			, S& _owner
			, const I& _deseired
		)
			: BB(_name, _owner, _deseired) {}
	};
	*/
	template < typename I> class scope_atom : public atom {
	protected:
		const I& input;
		atom::present_s present_;
	public:
		scope_atom(
			config_s& _config
			, const I& _deseired
		)
			: atom(_config, present_)
			, input(_input) {}
	};
}
#endif

