#ifndef __robo_application
#define  __robo_application
#include "core/robosd_common.hpp"

/*
#include "core/robosd_app_common.hpp"
#include "core/robosd_list.hpp"
#include "core/robosd_cstring.hpp"
#include "core/robosd_mem.h"
#include "net/serial/robosd_serial.hpp"


	
namespace robo{
	namespace app{
		class machine;
		class  ROBO_EXPORT wrapper{
		public:
			typedef list::ref<wrapper> ref;
			friend class machine;

			typedef module * (*query_f)(void);
			typedef void(*release_f)(module *);

		private:
			ROBO_REDECLARE_NEW
			void * handle_;
			module * module_;
			ref core_ref_;
			robo::string lib_;
			bool  load_(const robo_string_t  _key);
			void release_();
			void  free_();
			result_t startup_(void);
			result_t shutdown_(void);
			bool start_(void);
			bool stop_(void);
			wrapper(void);
			~wrapper(void);
			static bool  load(const robo_string_t  _key);
			friend class assembly;
		public:
		};

		class  ROBO_EXPORT machine {
		private:
			friend class wrapper;
			list::map<wrapper> modules_;
#if ROBO_DEBUG_LOG_ENABLED == 1
			bool begin_(const robo_string_t _ini, robo_log_print_f _print);
#else
			bool begin_(const robo_string_t _ini);
#endif
			void  finish_(void);
			bool  load_(void);
			void  free_(void);
			bool  start_(void);
			bool  stop_(void);
			bool  do_stop_(void);
			bool  do_start_(void);

			result_t startup_(void);
			result_t shutdown_(void);
			void machine_(void);
			void private_loop_(void);
			bool background_loop_(void);

			//bool  check_state_(module::state_t _state);
			static machine & instance(void);
			typedef enum{ START, STOP, ABORT } req_state_t;
			state_t wait_state;
			req_state_t req_state;
			machine();
			bool terminated_;
			static result_t startup(void){ return instance().startup_(); };
			static result_t shutdown(void){ return instance().shutdown_(); };
		public:
#if ROBO_DEBUG_LOG_ENABLED == 1
			static inline bool begin(const robo_string_t _ini, robo_log_print_f _print) { return instance().begin_(_ini, _print); }
#else
			static inline bool begin(const robo_string_t _ini){ return instance().begin_(_ini);}
#endif

			static inline void finish(){ instance().finish_(); }

			//static inline bool  start(){ return instance().start_(); };
			static inline bool  stop(){ return instance().stop_(); };
			static inline void  private_loop(){ instance().private_loop_(); };
			static inline bool  background_loop(){ return instance().background_loop_(); };
			//static inline bool  check_state(module::state_t _state){ return instance().check_state_(_state); };
			static inline bool  terminated(){ return instance().terminated_; }
		};
	}
}
*/

#endif


