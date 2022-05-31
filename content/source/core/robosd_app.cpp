#include "core/robosd_app.hpp"
#include "core/robosd_system.hpp"
#include "core/robosd_log.hpp"
#include "core/robosd_ini.hpp"
//todo подумать
#include "servo/robosd_backend.hpp"

#ifndef ROBO_APP_TRACE_ENABLED
#define ROBO_APP_TRACE_ENABLED 0
#endif

namespace robo {
	#if ROBO_APP_MODULE_ENABLED  == 1
	namespace app {
		node::node(void) : ref_(*this), own_ref_(*this, 0), owner_(nullptr), index_ref_(*this, 0) {}

		node::node(cstr _name, node* _owner) : ref_(*this), own_ref_(*this, 0), owner_(nullptr), index_ref_(*this, 0) {
			ROBO_ALARMN(init(_name, _owner));
		}


		node::~node(void) {
			init(nullptr, nullptr);
		}

		node::map& node::index_(void) {
			static map index__;
			return index__;
		}


		bool node::do_load(void) {
			alias_.tryload(current_path(), RT("ALIAS"));
			return true;
		};

		void node::do_clean(void) {
			alias_.clear();
		};


		bool node::init(cstr _name, node* _owner) {
			actual_state_ = state::unknown;
			if (_name) {
				store_name_ = _name;
				name_ = store_name_;
			}

			if (owner_ != nullptr) {
				ref_.dettach();
			}

			owner_ = _owner;
			if (owner_ != nullptr) {
				ref_.attach_to(owner_->disabled_);
			}

			actual_state_ = state::clean;

			/*if (name_ && name_[0]) {
				index_ref_.set_key(hash(ph));
				index_ref_.attach_to(index_());
			}
			*/
			return true;
		}

		void node::path_push_(void) {
			path_root& root_ = path_root::ref();
			if (owner_ == nullptr) {
				if (root_.buf_ == nullptr) {
					root_.buf_ = new char_t[path_root::size];
				}
				root_.buf_[0] = 0;
				root_.top_ = root_.buf_;
				store_top_ = root_.top_ = root_.buf_;
				root_.space_ = path_root::size-1;
			}
			else {
				size_t n;
				if (root_.space_ > 1) {
					if (root_.buf_[0] == 0) {
						size_t sz = store_name_.length();
						n = sz <= root_.space_ ? sz : root_.space_;
						std::copy_n(name_, n, root_.top_);
					}
					else {
						size_t sz = store_name_.length() + 1;
						n = sz <= root_.space_ ? sz : root_.space_;
						*(root_.top_) = '.';
						std::copy_n(name_, n - 1, root_.top_ + 1);
					}
				}
				else {
					n = 0;
				}
				store_top_ = root_.top_;
				root_.top_ += n;
				root_.space_ -= n;
				*root_.top_ = 0;
				if (!root_.common_.tryload(root_.buf_, RT("COMMON"))) {
					if (owner_ == nullptr) {
						root_.common_ = RT(".common");
					}
				}
			}
		}
		void node::path_pop_(void) {
			path_root& root_ = path_root::ref();
			if (owner_ == nullptr) {
				if (root_.buf_ == nullptr) {
					delete[] root_.buf_;
					root_.buf_ = nullptr;
				}
				root_.top_ = 0;
				root_.space_ = 0;
				store_top_ = nullptr;
			}
			else {
				root_.space_ += (root_.top_- store_top_);
				root_.top_ = store_top_;
				*root_.top_ = 0;
				store_top_ = nullptr;
			}
		}

		/*void node::path(string& _path) {
			if (_path.length() > 0)
				_path.format(RT("%s.%s"), name_, _path.c_str());
			else
				_path = name_;
			if (owner_ && owner_ != &machine::root()) owner_->path(_path);
		}*/

		node::path::path(node& _node) 
			: node_(_node) { 
			node_.path_push_(); 
		}
		node::path::~path(void) { 
			node_.path_pop_(); 
		}
		cstr node::path::value(void) {
			return path_root::ref().buf_;
		}
		cstr node::path::common(void) {
			return path_root::ref().common_;
		}
		cstr node::current_path(void) {
			cstr s = path_root::ref().buf_;
			return ( (s == nullptr) || (*s==0 ))? name_:s;
		}
		cstr node::common_path(void) {
			cstr s = path_root::ref().common_.c_str();
			return ((s == nullptr) || (*s == 0)) ? name_ : s;
		}
		
