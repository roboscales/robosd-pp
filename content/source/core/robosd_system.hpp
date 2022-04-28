#ifndef  robo_system_hpp
#define  robo_system_hpp
#include "core/robosd_common.hpp"
#include "core/robosd_log.hpp"
#include "core/robosd_list.hpp"
#include "core/robosd_delegat.hpp"

#ifndef ROBO_APP_ENV_TYPE 
#define ROBO_APP_ENV_TYPE  ROBO_APP_TYPE_NONE
#endif

#ifndef ROBO_APP_INI_TYPE 
#define ROBO_APP_INI_TYPE  ROBO_APP_TYPE_NONE
#endif

#ifndef ROBO_APP_LIB_TYPE 
#define ROBO_APP_LIB_TYPE  ROBO_APP_TYPE_NONE
#endif

#ifndef ROBO_APP_ALLOC_TYPE 
#define ROBO_APP_ALLOC_TYPE  ROBO_APP_TYPE_NONE
#endif

#ifndef ROBO_APP_PRINT_TYPE
#define ROBO_APP_PRINT_TYPE ROBO_APP_TYPE_NONE
#endif

#ifndef ROBO_APP_LOG_PRINT_TYPE
#define ROBO_APP_LOG_PRINT_TYPE ROBO_APP_TYPE_NONE
#endif

#ifndef ROBO_APP_FORMATING_TYPE
#define ROBO_APP_FORMATING_TYPE  ROBO_APP_TYPE_NONE
#endif

#ifndef ROBO_APP_SHARED_TYPE
#define ROBO_APP_SHARED_TYPE  ROBO_APP_TYPE_NONE
#endif

#ifndef ROBO_APP_CONSOL_TYPE
#define ROBO_APP_CONSOL_TYPE ROBO_APP_TYPE_NONE
#endif

#define ROBO_APP_CONSOL_ENABLED  (ROBO_APP_CONSOL_TYPE != ROBO_APP_TYPE_NONE)

#define ROBO_APP_ENV_ENABLED  (ROBO_APP_ENV_TYPE != ROBO_APP_TYPE_NONE)
#define ROBO_APP_INI_ENABLED  (ROBO_APP_INI_TYPE != ROBO_APP_TYPE_NONE)
#define ROBO_APP_LIB_ENABLED  (ROBO_APP_LIB_TYPE != ROBO_APP_TYPE_NONE)
#define ROBO_APP_SHARED_ENABLED (ROBO_APP_SHARED_TYPE != ROBO_APP_TYPE_NONE)
#define ROBO_APP_ALLOC_ENABLED (ROBO_APP_ALLOC_TYPE != ROBO_APP_TYPE_NONE)

#if ROBO_APP_SYSTEM_ENABLED  == 1
namespace robo {
	/*!
	 *  Функции аппаратного окружения (system::env), которые требуют отдельной реализации для каждой платформы или ОС
	 *  и их обертки (system).
	 *  Работа разбита на два максимально развязанных уровня- frontend и backend, которые взаимодействуют посредством
	 *  очередей , через которые проходят сообщения, содержащие обработчики.
	 *  backend работает вне контекста ОС, в реальном времени. Прерывать ее можно только на самое короткое
	 *  время. например, чтобы поставить сообщение в очередь.
	 */
	class ROBO_EXPORT system {
		/*!
		 *  Выделение памяти. Для
		 */
	#if ROBO_APP_ALLOC_ENABLED ==1
	public:
		struct ROBO_EXPORT mem {
			//статистика
			struct stat {
				struct {
					size_t size = 0;
					int count = 0;
				} used;
				struct {
					size_t payload = 0;
					size_t size = 0;
					int count = 0;
				} total;
			};
			static void* alloc(size_t _sz) { return system::instance_().mem_alloc_(_sz); }
			static void free(void* _memo) { system::instance_().mem_free_(_memo); }
		};
		static mem::stat& get_mem_statistic(void) { guard g__; return instance_().memstat_; }
	private:
		mem::stat memstat_;
	#endif
	private:
		static system & instance_(void);
		enum  class state { enabled = 178, unknown = -178 };
		state state_ = state::unknown;
		#if ROBO_APP_ENV_ENABLED == 1
		int lock_count_ = 0;
		int guest_count_ = 0;
		#endif
		void* enter_(void);
		void leave_(void* context_);
		void* critical_enter_(void);
		void critical_leave_(void* _context);

