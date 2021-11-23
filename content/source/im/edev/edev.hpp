#ifndef _edev_hpp
#define _edev_hpp
#include "core/robosd_common.hpp"
#include "core/robosd_list.hpp"
#include "core/robosd_system.hpp"
#include "core/robosd_string.hpp"
namespace robo {
	namespace edev {
		struct agent {
			typedef list::unique<agent, int> map;
			typedef map::ref ref;
			string type;
			string name;
			string lib;
			void* lib_instance;
			virtual bool run(double _period, double _time) = 0;
			virtual bool begin(void) = 0;
			virtual void finish(void) = 0;
		};


		struct channel : public agent {
			struct driver {
				enum class status { complete = 0, refuse = 1, run = 2 };
				typedef list::unsorted<driver> list;
				typedef list::ref ref;
				channel & owner;
				string name;
				//virtual status request( uint8_t * _in, size_t _in_size)=0;

			};
			string name;
			driver::list drivers;
			//virtual void receive(uint8_t* _out, size_t _sizs) = 0;
			//virtual void send(uint8_t * _out, size_t _sizs) = 0;
			//virtual bool busy(void) = 0;
		};
		ROBO_EXPORT agent* find_agent(int _id);
		ROBO_EXPORT agent* find_agent(cstr __name);
		ROBO_EXPORT void run(double _period, double _time);
		ROBO_EXPORT bool begin(void);
		ROBO_EXPORT void finish(void);
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
