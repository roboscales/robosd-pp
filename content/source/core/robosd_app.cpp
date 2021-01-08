#include "core/robosd_app.hpp"
#include "core/robosd_log.hpp"
#include "core/robosd_os.h"
#include "core/robosd_core.hpp"
#include "core/robosd_string.h"
#include "core/robosd_ini.hpp"
#include "robosd_target_api.h"
#include "core/robosd_app_periphery.hpp"
#include "core/robosd_system.hpp"
#include "core/robosd_trace.hpp"

#ifndef ROBO_MODULE_DELEGAT_IS_NOT_EXISTS
#define ROBO_MODULE_DELEGAT_IS_NOT_EXISTS " module delegat '%s' isn't  found in lib '%s'"
#endif

namespace robo{
	namespace app{
		wrapper::wrapper() :core_ref_(this){
		}
		wrapper::~wrapper(){
		}

		bool  wrapper::load(const robo_string_t  _key){
			wrapper * _wrapper = new wrapper();
			if (!_wrapper->load_(_key)){
				delete _wrapper;
				ROBO_BREAK();
			}
			else {
				return true;
			}
		}
		void  wrapper::release_(){
			if (module_){
				release_f module_release = (release_f)robo_os_lib_proc_get(handle_, RS(ROBO_EXPORT_FUNCTION_PREFIX "robo_module_release"));
				if (module_release == 0){
					robo_errlog(" function 'robo_module_release' isn't  found in lib '%s'", (const char *)lib_);
				}
				else{
					module_release(module_);
				}
				module_ = 0;
			}
		}

		bool  wrapper::load_(const robo_string_t  _key){
			char _lib[ROBO_LIB_NAME_MAX_LEN + 1]; // to do стека может и не хватить!
			ROBO_BREAKEN_LOAD_STR("MODULES", _key, _lib, ROBO_LIB_NAME_MAX_LEN);
			query_f module_query_;

			if (robo_os_lib_exists(_lib) == ROBO_ANSW_YES){
				ROBO_BREAKN(robo_os_lib_load(_lib, &handle_) >= ROBO_SUCCESS);
				module_query_ = (query_f)robo_os_lib_proc_get(handle_, RS(ROBO_EXPORT_FUNCTION_PREFIX "robo_module_query"));
				if (!module_query_){
					robo_os_lib_free(handle_);
					handle_ = 0;
					ROBO_BREAK_F(" function 'robo_module_query' isn't  found in lib '%s'", _lib);
				}
				else{
					module_ = module_query_();
					if (module_){
						lib_ = _lib;
						core_ref_.set_id((int)module_->id);
						if (core_ref_.attach_to(machine::instance().modules_) != ROBO_SUCCESS){
							release_();
							robo_os_lib_free(handle_);
							handle_ = 0;
							ROBO_BREAK_F("module isn't loaded (dupplicated id %d) %s", _lib, (int)module_->id);
						}
						if (!module_->load()){
							module_->free();
							release_();
							robo_os_lib_free(handle_);
							handle_ = 0;
							ROBO_BREAK_F("module isn't loaded  %s", _lib);
						}
						else{
							module_->state = READY;
							robo_infolog(" lib loading '%s' is complete", _lib);
							return true;
						}
					}
					else{
						ROBO_BREAK_F("module module is zero  %s", _lib);
					}
				}
			}
			else {
				ROBO_BREAK_F("module isn't found  %s", _lib);
			}

		}


		void  wrapper::free_(){
			module_->free();
			module_->state = CLEAN;
			release_();
			robo_os_lib_free(handle_);
			handle_ = 0;
		}

		bool  wrapper::start_(){
			if (module_){
				ROBO_BREAKN(module_->start());
				module_->state = STURTUP;
				return true;
			}
			else{
				return false;
			}
		}

		bool  wrapper::stop_(){
			if (module_){
				ROBO_BREAKN(module_->stop());
				module_->state = SHUTDOWN;
				return true;
			}
			else{
				return false;
			}
		}

