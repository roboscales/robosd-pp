#include "core/robosd_common.hpp"
#include "core/robosd_system.hpp"
#include "core/robosd_log.hpp"
namespace robo {
	component::component(component * _owner ):ref_(*this), owner_(nullptr){
		set_owner(_owner);
	}
	component::~component(void){
		set_owner(nullptr);
	}

	void component::set_owner(component * _owner){
		if( owner_ != nullptr ){
			ref_.dettach();
		}

		owner_ = _owner;

		if( owner_ != nullptr ){
			ref_.attach_to(owner_->disabled_);
		}
	}

	bool component::load(cstr _section){
		ini_section = _section;
		ROBO_BREAKN( do_load() );
		if(owner_)
			ref_.attach_to(owner_->stopped_);
		robo_infolog("component %s is loaded",alias.c_str());
		return true;
	}
	bool component::start(void){
		ROBO_BREAKN(disabled_.count() == 0)
		ROBO_BREAKN(startupped_.count() == 0)
		ROBO_BREAKN(active_.count() == 0)
		ROBO_BREAKN(shutdowned_.count() == 0)

		ref * r =  stopped_.first();
		while( r){
			component & c = r->owner();
			r = r->next();
			ROBO_BREAKN( c.start() );
		}
		ROBO_BREAKN(stopped_.count() == 0)
		
		ROBO_BREAKN(do_start());
		if(owner_)
			ref_.attach_to(owner_->startupped_);
		robo_infolog("component %s is begin start",alias.c_str());
		return true;
	}

	result component::startup(void){
		ref * r =  startupped_.first();
		while( r){
			component & c = r->owner();
			r = r->next();
			if( c.startup() == result::panic){
				return  result::panic;
			}
		}

		if( startupped_.count() == 0 ){
			if( do_startup() == result::complete ){
				if(owner_)
					ref_.attach_to(owner_->active_);
				robo_infolog("component %s is started",alias.c_str());
				return result::complete;
			} else {
				return result::resume;
			}
		} else {
			return result::resume;
		}
	}

	void component::stop(void){
		ref * r =  active_.first();
		while( r){
			component & c = r->owner();
			r = r->next();
			c.stop();
		}
		do_stop();
		if(owner_)
			ref_.attach_to(owner_->shutdowned_);
		robo_infolog("component %s is begin shutdown",alias.c_str());
	}


	result component::shutdown(void){

		ref * r =  shutdowned_.first();
		while( r){
			component & c = r->owner();
			r = r->next();
			if( c.shutdown() == result::panic){
				return  result::panic;
			}
		}

		if( shutdowned_.count() == 0 ){
			if( do_shutdown() == result::complete ){
				if(owner_)
					ref_.attach_to(owner_->stopped_);
				robo_infolog("component %s is stopped",alias.c_str());
				return result::complete;
			} else {
				return result::resume;
			}
		} else {
			return result::resume;
		}
	}


	void component::clean(void){
		ref * r =  stopped_.first();
		while( r){
			component & c = r->owner();
			r = r->next();
			c.clean();
		}
		do_clean();
		if(owner_)
			ref_.attach_to(owner_->disabled_);
		robo_infolog("component %s is cleaned",alias.c_str());
	}


	void crash(char const * _file, char const * _function, int _line){
		system::app::crash(_file,_function,_line);
	}
}