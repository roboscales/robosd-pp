#include "net/robosd_modbus.hpp"
namespace robo{
	namespace net{
		namespace modbus{		
			#if 0
			phy::phy(void): entry_(nullptr), ref_(*this,0){
				//некому вызывать функции, которыебы задействовали entry_
			};
			phy * phy::dummy_(void){
				static class dummy:public phy{
				public:
					dummy():phy(){};
				protected:
					virtual bool startReceive(void){return false;};
					virtual bool startSend(const uint8_t * /*_packet*/, size_t /*_size*/){return false;};
				} dummy__;
				return & dummy__;
			};
			phy::map & phy::map_(void){
				static map map__;
				return map__;
			}
			
			phy::phy(cstr _alias): entry_(entry::dummy_() ), ref_(*this,hash(_alias)){
				ref_.attach_to(map_());			
			};
			
			void phy::onReceive(const uint8_t * _packet, size_t _size){ 
				entry_->onReceive(_packet,_size);
			}
			void phy::onSend(void){ 
				entry_->onSend(); 
			}
			void phy::onSendError(void){ 
				entry_->onSendError(); 
			}
			void phy::onReceiveError(void){ 
				entry_->onReceiveError(); 
			}

			entry::entry(phy *  _dummy) : phy_(_dummy){}

			entry * entry::dummy_(void){
				static class dummy:public entry{
				public:
				//некому вызывать функции, которыебы задействовали phy_
					dummy():entry(nullptr){};
				protected:
				} dummy__;
				return & dummy__;
			};
			
			bool entry::start(cstr _phys){
				if( phy_ != phy::dummy_() ){
					phy_->entry_ = dummy_();
				}
				phy * ph = phy::map_().find(hash(_phys, 0));			
				if(ph!=nullptr){
					phy_ = ph;
					ph->entry_ = this;
					return true;
				} else {
					phy_ = phy::dummy_();
					return false;
				}
			}
			void entry::onReceive(const uint8_t * _packet, size_t _size){
			}
			void entry::onSend(void){
			}
			void entry::onSendError(void){
			}
			void entry::onReceiveError(void){
			}
	//
			entry::entry(void): phy_(phy::dummy_()){
			}
			#endif
		}
	}

}
