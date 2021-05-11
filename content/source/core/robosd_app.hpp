#ifndef __robosd_app_hpp
#define __robosd_app_hpp

#include "core/robosd_log.hpp"
#include "core/robosd_list.hpp"
#include "core/robosd_common.hpp"
#include "core/robosd_string.hpp"
#include "core/robosd_system.hpp"


namespace robo {
	namespace app {
		class ROBO_EXPORT component {
		public:
			enum class state { unknown = -2, panic = -1, clean = 0, stopped, startup, execute, shutdown };
		private:
			typedef  ::robo::list::unsorted<component> list;
			typedef  ::robo::list::unique<component,int> map;
			typedef  list::ref ref;
			typedef  map::ref mref;
			component* owner_ = nullptr;
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
			int id(void) { return index_ref_.key();  }
			virtual bool do_load(void);
			virtual void do_clean(void);
			virtual result do_startup(void) { return result::complete; }
			virtual result do_shutdown(void) { return result::complete; }
			virtual bool do_start(void) { return true; }
			virtual void do_stop(void) { }
			virtual void do_panic(void) { }
			void panic(void);
			bool load(void);
			void clean(void);
			result startup(void);
			result shutdown(void);
			bool start(void);
			void stop(void);
			bool init(cstr _name, component* _owner);
			component(cstr _name, component* _owner);			
		public:
			cstr name(void) { return name_; }
			void path(string& _path);
			state actual_state(void) {	return actual_state_; }
			const cstr alias(void) { return alias_.length()== 0 ? name_ : alias_.c_str();  };

			component(void);
			virtual ~component(void);

			static component* find(cstr _path) { return index_().find(fast_hash(_path)); }
		};

		template <class C > class ROBO_EXPORT injection: public component{
			C& owner_;
		protected:
			virtual bool do_load(void) { ROBO_LRET(owner_.C::node_load()); }
			virtual void do_clean(void) { owner_.C::node_clean(); }
			virtual bool do_start(void) { ROBO_LRET(owner_.C::node_start()); }
			virtual void do_stop(void) { owner_.C::node_stop();  }
			virtual void do_panic(void) { owner_.C::node_panic(); }
		public:
			injection(C& _owner, cstr _name, component* _parent) : component(_name,_parent), owner_(_owner) {}
		};

		class node {
			friend class injection<node>;
			injection<node>  node_;
		protected:
			virtual bool node_load(void) { return true; }
			virtual void node_clean(void) {}
			virtual bool node_start(void) { return true; }
			virtual void node_stop(void) {}
			virtual void node_panic(void) {}
		public:
			operator component *() { return &node_; }
			cstr name(void) { return node_.name(); }
			void path(string& _path) { node_.path(_path);  };
			component::state node_state(void) { return node_.actual_state(); }
			const cstr alias(void) { return node_.alias(); };
			node(cstr _name, component* _parent) :node_(*this,_name,_parent) {
				ROBO_ALARMN_F(_name && _name[0], "name is empty");
				ROBO_ALARMN_F(_parent != nullptr, "parent is null ('%s')", _name );
			}
		};


		class wrapper;
		class machine;

		class ROBO_EXPORT module: public component {
		protected:
			friend class wrapper;
			friend class machine;
			virtual void frontend_loop(void) = 0;
			virtual void backend_loop(void) = 0;
			module(cstr _name) : component(_name,nullptr) {}
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
		
		class  ROBO_EXPORT machine: public component {
		private:
			friend class wrapper;
			bool terminated_ = true;
#if ROBO_APP_DEBUG_LOG_ENABLED == 1
			bool begin_(cstr _ini, log::print_f _print);
#else
			bool begin_(cstr _ini);
#endif
			void finish_(void);
			void stop_(void);
			bool start_(void);

			void machine_(void);
			void frontend_loop_(void);
			void backend_loop_(void);
			bool terminated__(void);

			enum class req_state  { start, stop };

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
#if ROBO_APP_DEBUG_LOG_ENABLED == 1
			static inline bool begin(cstr _ini, log::print_f _print) { return root().begin_(_ini, _print); }
#else
			static inline bool begin(cstr _ini) { return instance().begin_(_ini); }
#endif

			static inline void finish(void) { root().finish_(); }
			static inline void  stop(void) { root().stop_(); };
			static inline void  start(void) { root().start_(); };

			static inline void  backend_loop(void) { root().backend_loop_(); };
			static inline void  frontend_loop(void) { root().frontend_loop_(); };
			static inline bool  terminated(void) { return root().terminated__(); }
		};

#endif
	}
}




#endif