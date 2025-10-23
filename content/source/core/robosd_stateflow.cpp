#include "core/robosd_stateflow.hpp"
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
		if (command() == commands::start) {
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
				if ( onFinish() ==repeate::no){
					state_ = state::stopped;
					return true;
				} else{
					state_ = state::stopped;
					start_();
			}
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
		stop();
	}

	void controller::switchto(controller::base* _task) {
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

//===================================
	bool controller::node::run(void) {
		if (command() == commands::start) {
			switch (state_) {
			case states::stopped:
				onEnter();
				state_ = states::entered;
			case states::entered:
				if (doEnter() == result::success) {
					state_ = states::execute;
					onExecute();
				}
				else
					break;
			case  states::execute:
				doExecute();
				break;
			case states::leaved:
				if (doLeave() == result::success) {
					state_ = states::stopped;
					onFinish();
				}
				else
					break;
			}
		}
		else {
			switch (state_) {
			case states::stopped:
				onIdle();
				break;
			case states::entered:
			case states::execute:
				state_ = states::leaved;
				onLeave();
			case states::leaved:
				if (doLeave() == result::success) {
					state_ = states::stopped;
					onFinish();
					return true;
				}
				else
					break;
			}
		}
		return false;
	}

	void controller::node::terminate(void) {
		state_ = states::stopped;
		stop();
		onTerminate();
	}



}
