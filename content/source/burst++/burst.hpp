#ifndef burst_hpp
#define burst_hpp

#include "burst++/burst_common.hpp"
#include "burst++/dev.front.hpp"
#include "core/robosd_list.hpp"
#include "core/robosd_system.hpp"
#include "prf/led.hpp"
#include "core/robosd_delegat.hpp"
#include "burst++/vartree.hpp"
//#include "burst++/burst_signal.hpp"
namespace burst {
	/**
	@brief  Структура - описатель режима работы
	*/
	/*

	*/
	typedef robo::time_us_t time_us_t;
	typedef robo::time_ms_t time_ms_t;
	static inline time_us_t time_us(void) {
		#if ROBO_APP_SYSTEM_ENABLED
		return  ::robo::system::time_us();
		#else
		return 0;
		#endif
	}
	static inline time_ms_t time_ms(void) {
		#if ROBO_APP_SYSTEM_ENABLED
		return  ::robo::system::time_ms();
		#else
		return 0;
		#endif
	}

	class request {
		void front_perfom_(void);
		void backend_perfom_(void);
		enum class statuses { none = 0, query = 1, confirm = 2, panic = -1 };
		statuses status_ = statuses::none;
	protected:
		::robo::delegat::ref<void>* query = nullptr;
		::robo::delegat::ref<void>* confirm = nullptr;
	public:
		void post(void);
		static void front_perfom(void);
		static void backend_perfom(void);
	};
	
	class dev {
		friend class board;
	public:
		typedef robo::list::unique<dev, int> map;
		typedef map::ref ref;
		typedef front::dev::action_s action_s;
		typedef front::dev::feedback_s feedback_s;

		struct present_s {
			uint32_t mode;
			bool action_actual;
			uint32_t panic;
			#if BURST_PANICS_MASTER_LOST_ENABLED == 1
			time_us_t master_alive_tm;
			bool master_exists;
			#endif
		};
				
		struct config_s {
			int tag;
			#if BURST_PANICS_MASTER_LOST_ENABLED == 1
			time_us_t alive_period_us;
			#endif
		};
		#if BURST_VAR_ENABLED == 1
		virtual void varreg(void) {};
		#endif

		#if BURST_PANICS_MASTER_LOST_ENABLED == 1
		#define 	BURST_PANICS_MASTER_LOST_CO(a) ,a##_ALIVE_PERIOD_US 
		#else
		#define 	BURST_PANICS_MASTER_LOST_CO(a)
		#endif

		#define DEV_CONFIG(a) DEV_CONFIG_(a)
		#define DEV_CONFIG_(a)\
		{\
			a##_TAG\
			BURST_PANICS_MASTER_LOST_CO(a)\
		}

		/**
		@brief Структура. Базовое устройство.
		*/
		



		class  mode {
			friend class dev;
		public:
			typedef robo::list::unique<mode, int> map;
			typedef map::ref ref;
		protected:
			virtual void	applay_action(void) = 0;
			virtual void	start(void) = 0;
			virtual void	stop(void) = 0;
			virtual void	loopA(void) = 0;
			virtual void	loopB(void) = 0;
			virtual void	loopC(void) = 0;
			virtual void	frontend_loop(void) = 0;
		private:
			ref ref_;
			dev& dev_;
		public:
			mode(int _id, dev& _dev);
		};
		
		friend class mode;
		class request : public ::burst::request {
		protected:
			dev & owner;
			request(dev& _owner) : owner(_owner) {}
		};

	private:
		ref ref_;
		mode* actual_mode_ = nullptr;
		mode::map modes_;
		config_s& config_;
		action_s& action_;
		feedback_s& feedback_;
		present_s& present_;
	protected:
		dev(
			int _dev_id
			, config_s& _config
			, present_s& _present
			, action_s& _action
			, feedback_s& _feedback
		);

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
			return reinterpret_cast <typename T::feedback_s&>(feedback_);
		}
		
