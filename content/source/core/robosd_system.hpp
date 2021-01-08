#ifndef __robo_system_hpp
#define  __robo_system_hpp
#include "core/robosd_common.hpp"

#ifndef ROBO_APP_SYSTEM_ENABLED 
#define ROBO_APP_SYSTEM_ENABLED  0
#endif

#if ROBO_APP_SYSTEM_ENABLED == 1

#ifndef ROBO_APP_SYSTEM_MULTYTHRAD_ENABLED
#define ROBO_APP_SYSTEM_MULTYTHRAD_ENABLED 0
#endif 

#ifndef ROBO_APP_SYSTEM_TIME_ENABLED
#define ROBO_APP_SYSTEM_TIME_ENABLED 0
#endif 

#ifndef ROBO_APP_SYSTEM_RANDOM_ENABLED
#define ROBO_APP_SYSTEM_RANDOM_ENABLED 0
#endif 

namespace robo{
	class system{
	public:
#if ROBO_APP_SYSTEM_MULTYTHRAD_ENABLED == 1
		//останавливает ядро системы
		class ROBO_EXPORT guard{
			void * context_ = nullptr;
		public:
			guard(void);
			~guard(void);
		};
		
		class ROBO_EXPORT critical{
		public:
			critical(void);
			~critical(void);
		};
#endif

		void begin(void);
		void finish(void);

		class ROBO_EXPORT os{
			friend class system;
#if ROBO_APP_SYSTEM_MULTYTHRAD_ENABLED == 1
			friend class guard;
			static void *  enter(void);
			static void leave(void *);
			static void lock(void);
			static void unlock(void);
			static void critical_lock(void);
			static void critical_unlock(void);
#endif

#if ROBO_APP_SYSTEM_TIME_ENABLED == 1
			static time_us_t realtime_us(void);
#endif
			static void begin(void);
			static void finish(void);
		};

		class ROBO_EXPORT app{
			friend class system;
			static void begin(void);
			static void finish(void);
#if ROBO_APP_SYSTEM_MULTYTHRAD_ENABLED == 1
			static bool is_backend(void);
#endif
		public:
			static time_us_t time_us(void);
			static time_ms_t time_ms(void);
			static time_us_t period_us(void);
#if ROBO_APP_SYSTEM_MULTYTHRAD_ENABLED == 1
			static void yeld (void); //вернуть контекст
			static void wakeup(void); //разбудить систему
#endif
#if ROBO_APP_SYSTEM_RANDOM_ENABLED == 1
			static random_t rand(random_t _max);
#endif
		public:
			static void crash(char const * _file, char const * _function, int _line);
		};
	};
}
#endif

#endif




