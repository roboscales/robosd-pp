#ifndef robo_stateflow_hpp
#define robo_stateflow_hpp
#include <stdint.h>
#include "core/robosd_common.hpp"
namespace robo {
	class stateflow {
	public:
		class node {
			friend class stateflow;
		protected:
			virtual void onEnter(void) = 0;
			virtual void doRun(void) = 0;
			virtual void onLeave(void) = 0;
		};

	private:
		node* runned_ = 0;
	public:
		void switchto(node* _node);
		void run(void);
	};

	class controller {
	protected:
		class base {
		protected:
			friend class controller;
			enum class result {
				wait = false,
				success = true
			};
			enum class commands {
				stop = 0,
				start = 1
			};
		private:
			commands command_;
			void start_(void) { command_ = commands::start; };
		protected:
			commands command() { return command_;  };
			virtual ~base(void) {}
			virtual bool run(void) = 0;
		public:
			virtual void terminate(void) = 0;
			void stop(void) { command_ = commands::stop; };
		};
	public:
		class node: public base {

			enum class states {
				stopped = 0,
				entered = 1,
				execute = 2,
				leaved = 3,
			};
			states state_ = states::stopped;
		protected:
			virtual bool run(void);
			virtual void onEnter(void) = 0;
			virtual result doEnter(void) = 0;
			virtual void onExecute(void) = 0;
			virtual void doExecute(void) = 0;
			virtual void onLeave(void) = 0;
			virtual result doLeave(void) = 0;
			virtual void onFinish(void) = 0;
			virtual void onTerminate(void) = 0;
			virtual void onIdle(void) = 0;
		public:
			virtual void terminate(void) ;
			virtual ~node(void) {}
		};

		class process : public base {
		protected:
			enum class result {
				wait = false,
				success = true
			};
			enum class repeate {
				no = false,
				go = true
			};
		private:

			enum class state {
				stopped = 0,
				prepare = 1,
				startup = 3,
				execute = 4,
				shutdown = 5,
				relax = 6
			};
			state state_ = state::stopped;
		protected:
			virtual bool run(void);
			virtual void onPrepare(void) {}
			virtual void onStartup(void) {}
			virtual void onExecute(void) {}
			virtual void onShutdown(void) {}
			virtual void onRelax(void) {}
			virtual repeate onFinish(void) { return repeate::no; }
				
			virtual result doPrepare(void) { return result::success; }
			virtual result doStartup(void) { return result::success; }
			virtual result doExecute(void) { return result::success; }
			virtual result doShutdown(void) { return result::success; }
			virtual result doRelax(void) { return result::success; }
			virtual void doIdle(void) {};
			virtual void doTerminate(void) {};
		protected:
			process(void):base() {};
			void restart(void) { if (command() == commands::stop && state_ == state::relax) state_ = state::stopped; };
			void terminate(void);
			virtual ~process(void) {};
		};
		class independed : public process {
		protected:
			independed() {}
		public:
			bool run(void) {
				return process::run();
			}
		};
	private:
		base* selected_ = 0;
		base* runned_ = 0;
	protected:
		virtual void doTerminate() {};
	public:
		void switchto(base* _task);
		void stop(void);
		void run(void);
		virtual void terminate(void);
	};
		
}
#endif