		#define DEV_ALIEN_PRESENT_S(T,d,s) typename T::present_s& s = d.dev::template present<typename  T::present_s>()
		#define DEV_PRESENT_S(p) present_s& p= dev::template present<present_s>()
		#define DEV_CONFIG_S(c) const config_s& c= dev::template config<config_s>()
		#define DEV_ACTION_S(a) action_s& a= dev::template action<action_s>()
		#define DEV_FEEDBACK_S(f) feedback_s& f= dev::template feedback<feedback_s>()

		#if BURST_PROTECTION_ENABLED == 1
		virtual void realtime_protection(void) = 0;
		virtual void frontend_protection(void) = 0;
		#endif
		void raise_panic(uint32_t _flag);
		void reset_panic(uint32_t _flag);
		#if BURST_PANICS_MASTER_LOST_ENABLED == 1
		void master_alive(void);
		#endif
		virtual void reset(void) = 0;
		virtual void start(void) = 0;
		virtual void realtime_loop(void) = 0;
		virtual void frontend_loop(void) = 0;
		virtual void  on_perform_panic(void) {}
		void  perform_panic(void);
		private:
			void switch_to_idle_(void);
	};
	

	/*
	class idle_mode_c: public dev::mode {
		virtual void	applay_action(void) {};
		virtual void	start(void) {};
		virtual void	stop(void) {};
		virtual void	loopA(void) {};
		virtual void	loopB(void) {};
		virtual void	loopC(void) {};
		virtual void	frontend_loop(void) {};
	} idle_mode;
	*/

	class board {
		bool startuped_ = false;
	public:
		struct config_s {
			int vercion;
			struct {
				#if BURST_PROTECTION_ENABLED == 1
				time_us_t reset_timeout_us;
				#if BURST_PANICS_BOARD_TEMPER_ENABLED == 1 
				hyst_t<short> temp_pp;
				#endif
				#if BURST_PANICS_BOARD_VOLTAGE_ENABLED == 1 
				short overvoltage_pp;
				short lovoltage_pp;
				#endif
				#endif
				#if BURST_PANICS_BOARD_CURRENT_ENABLED == 1 
				short overcurrent_pp;
				short locurrent_pp;
				#endif
			} panics;
		} * config_;

		#if BURST_VAR_ENABLED == 1
		static void varreg(void);
		#endif

		#if BURST_PANICS_BOARD_TEMPER_ENABLED ==1 && BURST_PROTECTION_ENABLED == 1
		#define BURST_PANICS_BOARD_TEMPER_CO()\
			,{\
				BURST_PANICS_BOARD_TEMPER_OVERHI_PP\
				, BURST_PANICS_BOARD_TEMPER_HI_PP\
				, BURST_PANICS_BOARD_TEMPER_LO_PP\
				, BURST_PANICS_BOARD_TEMPER_ULTRALO_PP\
			}
		#else
		#define BURST_PANICS_BOARD_TEMPER_CO()
		#endif

		#if BURST_PANICS_BOARD_VOLTAGE_ENABLED ==1 && BURST_PROTECTION_ENABLED == 1
		#define BURST_PANICS_BOARD_VOLTAGE_CO()\
			, BURST_PANICS_BOARD_VOLTAGE_OVERHI_PP\
			, BURST_PANICS_BOARD_VOLTAGE_ULTRALO_PP
		#else
		#define BURST_PANICS_BOARD_VOLTAGE_CO()
		#endif

		#if BURST_PANICS_BOARD_CURRENT_ENABLED ==1 && BURST_PROTECTION_ENABLED == 1
		#define BURST_PANICS_BOARD_CURRENT_CO()\
			,BURST_PANICS_BOARD_OVERCURRENT_PP\
			,BURST_PANICS_BOARD_LOCURRENT_PP
		#else
		#define BURST_PANICS_BOARD_CURRENT_CO()
		#endif
		#if BURST_PROTECTION_ENABLED == 1
		#define BURST_PANICS_BOARD_RESET_TIMEOUT_CO() BURST_PANICS_BOARD_RESET_TIMEOUT_PP
		#else
		#define BURST_PANICS_BOARD_RESET_TIMEOUT_CO()
		#endif
		#define BURST_CONFIG() {\
				BURST_VERCION\
				,{\
					BURST_PANICS_BOARD_RESET_TIMEOUT_CO()\
					BURST_PANICS_BOARD_TEMPER_CO()\
					BURST_PANICS_BOARD_VOLTAGE_CO()\
					BURST_PANICS_BOARD_CURRENT_CO()\
				}\
			}


		enum { slot_count = ROBO_APP_BURST_SLOT_COUNT };
		class slots;
		class slot {
		public:
			enum class kind { begin = slot_count, start, startup, realtime, backend, frontend, raise_fault };
			class delegat : public ::robo::delegat::ref<void> {
				friend class link;
			public:
				typedef ::robo::list::unsorted<delegat> list;
				typedef list::ref ref;
				
				
				typedef ::robo::list::unidir::store_t<ref, delegat & > links;

				virtual ~delegat(void) {}
				void attach(slot::kind _kind, delegat* _prev = nullptr);
				static void attach(ref& _ref, int _index, delegat* _prev);
				static void attach(ref& _ref, slot::kind _kind, delegat* _prev);
				void attach(int _index, delegat* _prev = nullptr);

				void attach(const int* _index, int _count, delegat* _prev = nullptr) {
					for (int i = 0; i < _count; ++i, ++_index) {
						attach(*_index, _prev);
					}
				}
				template <size_t N> void attach(int(&index)[N], delegat* _prev = nullptr) {
					attach(index, N, _prev);
				}
				void attach(std::initializer_list<int> _index, delegat* _prev = nullptr) {
					for (const auto x : _index) {
						attach(x, _prev);
					}
				}
				bool attached(void) { return links_.count() > 0;  }
				void dettach(slot::kind _kind);
				void dettach(int _index);
				void dettach() {
					links_.free();
				}
				void dettach(const int* _index, int _count, delegat* _prev) {
					for (int i = 0; i < _count; ++i, ++_index) {
						dettach(*_index);
					}
				}
				template <size_t N> void dettach(int(&index)[N], delegat* _prev) {
					dettach(index, N, _prev);
				}
				void dettach(std::initializer_list<int> _index, delegat* _prev) {
					for (const auto x : _index) {
						dettach(x);
					}
				}
				void startup_sucess(void) {
					dettach(burst::board::slot::kind::startup);
				}
			private:
				links links_;
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

			template<typename C> class member : public ::robo::delegat::rmember<delegat, C, void> {
				typedef ::robo::delegat::rmember<delegat, C, void> A;
			public:
				member(C& _instance, void (C::* _member) (void)) : A(_instance, _member) {}
				member(slot::kind _kind, C& _instance, void(C::* _member)(void), delegat* _prev = nullptr) : A(_instance, _member) {
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
			friend class board;
			friend class slots;
			delegat::list delegats_;
			bool isempty(void) { return delegats_.count() == 0; }
			slot(void);
			~slot(void);
			void free(void);
			void execute(void);
			delegat::ref* locate(delegat* _d) { return (delegat::ref*)delegats_.locate(_d); }
		};

		class slots {
			friend class slot;
			friend class board;
			slot begin;
			slot start;
			slot startup;
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
		dev::map & devs_ref_;
		int slot_index_;
		static slots& slots_(void);
		static dev::map& devs_(void);
		//		void begin_(void);
		void begin_(time_us_t _period_us);
		#if ROBO_APP_MEXO_REALTIME_SLOT_ENABLE == 1
		void realtime_loop_(void);
		#endif
		void realtime_loop_(void);
		void backend_loop_(void);
		void frontend_loop_(void);
		void raise_fault_(void);
		void setup_(config_s& _config) { config_ = &_config; }
		void reset_(void);
		static board instance_;
	public:
		board(void);
		~board(void);
		//static void start(time_us_t _period_us) { instance_.start_(_period_us); }
		static void begin(time_us_t _period_us) { instance_.begin_(_period_us); }
		#ifdef ROBO_APP_MEXO_SAMPLE_US
		//		static void start(void) { instance_.start_(ROBO_APP_MEXO_SAMPLE_US); }
		static void begin(void) { instance_.begin_(ROBO_APP_MEXO_SAMPLE_US); }
		#endif		
		#if ROBO_APP_BURST_REALTIME_SLOT_ENABLE == 1
		static void realtime_loop(void) { instance_.realtime_loop_(); }
		#endif
		static void setup(config_s& _config) { instance_.setup_(_config); }
		static void backend_loop(void) { instance_.backend_loop_(); }
		static void frontend_loop(void) { instance_.frontend_loop_(); }
		static int slot_index(void) { return instance_.slot_index_; }
		static void raise_fault(void) { instance_.raise_fault_(); }
		static void reset(void) { instance_.reset_(); }

		
		struct present_s {
			uint32_t panics;
			time_us_t last_panic_us;
		} &present_;

		//void perform_panic(void);
		void raise_panic(uint32_t _flag);
		void reset_panic(uint32_t _flag);
		uint32_t panics(void);
		void reset_panics(void);
	private:
		#if BURST_PROTECTION_ENABLED == 1
		void realtime_protection(void);
		void frontend_protection(void);
		#endif

	public:
		#if BURST_PANICS_BOARD_TEMPER_ENABLED == 1
		short temper_get_lo_pp(void);
		short temper_get_hi_pp(void);
		#endif

		#if BURST_PANICS_BOARD_VOLTAGE_ENABLED == 1 
		short voltage_get_pp(void);
		#endif

		#if BURST_PANICS_BOARD_CURRENT_ENABLED == 1
		short current_get_pp(void);
		#endif
	};
	#if BURST_DEBUG_TP_ENABLED == 1
	class tp_driver {
		//реализацию принудительно делегируем в perephery проекта
	protected:
		void on(void);
		void off(void);
	};
	typedef robo::prf::led_t<tp_driver>  tp;
	#endif

	template< class K, class D> class timer_t : public D {
	public:
		using tm = time_us_t;
	private:
		tm last_ = tm(0);
		tm period_ = tm(0);
		bool once_;
		bool started_;
		const K key_;
	protected:
		virtual void operator ()(void) {
			if (started_) {
				::robo::time_us_t now = time_us();
				if (now - last_ > period_) {
					last_ = now;
					D::operator () ();
					if (once_) {
						stop();
					}
				}
			}
		}
	public:
		template <typename ... Args> timer_t(const K& _key, tm _period, bool _once, Args ... args) : D(_key, args...), key_(_key) {
			start(_period, _once);
		}
		template <typename ... Args> timer_t(const K& _key, tm _period, Args ... args) : D(_key, args...), key_(_key) {
			start(_period);
		}
		template <typename ... Args> timer_t(const K& _key, Args ... args) : D(_key, args...), key_(_key) {}

		void start(tm _period, bool _once = false) {
			period_ = _period;
			once_ = _once;
			last_ = time_us();
			//D::attach(key_);
			started_ = true;
		}
		void stop(void) {
			//D::dettach(key_);
			started_ = false;
		}
		bool started(void) { return started_; }
		~timer_t(void) {
			D::dettach(key_);
		}
	};

	//template< class K, class D> class box_t : public D


};

#if BURST_DEBUG_TP_ENABLED == 1
#define debug_tp_on(n)  burst::tp::on(n)
#define debug_tp_off(n)  burst::tp::off(n)
#define debug_set_verb(n)  burst::tp::set_verb(n)
#else

#define debug_tp_on(n)
#define debug_tp_off(n)
#define debug_set_verb(n) 
#endif


#endif