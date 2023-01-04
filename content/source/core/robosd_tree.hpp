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
			list* childs_;
			list::ref  ref_;
		protected:
			size_t path_len(size_t _carrent, size_t _max);
			template <typename T> T& branch(void) { return (T&)(*branch_); }
		public:
			string name;
			int key() const { return ref_.key(); }
		protected:
			item(cstr _name, item* _branch);
			virtual bool do_load(cstr _path) { return true;  }
			virtual void do_clean(void) {}

			bool load(char_t* _beg_path, char_t* _end_path, size_t  _path_size);
		public:
			void clean(void);
			bool load(cstr _path);
			virtual ~item(void);
		};

	
	}

}

#endif