		#if ROBO_APP_ALLOC_ENABLED == 1
		void* mem_alloc_(size_t _sz);
		void mem_free_(void* _memo);
		#endif
		friend class critical;
		system(void);
		~system(void);
	public:
		/*!
		*  если выполняется в контексте backend - все конкурирующие потоки frontend ( в контекесте ОС )
		*  если выполняется в контексте frontend  - останавливает поток backend  и все конкурирующие потоки frontend ( в контекесте ОС )
		*/
		class ROBO_EXPORT guard {
			void* context_;
		public:
			guard(void);
			~guard(void);
		};
		/*!
		 *  если выполняется в контексте frontend  - останавливает все конкурирующие потоки frontend ( в контекесте ОС )
		 *  в backend ни вкоем случае запукать нельзя!
		 */
		class ROBO_EXPORT critical {
			void* context_;
		public:
			critical(void);
			~critical(void);
		};
		/*!
		 *  замеряем время от создания объекта
		 */
		class ROBO_EXPORT lazzyboy {
			time_us_t	 sleep_us_;
		public:
			lazzyboy(void);
			~lazzyboy(void);
			/*!
			 *  Resets the lazzyboy.
			 *
			 *      @return теперь время отсчитывается от момента вызова reset
			 */
			time_us_t	 reset(void);
			/*!
			 *  Idles the us.
			 *
			 *      @return время от создания объекта или от сброса
			 */
			time_us_t	 idle_us(void);
		};

		enum class context { backend, frontend };
		/*!
		 *  Данный поток обозначает себя, как backend
		 */
		class ROBO_EXPORT fall {
		public:
			fall(void);
			~fall(void);
		};

		#if ROBO_APP_ENV_ENABLED ==1
		/*!
		 *  Это специфичные функции для аппаратуры и компилятора
		 */
		class ROBO_EXPORT  env {
			friend class guard;
			friend class fall;
			friend class system;

			/*!
			 *  Заходим в критическую секцию
			 *
			 *      @return возвращает контекст - указатель на специфическую структуру (опционно). ее надо вернуть в critical_leave
			 */
			static void* critical_enter(void);

			/*!
			 *  Покидаем критическую секцию.
			 *
			 *      @param in _context - эту структуру выдает critical_enter
			 */
			static void critical_leave(void* _context);

			/*!
			 *  Блокируем backend
			 *
			 *      @return возвращает контекст - указатель на специфическую структуру (опционно). ее надо вернуть в leave
			 */
			static void* enter(void);

			/*!
			 *  Освобождаем backend
			 *
			 *      @param in _context  _context - эту структуру выдает critical_enter
			 */
			static void leave(void* _context);

			/*!
			 *  Защищаем  backend - ни кто его не остановит!
			 */
			static void lock(void);

			/*!
			 *  Теперь  backend можно приостановить
			 */
			static void unlock(void);

			/*!
			 *  поток обозначает, что он реализует backend
			 */
			static void fall(void);

			/*!
			 *  поток обозначает, что он перестал реализовать backend
			 */
			static void comeback(void);

			#if ROBO_APP_ALLOC_ENABLED ==1

			/*!
			 *  специфическая функция выделения памяти в куче. Работает по разному для backend и frontend
			 *
			 *      @param [in] _sz
			 *
			 *      @return возвращает указатель на выделеную память. Не смогли вызвать - возвращаем null
			 */
			static void* mem_alloc(size_t _sz);
			/*!
			 *  специфическая функция освобождения памяти в куче
			 *
			 *      @param [in,out] _memo
			 */
			static void mem_free(void* _memo);

			#endif
			#if ROBO_APP_MODULE_ENABLED == 1
			static void frontend_loop(void);
			#endif
		public:

			/*!
			 *  Returns true if the env is frontend.
			 *
			 *      @return True if frontend. False if not.
			 */
			static bool is_frontend(void);

			/*!
			 *  Returns true if the env is backend.
			 *
			 *      @return True if backend. False if not.
			 */
			static bool is_backend(void);

			/*!
			 *  Aborts the env. Просто вырубаем прилрожение, где бы оно не работало
			 */
			static void abort(void);
			#if ROBO_APP_MODULE_ENABLED == 1

