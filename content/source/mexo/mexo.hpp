#ifndef mexo_hpp
#define mexo_hpp
#include "core/robosd_list.hpp"
#include "core/robosd_delegat.hpp"
#include "core/robosd_string.hpp"
#include "core/robosd_system.hpp"
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

using namespace robo;
namespace mexo {
	typedef APP_MEXO_SIGNAL_T signal_t;
	typedef APP_MEXO_LONG_SIGNAL_T long_signal_t;
	typedef APP_MEXO_PARAMETR_T parametr_t;
	class machine {
	public:
		enum { slot_count = APP_MEXO_SLOT_COUNT };
		class slots;
		class slot {
		public:
			enum class kind { begin= slot_count, start, priority, backend, frontend };
			class delegat : public ::robo::delegat::base<void> {
			public:
				typedef list::unsorted<delegat> list;
				typedef list::ref ref;
				virtual ~delegat(void) {}
				void attach( slot::kind _kind );
				static void attach( ref & _ref, int _index);
				static void attach(ref& _ref, slot::kind _kind);
				void attach(int _index);

				void attach(const int* _index, int _count) {
					for (int i = 0; i < _count; ++i,++_index) {
						attach( *_index );
					}
				}
				template <size_t N> void attach(int (&index)[N]) {
					attach(index, N);
				}
				void attach(std::initializer_list<int> _index) {
					for (const auto x : _index) {
						attach(x);
					}
				}
			};
			typedef ::robo::delegat::lambda<delegat, void> lambda;
			typedef ::robo::delegat::simple<delegat, void> simple;
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
		slots & slots_ref_;
		int slot_index_;
		static slots & slots_(void);
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
		typedef robo::list::unique<node,int> map;
		typedef map::ref map_ref;
		typedef robo::list::unsorted<node> list;
		typedef list::ref ref;
	private:
		ref ref_;
		list childs_;
		map_ref map_ref_;
		
		cstr name_;
		node* owner_;

		bool auto_enabled_ = false;
		node(void);

		static node& root_(void);
		static map& map_(void);
	protected:
		list& childs(void) { return childs_; }
	public:
		node* owner() { return owner_;  };
		cstr name(void) { return name_; };

		static bool begin(void);
			
		bool reconfig(void);
		virtual bool do_reconfig(void) { return true;  };

		node(cstr _name, bool _auto_enabled, node* _owner = nullptr);

		static node* find(int _key) {
			map_().find(_key);
		}
		virtual void do_enable(void) {};
		virtual void do_disable(void) {};
		void enable(bool _hand = false);
		void disable(void);
	};
	class block:  public node {
		friend class subsystem;
	public:
		struct config_s {
			int ver;
		};
		typedef robo::list::unsorted<block> list;
		typedef list::ref ref;
	private:
		ref ref_;
	protected:
		config_s& config;
	public:
		template <typename T> class output_t {
			T value_;			
		public:

			const T& value(void) const { return value_;  }

			output_t& operator = (const T& _value) {
				value_ = _value;
				return *this;
			}

			bool operator == (const T& _value) {
				return value_ == _value;
			}

			output_t(const T& _value) : value_(_value) {}
			output_t(void) : value_((T)0) {}
		};

		template <typename T> class input_t {
			output_t<T> dummy_;
			const output_t<T>* output_;
		public:
			const T& value(void) { return output_->value() ; }

			input_t(const T& _dummy) : dummy_(_dummy), output_(&dummy_) {}
			input_t(void) :  dummy_((T)0), output_(&dummy_) {}
			void link_to(const output_t<T>* _output) {
				if (_output == nullptr) {
					output_ = &dummy_;
				}
				else {
					output_ = _output;
				}
			}
		};

	protected:

		block(subsystem& _subsystem, cstr  _name, config_s& _config);

		virtual void execute(void) = 0;
	};

	class subsystem : public node, public machine::slot::delegat {
		friend class block;
	private:
		block::list blocks_;
		typedef machine::slot::delegat::ref ref;
	protected:
		subsystem(cstr  _name, bool _auto_enable, node* _owner = nullptr);
	public:
		virtual	void operator()(void);
		virtual	void do_execute(void){};
	};

	class prioritet_subsystem : public subsystem {
		machine::slot::delegat::ref ref_;
	public:
		virtual void do_enable(void) { robo::system::guard g__; machine::slot::delegat::attach(ref_, machine::slot::kind::priority); };
		virtual void do_disable(void) { robo::system::guard g__; ref_.dettach();  };
		prioritet_subsystem(cstr  _name, node* _owner = nullptr) : subsystem(_name,true, _owner), ref_(*this){};
	};

	class frontend_subsystem : public subsystem {
		machine::slot::delegat::ref ref_;
	public:
		virtual void do_enable(void) { robo::system::guard g__; machine::slot::delegat::attach(ref_, machine::slot::kind::frontend); };
		virtual void do_disable(void) { robo::system::guard g__; ref_.dettach(); };
		frontend_subsystem(cstr  _name, node* _owner = nullptr) : subsystem(_name, true, _owner), ref_(*this) {};
	};


	class periodic_subsystem : public subsystem {
		machine::slot::delegat::ref ** refs_ = nullptr;
		size_t ref_count_ = 0;
	public:
		virtual void do_enable(void) {
			machine::slot::delegat::ref** pref = refs_;
			for (size_t n = 0; n < ref_count_; ++n, ++pref) {
				machine::slot::delegat::attach(**pref, n);
			}
		};
		virtual void do_disable(void) {
			machine::slot::delegat::ref ** pref = refs_;
			for (size_t n = 0; n < ref_count_; ++n, ++pref) {
				(*pref)-> dettach();
			}
		};
		periodic_subsystem(cstr  _name, std::initializer_list<int> _index, node* _owner = nullptr) : subsystem(_name, true, _owner) {
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
				delete *pref;
			}
			delete[] refs_;
		}
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

	template < typename S > class block_t : public  block {		
	protected: 
	public:
		block_t(subsystem & _subsystem, cstr  _name, S & _config) : block (_subsystem, _name, reinterpret_cast<config_s& >( _config)) {};
		//S - это старая добрая сишная структура
		virtual bool do_reconfig(void) {
			return applay(reinterpret_cast<S&> (block::config) );
		}

		virtual bool applay( const  S& _config ) = 0;
	};

}
#endif
