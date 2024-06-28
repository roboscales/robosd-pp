#include "core\robosd_stateflow.hpp"
namespace robo {
	void stateflow::switchto(node* _node) {
		if (runned_) {
			runned_->onLeave();
		}
		runned_ = _node;
		if (runned_) {
			runned_->onEnter();
		}
	}

	void stateflow::run(void) {
		if (runned_) runned_->doRun();
	}

	bool controller::process::run(void) {
		if (command_ == command::start) {
			switch (state_) {
			case state::stopped:
			onPrepare();
			state_ = state::prepare;
			case state::prepare:
			if (doPrepare() == result::success) {
				state_ = state::startup;
				onStartup();
			}
			else
				break;
			case  state::startup:
			if (doStartup() == result::success) {
				state_ = state::execute;
				onExecute();
			}
			else
				break;
			case state::execute:
			if (doExecute() == result::success) {
				onShutdown();
				stop();
			}
			else
				break;
			case state::shutdown:
			if (doShutdown() == result::success) {
				state_ = state::relax;
				onRelax();
			}
			else
				break;
			case state::relax:
			if (doRelax() == result::success) {
				onFinish();
				state_ = state::stopped;
			}
			else
				break;
			}
		}
		else {
			switch (state_) {
			case state::stopped:
			doIdle();
			break;
			case state::prepare:
			case state::startup:
			case state::execute:
			state_ = state::shutdown;
			onShutdown();
			case state::shutdown:
			if (doShutdown() == result::success) {
				state_ = state::relax;
				onRelax();
			}
			else
				break;
			case state::relax:
			if (doRelax() == result::success) {
				onFinish();
				state_ = state::stopped;
				return true;
			}
			else
				break;
			}
		}
		return false;
	}

	void controller::process::terminate(void) {
		doTerminate();
		state_ = state::stopped;
		command_ = command::stop;
	}

	void controller::switchto(controller::process* _task) {
		if (selected_ == 0 && runned_ == _task) {
			if (runned_)
				runned_->start_();
		}
		else {
			selected_ = _task;
			if (runned_) runned_->stop();
		}
	}

	void controller::run(void) {
		if (runned_ == 0) {
			if (selected_ != 0) {
				runned_ = selected_;
				selected_ = 0;
				runned_->start_();
			}
			else {
				return;
			}
		}
		if (runned_->run()) {
			runned_ = 0;
		}
	}
	void controller::stop() {
		selected_ = 0;
		if (runned_) runned_->stop();
	}
	void controller::terminate() {
		selected_ = 0;
		if (runned_) runned_->terminate();
		runned_ = 0;
		doTerminate();
	}

}
