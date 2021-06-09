#ifndef  robo_system_hpp
#define  robo_system_hpp
#include "core/robosd_common.hpp"



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


#define ROBO_APP_ENV_ENABLED  (ROBO_APP_ENV_TYPE != ROBO_APP_TYPE_NONE)
#define ROBO_APP_INI_ENABLED  (ROBO_APP_INI_TYPE != ROBO_APP_TYPE_NONE)
#define ROBO_APP_LIB_ENABLED  (ROBO_APP_LIB_TYPE != ROBO_APP_TYPE_NONE)
#define ROBO_APP_ALLOC_ENABLED (ROBO_APP_ALLOC_TYPE != ROBO_APP_TYPE_NONE)

#if ROBO_APP_SYSTEM_ENABLED  == 1
namespace robo {
	class system {
#if ROBO_APP_ALLOC_ENABLED ==1
	public:
		struct mem {
			struct stat {
				struct {
					int size = 0;
					int count = 0;
				} used;
				struct {
					int size = 0;
					int count = 0;
				} total;
			};
			static void* alloc(size_t _sz) { return instance_.mem_alloc_(_sz); }
			static void free(void* _memo) { instance_.mem_free_(_memo); }
		};
		static mem::stat& get_mem_statistic(void) { guard g__; return instance_.memstat_; }
	private:
		mem::stat memstat_;
#endif
	private:
		enum  class state { enabled = 178, unknown = -178 };
		state state_ = state::unknown;
#if ROBO_APP_ENV_ENABLED == 1
		int lock_count_ = 0;
		int guest_count_ = 0;
#endif
		static system instance_;
		void *  enter_(void);
		void leave_(void * context_);
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
		//останавливает ядро системы
		class ROBO_EXPORT guard {
			void* context_;
		public:
			guard(void);
			~guard(void);
		};

		class ROBO_EXPORT lazzyboy {
			time_us_t	 sleep_us_;
		public:
			lazzyboy(void);
			~lazzyboy(void);
			time_us_t	 idle_us(void);
		};

		enum class context { backend, frontend };

		class ROBO_EXPORT fall {
		public:
			fall(void);
			~fall(void);
		};

		class ROBO_EXPORT critical {
			void * context_;
		public:
			critical(void);
			~critical(void);
		};

#if ROBO_APP_ENV_ENABLED ==1
		class  env {
			friend class guard;
			friend class fall;
			friend class system;

			static void* critical_enter(void);
			static void critical_leave(void* _context);

			static void* enter(void);
			static void leave(void* _context);
			static void lock(void);
			static void unlock(void);
			static void fall(void);
			static void comeback(void);
#if ROBO_APP_ALLOC_ENABLED ==1
			static void* mem_alloc(size_t _sz);
			static void mem_free(void* _memo);
#endif
		public:
			static bool is_frontend(void);
			static bool is_backend(void);
			static void abort(void);
			static bool begin(void);
			static void finish(void);
			static bool start(void);
			static void stop(void);
			static result startup(void);
			static result shutdown(void);
			static void frontend_loop(void);
			static void backend_loop(void);

			static time_us_t time_us(void);
			static time_us_t realtime_us(void);
			static time_ms_t time_ms(void);
			static random_t rand(random_t _max);
			static void wakeup(void);
			static time_us_t period_us(void);
			static void sleep(void); //вернуть контекст
			static size_t sprintf(char_t* _dst, size_t _max_sz, cstr _format, va_list _args);
			static void print( cstr  _s);
		};
#endif
		static void printf( cstr _format, va_list _args);
		static void printf( cstr _format, ...);
		static size_t sprintf(char_t* _dst, size_t _max_sz, cstr _format, ...);

#if ROBO_APP_INI_ENABLED ==1
		struct ini {
			static bool begin(cstr _ini);
			static void finish(void);
			static bool load_str(char_t* _dst, size_t _max_sz, cstr _section, cstr _key);
		};
#endif
#if ROBO_APP_LIB_ENABLED ==1
		struct lib {
			static void * proc_get(void* _handle, cstr _proc_name);
			static bool exists(cstr _proc_name);
			static void* load(cstr _lib_name);
			static void free(void* _instance);
		};
#endif

	};
}
#endif
#endif
