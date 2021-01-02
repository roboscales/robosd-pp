#ifndef __robo_list__hpp
#define __robo_list__hpp

#include "core/robosd_common.hpp"

namespace robo{
	namespace list{
		template<typename T> class ROBO_EXPORT base;
		
		template<typename T> class ROBO_EXPORT ref{
			friend class base<T>;
		protected:
			/**—ледующа€ €чейка в списке.*/
			robo_cell_t * next_ = nullptr;
			/**ѕредыдуща€ €чейка в списке.*/
			robo_cell_t * prev_ = nullptr;	
			T & owner_;
			base<T> * list_ = nullptr;
			ref(T & _owner) : owner_(_owner){};
		public:
			virtual ~ref(void){
				dettach();
			}
			bool attached(void) {return list_ != nullptr;}
			void dettach(void){
				if(list_!=nullptr){
					list_->drop_p(this);
				}
			}
			T & owner(void) { return owner_; }
		};


		template<typename T> class ROBO_EXPORT base {
		protected:
			void  add_p( ref<T> * _ref, ref<T> * _older){
				if (first_ == nullptr){
					first_=_ref;
					last_ = _ref;
					_ref->prev = nullptr;
					_ref->next = nullptr;
				}else{
					;
					if (_older == first_){
						_older->prev_ = _ref;
						_ref->next = _older;
						_ref->prev_ = nullptr;
						first_ = _ref;
					}
					else {
						_older->prev_->next_ = _ref;
						_ref->prev_ = _older->prev_;
						_older->prev_ = _ref;
						_ref->next_ = _older;
					}
				}
				count_ ++;
				ref_ -> list_ = this;
			}

			void drop_p(ref<T> * _ref){
   				count_--;
				if (first_==last_){
					first_ = nullptr;
   					last_ = nullptr;
				}else{
					if (_ref == first_){
						first_ = ref_->next;
   						ref_->next_->prev_ = nullptr;
    				}else if( ref_ == last_ ){
	    				last_ = ref_->prev;
						ref_->prev-_>next_ = nullptr;
   					}else{
    					ref_-> prev_ -> next_ = ref_->next_;
	    				ref_ -> next_ -> prev_ = ref_ -> prev _;
   					}
				}
   				ref_ -> next_ = nullptr;
   				ref_ -> prev_ = nullptr;
				ref_ -> list_ = nullptr;
			}
			ref<T> * first_ = nullptr;
			ref<T> * last_= nullptr;
			int count_ = 0;
		public:
			typedef ref<T> ref;
			int count(void)(void){  return count_; } /**<  оличество €чеек. */
		};

		template<typename T> class ROBO_EXPORT unsorted  : public base<T>
			: public base<T> {
		public:
			class ref: public ref<T>{
			public:
				ref(T & _owner) : base<T>::ref(_owner){};
				void attach_to( unsorted & _unsorted){
					_unsorted->add_p(this, next());
				}
				ref * prev(void) { return (ref *)ref<T>::prev_; }
				ref * next(void) { return (ref *)ref<T>::next_; }

			};
			ref * first(void){  return (ref *)base<T>::first_; }; /**< ѕерва€ €чейка. */
			ref * last(void){  return (ref *)base<T>::last_; }; /**< ѕоследн€€ €чейка. */
		};
	

		template<typename T, typename L, typename K>  class ROBO_EXPORT pair: public ref<T>{		
			K key_;
		protected:
		public:
			void attach_to( L & _list){
				if(coiunt_>0){
					for ( ref * p = first(); p != nullptr; p = p->next()){
						if( p->key_ > key_){
							_list->L::add(this, p)
							return;
						}
					}
				} else {
					_list->L::add(this, nullptr)
				}
			}
			void set_key( const K &_key){
				sorted * old =(sorted *)own_list();
				if(old != nullptr){
					dettach();
					key_ = _key;
					attach_to(*old)
				} else {
					key_ = _key;
				}
			}
			const K &  key(void){ return key_; }
			pair(T & _owner) : ref<T>(_owner){};			
		};



	template<typename T, typename K> class ROBO_EXPORT sorted 
		: public base<T> {
		friend class ref;
	public:
		class ROBO_EXPORT ref: public pair<T>{
			friend class sorted;
		public:
			ref * prev(void) { return (ref *)list<T>::ref::prev_; }
			ref * next(void) { return (ref *)list<T>::ref::next_; }
			void set_key( const K &_key){
				sorted * old =(sorted *)own_list();
				if(old != nullptr){
					dettach();
					key_ = _key;
					attach_to(*old)
				} else {
					key_ = _key;
				}
			}
			const K &  key(void){ return key_; }
			ref(T & _owner) : list<T>::ref(_owner){};
			void attach_to( sorted & _sorted){
				if(coiunt_>0){
					for ( ref * p = first(); p != nullptr; p = p->next()){
						if( p->key_ > key_){
							_sorted->add(this, p)
							return;
						}
					}
				} else {
					_sorted->add(this, nullptr)
				}
			}
		};
		ref * first(void){  return (ref *)list<T>::first_; };
		ref * last(void){  return (ref *)list<T>::last_; };
	private:
		void add(ref * _ref, ref * _older ){
			add_p(_ref,_older);
		}
	};

