#ifndef __robo_system_hpp
#define  __robo_system_hpp
#include "core/robosd_common.hpp"
#ifndef ROBO_APP_SYSTEM_ENABLED 
#define ROBO_APP_SYSTEM_ENABLED  0
#endif
#ifndef ROBO_APP_INI_ENABLED 
#define ROBO_APP_INI_ENABLED  0
#endif


#if ROBO_APP_SYSTEM_ENABLED == 1
namespace robo {
	class system {
	public:
		//останавливает ядро системы
		class ROBO_EXPORT guard {
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

		class fall {
		public:
			fall(void);
			~fall(void);
		};

		class os {
			friend class guard;
			friend class fall;
			friend class system;
			static bool is_frontend(void);
			static bool is_backend(void);
			static void* enter(void);
			static void leave(void* _context);
			static void lock(void);
			static void unlock(void);
			static void fall(void);
			static void comeback(void);
			static void begin(void);
			static void finish(void);
			static void * mem_alloc(size_t _sz);
			static void mem_free(void* _memo);
		public:
#if ROBO_APP_INI_ENABLED ==1
			static bool ini_init(cstr _ini);
			static void ini_finish(void);
			static bool ini_load_str(char_t* _dst, size_t _max_sz, cstr _section, cstr _key);
#endif
			static time_us_t time_us(void);
			static time_us_t realtime_us(void);
			static time_ms_t time_ms(void);
			static random_t rand(random_t _max);
			static void wakeup(void);
			static time_us_t period_us(void);
			static void sleep(void); //вернуть контекст
			static bool sprintf(char_t* _dst, size_t _max_sz, cstr _format, va_list _args);
		};
		static void* mem_alloc(size_t _sz) { return instance_.mem_alloc_(_sz); }
		static void mem_free(void* _memo) { instance_.mem_free_( _memo );  }
		struct memstat {
			struct {
				int size = 0;
				int count = 0;
			} used;
			struct {
				int size = 0;
				int count = 0;
			} total;
		};
		static memstat& get_mem_statistic(void) { guard g__; return instance_.memstat_; }
	private:
		enum  class state { enabled = 178, unknown = -178 };
		state state_ = state::unknown;
		void* context_ = nullptr;
		int lock_count_ = 0;
		int guest_count_ = 0;
		static system instance_;
		memstat memstat_;
		void enter_(void);
		void leave_(void);
		void* mem_alloc_(size_t _sz);
		 void mem_free_(void* _memo);
		system(void);
		~system(void);
	};
}
#endif
#endif
