#include "im/models/models.hpp"
namespace robo{
	namespace models{
		block::guard::guard(block & _owner)
			: owner_(_owner){
			_owner.update_begin_();
		}

		block::guard::~guard(void){
			owner_.update_end_();
		}

		void block::update_begin_(void){
			update_count_++;
		}

		void block::update_end_(void){
			update_count_--;
			if (update_count_ == 0){
				reconfig();
			}
		}
		bool block::configure(const robo_string_t _ini_section, float _model_period_sec){
			guard g__(*this);
			ini_section = _ini_section;
			model_period_sec = _model_period_sec;
			return setup();
		}
	}
}
