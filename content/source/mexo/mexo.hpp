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

#ifndef APP_MEXO_SIGNAL_T
#define APP_MEXO_SIGNAL_T float
#endif

#ifndef APP_MEXO_PARAMETR_T
#define APP_MEXO_PARAMETR_T float
#endif

#ifndef APP_MEXO_LONG_SIGNAL_T
#define APP_MEXO_LONG_SIGNAL_T double
#endif

#ifndef MEXO_DEBUG_TP1_ENABLED
#define MEXO_DEBUG_TP1_ENABLED 0
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

	typedef APP_MEXO_SIGNAL_T signal_t;
	typedef APP_MEXO_LONG_SIGNAL_T long_signal_t;
	typedef APP_MEXO_PARAMETR_T parametr_t;

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

			//typedef delegat_t< ::robo::delegat::lambda<delegat, void>, ::robo::lambda< void(void)> > lambda;
			//typedef delegat_t < ::robo::delegat::simple<delegat, void>, void (*)(void) >  simple;
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
		//		virtual void do_enable(void) {};
		//		virtual void do_disable(void) {};
		virtual bool do_reconfig(void) { return true; };
	public:
		node* owner() { return owner_; };
		cstr name(void) { return name_; };

		static bool begin(void);

		bool reconfig(void);

		node(cstr _name, /*bool _auto_enabled,*/ node* _owner = nullptr);

		static node* find(int _key) {
			return map_().find(_key);
		}
		//		void enable(bool _hand = false);
		//		void disable(void);
		bool enabled(void) { return state_ == state::ready; };
	};
	class isubsystem;
	class iblock : public node {
		friend class isubsystem;
	public:
		struct config_s {
			int ver;
		};
		typedef robo::list::unsorted<iblock> list;
		typedef list::ref ref;
	private:
		ref ref_;
	protected:
		config_s& config;
	public:
		template <typename T> class output_t {
		public:

			const T& value;

			bool operator == (const T& _value) {
				return value == _value;
			}

			output_t(const T& _value) : value(_value) {}
		};

		template <typename T> class input_t {
			output_t<T> dummy_;
			const output_t<T>* output_;
		public:
			const T& value(void) { return output_->value; }

			input_t(const T& _dummy) : dummy_(_dummy), output_(&dummy_) {}
			void link_to(const output_t<T>* _output) {
				if (_output == nullptr) {
					output_ = &dummy_;
				}
				else {
					output_ = _output;
				}
			}
		};

		enum class satstate { none, both, low, up };

	protected:

		iblock(isubsystem& _subsystem, cstr  _name, config_s& _config);

		virtual void execute(void) = 0;
	};


	class isubsystem : public node {
		friend class iblock;
	public:
		typedef robo::list::unsorted<isubsystem> list;
		typedef robo::list::unsorted<isubsystem>::ref ref;
	private:
		iblock::list blocks_;
		list childs_;
		ref ref_;
		bool autostart_;
	protected:
		virtual void  do_start(void) = 0;
		virtual void do_stop(void) = 0;
		isubsystem(cstr  _name, bool _autostart, node* _owner = nullptr);
		void execute(void);
	public:
		void  start(void);
		void stop(void);
		virtual bool do_reconfig(void) {
			ROBO_LBREAKN(node::do_reconfig());
			if (autostart_) start();
			return true;
		}
	};

	class subsystem : public isubsystem, public machine::slot::delegat {

	protected:
		subsystem(cstr  _name, bool _autostart, node* _owner = nullptr);
	public:
		virtual	void operator()(void);
	};


	class prioritet_subsystem : public subsystem {
		machine::slot::delegat::ref ref_;
	protected:
		virtual void do_start(void) {
			guard__; machine::slot::delegat::attach(ref_, machine::slot::kind::priority);
		};
		virtual void do_stop(void) { guard__; ref_.dettach(); };
	public:
		prioritet_subsystem(cstr  _name, bool _autostart, node* _owner = nullptr) : subsystem(_name, _autostart, _owner), ref_(*this) {};
	};

	class backend_subsystem : public subsystem {
		machine::slot::delegat::ref ref_;
	public:
		virtual void do_start(void) { guard__; machine::slot::delegat::attach(ref_, machine::slot::kind::backend); };
		virtual void do_stop(void) { guard__; ref_.dettach(); };
		backend_subsystem(cstr  _name, bool _autostart, node* _owner = nullptr) : subsystem(_name, _autostart, _owner), ref_(*this) {};
	};

	class frontend_subsystem : public subsystem {
		machine::slot::delegat::ref ref_;
	public:
		virtual void do_start(void) { guard__; machine::slot::delegat::attach(ref_, machine::slot::kind::frontend); };
		virtual void do_stop(void) { guard__; ref_.dettach(); };
		frontend_subsystem(cstr  _name, bool _autostart, node* _owner = nullptr) : subsystem(_name, _autostart, _owner), ref_(*this) {};
	};


	class periodic_subsystem : public subsystem {
		machine::slot::delegat::ref** refs_ = nullptr;
		size_t ref_count_ = 0;
	public:
		virtual void do_start(void) {
			machine::slot::delegat::ref** pref = refs_;
			for (size_t n = 0; n < ref_count_; ++n, ++pref) {
				machine::slot::delegat::attach(**pref, n);
			}
		};
		virtual void do_stop(void) {
			machine::slot::delegat::ref** pref = refs_;
			for (size_t n = 0; n < ref_count_; ++n, ++pref) {
				(*pref)->dettach();
			}
		};
		periodic_subsystem(cstr  _name, bool _autostart, std::initializer_list<int> _index, node* _owner = nullptr) : subsystem(_name, _autostart, _owner) {
			ref_count_ = _index.end() - _index.begin() + 1;
			refs_ = new machine::slot::delegat::ref * [ref_count_];
			machine::slot::delegat::ref** pref = refs_;
			for (size_t n = 0; n < ref_count_; ++n, ++pref) {
				*pref = new machine::slot::delegat::ref(*this);
			}
		};
		virtual ~periodic_subsystem(void) {
			machine::slot::delegat::ref** pref = refs_;
			for (size_t n = 0; n < ref_count_; ++n, ++pref) {
				delete* pref;
			}
			delete[] refs_;
		}
	};


	class dev : public node {
		friend class mode;
	protected:
		virtual void on_idle(void) {};
	public:
		enum { idle_id = 0 };
		struct action {
			bool actual;
			int mode = idle_id;
		};
		struct snapshot {
			int mode = idle_id;
		};



		class mode : public isubsystem {
			friend class dev;
		public:
			typedef robo::list::unique<mode, int> map;
			typedef map::ref ref;

		private:
			typedef robo::list::unsorted<subsystem> subsystems_;
			ref ref_;
		protected:
			dev& owner() { return *((dev*)node::owner()); };
			virtual void applay_action(void) {};
			virtual void do_start(void) {};
			virtual void do_stop(void) {};
		public:
			mode(int _index, cstr  _name, dev& _dev);
		};

		class idle_mode : public mode {
		protected:
			virtual void do_start(void) {};
			virtual void do_stop(void) {};
			virtual	void do_execute(void) {};

		public:
			virtual	void operator()(void) {};
			idle_mode(dev& _dev) : dev::mode(idle_id, RT("idle"), _dev) {};
		};

		idle_mode idle;
		dev(cstr  _name, action& _action, snapshot& _snapshot);
		void switch_to(int _mode);
	private:
		friend class mode;
		mode::map modes_;
		mode* actual_mode_;
		int actual_mode_id_;
		::robo::delegat::member< mexo::machine::slot::delegat, dev, void> backend_;
		::mexo::machine::slot::delegat::ref  backend_ref_;
		action& action_;
		snapshot& snapshot_;
		void backend__(void);
	};



	/*
	class tandem_subsystem : public node {
	public:
		class backend : public ::robo::delegat::member< machine::slot::delegat, tandem_subsystem, void> {
			friend class tandem_subsystem;
			machine::slot::delegat::ref ref_;
		public:
			backend(tandem_subsystem & _owner)
				: ::robo::delegat::member< machine::slot::delegat, tandem_subsystem, void>(&_owner, tandem_subsystem::backend_execute_), ref_(*this) {}
		};
		class frontend : public ::robo::delegat::member< machine::slot::delegat, tandem_subsystem, void> {
			friend class tandem_subsystem;
			machine::slot::delegat::ref ref_;
		public:
			frontend(tandem_subsystem& _owner)
				: ::robo::delegat::member< machine::slot::delegat, tandem_subsystem, void>(&_owner, tandem_subsystem::frontend_execute_), ref_(*this) {}
		};
	private:
		friend class backend;
		friend class frontend;
		backend backend_;
		frontend frontend_;
		virtual void backend_execute_(void) {};
		virtual void frontend_execute_();
	public:
	};
	*/


	template < typename S > class block_t : public  iblock {
	protected:
	public:
		//S - это старая добрая сишная структура
		typedef  S config_s;
		block_t(isubsystem& _subsystem, cstr  _name, config_s& _config) : iblock(_subsystem, _name, reinterpret_cast<iblock::config_s&> (_config)) {};
		virtual bool do_reconfig(void) {
			return applay(reinterpret_cast<config_s&> (iblock::config));
		}

		virtual bool applay(const  config_s& _config) = 0;
	};


	template <typename A> struct range_s {
		A low;
		A hi;
	};

	template < typename A> class controller_block_t : public  block_t<typename A::config_s>, public A {
	protected:
		iblock::satstate satstate_ = iblock::satstate::none;
	public:
		typedef  typename A::input_t input_t;
		typedef  typename A::output_t output_t;
		typedef  typename A::config_s config_s;

		input_t standalone_input;
		iblock::input_t<input_t> input;

		iblock::satstate standalone_master_satstate = iblock::satstate::none;
		iblock::input_t<iblock::satstate> master_satstate;

		range_s<output_t> standalone_range = {};
		iblock::input_t<range_s<output_t>> range;

		iblock::output_t<output_t> output;
		iblock::output_t<iblock::satstate> satstate;


		controller_block_t(isubsystem& _subsystem, cstr  _name, config_s& _config)
			: block_t<config_s>(_subsystem, _name, _config)
			, input(standalone_input)
			, master_satstate(standalone_master_satstate)
			, range(standalone_range)
			, output(A::output_value())
			, satstate(satstate_) {
			standalone_input = {};
		}

		virtual bool applay(const config_s& _config) {
			standalone_range = ((const typename A::config_s&)_config).range;
			ROBO_LBREAKN(standalone_range.low <= standalone_range.hi);
			ROBO_LRET(A::applay(_config));
		}

		virtual void execute(void) {
			satstate_ = A::execute(input.value(), master_satstate.value(), range.value());
		}

		template < typename M> void link_to(M& _controller) {
			input.link_to(&_controller.output);
			master_satstate.link_to(&_controller.satstate);
		}

	};

	template < typename A> class sence_block_t : public  block_t<typename A::config_s>, public A {
	public:
		typedef  typename A::output_t output_t;
		typedef  typename A::config_s config_s;

		iblock::output_t<output_t> output;


		sence_block_t(isubsystem& _subsystem, cstr  _name, config_s& _config)
			: block_t<config_s>(_subsystem, _name, _config)
			, output(A::output_value()) {}

		virtual bool applay(const config_s& _config) {
			ROBO_LRET(A::applay(_config));
		}

		virtual void execute(void) {
			A::update_output();
			A::query();
		}

	};
	template < typename A> class atom_block_t : public  block_t<typename A::config_s>, public A {
	public:
		typedef  typename A::output_t output_t;
		typedef  typename A::input_t input_t;
		typedef  typename A::config_s config_s;

		iblock::output_t<output_t> output;
		input_t standalone_input;
		iblock::input_t<input_t> input;


		atom_block_t(isubsystem& _subsystem, cstr  _name, config_s& _config)
			: block_t<config_s>(_subsystem, _name, _config)
			, output(A::output_value())
			, input(standalone_input) {
			standalone_input = {};
		}

		virtual bool applay(const config_s& _config) {
			ROBO_LRET(A::applay(_config));
		}

		virtual void execute(void) {
			input_t tmp = input.value();
			A::execute(tmp);
		}

	};


}
#endif
