#ifndef robo_list_hpp
#define robo_list_hpp

#include "core/robosd_common.hpp"

namespace robo {

	namespace list {

		template<typename T> class ROBO_EXPORT base;

		template<typename T> class ROBO_EXPORT base_ref {
			friend class base<T>;
			T& owner_;
			base<T>* list_ = nullptr;
		protected:
			base_ref* next = nullptr;
			base_ref* prev = nullptr;
			base_ref(T& _owner) : owner_(_owner) {};
			void attach_to(base<T>& _list) {
				dettach();
				_list.add_before_p(this, nullptr);
			}
			void attach_after(base<T>& _list, base_ref<T>* _prev) {
				dettach();
				_list.add_after_p(this, _prev);
			}
			void attach_before(base<T>& _list, base_ref<T>* _next) {
				dettach();
				_list.add_before_p(this, _next);
			}
		public:
			virtual ~base_ref(void) {
				dettach();
			}

			/**ссылка на объект находитс¤ в списке*/
			bool attached(void) { return list_ != nullptr; }

			/**убрать ссылку из текущего списка*/
			void dettach(void) {
				if (attached()) {
					list_->drop_p(this);
				}
			}

			/**владелец ссылки (объект)*/
			T& owner(void) { return owner_; }

			/**текущий список*/
			base<T>* own_list(void) { return list_; };
		};

		template<typename T> class ROBO_EXPORT base {
			int count_ = 0;
		protected:
			friend class base_ref<T>;
			void  add_before_p(base_ref<T>* _ref, base_ref<T>* _next) {
				if (_next) {
					ROBO_APP_ASSERT(_next->list_ == this)
				}
				if (first == nullptr) {
					first = _ref;
					last = _ref;
					_ref->prev = nullptr;
					_ref->next = nullptr;
				}
				else {
					if (_next == nullptr) {
						last->next = _ref;
						_ref->prev = last;
						last = _ref;
					}
					else {
						if (_next == first) {
							_next->prev = _ref;
							_ref->next = _next;
							_ref->prev = nullptr;
							first = _ref;
						}
						else {
							_next->prev->next = _ref;
							_ref->prev = _next->prev;
							_next->prev = _ref;
							_ref->next = _next;
						}
					}
				}
				count_++;
				_ref->list_ = this;
				ROBO_APP_ASSERT(last->next == 0);
				if (_ref != first) {
					ROBO_APP_ASSERT(_ref->prev != 0);
				}
			}

			void  add_after_p(base_ref<T>* _ref, base_ref<T>* _prev) {
				if (_prev) {
					ROBO_APP_ASSERT(_prev->list_ == this)
				}
				if (first == nullptr) {
					first = _ref;
					last = _ref;
					_ref->prev = nullptr;
					_ref->next = nullptr;
				}
				else {
					if (_prev == nullptr) {
						last->next = _ref;
						_ref->prev = last;
						last = _ref;
					}
					else {
						if (_prev == last) {
							_prev->next = _ref;
							_ref->prev = _prev;
							_ref->next = nullptr;
							last = _ref;
						}
						else {
							_ref->next = _prev->next;
							_ref->next->prev = _ref;
							_prev->next = _ref;
							_ref->prev = _prev;
						}
					}
				}
				count_++;
				_ref->list_ = this;
				ROBO_APP_ASSERT(last->next == 0);
				if (_ref != first) {
					ROBO_APP_ASSERT(_ref->prev != 0);
				}
			}

			void drop_p(base_ref<T>* _ref) {
				count_--;
				if (first == last) {
					first = nullptr;
					last = nullptr;
				}
				else {
					if (_ref == first) {
						first = _ref->next;
						_ref->next->prev = nullptr;
					}
					else if (_ref == last) {
						last = _ref->prev;
						_ref->prev->next = nullptr;
					}
					else {
						_ref->prev->next = _ref->next;
						_ref->next->prev = _ref->prev;
					}
				}
				_ref->next = nullptr;
				_ref->prev = nullptr;
				_ref->list_ = nullptr;
			}
			base_ref<T>* first = nullptr;
			base_ref<T>* last = nullptr;
		public:
			/**  оличество элементов списка */
			int count(void) { return count_; }

			T* pop(void) {
				base_ref<T>* r = first;
				if (r) {
					r->dettach();
					return &(r->owner());
				}
				else {
					return nullptr;
				}
			}

			base_ref<T> * locate(T * _own) {
				for (base_ref<T>* p = first; p != nullptr; p = p->next) {
					if (&(p->owner()) == _own) {
						return p;
					}
				}
				return nullptr;
			}
		};

		template<typename T> class ROBO_EXPORT unsorted : public base<T> {
		public:
			class ROBO_EXPORT ref : public base_ref<T> {
			public:
				ref(T& _owner) : base_ref<T>(_owner) {};
				/** добавить ссылку в конец списка */
				void attach_to(unsorted& _L) {
					base_ref<T>::attach_to(_L);
				}
				void attach_before(unsorted& _L, base_ref<T>* _prev) {
					base_ref<T>::attach_before(_L, _prev);
				}
				void attach_after(unsorted& _L, base_ref<T>* _next) {
					base_ref<T>::attach_after(_L, _next);
				}

				void attach_before(unsorted& _L, T * _prev) {
					base_ref<T>::attach_before(_L, _L.locate(_prev));
				}
				void attach_after(unsorted& _L, T * _next) {
					base_ref<T>::attach_after(_L, _L.locate(_next));
				}

				/** предыдущая ссылка в списке */
				ref* prev(void) { return (ref*)base_ref<T>::prev; }

				/** следующая ссылка в списке */
				ref* next(void) { return (ref*)base_ref<T>::next; }

			};

			/** первая ссылка в списке */
			ref* first(void) { return (ref*)base<T>::first; };

			/** последняя ссылка в списке */
			ref* last(void) { return (ref*)base<T>::last; };
		};

