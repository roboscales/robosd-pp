#include "core/robosd_app.hpp"
#include "core/robosd_system.hpp"
#include "core/robosd_log.hpp"
#include "core/robosd_ini.hpp"

#ifndef ROBO_APP_TRACE_ENABLED
#define ROBO_APP_TRACE_ENABLED 0
#endif

namespace robo {
	namespace app {
		component::component(void) : ref_(*this), own_ref_(*this, 0), owner_(nullptr) {
		}
		component::component(cstr _name, component* _owner) : ref_(*this), own_ref_(*this,0), owner_(nullptr) {
			ROBO_ALARMN(init(_name, _owner));
		}
		component::~component(void) {
			init(nullptr, nullptr);
		}

		bool component::do_load(void) { 
			alias_.tryload(name_, RT("ALIAS"));
			return true; 
		};
		
		void component::do_clean(void) { 
			alias_.clear(); 
		};

		
		bool component::init(cstr _name, component* _owner) {
			actual_state_ = state::unknown;

			if (_name) name_ = _name; else name_ = RT("");

			if (owner_ != nullptr) {
				ref_.dettach();
			}

			owner_ = _owner;

			ROBO_LBREAKN(own_ref_.set_key( hash(name_) ))

			if (owner_ != nullptr) {
				ROBO_LBREAKN(own_ref_.attach_to(owner_->owned));
				ref_.attach_to(owner_->disabled_);
			}
			actual_state_ = state::clean;
			return true;
		}

		
		bool component::load(void) {

			ROBO_LBREAKN(do_load());
			if (owner_)
				ref_.attach_to(owner_->stopped_);

			ref* r = disabled_.first();
			while (r) {
				component& c = r->owner();
				r = r->next();
				ROBO_LBREAKN(c.load());
			}
			ROBO_LBREAKN(disabled_.count() == 0)


			robo_applog("component '%s' is loaded", alias().c_str());
			actual_state_ = state::stopped;
			return true;
		}

		bool component::start(void) {
			ROBO_LBREAKN(disabled_.count() == 0);
			ROBO_LBREAKN(startupped_.count() == 0);
			ROBO_LBREAKN(active_.count() == 0);
			ROBO_LBREAKN(shutdowned_.count() == 0);

			ref* r = stopped_.first();

			while (r) {
				component& c = r->owner();
				r = r->next();
				ROBO_LBREAKN(c.start());
			}
			ROBO_LBREAKN(stopped_.count() == 0)

			ROBO_LBREAKN(do_start());

			if (owner_)
				ref_.attach_to(owner_->startupped_);

			robo_applog("component '%s' is begin start", alias().c_str());

			actual_state_ = state::startup;
			return true;
		}
		void component::panic(void) {
			if (owner_)
				ref_.attach_to(owner_->disabled_);
			actual_state_ = state::panic;
		}

		result component::startup(void) {
			ref* r = startupped_.first();
			
			while (r) {
				component& c = r->owner();
				r = r->next();
				if (c.startup() == result::panic) {
					panic();
					return result::panic;
				}
			}

			if (startupped_.count() == 0) {
				switch(do_startup()){
				case result::complete:
					if (owner_)
						ref_.attach_to(owner_->active_);
					robo_applog("component '%s' is started", alias().c_str());
					actual_state_ = state::execute;
					return result::complete;
				case result::resume:
					return result::resume;
				case result::panic:
					panic();
					return result::panic;
				}
			}
			return result::resume;
		}

		void component::stop(void) {
			ref* r = active_.first();
			while (r) {
				component& c = r->owner();
				r = r->next();
				c.stop();
			}
			do_stop();
			if (owner_)
				ref_.attach_to(owner_->shutdowned_);
			actual_state_ = state::shutdown;
			robo_applog("component '%s' is begin shutdown", alias().c_str());
		}


		result component::shutdown(void) {

			ref* r = shutdowned_.first();
			while (r) {
				component& c = r->owner();
				r = r->next();
				if (c.shutdown() == result::panic) {
					panic();
					return result::panic;
				}
			}

			if (shutdowned_.count() == 0) {
				switch (do_shutdown()) {
				case result::complete:
					if (owner_)
						ref_.attach_to(owner_->stopped_);
					robo_applog("component %s is stopped", alias().c_str());
					actual_state_ = state::stopped;
					return result::complete;
				case result::resume:
					return result::resume;
				case result::panic:
					panic();
					return result::panic;
				}
			}
			return result::resume;
		}