			/*!
			 *  Begins the env. Вызывается автоматически, когда перед стартом frontend и backend
			 *  Здесь следует проводить инициализацию аппаратуры и специфичного ПО
			 *
			 *      @return возвращает true, если норм
			 */
			static bool begin(void);

			/*!
			 *  Finishes the env. Завершения работы аппаратуры  и специфичного ПО
			 */
			static void finish(void);
			static bool start(void);
			static void stop(void);
			static result startup(void);
			static result shutdown(void);
			static void backend_loop(void);
			#endif
			static time_us_t time_us(void);
			static time_us_t realtime_us(void);
			static time_ms_t time_ms(void);
			static random_t rand(random_t _max);
			static void wakeup(void);
			static time_us_t period_us(void);
			static void sleep(void); //вернуть контекст
			#if ROBO_APP_FORMATING_TYPE != ROBO_APP_TYPE_NONE
			static size_t sprintf(char_t* _dst, size_t _max_sz, cstr _format, va_list _args);
			#endif
			#if ROBO_APP_PRINT_TYPE != ROBO_APP_TYPE_NONE
			static void print(cstr  _s);
			#if ROBO_APP_DEBUG_LOG_ENABLED == 1
			static void print(robo::log::verb _verb, cstr _format, va_list  _args);
			#endif
			#endif
		};
		#endif

		static void frontend_loop(void);
		#if ROBO_APP_FORMATING_TYPE != ROBO_APP_TYPE_NONE
		static void printf(cstr _format, va_list _args);
		static void printf(cstr _format, ...);		
		static size_t sprintf(char_t* _dst, size_t _max_sz, cstr _format, ...);
		#endif

		#if ROBO_APP_INI_ENABLED ==1
		struct ROBO_EXPORT ini {
			static bool begin(cstr _ini);
			static cstr source(void);
			static void finish(void);
			static bool load_str(char_t* _dst, size_t _max_sz, cstr _section, cstr _key);
		};
		#endif

		#if ROBO_APP_LIB_ENABLED ==1
		struct ROBO_EXPORT lib {
			static void* proc_get(void* _handle, cstr _proc_name);
			static bool exists(cstr _lib_name);
			static void* load(cstr _lib_name);
			static void free(void* _instance);
			static bool copy(cstr _src, cstr _dst);
			static bool remove(cstr _lib_name);
		};
		#endif

		#if ROBO_APP_SHARED_ENABLED ==1
		class ROBO_EXPORT shared {
			friend class guard;
			void driver_lock(void);
			void driver_unlock(void);
			bool driver_open(cstr _path, size_t _sz);
			void driver_close(void);
			class driver;
			driver* driver_=nullptr;
		public:
			typedef list::unique< shared, int> map;
			typedef map::ref ref;			
		private:
			ref ref_;
		public:
			class guard {
				shared& owner_;
			public:
				guard(shared& _owner) : owner_(_owner) { _owner.driver_lock(); }
				~guard(void) { owner_.driver_unlock(); }
			};
			void* memo(void);
			size_t size(void);
			shared(void);
			~shared(void);
			bool open(cstr _path, size_t _sz);
			void close(void);
			static shared * find(cstr _name);
			static shared* find(int _id);

		};
		#endif

		#if ROBO_APP_CONSOL_ENABLED == 1
		class ROBO_EXPORT consol {
		private:
			static bool driver_begin(void);
			static void driver_finish(void);
		public:
			enum class event {app, keypbrd, other};
			typedef lambda<void(event)> on_break_f;
			static void stop(event _ev);
			static bool begin(const on_break_f & _on_break);
			static void finish(void);
		};
		#endif
	};
}
#endif
#endif
#if ROBO_APP_SYSTEM_ENABLED == 1
#define guard__ ::robo::system::guard g__
#define fall__ ::robo::system::fall f__
#define critical__ ::robo::system::critical c__
#if ROBO_APP_ENV_ENABLED == 1
#define is_frontend__ ::robo::system::env::is_frontend()
#define is_backend__ ::robo::system::env::is_backend()
#else
#define is_frontend__ true
#define is_backend__ true
#endif
#else
#define guard__
#define fall__ 
#define is_frontend__ true
#define is_backend__ true
#define critical__
#endif
