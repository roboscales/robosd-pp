#include "core/robosd_tree.hpp"

namespace robo {
	namespace tree {

		item::item(cstr _name, item* _branch)
			:name(_name)
			, branch_(_branch)
			, ref_(*this, hash(_name)) {
			if (branch_ != nullptr) {
				if (branch_->childs_ == nullptr) {
					branch_->childs_ = new list;
				}
				ref_.attach_to(*(branch_->childs_));
			}
		}
		size_t item::path_len(size_t _current, size_t _max) {
			_current += (name.length()+1);
			if (_current > _max) {
				_max = _current;
			}
			if (childs_ != nullptr) {
				for (list::ref* r = childs_->first(); r; r = r->next()) {
					_max = r->owner().path_len(_current, _max);
				}
			}
			return  _max;
		}

		bool item::load(char_t* _beg_path, char_t* _end_path, size_t  _path_size) {
			size_t n = name.length() + 1;
			ROBO_APP_ASSERT(_path_size >= n + 1);
			n = system::sprintf(_end_path, _path_size, RT(".%s"), name.c_str());
			_end_path[n]=0;
			_end_path += n;
			_path_size -= n;
			ROBO_LBREAKN(do_load(_beg_path));
			if (childs_ != nullptr) {
				for (list::ref* r = childs_->first(); r; r = r->next()) {
					ROBO_LBREAKN( r->owner().load(_beg_path, _end_path, _path_size) );
				}
			}
			return true;
		}
		void item::clean(void) {
			if (childs_ != nullptr) {
				for (list::ref* r = childs_->first(); r; r = r->next()) {
					r->owner().clean();
				}
			}
			do_clean();
		}

		item::~item(void) {
			if (childs_ != nullptr) {
				delete  childs_;
			}
		}

		bool item::load(cstr _path) {
			#if ROBO_UNICODE_ENABLED == 1
			size_t pr_size = std::wcslen(_path);
			#else
			size_t pr_size = std::strlen(_path);
			#endif
			size_t max_size = pr_size + 1;
			size_t current_size = max_size;
			max_size = path_len(current_size,max_size);
			char_t* beg_path = new char_t[max_size];
			char_t* end_path = beg_path;
			size_t n = system::sprintf(end_path, max_size, RT("%s"), _path);
			end_path[n] = 0;
			end_path += n;
			max_size -= n;
			ROBO_LRET( load(beg_path, end_path, max_size))
		}

	}

}