		template<typename T, typename K, typename L, bool unique>  class ROBO_EXPORT pair : public base_ref<T> {
			K key_;
		public:
			void hack_key_(const K& _key) {
				key_ = _key;
			}

			/** добавить ссылку в ссписок согласно пор¤дку  */
			bool attach_to(L& _list) {
				if (_list.count() > 0) {
					if (_list.L::sort()) {
						for (pair* p = _list.first(); p != nullptr; p = p->next()) {
							if (unique && p->key_ == key_) return false;
							if (p->key_ > key_) {
								base_ref<T>::attach_before(_list, p);
								return true;
							}
						}
					}
					else {
						base_ref<T>::attach_to(_list);
						return true;
					}
				}
				base_ref<T>::attach_to(_list);
				return true;
			}

			/** изменить ключ  */


			bool set_key(const K& _key) {
				L* old = (L*)base_ref<T>::own_list();
				if (old != nullptr) {
					base_ref<T>::dettach();
					key_ = _key;
					return attach_to(*old);
				}
				else {
					key_ = _key;
					return true;
				}
			}

			/** текущее значение ключа  */
			const K& key(void) { return key_; }

			pair(T& _owner, const K& _key) : base_ref<T>(_owner), key_(_key) {};
			pair* prev(void) { return (pair*)base_ref<T>::prev; }
			pair* next(void) { return (pair*)base_ref<T>::next; }



		};

		template<typename T, typename K> class ROBO_EXPORT sorted
			: public base<T> {
		protected:
			bool  sort_ = true;
		public:
			bool sort(void) { return sort_; }
			typedef pair<T, K, sorted<T, K>, false > ref;
			ref* first(void) { return (ref*)base<T>::first; }; /**первая ячейка. */
			ref* last(void) { return (ref*)base<T>::last; }; /**последняя ячейка. */
			void inc_key(const K& _delta) {
				for (ref* _ref = first(); _ref; _ref = _ref->next()) {
					K key = _ref->key();
					key = key + _delta;
					_ref->hack_key_(key);
				}
			}

		};

		template<typename T, typename K> class ROBO_EXPORT pool : public sorted< T, K > {
		public:
			pool(void) :sorted< T, K>() { sorted< T, K >::sort_ = false; }
		};

		template<typename T, typename K> class ROBO_EXPORT unique
			: public base<T> {
		public:
			bool sort(void) { return true; }
			typedef pair<T, K, unique<T, K>, true > ref;
			ref* first(void) { return (ref*)base<T>::first; }; /**первая ячейка. */
			ref* last(void) { return (ref*)base<T>::last; };  /**последняя ячейка. */
			ref* at(const K& _key) {
				for (ref* _ref = first(); _ref; _ref = _ref->next()) {
					if (_ref->key() == _key) {
						return  _ref;
					}
				}
				return nullptr;
			}

			T* find(const K& _key) {
				ref* r = at(_key);
				if (r) return &(r->owner());
				return nullptr;
			}

		};
			

		template <typename T,  typename...Arg> class ROBO_EXPORT unidir_t{
			public:
				class ROBO_EXPORT item : public T{
					friend class unidir_t;
					item * next_ = nullptr;				
				public:
					item* next(void) { return next_;  }
					item(Arg... arg):T(arg...){};
					~item(void){  }
				};	
			private:
				item * first_ = nullptr;
				item * last_ = nullptr;				
				int count_ = 0;
			public:
				item* first(void) { return first_; };
				item* last(void) { return last_; };
				int count(void) { return count_; }
				unidir_t(void){
				}
				~unidir_t(void){
				}
				void push(item & _item) {
					ROBO_APP_ASSERT(_item.next_==nullptr);
					if(last_==nullptr){
						last_= first_ = &_item;
					} else{
						last_ = (last_->next_ = &_item);						
					}
					count_ ++;
				}
				item * pop(void) {
					if(first_!= nullptr){
						item * tmp = first_;
						if( tmp->next_ == nullptr ){
							first_ = last_ = nullptr;
						} else {
							first_ = tmp->next_;
						}
						ROBO_APP_ASSERT( count_ > 0 );						
						return tmp;
					}	else {
						return nullptr;
					}
				}
				void reset(void){
					while(first_){
						first_->pop();
					}
				}
				void free(void){
					while(first_){
						item * tmp = pop();
						delete tmp;
					}
				}
		};
	}
	namespace queue {
		template<typename T, typename L> class ROBO_EXPORT base
			: public L {
		public:
			typedef typename L::ref ref;
			void push(T* _t) {
				if (_t) {
					ref& tmp = (ref&)(*_t);
					tmp.ref::attach_to(*this);
				}
			}
		};

		template<typename T> class ROBO_EXPORT fifo : public base< T, ::robo::list::unsorted<T> > {};
		template<typename T> class ROBO_EXPORT priority : public base< T, ::robo::list::sorted<T, typename T::priority_t> > {};
	}
}

#endif