		bool node::load(void) {
			ROBO_LBREAKN(name_ != nullptr);
			{
				path pt_(*this);

				ROBO_LBREAKN(do_load());
				if (alias_.length() == 0) {
					index_ref_.set_key(hash(pt_.value()));
				}
				else {
					index_ref_.set_key(hash(alias_));
				}
				index_ref_.attach_to(index_());

				if (owner_ != nullptr) {
					ROBO_LBREAKN(own_ref_.set_key(index_ref_.key()));
					ROBO_LBREAKN(own_ref_.attach_to(owner_->owned));
				}

				if (owner_)
					ref_.attach_to(owner_->stopped_);

				ref* r = disabled_.first();
				while (r) {
					node& c = r->owner();
					r = r->next();
					ROBO_LBREAKN(c.load());
				}
				ROBO_LBREAKN(disabled_.count() == 0)

				robo_applog("node '%s' is loaded", alias());


				actual_state_ = state::stopped;
			}
			return true;
		}

		bool node::start(void) {
			ROBO_LBREAKN(disabled_.count() == 0);
			ROBO_LBREAKN(startupped_.count() == 0);
			ROBO_LBREAKN(active_.count() == 0);
			ROBO_LBREAKN(shutdowned_.count() == 0);

			ref* r = stopped_.first();

			while (r) {
				node& c = r->owner();
				r = r->next();
				ROBO_LBREAKN(c.start());
			}
			ROBO_LBREAKN(stopped_.count() == 0)

			ROBO_LBREAKN(do_start());

			if (owner_)
				ref_.attach_to(owner_->startupped_);

			robo_applog("node '%s' is begin start", alias());

			actual_state_ = state::startup;
			return true;
		}
		void node::panic(void) {
			if (owner_)
				ref_.attach_to(owner_->disabled_);
			actual_state_ = state::panic;
			do_panic();
		}

