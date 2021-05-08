#ifndef __robo_system_hpp
#define  __robo_system_hpp
#include "core/robosd_common.hpp"
#ifndef ROBO_APP_SYSTEM_ENABLED 
#define ROBO_APP_SYSTEM_ENABLED  0
#endif
#if ROBO_APP_SYSTEM_ENABLED == 1
namespace robo {
	namespace system {

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

		class fall {
		public:
			fall(void);
			~fall(void);
		};

		static  void enable(void);
		static  void disable(void);

		class os {
			friend class guard;
			friend class fall;
			static bool is_frontend(void);
			static bool is_backend(void);
			static void * enter(void);
			static void leave(void * _context);
			static void lock(void);
			static void unlock(void);
			static void fall(void);
			static void comeback(void);

		 public:
			static time_us_t time_us(void);
			static time_us_t realtime_us(void);
			static time_ms_t time_ms(void);
			static random_t rand(random_t _max);
			static void wakeup(void);
			static time_us_t period_us(void);
			static void sleep(void); //вернуть контекст
		};

	}
}
#endif
#endif
