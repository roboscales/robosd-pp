#include "mexo/vartable.hpp"
namespace mexo{
	namespace var {
		void record::setup_(
				types _type
			, const void * _addr
			, robo::cstr _name
			, int  _master_key
		)
		{
			addr = _addr;
			name = _name;
			key = robo::fast_hash(RT("."),_master_key);
			key = robo::fast_hash(_name,key);
			desc.memo = _type;
			//machine::reg(this);
		}			

		machine & machine::instance_(void){
			static machine machine_;
			return machine_;
		}
		
		void machine::begin_(int _pool_size){
			if (index_ !=nullptr) {
				delete [] index_;
				index_ = nullptr;
			}
			index_size_ = _pool_size;
			if(index_size_  > 0 ){
				index_ =  new const record * [index_size_];
			}
			count_ = 0;
		}
		
		machine::~machine(void){
			if (index_ !=nullptr) delete [] index_;
		}
		
		int machine::find_(int _key){
			const record ** r = index_;
			for(int i=0;i<count_; ++i, ++r ){
				if((*r)->key == _key){
					return i;
				}
			}
			return -1;
		}
		const record * machine::get_(int _index){
			if(_index>=0 && _index<count_){
				return index_[_index];
			} else return nullptr;
		}
		void  machine::reg_(
			const record& _precord
		){
			ROBO_APP_ASSERT(count_ < index_size_);
			int ix = find_(_precord.key);
			ROBO_APP_ASSERT(ix<0);
			index_[ count_ ] = &_precord;
			count_++;
		}
	}
}