	template<typename T, typename K> class ROBO_EXPORT map 
		: public list<T> {
		friend class ref;
	public:
		class ROBO_EXPORT ref: public list<T>{
			friend class list<T>;
			K key_;
		public:
			ref * prev(void) { return (ref *)list<T>::ref::prev_; }
			ref * next(void) { return (ref *)list<T>::ref::next_; }
			void set_key( const K &_key){
				sorted * old =(sorted *)own_list();
				if(old != nullptr){
					dettach();
					key_ = _key;
					attach_to(*old)
				} else {
					key_ = _key;
				}
			}
			const K &  key(void){ return key_; }
			ref(T & _owner) : list<T>::ref(_owner){};
			bool attach_to( sorted & _sorted){
				if(coiunt_>0){
					for ( ref * p = first(); p != nullptr; p = p->next()){
						ROBO_BREAKN( p->key_== key_)
						if( p->key_ > key_){
							_sorted->add(this, p);
							return true;
						}
					}
				} else {
					_sorted->add(this, nullptr)
				}
				return true;
			}
		};
		ref * first(void){  return (ref *)list<T>::first_; }; 
		ref * last(void){  return (ref *)list<T>::last_; }; 
		
		T * pop(void){
			ref * r  = base<T>::first_;
			if (r){
				r -> dettach();
				return (T *) &(r->owner()) );
			}
			else {
				return nullptr;
			}
		}

		void push(T * _t){
			ROBO_VBREAK( _t != nullptr );
			((ref *)(_t)) -> attach_to(*this);
		}

	private:
		void add(ref * _ref, ref * _older ){
			add_p(_ref,_older);
		}
	};

/*	namespace list{
		
		template<typename T> class ROBO_EXPORT base;
		
		template<typename T> class ROBO_EXPORT  ref  {
			robo_ref_t ref_;
			robo_list_p list__;
		public:
			ref(T * _owner) :  list__(0){
				robo_cell_init_p(&(ref_.cell));
				ref_.owner = _owner;
			}
			ref(T * _owner, int _id) : list__(0){
				robo_cell_init_p(&(ref_.cell));
				ref_.owner = _owner;
				ref_.cell.id = _id;
			}
			~ref(){
				dettach(); 
			}
			void dettach(){
				if (list__){
					robo_cell_drop_p(list__, &(ref_.cell));
					list__ = 0;
				}
			}
			robo_result_t attach_to(robo_list_p _list){
				if (list__ != _list){
					dettach();
					ROBO_CHECKRET(robo_cell_add_p(_list, &(ref_.cell)));
					list__ = _list;
				}
				return ROBO_SUCCESS;
			}
			robo_result_t set_id(int _id){
				robo_list_p tmp = list__;
				if (tmp){
					dettach();
					ref_.cell.id = _id;
					ROBO_RETEX(attach_to(tmp));
				}
				else{
					ref_.cell.id = _id;
					return ROBO_SUCCESS;
				}
			}
			inline ref * next(){ return (ref *)(ref_.cell.next ); }
			inline ref * prev(){ return (ref *)(ref_.cell.prev); }
			inline T * owner() { return (T *)(ref_.owner); }
			inline operator robo_list_p (){
				return list__;
			}
			inline int id(){ return ref_.cell.id;  }
			inline bool attached(){ return list__!=0; }
		};
		
		template<typename T> class ROBO_EXPORT base{
		protected:
			robo_list_t list__;
		public:
			inline int count(){ return list__.count; }

			inline operator robo_list_p (){
				return &list__;
			}
			void inc_id(int _delta){
				for (robo_cell_p cell = list__.first; cell; cell = cell->next){
					cell->id += _delta;
				}
			}
			void dec_id(int _delta){
				for (robo_cell_p cell = list__.first; cell; cell = cell->next){
					cell->id -= _delta;
				}
			}

			void forall(lambda< void(T *) > _operator){
				for (robo_cell_p cell = list__.first; cell; cell = cell->next){
					_operator( (T *)(((robo_ref_p)cell)->owner) );
				}
			}

			inline ref<T> * first(){ return (ref<T> *)(list__.first); }
			inline ref<T> * last(){ return (ref<T> *)(list__.last); }
		protected:
			base(robo_list_style_t _style){
				robo_list_init_p(&list__, _style);
			}
			~base(){
				robo_list_clean_p(&list__);
			}
		};

		template<typename T> class ROBO_EXPORT  map : public base<T>  {
		public:
			map() :base<T>(ROBO_LIST_MAP){}
			~map(){	}
			T * find(int _id){
				robo_cell_p cell = robo_cell_get(&(base<T>::list__), _id);
				if (cell){
					return ((T *)(((robo_ref_p)cell)->owner));
				}
				else {
					return 0;
				}
			}
			
			T * operator [](int _id){
				T * ret = find(_id);
				if (ret){
					return ret;
				}
				else {
					robo_errlog("item with id %d is't found", _id);
					return 0;
				}
			}

		};

		template<typename T> class ROBO_EXPORT  queue : public base<T>{
		public:
			typedef list::ref<T> ref;
			queue(bool _sorted = false) :base<T>(_sorted ? ROBO_LIST_SORTED : ROBO_LIST_QUEUE){}
			~queue(){}
			void push(T * _t){
				if (_t){
					((ref *)(*_t))->attach_to(*this);
				}
			}
			T * pop(){
				robo_cell_p cell = base<T>::list__.first;
				if (cell){
					((ref *)cell)->dettach();
					return (T *)(((robo_ref_p)(cell))->owner);
				}
				else {
					return 0;
				}
			}
		};
		
		template<typename T> class ROBO_EXPORT  sorted : public base<T>{
		public:
			sorted() :base<T>(ROBO_LIST_SORTED){}
			~sorted(){}
		};
		template<typename T> class ROBO_EXPORT  unsorted : public base<T>{
		public:
			unsorted() :base<T>(ROBO_LIST_QUEUE){}
			~unsorted(){}
		};
		
	}*/
}
#endif
