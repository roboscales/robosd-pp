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
			string store_name_;
			string display_alias_;
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

			bool init(cstr _name, node* _owner);
			node(cstr _name, node* _owner);
		private:
			#ifndef ROBO_APP_NODE_PATH_SIZE
			#define ROBO_APP_NODE_PATH_SIZE 1024
			#endif 
			struct path_root {
				enum { size = ROBO_APP_NODE_PATH_SIZE };
				char_t* buf_ = nullptr;
				char_t* top_ = nullptr;
				size_t space_ = 0;
				static path_root& ref(void) {
					static path_root path_root_;
					return path_root_;
				}
				struct defaults {
					string value;
					int use_count = 1;
					typedef ::robo::list::unsorted<defaults> stack;
					stack::ref ref;
					defaults(cstr _value) : ref(*this)  {
						ref.attach_to( path_root::ref().defaults_stack );
						value = _value;
					}
					void use() { use_count++; }
					void release() { 
						if (use_count > 0) { 
							use_count--; 
							if (use_count == 0) 
								delete this; 
						} 
					}
				};
				defaults::stack defaults_stack;
				static cstr defaults_path(void) {
					const string* p = ref().defaults_path_ptr();
					if (p!=nullptr) {
						return p->c_str();
					}
					else {
						return nullptr;
					}
				}
				const string *  defaults_path_ptr(void) {
				//	path_root& root_ = path_root::ref();
					if (defaults_stack.count() > 0) {
						return & defaults_stack.last()->owner().value;
					}
					else {
						return nullptr;
					}
				}
				bool try_use_last(void) {
					if (defaults_stack.count() > 0) {
						defaults_stack.last()->owner().use();
						return true;
					}
					else {
						return false;
					}
				}
			};
			char_t* store_top_ = nullptr;
			void path_push_(void);
			void path_pop_(void);
		protected:
			friend class path;
			struct path {
				node& node_;
			public:
				path(node& _node);
				~path(void);
				cstr value(void);
				cstr defaults(void);
			};
			cstr current_path(void);
			cstr defaults_path(void);
		public:			
			//const node* owner_ptr(void) { return owner_; }
			bool load(void);
			void clean(void);
			void reboot(void) {
				robo::system::guard g__;
				stop();
				clean();
				ROBO_VBREAKN(load());
				ROBO_VBREAKN(start());
			}
			cstr name(void) { return name_; }
			//void push_path(string& _path);
			cstr tree_path(void);
			state actual_state(void) { return actual_state_; }
			const cstr alias(void) {
				if (alias_.length() != 0) {
					return alias_.c_str();
				}
				else {
					return name();
				}
			}
			const cstr display_alias(void) { 
				if (alias_.length() != 0) {
					return alias_.c_str();
				}
				else {
					if (display_alias_.length() == 0) {
						display_alias_.format(RT("%s.%s"), owner_ !=nullptr ? owner_->alias() : RT(""), name());
					}
					return display_alias_.c_str();
				}
				
			};
			template <typename T> T& owner_ref(void) {
				ROBO_APP_ASSERT( dynamic_cast< T * > (owner_)  != nullptr);
				return *(T*)owner_;
			}
			node(void);
			virtual ~node(void);

			static node* find(cstr _path) { return index_().find(hash(_path)); }
			static node* find(int _hash) { return index_().find(_hash); }
			template <class T> static T* find(cstr _path) { return dynamic_cast<T*>(find(_path)); }
			template <class T> static T* find(int _hash) { return dynamic_cast<T*>(find(_hash)); }
		};

		class wrapper;
		class machine;

		class ROBO_EXPORT module : public node {
		protected:
			friend class wrapper;
			friend class machine;
			virtual void frontend_loop(void) = 0;
			virtual void backend_loop(void) = 0;
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
			string lib_;

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

			void frontend_machine_(void);
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