		void component::clean(void) {
			ref* r = stopped_.first();
			while (r) {
				component& c = r->owner();
				r = r->next();
				c.clean();
			}
			do_clean();
			if (owner_)
				ref_.attach_to(owner_->disabled_);
			robo_applog("component %s is cleaned", alias().c_str());
			actual_state_ = state::clean;
		}

#if ROBO_APP_LIB_ENABLED == 1
		wrapper::wrapper(void): ref_(*this,0) {
		}
		wrapper::~wrapper(void) {
		}

		bool  wrapper::begin(cstr _key) {
			wrapper* _wrapper = new wrapper();
			if (!_wrapper->begin_(_key) ) {
				delete _wrapper;
				ROBO_LBREAK();
			}
			else {
				return true;
			}
		}
		
		void  wrapper::finish() {
			if (handle_) {
				if (module_) {
					module_->init(nullptr,nullptr);
					release_f module_release = (release_f)system::lib::proc_get(handle_, ROBO_EXPORT_FUNCTION_PREFIX RT("robo_module_release"));
					if (module_release == 0) {
						robo_errlog(" function 'robo_module_release' isn't  found in lib '%s'", lib_.c_str());
					}
					else {
						module_release(module_);
					}
					module_ = nullptr;
				}
				system::lib::free(handle_);
				handle_ = nullptr;
			}
		}

		
		bool  wrapper::begin_(cstr  _key) {
			ROBO_LBREAKN( lib_.load(RT("MODULES"), _key) );

			ref_.set_key(hash(lib_));

			if (!ref_.attach_to(machine::instance().wrappers_)) {
				ROBO_LBREAK_F("module isn't loaded (dupplicated lib names) %s", lib_);
			}

			ROBO_LBREAKN_F(system::lib::exists(lib_), "module isn't found  %s", lib_.c_str() );

			handle_ = system::lib::load(lib_);
			ROBO_LBREAKN_F(handle_!=nullptr, "module '%s' isn't found  ", lib_.c_str() );

			query_f module_query_;
			module_query_ = (query_f)system::lib::proc_get(handle_, ROBO_EXPORT_FUNCTION_PREFIX RT("robo_module_query"));
			if (!module_query_) {
				finish();
				ROBO_LBREAK_F("function 'robo_module_query' isn't  found in lib '%s'", lib_.c_str());
			}
			
			module_ = module_query_();
			if (module_ == nullptr) {
				finish();
				ROBO_LBREAK_F("module is zero in lib '%s'", lib_.c_str());
			}

			
			if ( !module_->init( string(RT("%s.module"), lib_.c_str() ), &machine::instance()) ) {
				finish();
				ROBO_LBREAK_F("module isn't init  from  lib  '%s'", lib_.c_str());
			}

			robo_applog(" lib loading '%s' is complete", lib_.c_str());

			return true;
		}


#if ROBO_APP_DEBUG_LOG_ENABLED == 1
		bool machine::begin_(cstr _ini, log::print_f _print) {
#else
		bool machine::begin_(cstr _ini) {
#endif
			init(RT("app.machine"), nullptr);
			ROBO_LBREAKN( system::env::begin());
			
#if ROBO_APP_DEBUG_LOG_ENABLED == 1
			log::begin(log::verb::detail_7, log::mask::disabled, _print);
#endif
			robo_applog("load from %s ", _ini);
			ROBO_LBREAKN( system::ini::begin(_ini));

#if ROBO_APP_DEBUG_LOG_ENABLED == 1
			log::verb verb = log::verb::detail_1;
			unsigned int mask = 0;
			enum { mask_max_count = 32 };
			unsigned int masks[mask_max_count];
			size_t mask_count = 0;
			int tmp;
			ini::try_load(RT("SETTINGS"), RT("DEBUG_VERB"), tmp);
			if (tmp >= 0 && tmp <= 8) {
				verb = (log::verb)tmp;
			}

			if ( ini::try_load_list(RT("SETTINGS"), RT("DEBUG_MASK_BITS"), (size_t)mask_max_count, masks, mask_count) ) {
				if (mask_count > 0) {
					mask = 0;
					for (size_t n = 0; n < mask_count; ++n) {
						mask |= (unsigned int)(1 << masks[n]);
					}
				}
				else {
					mask = (unsigned int)-1;
				}
			}


			robo::log::begin(verb, mask, _print);
			
#endif
			ROBO_LBREAKN(component::load());
#if ROBO_APP_TRACE_ENABLED == 1
			ROBO_BREAKN(robo::trace::begin());
#endif
			
			return true;
		}

		void machine::frontend_loop_(void) {
			system::fall f__;
			machine_();
			system::env::frontend_loop();
			if (actual_state() > state::stopped) {
				for (wrapper::ref* r = wrappers_.first(); r; r = r->next()) {
					r->owner().module_->frontend_loop();
				}
			}
		}

		void machine::backend_loop_(void) {
			system::env::backend_loop();
			if (actual_state() > state::stopped) {
				for (wrapper::ref* r = wrappers_.first(); r; r = r->next()) {
					r->owner().module_->backend_loop();
				}
			}
		}
		
		machine& machine::instance() {
			static machine instance_;
			return instance_;
		}

		void  machine::finish_() {
			component::clean();
			system::ini::finish();
#if ROBO_APP_DEBUG_LOG_ENABLED == 1
			log::finish();
#endif
			system::env::finish();
#if APP_TRACE_ENABLED == 1
			trace::finish();
#endif
		}

		void machine::machine_(void) {
			if (req_state_ == req_state::stop) {
				switch ( actual_state() ) {
				case state::stopped:
					break;
				case state::startup:
				case state::execute:
					component::stop();
					break;
				case state::shutdown:
					component::shutdown();
					break;
				default:
					panic();
				}
			}
			else {
				switch ( actual_state() ) {
				case state::stopped:
					component::start();
					break;
				case  state::startup:
					component::startup();
					break;
				case state::execute:
					break;
				case state::shutdown:
					component::shutdown();
					break;
				default:
					panic();
				}
			}
			if (!terminated_)
				terminated_ = ( actual_state() <= state::stopped );
		}
		bool machine::terminated__(void) { 
			system::guard g__;
			return terminated_; 
		}

		bool machine::start_(void) {

			ROBO_LBREAKN_F( actual_state() > state::clean, "application is aborted!" );
			ROBO_LBREAKN_F( system::env::start(),"application is aborted" );
			req_state_ = req_state::start;
			return true;
		}

		void machine::stop_(void) {
			ROBO_VBREAKN_F(actual_state() > state::clean, "application is aborted!");
			req_state_ = req_state::stop;
		}

		bool machine::do_load(void) {
			req_state_ = req_state::stop;
			int modules_count = 0;
			ROBO_LBREAKN(ini::load(RT("MODULES"), RT("COUNT"), modules_count));
			for (int i = 0; i < modules_count; i++) {				
				ROBO_LBREAKN_F(wrapper::begin(string(RT("M_%d"), i+1)),"modules loading is brake");
			}
			terminated_ = false;
			return true;
		}

		result machine::do_shutdown(void) {
			system::env::stop();
			return result::complete;
		}

		void machine::do_clean(void) {
			while (wrappers_.count()) {
				wrapper * _module = & wrappers_.first()->owner();
				_module->finish();
				delete _module;
			}
			req_state_ = req_state::stop;
			system::env::finish();
		}
#endif

	}

	int hash(cstr _src, int _begin) {
		const int p = 15;
		const int m = 10009;
		int hash_value = _begin;
		int p_pow = 1;
		const char_t * c;
		for (c = _src; *c; c++) {
			hash_value = (hash_value + (*c - 'a' + 1) * p_pow) % m;
			p_pow = (p_pow * p) % m;
		}
		return hash_value;

	}

	int fast_hash(cstr _src, int _begin) {
		unsigned short  x = (unsigned short)_begin;
		const char_t * c;
		for (c = _src; *c; c++) {
			x += 44111 * (*c);
		}
		return (int)x;
	}

}