		result_t wrapper::startup_(void){
			if (module_){
				if (module_->state == STURTUP){
					switch (module_->startup()){
					case COMPLETE:
						module_->state = EXECUTE;
						return COMPLETE;
					case CONTINUE:
						return CONTINUE;
					default:
						break;
					}
				}
			} else {
				return COMPLETE;
			}
			return BREAK;
		}
		result_t  wrapper::shutdown_(void){
			if (module_){
				if (module_->state == SHUTDOWN){
					switch (module_->shutdown()){
					case COMPLETE:
						module_->state = STOPPED;
						return COMPLETE;
					case CONTINUE:
						return CONTINUE;
					default:
						break;
					}
				} else {
					return COMPLETE;
				}
			}
			return BREAK;
		}

		machine::machine() :
			wait_state(app::UNKOWN)
			, req_state(ABORT), terminated_(false){
		}

#if ROBO_DEBUG_LOG_ENABLED == 1
		bool machine::load_(void){
#else
		bool machine::load_(void){
#define _print 0
#endif
			int count;
			wait_state = app::UNKOWN;
			req_state = ABORT;

			char key[ROBO_INI_KEY_MAX_LEN + 1];
			ROBO_BREAKEN_LOAD_INT("MODULES", "COUNT", &count);
			for (int i = 0; i < count; i++){
				robo_sprintf(key, ROBO_INI_KEY_MAX_LEN, RS("M_%d"), i);
				if (!wrapper::load(key)){
					robo_errlog("module loading is brake");
					free_();
					return false;
				}
			}
			wait_state = app::READY;
			req_state = STOP;
			terminated_ = false;
			return true;
		}
		void  machine::free_(){
			while (modules_.count()){
				wrapper * _module = modules_.first()->owner();
				_module->free_();
				delete _module;
			}
			wait_state = app::CLEAN;
			req_state = STOP;
		}

		result_t  machine::startup_(){
			bool fcontinue = false;
			for (wrapper::ref * r = modules_.first(); r; r = r->next()){
				wrapper * _wrapper = r->owner();
				switch (_wrapper->startup_()){
				case app::result_t::CONTINUE:
					fcontinue = true;
					continue;
				case app::result_t::COMPLETE:
					break;
				default:
					return app::result_t::BREAK;
				}
			}
			if (!fcontinue){
				return periphery::startup();
			}
			else {
				return app::result_t::CONTINUE;
			}
		}

		result_t  machine::shutdown_(){
			bool fcontinue = false;
			for (wrapper::ref * r = modules_.first(); r; r = r->next()){
				wrapper * _wrapper = r->owner();
				switch (_wrapper->shutdown_()){
				case app::result_t::CONTINUE:
					fcontinue = true;
					break;
				case app::result_t::COMPLETE:
					break;
				default:
					return app::result_t::BREAK;
				}
			}
			if (!fcontinue){
				return periphery::shutdown();
			}
			else{
				return app::result_t::COMPLETE;
			}
		}

		machine & machine::instance(){
			static machine instance_;
			return instance_;
		}

		bool  machine::start_(){
			if (req_state == ABORT){
				return false;
			}
			else {
				req_state = START;
				return true;
			}
		}

		bool  machine::stop_(){
			if (req_state == ABORT){
				return false;
			}
			else {
				req_state = STOP;
				return true;
			}
		}

		bool  machine::do_start_(){
			for (wrapper::ref * r = modules_.first(); r; r = r->next()){
				ROBO_BREAKN(r->owner()->start_());
			}
			return true;
		}
		bool  machine::do_stop_(){
			for (wrapper::ref * r = modules_.first(); r; r = r->next()){
				ROBO_BREAKN(r->owner()->stop_());
			}
			return true;
		}

		void machine::machine_(void){

			if (req_state == ABORT){
				switch (wait_state){
				case app::READY:
					wait_state = app::ABORT;
					break;

				case app::EXECUTE:
					wait_state = app::SHUTDOWN;
					do_stop_();
					break;

				case app::SHUTDOWN:
					switch (shutdown()){
					case app::result_t::COMPLETE:
						robo_errlog("application was aborted but it is shutdowned ");
						wait_state = app::ABORT;
						break;
					case app::result_t::CONTINUE:
						break;
					case app::result_t::BREAK:
						robo_errlog("application was aborted and doesn't shuttdown");
						wait_state = app::ABORT;
						break;
					default:
						break;

					}
					break;
				default:
					wait_state = app::SHUTDOWN;
				}
			}
			else{
				switch (req_state){
				case START:
					if (wait_state == app::READY){
						if (do_start_()){
							wait_state = app::STURTUP;
						}
						else{
							req_state = ABORT;
							return;
						}
					}
					break;
				case STOP:
					if (wait_state == app::EXECUTE){
						if (do_stop_()){
							wait_state = app::SHUTDOWN;
						}
						else{
							req_state = ABORT;
							return;
						}
					}
					break;
				default:
					break;
				}

				switch (wait_state){
				case app::READY:
					break;

				case  app::STURTUP:
					switch (startup()){
					case app::result_t::BREAK:
						req_state = ABORT;
						break;
					case app::result_t::COMPLETE:
						wait_state = app::EXECUTE;
						break;
					case app::result_t::CONTINUE:
						break;
					default:
						break;
					}
					break;
				case app::STOPPED:
				case app::EXECUTE:
					break;
				case app::SHUTDOWN:
					switch (shutdown()){
					case app::result_t::BREAK:
						req_state = ABORT;
						return;
					case app::result_t::COMPLETE:
						wait_state = app::STOPPED;
						break;
					case app::result_t::CONTINUE:
						break;
					default:
						break;
					}
				default:
					break;

				}
			}
			if (!terminated_)
				terminated_ = (wait_state < app::READY);
		}
#if ROBO_DEBUG_LOG_ENABLED == 1
#ifndef ROBO_DEBUG_LOG_MAX_MASK_COUNT
#define ROBO_DEBUG_LOG_MAX_MASK_COUNT 32
#endif
		bool machine::begin_(const robo_string_t _ini, robo_log_print_f _print){
#else
		bool machine::begin_(const robo_string_t _ini){
#define _print 0
#endif
			ROBO_BREAKN(robo_os_init() >= ROBO_SUCCESS);
			//todo verb and mask debug load from ini
			robo_log_init(0, 0, _print);
			robo_log(-1, 0, "load from %s ", _ini);
			ROBO_BREAKN(robo_ini_init(_ini) >= ROBO_SUCCESS);

#if ROBO_DEBUG_LOG_ENABLED == 1
			int verb;
			unsigned int mask;
			robo_size_t mask_count;
			int  masks[ROBO_DEBUG_LOG_MAX_MASK_COUNT];
			ROBO_BREAKEN_LOAD_INT_LIST("SETTINGS", "DEBUG_MASK_BIT", ROBO_DEBUG_LOG_MAX_MASK_COUNT, &masks[0], &mask_count);
			ROBO_BREAKEN_LOAD_INT("SETTINGS", "DEBUG_VERB", &verb);
			
			if (mask_count > 0) {
				mask = 0;
				for (robo_size_t n = 0; n < mask_count; ++n) {
					mask |= (unsigned int)(1 << masks[n]);
				}
			} else {
				mask = (unsigned int) - 1;
			}
			
			robo_log_init(verb, mask, _print);
				
#endif

			ROBO_BREAKN(periphery::begin());
			
			system::enable(); 

			{
				::robo::system::guard __g;
				ROBO_BREAKN(load_());
				ROBO_BREAKN(start_());
#if APP_TRACE_ENABLED == 1
				ROBO_BREAKN(robo::trace::begin());
#endif
				periphery::start();
				return true;
			}
		}
		void  machine::finish_(){
			system::disable();
			free_();
			periphery::free();
			robo_ini_deinit();
			robo_log_deinit();
			robo_os_deinit();
#if APP_TRACE_ENABLED == 1
			robo::trace::finish();
#endif
		}
		void machine::private_loop_(void){
			system::fall f__;
			machine_();
			if (wait_state == app::EXECUTE){
				periphery::private_loop();
				for (wrapper::ref * r = modules_.first(); r; r = r->next()){
					r->owner()->module_->private_loop();
				}
			}
		}
		bool machine::background_loop_(void){
			periphery::background_loop();
			if (wait_state == app::EXECUTE){
				for (wrapper::ref * r = modules_.first(); r; r = r->next()){
					r->owner()->module_->background_loop();
				}
			}
			return !terminated_;
		}
	}
}

