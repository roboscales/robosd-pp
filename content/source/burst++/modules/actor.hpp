#ifndef burst_actor_hpp
#define burst_actor_hpp

#include "core/robosd_common.hpp"


#if ROBO_APP_ULTRACOMPACT == 0
#include "core/robosd_list.hpp"
#include "burst++/vartree.hpp"
#endif

namespace burst {
	#if ROBO_APP_ULTRACOMPACT == 0
	class subsystem;
	#endif
	class actor {
	public:
		struct config_s {
		};
		#define ACTOR_CONFIG(a) ACTOR_CONFIG_(a)
		#define ACTOR_CONFIG_(a)\
		{\
		}
		struct present_s {};
#if ROBO_APP_ULTRACOMPACT == 0
		typedef robo::list::unsorted<actor> list;
		typedef list::ref ref;
#endif
	private:
		const config_s& config_;
		present_s& present_;
#if ROBO_APP_ULTRACOMPACT == 0
		ref ref_;
#endif
	protected:
	public:
		virtual void begin(void) {};
		virtual void finish(void) {};
		virtual void run(void) {};
		#if ROBO_APP_BURST_VARTREE_ENABLED
		void regvar_present(robo::cstr _name);
		virtual void do_regvar_present(void) = 0;
		void regvar_conf(robo::cstr _name);
		virtual void do_regvar_conf(void) = 0;
		#endif

		actor(const config_s& _config, present_s& _present);
		#if ROBO_APP_ULTRACOMPACT == 0
		actor(const config_s& _config, present_s& _present, subsystem& _subsystem);
		void add(subsystem& _subsystem);
		void remove(void);
		#endif
		virtual ~actor(void) {};
		template <typename T>  T & present(void) {
			return reinterpret_cast < T&>(present_);
		}
		template <typename T> const   T & config(void) {
			return reinterpret_cast <const  T&>(config_);
		}
		#define ACTOR_ALIEN_PRESENT_S(T,a,s) typename T::present_s& s = a.actor::template present<typename  T::present_s>()
		#define ACTOR_PRESENT_S(s) present_s& s= actor::template present<present_s>()
		#define ACTOR_CONFIG_S(s) const config_s& s= actor::template config<config_s>()
	};
	#if ROBO_APP_ULTRACOMPACT == 0

	template<typename I> I& standby() { static I inst_ = {}; return inst_; };
	template<typename I> void connectto(I*& _to, I* _from) {
		if (_from) {
			_to = _from;
		}
		else {
			_to = &standby<I>();
		}
	}
	#endif
	
	template<typename I> class sink_t : public actor {
		#if ROBO_APP_ULTRACOMPACT == 1
		I & input_;
		#endif
	protected:
		#if ROBO_APP_ULTRACOMPACT == 0
		I* input = &standby<I>();
		#endif
	public:
		#if ROBO_APP_ULTRACOMPACT == 0
		virtual void connect(I* _input) {
			connectto(input, _input );
		}
		#endif
		#if ROBO_APP_ULTRACOMPACT == 0
		sink_t(const config_s& _config, present_s& _present, subsystem& _subsystem)
			: actor(_config, _present, _subsystem) {}
		sink_t(const config_s& _config, present_s& _present)
			: actor(_config, _present) {}
		#else
		sink_t(const config_s& _config, present_s& _present,I & _input) : actor(_config, _present), input_(_input) {}
		#endif
	};
	
	#if ROBO_APP_ULTRACOMPACT == 0
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
		void run(void);
		void finish(void);
	};
	#endif
	
	template <typename driver, typename actor> class task_t : public driver, public actor {
	public:
		virtual void operator ()(void) {
			actor::run();
		}
		template<typename...Arg> task_t(const typename actor::config_s& _config, typename actor::present_s& _present, Arg ... arg)
			:actor(_config, _present, arg...) {}
	};	
}
#endif

