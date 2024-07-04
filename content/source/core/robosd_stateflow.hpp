#ifndef robo_stateflow_hpp
#define robo_stateflow_hpp
#include <stdint.h>
#include "core\robosd_common.hpp"
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
	public:
		class process {
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
			friend class controller;

			enum class command {
				stop = 0,
				start = 1
			};
			command command_ = command::stop;
			enum class state {
				stopped = 0,
				prepare = 1,
				startup = 3,
				execute = 4,
				shutdown = 5,
				relax = 6
			};
			state state_ = state::stopped;
			void start_(void) { command_ = command::start; };
		protected:
			bool run(void);
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
			process(void) {};
			void stop(void) { command_ = command::stop; };
			void restart(void) { if (command_ == command::stop && state_ == state::relax) state_ = state::stopped; };
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
		process* selected_ = 0;
		process* runned_ = 0;
	protected:
		virtual void doTerminate() {};
	public:
		void switchto(process* _task);
		void stop(void);
		void run(void);
		void terminate(void);
	};
}
#endif