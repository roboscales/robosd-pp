#ifndef __robo_list_hpp
#define __robo_list_hpp

#include "core/robosd_common.hpp"

namespace robo{

	namespace list{

		template<typename T> class ROBO_EXPORT base;
		
		template<typename T> class ROBO_EXPORT base_ref{
			friend class base<T>;
			T & owner_;
			base<T> * list_ = nullptr;
		protected:			
			base_ref * next = nullptr;			
			base_ref * prev = nullptr;	
			base_ref(T & _owner) : owner_(_owner){};
			void attach_to( base<T> & _list, base_ref<T> * _next){
				_list.add_p( this, _next );
			}
		public:
			virtual ~base_ref(void){
				dettach();
			}
			
			/**ссылка на объект находится в списке*/
			bool attached(void) {return list_ != nullptr;}
			
			/**убрать ссылку из текущего списка*/
			void dettach(void){
				if(attached()){
					list_->drop_p(this);
				}
			}

			/**владелец ссылки (объект)*/
			T & owner(void) { return owner_; }

			/**текущий список*/
			base<T> * own_list(void) { return list_; };
		};
		
		template<typename T> class ROBO_EXPORT base {
			int count_ = 0;
		protected:
			friend class base_ref<T>;
			void  add_p( base_ref<T> * _ref, base_ref<T> * _next){
				if (first == nullptr){
					first =_ref;
					last = _ref;
					_ref->prev = nullptr;
					_ref->next = nullptr;
				}else{
					if(_next == nullptr){
						last->next = _ref;
						_ref->prev = last;
						last = _ref;
					} else {
						if (_next == first){
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
				count_ ++;
				_ref -> list_ = this;
			}

			void drop_p(base_ref<T>* _ref){
   				count_--;
				if (first==last){
					first = nullptr;
   					last = nullptr;
				}else{
					if (_ref == first){
						first = _ref->next;
   						_ref->next->prev = nullptr;
    				}else if( _ref == last ){
	    				last = _ref->prev;
						_ref->prev->next = nullptr;
   					}else{
    					_ref -> prev -> next = _ref->next;
	    				_ref -> next -> prev = _ref -> prev;
   					}
				}
   				_ref -> next = nullptr;
   				_ref -> prev = nullptr;
				_ref -> list_ = nullptr;
			}
			base_ref<T> * first = nullptr;
			base_ref<T> * last= nullptr;
		public:
			/** Количество элементов списка */
			int count(void){  return count_; } 
		};

		template<typename T> class ROBO_EXPORT unsorted  : public base<T> {
			public:
				class ROBO_EXPORT ref: public base_ref<T>{
				public:
					ref(T & _owner) : base_ref<T>(_owner){};
					/** Добавить ссылку в конец списка */
					void attach_to( unsorted & _L){
						base_ref<T>::dettach();
						base_ref<T>::attach_to(_L, nullptr);
					}

					/** предыдущая ссылка в списке */
					ref * prev(void) { return (ref *)base_ref<T>::prev; }

					/** следующая ссылка в списке */
					ref * next(void) { return (ref *)base_ref<T>::next; }

				};

				/** первая ссылка в списке */
				ref * first(void){  return (ref *)base<T>::first; };

				/** последняя ссылка в списке */
				ref * last(void){  return (ref *)base<T>::last; };
		};

		template<typename T, typename K, typename L, bool unique>  class ROBO_EXPORT pair: public base_ref<T>{		
			K key_ ;
		public:

			/** Добавить ссылку в ссписок согласно порядку  */
			bool attach_to( L & _list){
				if(_list.count()>0){
					for ( pair * p = _list.first(); p != nullptr; p = p->next()){
						if(unique && p->key_ == key_) return false;
						if( p->key_ > key_){
							base_ref<T>::dettach();
							base_ref<T>::attach_to(_list, p);
							return true;
						}
					}
				} 
				base_ref<T>::attach_to(_list, nullptr);
				return true;
			}

			/** Изменить ключ  */
			bool set_key( const K &_key){
				L * old =(L *)own_list();
				if(old != nullptr){
					dettach();
					key_ = _key;
					return attach_to(*old);
				} else {
					key_ = _key;
					return true;
				}
			}

			/** Текущее значение ключа  */
			const K &  key(void){ return key_; }

			pair(T & _owner, const K & _key) : base_ref<T>(_owner), key_(_key){};			
			pair * prev(void) { return (pair *)base_ref<T>::prev; }
			pair * next(void) { return (pair *)base_ref<T>::next; }

		};
		
		template<typename T, typename K> class ROBO_EXPORT sorted 
			: public base<T> {
		public:
			typedef pair<T,K, sorted<T,K>, false > ref;
			ref * first(void){  return (ref *)base<T>::first; }; /**< Первая ячейка. */
			ref * last(void){  return (ref *)base<T>::last; }; /**< Последняя ячейка. */
		};

		template<typename T, typename K> class ROBO_EXPORT unique 
			: public base<T> {
		public:
			typedef pair<T,K, unique<T,K>, true > ref;
			ref * first(void){  return (ref *)base<T>::first; }; /**< Первая ячейка. */
			ref * last(void){  return (ref *)base<T>::last; }; /**< Последняя ячейка. */
		};
	}

	template<typename T, typename L> class ROBO_EXPORT queue_base
		: public L {
		public:

		void push(T * _t){
			if (_t){
				((L::ref &)(*_t)).attach_to(*this);
			}
		}

		T * pop(){
			L::ref * r = L::first();
			if (r){
				r->dettach();
				return & (r->owner());
			}
			else {
				return nullptr;
			}
		}
	};

	namespace queue{			
		template<typename T> class ROBO_EXPORT fifo  : public queue_base< T, ::robo::list::unsorted<T> > {};			
		template<typename T> class ROBO_EXPORT priority   : public queue_base< T, ::robo::list::sorted<T,typename T::priority_t> > {};			
	};
}
#endif