		result node::startup(void) {
			ref* r = startupped_.first();

			while (r) {
				node& c = r->owner();
				r = r->next();
				if (c.startup() == result::panic) {
					panic();
					return result::panic;
				}
			}

			if (startupped_.count() == 0) {
				switch (do_startup()) {
				case result::complete:
				if (owner_)
					ref_.attach_to(owner_->active_);
				robo_applog("node '%s' is started", alias());
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

		void node::stop(void) {
			ref* r = active_.first();
			while (r) {
				node& c = r->owner();
				r = r->next();
				c.stop();
			}
			do_stop();
			if (owner_)
				ref_.attach_to(owner_->shutdowned_);
			actual_state_ = state::shutdown;
			robo_applog("node '%s' is begin shutdown", alias());
		}


		result node::shutdown(void) {

			ref* r = shutdowned_.first();
			while (r) {
				node& c = r->owner();
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
				robo_applog("node %s is stopped", alias());
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


		void node::clean(void) {
			ref* r = stopped_.first();
			while (r) {
				node& c = r->owner();
				r = r->next();
				c.clean();
			}
			do_clean();
			if (owner_)
				ref_.attach_to(owner_->disabled_);
			robo_applog("node %s is cleaned", alias());
			actual_state_ = state::clean;
		}

		#if ROBO_APP_LIB_ENABLED == 1
		wrapper::wrapper(void) : ref_(*this, 0) {}
		wrapper::~wrapper(void) {}

		bool  wrapper::begin(cstr _key) {
			wrapper* _wrapper = new wrapper();
			if (!_wrapper->begin_(_key)) {
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
					module_->init(nullptr, nullptr);
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
			ROBO_LBREAKN(lib_.load(RT("MODULES"), _key));

			ref_.set_key(hash(lib_));

			if (!ref_.attach_to(machine::root().wrappers_)) {
				ROBO_LBREAK_F("module isn't loaded (dupplicated lib names) %s", lib_.c_str());
			}

			ROBO_LBREAKN_F(system::lib::exists(lib_), "module isn't found  %s", lib_.c_str());

			handle_ = system::lib::load(lib_);
			ROBO_LBREAKN_F(handle_ != nullptr, "module '%s' isn't found  ", lib_.c_str());

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


			if (!module_->init(nullptr, &machine::root())) {
				finish();
				ROBO_LBREAK_F("module isn't init  from  lib  '%s'", lib_.c_str());
			}

			robo_applog(" lib loading '%s' is complete", lib_.c_str());

			return true;
		}


		bool machine::begin_(cstr _ini) {
			init(RT("app.machine"), nullptr);
			#if ROBO_APP_DEBUG_LOG_ENABLED == 1
			log::begin(log::verb::detail_7, log::mask::disabled);
			#endif
			ROBO_LBREAKN(system::env::begin());

			robo_infolog("load from %s ", _ini);
			ROBO_LBREAKN(system::ini::begin(_ini));

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

			if (ini::try_load_list(RT("SETTINGS"), RT("DEBUG_MASK_BITS"), (size_t)mask_max_count, masks, mask_count)) {
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
			else {
				mask = (unsigned int)-1;
			}

			robo::log::begin(verb, mask);

			#endif
			ROBO_LBREAKN(node::load());
			#if ROBO_APP_TRACE_ENABLED == 1
			ROBO_BREAKN(robo::trace::begin());
			#endif

			return true;
		}

		void machine::frontend_loop_(void) {
			system::frontend_loop();
			frontend_machine_();
			if (actual_state() > state::stopped) {
				for (wrapper::ref* r = wrappers_.first(); r; r = r->next()) {
					r->owner().module_->frontend_loop();
				}
			}
			frontend::queue::poll();
		}

		void machine::backend_loop_(void) {
			system::fall f__;
			system::env::backend_loop();
			if (actual_state() > state::stopped) {
				for (wrapper::ref* r = wrappers_.first(); r; r = r->next()) {
					r->owner().module_->backend_loop();
				}
				//todo подумать
				backend::bus::perform();
			}
			backend::queue::poll();
			backend::task::machine::execute();
		}

		machine& machine::root() {
			static machine instance_;
			return instance_;
		}

		void  machine::finish_() {
			node::clean();
			system::ini::finish();
			#if ROBO_APP_DEBUG_LOG_ENABLED == 1
			log::finish();
			#endif
			system::env::finish();
			#if APP_TRACE_ENABLED == 1
			trace::finish();
			#endif
		}

		void machine::frontend_machine_(void) {
			if (req_state_ == req_state::stop) {
				switch (actual_state()) {
				case state::stopped:
				break;
				case state::startup:
				case state::execute:
				node::stop();
				break;
				case state::shutdown:
				node::shutdown();
				break;
				default:
				panic();
				}
			}
			else {
				switch (actual_state()) {
				case state::stopped:
				node::start();
				break;
				case  state::startup:
				node::startup();
				break;
				case state::execute:
				break;
				case state::shutdown:
				node::shutdown();
				break;
				default:
				panic();
				}
			}
			if (!terminated_)
				terminated_ = (actual_state() <= state::stopped);
		}
		bool machine::terminated__(void) {
			system::guard g__;
			return terminated_;
		}

		bool machine::start_(void) {

			ROBO_LBREAKN_F(actual_state() > state::clean, "application is aborted!");
			ROBO_LBREAKN_F(system::env::start(), "application is aborted");
			req_state_ = req_state::start;
			return true;
		}

		void machine::stop_(void) {
			ROBO_VBREAKN_F(actual_state() > state::clean, "application is aborted!");
			req_state_ = req_state::stop;
		}

		bool machine::do_load(void) {
			ROBO_LBREAKN(node::do_load());
			req_state_ = req_state::stop;
			int modules_count = 0;
			ROBO_LBREAKN(ini::load(RT("MODULES"), RT("COUNT"), modules_count));
			for (int i = 0; i < modules_count; i++) {
				ROBO_LBREAKN_F(wrapper::begin(string(RT("M_%d"), i + 1)), "modules loading is brake");
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
				wrapper* _module = &wrappers_.first()->owner();
				_module->finish();
				delete _module;
			}
			req_state_ = req_state::stop;
			system::env::finish();
		}
		#endif

	}
	#endif
}