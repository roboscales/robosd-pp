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
				virtual bool do_load(cstr _section) = 0;
				virtual void do_try_load(cstr _section) = 0;
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
		protected:
			string type;
			string name;
			string lib;
			void* lib_instance;
			virtual bool do_priotitet_run(double _time) = 0;
			virtual bool do_background_run(double _time) = 0;
			virtual bool do_begin(void);
			virtual void do_reconfig(void) = 0;
			virtual void do_finish(void) = 0;
			agent(void) :ref_(*this,-1) {}
		public:
			double sample_time;
			static agent* find(int _id);
			static agent* find(cstr __name);
			static void run(double _time);
			bool begin(void);
			void reconfig(void);
			void finish(void);
		};


		struct ROBO_EXPORT channel : public agent {
			struct driver {
				enum class status { complete = 0, refuse = 1, run = 2 };
				typedef list::unsorted<driver> list;
				typedef list::ref ref;
				channel & owner;
				string name;
				//virtual status request( uint8_t * _in, size_t _in_size)=0;

			};
			driver::list drivers;
			//virtual void receive(uint8_t* _out, size_t _sizs) = 0;
			//virtual void send(uint8_t * _out, size_t _sizs) = 0;
			//virtual bool busy(void) = 0;
		};
		/*
		
		ROBO_EXPORT static bool load(agent & _agent, cstr _lib);

		ROBO_EXPORT robo_result_t ROBO_DECL emu_chan_receive(emu_chan_p _agent, robo_byte_p _buf, robo_size_t _len);
		ROBO_EXPORT robo_result_t ROBO_DECL emu_chan_send(emu_chan_p _agent, robo_byte_p _out, robo_size_t  _out_size);

		ROBO_EXPORT robo_result_t ROBO_DECL dev_reg_net_driver(int channel_id, emu_chan_driver_p _driver);
		ROBO_EXPORT void ROBO_DECL dev_unreg_net_driver_p(emu_chan_driver_p _driver);*/
	}
}
extern "C" {
	typedef robo::edev::agent* (ROBO_EXPORT_RUNTIME_DECL* robo_edev_query_f)(void);
}

#endif
