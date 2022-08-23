#ifndef robosd_net_modbus_hpp
#define robosd_net_modbus_hpp
#include "core/robosd_list.hpp"
namespace robo{
	namespace net{
		namespace modbus{
			class entry;
			class phy{				
				friend class entry;
				public:
				typedef ::robo::list::unique<phy, int> map;
				typedef map::ref ref;
				private:
				entry * entry_;
				ref ref_;
				static map & map_(void);
				static phy * find_(cstr _alias);
				
				static phy * dummy_(void);
				phy(void);
				
				public:
					phy(cstr _alias);
				public:
					void onReceive(const uint8_t * _packet, size_t _size);
					void onSend(void);
					void onSendError(void);
					void onReceiveError(void);
				protected:
					virtual bool startReceive()=0;
					virtual bool startSend(const uint8_t * _packet, size_t _size)=0;
			};
			
			class entry{
				friend class phy;
				phy * phy_;
				static entry * dummy_(void);
				entry(phy * _dummy);
				public:
					entry(void);
					bool start(cstr _phys);				
					void onReceive(const uint8_t * _packet, size_t _size);
					void onSend(void);
					void onSendError(void);
					void onReceiveError(void);
					virtual bool startReceive(void){ return phy_->startReceive();}
					virtual bool startSend(const uint8_t * _packet, size_t _size){ return phy_->startSend(_packet,_size);};
			};
		}
	}
}
#endif