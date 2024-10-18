#ifndef __robo_element_hpp
#define  __robo_element_hpp
#include "core/robosd_string.hpp"
#include "core/robosd_list.hpp"

namespace robo {
	namespace tree {

		class item {
		public:
			typedef ::robo::list::unique<item, int> list;
		private:
			item* branch_;
			list* childs_ = nullptr;
			list::ref  ref_;
		protected:
			size_t path_len(size_t _carrent, size_t _max);
			template <typename T> T& branch(void) { return (T&)(*branch_); }
			template <typename T> T* pbranch(void) { return (T*)branch_; }
		public:
			string name;
			int key() const { return ref_.key(); }
		protected:
			item(cstr _name, item* _branch);
			virtual bool do_load(cstr _path) { return true;  }
			virtual void do_clean(void) {}

			bool load(char_t* _beg_path, char_t* _end_path, size_t  _path_size);
		public:
			template <typename T> bool forall(char_t* _beg_path, char_t* _end_path, size_t  _path_size, T _fun) {
				size_t n = name.length() + 1;
				ROBO_APP_ASSERT(_path_size >= n + 1);
				n = system::sprintf(_end_path, _path_size, RT(".%s"), name.c_str());
				_end_path[n] = 0;
				_end_path += n;
				_path_size -= n;
				ROBO_LBREAKN(_fun(*this, _beg_path));
				if (childs_ != nullptr) {
					for (list::ref* r = childs_->first(); r; r = r->next()) {
						ROBO_LBREAKN(r->owner().forall(_beg_path, _end_path, _path_size, _fun));
					}
				}
				return true;
			}
			void clean(void);
			bool load(cstr _path);
			virtual ~item(void);
		};

	
	}

}

#endif
