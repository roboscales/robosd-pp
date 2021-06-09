#include "mexo/ps.hpp"
namespace mexo {
	namespace ps {
		dev::mode::mode(int _index, cstr  _name, dev& _dev) 
			: subsystem(_name, false, &_dev)
			, ref_(*this, _index) {
			ref_.attach_to(_dev.modes_);
		};
	
		dev::dev(cstr  _name, ps::power& _power, action & _action, snapshot & _snapshot)
			: node(_name, true, nullptr)
			, power(_power)
			, idle(*this)
			, actual_mode_(&idle)
			, actual_mode_id_(idle_id)
//			, frontend_(this, &dev::frontend__)
			, backend_(this, &dev::backend__)
//			, frontend_ref_(frontend_)
			, backend_ref_(backend_)
			, action_ref(_action)
			, snapshot_ref(_snapshot)
		{
		}

		void dev::switch_to(int _mode_id) {
			ROBO_APP_ASSERT(system::env::is_backend() );
			if (_mode_id != actual_mode_id_) {
				if (actual_mode_id_ != idle_id) {
					robo::system::guard g__;
					actual_mode_->disable();
					power.off();
				}

				if (_mode_id == idle_id) {
					actual_mode_ = &idle;
					actual_mode_id_ = idle_id;
				}
				else {
					mode* m = modes_.find(_mode_id);
					if (m == nullptr) {
						actual_mode_ = &idle;
						actual_mode_id_ = idle_id;
					}
					else {
						robo::system::guard g__;
						m->enable(true);
						power.set(m->reset());
						actual_mode_ = m;
						actual_mode_id_ = _mode_id;
					}
				}
			}
		}
		void dev::do_enable(void) {
			robo::system::guard g__; 
			//mexo::machine::slot::delegat::attach(frontend_ref_, mexo::machine::slot::kind::frontend);
			mexo::machine::slot::delegat::attach(backend_ref_, mexo::machine::slot::kind::backend);
		};

		void dev::do_disable(void) {
			robo::system::guard g__; 
			//frontend_ref_.dettach();
			backend_ref_.dettach();
		};

		/*void dev::switch_to(mode& _mode) {
			ROBO_APP_ASSERT(system::env::is_backend());
			if (actual_mode_ != &_mode) {
				actual_mode_->disable();
				power.set(_mode.reset());
				actual_mode_ = &(_mode);
				actual_mode_id_ = _mode.ref_.key();
			}
		}*/

	}
}
