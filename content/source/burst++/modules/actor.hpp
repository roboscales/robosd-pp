#ifndef burst_actor_hpp
#define burst_actor_hpp

#include "burst++/burst.hpp"

#include "core/robosd_list.hpp"

namespace burst {
	class subsystem;
	class actor: public burst::board::slot::delegat {
	public:
		struct config_s {};
		#define ACTOR_CONFIG(a) ACTOR_CONFIG_(a)
		#define ACTOR_CONFIG_(a)\
		{\
		}
		struct present_s {};
		typedef robo::list::unsorted<actor> list;
		typedef list::ref ref;
	private:
		ref ref_;
		const config_s & config_;
		present_s & present_;
	public:
		virtual void reset(void) {};
		#if ROBO_APP_BURST_VARTABLE_ENABLE
		virtual void regvar(void) = 0;
		#endif
		actor(const config_s& _config, present_s& _present);
		actor(const config_s& _config, present_s& _present, subsystem & _subsystem);
		void add(subsystem& _subsystem);
		void remove(void);
		void finish(void) {
			delegat::dettach();
		}
		template <typename T> typename T::present_s& present(void) {
			return reinterpret_cast <typename T::present_s&>(present_);
		}

		template <typename T> const  typename T::config_s& config(void) {
			return reinterpret_cast <const typename T::config_s&>(config_);
		}
	};
	class subsystem {
		friend class actor;
	public:
		typedef robo::list::unsorted<subsystem> list;
		typedef list::ref ref;
	private:
		ref ref_;
		list childs_;
		actor::list actors_;
	public:
		subsystem(void);
		subsystem(subsystem& _subsystem);
		void add(subsystem& _subsystem);
		void remove(void);
		void begin(void);
		void finish(void);
	};
}
#endif

