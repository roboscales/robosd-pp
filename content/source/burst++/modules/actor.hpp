#ifndef burst_actor_hpp
#define burst_actor_hpp

#include "burst++/burst.hpp"

#include "core/robosd_list.hpp"

#include "burst++/vartree.hpp"

namespace burst {
	class subsystem;
	class actor {
	public:
		struct config_s {
		};
		#define ACTOR_CONFIG(a) ACTOR_CONFIG_(a)
		#define ACTOR_CONFIG_(a)\
		{\
		}
		struct present_s {};
		typedef robo::list::unsorted<actor> list;
		typedef list::ref ref;
	private:
		const config_s& config_;
		present_s& present_;
		ref ref_;
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
		actor(const config_s& _config, present_s& _present, subsystem& _subsystem);
		void add(subsystem& _subsystem);
		void remove(void);
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
	template<typename I> I& standby() { static I inst_ = {}; return inst_; };
	template<typename I> void connectto(I*& _to, I* _from) {
		if (_from) {
			_to = _from;
		}
		else {
			_to = &standby<I>();
		}
	}

	template<typename I> class sink_t : public actor {
	protected:
		I* input = &standby<I>();
	public:
		virtual void connect(I* _input) {
			connectto(input, _input );
		}
		sink_t(const config_s& _config, present_s& _present) : actor(_config, _present) {}
		sink_t(const config_s& _config, present_s& _present, subsystem& _subsystem)
			: actor(_config, _present, _subsystem) {}
		
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
		void run(void);
		void finish(void);
	};
	template <typename driver, typename actor> class task_t : public driver, public actor {
	public:
		virtual void operator ()(void) {
			actor::run();
		}
		template<typename...Arg> task_t(const typename actor::config_s& _config, typename actor::present_s& _present, Arg ... arg)
			:actor(_config, _present, arg...) {}
	};
	#if 0
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
		#if ROBO_APP_BURST_VARTREE_ENABLE
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
	#endif
}
#endif

