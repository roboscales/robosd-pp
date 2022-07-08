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

		private:
			friend class block;
			ref ref_;
			double next_time_ = 0.;
			block::list blocks_;
			void run_(double _time);
			bool begin_(void);
			bool attach_(cstr _name, cstr _lib, cstr _type, void* _instance);
			static bool try_attach_(cstr _name, cstr _lib, cstr _type, void* _instance);
			static void reconfig_(void);
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
			agent(void) :ref_(*this,-1){}
		public:
			double sample_time;
			static agent* find(int _id);
			static agent* find(cstr __name);
			static void run(double _time);
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
