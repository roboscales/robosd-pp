#ifndef _edev_hpp
#define _edev_hpp
#include "core/robosd_common.hpp"
#include "core/robosd_list.hpp"
#include "core/robosd_system.hpp"
#include "core/robosd_string.hpp"
namespace robo {
	namespace edev {

		class ROBO_EXPORT agent {
		public:
			class ROBO_EXPORT block {
			public:
				typedef list::unsorted<block> list;
				typedef list::ref ref;
			private:
				ref ref_;
				bool load_(void);
				friend class agent;
			protected:
				agent& owner;
				string name;
				string path;
				block(agent& _agent, cstr _name);
				virtual bool do_load(cstr _specific_sect, cstr _common_sect) = 0;
				virtual void do_reconfig(void) = 0;
				virtual void do_run(void) = 0;
			};

			typedef list::unique<agent, int> map;
			typedef map::ref ref;
			typedef list::unsorted<agent> list;
			typedef list::ref threadref;

		private:
			friend class block;
			ref ref_;
			threadref thread_ref_;
			double next_time_ = 0.;
			block::list blocks_;
			void run_(double _time);
			bool begin_(void);
			bool attach_(cstr _name, cstr _lib, cstr _type, void* _instance);
			static bool try_attach_(cstr _name, cstr _lib, cstr _type, void* _instance);
			static void reconfig_(void);

		public:
			class ROBO_EXPORT thread {
				friend class agent;
				friend class threads;
				typedef ::robo::list::unique<thread, int> list;
				typedef list::ref ref;
				agent::list agents_;
				ref ref_;
			public:
				void run(double _time) {
					for (agent::threadref * p = agents_.first(); p; p = p->next()) {
						p->owner().run_(_time);
					}
				}
			private:
				thread(int _id): ref_(*this, _id) {
					ref_.attach_to(threads());
				}
			};
				
			static thread::list& threads(void);
		private:
			bool attach_to_thread_(int _id) {
				auto th = threads().find(_id);
				if (th == 0) {
					th = new thread(_id);
				}
				auto it = th->agents_.last();
				if ( (it && it->owner().sample_time == sample_time) || it == nullptr) {
					thread_ref_.attach_to(th->agents_);
					return true;
				}
				else {
					robo_errlog(RT("different quantization times in the same tgread (id:%d, need:%f, exists: %f)"), _id, it->owner().sample_time, sample_time);
					return false;
				}
			}
		protected:
			string type;
			string name;
			string lib;
			void* lib_instance;
			virtual void do_priotitet_run(double _time) = 0;
			virtual void do_background_run(double _time) = 0;
			virtual bool do_begin(void);
			virtual void do_reconfig(void) = 0;
			virtual void do_finish(void) = 0;
			agent(void) :ref_(*this,-1), thread_ref_(*this){}
		public:
			virtual void perform_command(int /*_cmd*/) {};
			double sample_time;
			static agent* find(int _id);
			static agent* find(cstr __name);
			static void run(double _time);
			//static void stop(void);
			static void backgrounf_run(double time);
			static bool begin(cstr _ini);
			static void finish(void);
			virtual void set_local_ini(cstr _ini) =0;
		};
	}
}
extern "C" {
	typedef robo::edev::agent* (ROBO_EXPORT_RUNTIME_DECL* robo_edev_query_f)(void);
}

#endif
