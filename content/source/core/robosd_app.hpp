#ifndef __robosd_app_hpp
#define __robosd_app_hpp

#include "core/robosd_log.hpp"
#include "core/robosd_list.hpp"
#include "core/robosd_common.hpp"
#include "core/robosd_string.hpp"
#include "core/robosd_system.hpp"
#if ROBO_APP_MODULE_ENABLED  == 1

namespace robo {
	namespace app {
		class ROBO_EXPORT node {
		public:
			enum class state { unknown = -2, panic = -1, clean = 0, stopped, startup, execute, shutdown };
		private:
			typedef  ::robo::list::unsorted<node> list;
			typedef  ::robo::list::unique<node, int> map;
			typedef  list::ref ref;
			typedef  map::ref mref;
			robo::string store_name_;
			node* owner_ = nullptr;
			ref ref_;
			mref own_ref_;
			mref index_ref_;
			list disabled_;
			list stopped_;
			list startupped_;
			list active_;
			list shutdowned_;
			cstr name_;
			string alias_;
			state actual_state_ = state::unknown;
			static map& index_(void);
		protected:
			map owned;
			int id(void) { return index_ref_.key(); }
			virtual bool do_load(void);
			virtual void do_clean(void);
			virtual result do_startup(void) { return result::complete; }
			virtual result do_shutdown(void) { return result::complete; }
			virtual bool do_start(void) { return true; }
			virtual void do_stop(void) {}
			virtual void do_panic(void) {}
			void panic(void);
			result startup(void);
			result shutdown(void);
			bool start(void);
			void stop(void);
			void backend_loop(void);
			void frontend_loop(void);
			virtual void do_backend_loop(void) {};
			virtual void do_frontend_loop(void) {};

			bool init(cstr _name, node* _owner);
			node(cstr _name, node* _owner);
		public:
			bool load(void);
			void clean(void);
			cstr name(void) { return name_; }
			void path(string& _path);
			state actual_state(void) { return actual_state_; }
			const cstr alias(void) { return alias_.length() == 0 ? name_ : alias_.c_str(); };

			node(void);
			virtual ~node(void);

			static node* find(cstr _path) { return index_().find(hash(_path)); }
		};

		class wrapper;
		class machine;

		class ROBO_EXPORT module : public node {
		protected:
			friend class wrapper;
			friend class machine;
			//virtual void frontend_loop(void) = 0;
			//virtual void backend_loop(void) = 0;
			module(cstr _name) : node(_name, nullptr) {}
		};

		#if ROBO_APP_LIB_ENABLED == 1
		class  ROBO_EXPORT wrapper {
			friend class machine;

			typedef  ::robo::list::unique<wrapper, int> map;
			typedef  map::ref ref;

			typedef module* (*query_f)(void);
			typedef void(*release_f)(module*);

			void* handle_ = nullptr;
			module* module_ = nullptr;
			robo::string lib_;

			ref ref_;

			wrapper(void);
			~wrapper(void);

			bool begin_(cstr _key);

			static bool begin(cstr _key);
			void finish(void);
		};

		class  ROBO_EXPORT machine : public node {
		private:
			friend class wrapper;
			bool terminated_ = true;
			bool begin_(cstr _ini);
			void finish_(void);
			void stop_(void);
			bool start_(void);

			void machine_(void);
			void frontend_loop_(void);
			void backend_loop_(void);
			bool terminated__(void);

			enum class req_state { start, stop };

			req_state req_state_ = req_state::stop;
			wrapper::map wrappers_;
			machine(void) {};
			virtual ~machine(void) {};
		protected:
			virtual bool do_load(void);
			virtual void do_clean(void);
			virtual result do_shutdown(void);

		public:
			static machine& root(void);
			static inline bool begin(cstr _ini) { return root().begin_(_ini); }

			static inline void finish(void) { root().finish_(); }
			static inline void  stop(void) { root().stop_(); };
			static inline bool  start(void) { return root().start_(); };

			static inline void  backend_loop(void) { root().backend_loop_(); };
			static inline void  frontend_loop(void) { root().frontend_loop_(); };
			static inline bool  terminated(void) { return root().terminated__(); }
		};

		#endif
	}
}
#endif

#endif