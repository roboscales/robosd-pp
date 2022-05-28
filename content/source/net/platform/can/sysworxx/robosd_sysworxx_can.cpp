#include "net\platform\can\sysworxx\robosd_sysworxx_can.hpp"
#include "core\robosd_log.hpp"
#include "core\robosd_ini.hpp"

namespace robo {
	namespace net {
		namespace can {
			namespace sysworxx {
				port * port::ports_[port::chan_count] = {};
				
				void port::on_read_(BYTE bChannel_p, tCanMsgStruct* pCanMsg_p) {
					port *  can = ports_[bChannel_p];
					if (can && can->on_receive)
						(*(can->on_receive))(*can, pCanMsg_p->m_dwID, pCanMsg_p->m_bData, (DWORD)pCanMsg_p->m_bDLC);
				}

				port::port(void) : ican() {}
				bool port::open_(void) {
					channel_ = -1;
					ROBO_JAMPN(ini::load(name_, RT("bitrate_presc"), bitrate_), error);
					ROBO_JAMPN(ini::load(name_, RT("channel"), channel_), error);

					ROBO_JAMPN_F(channel_< port::chan_count, error, "Failed to open channel %d  (is greater than the allowed value)", channel_);
					ROBO_JAMPN_F(ports_[channel_]==nullptr, error, "Failed to open channel %d  (allready used)", channel_);
					ROBO_JAMPN_F(UCanInit(on_read_, USBCAN_CHANNEL_CH0, USBCAN_ANY_MODULE, bitrate_, channel_, 0x7FF) == TRUE, error, "Failed to open channel %d with bitrate (%d)", channel_, bitrate_);
					ports_[channel_] = this;

					return true;
				error:
					close();
					ROBO_LBREAK();
				}

				bool port::open(cstr _name) {
					name_ = _name;
					ROBO_LRET(open_());
				}
				void port::close(void) {
					if (channel_ > 0) {
						UCanShutDown(channel_);
						ports_[channel_] = nullptr;
					}
				}
				bool port::send(uint32_t _id, const uint8_t* _buf, uint8_t  _len) {
					tCanMsgStruct tmp = {};					
					tmp.m_dwID = _id;
					tmp.m_bDLC = _len;
					std::copy_n(_buf,_len,tmp.m_bData);
					ROBO_LRET( UCanWriteMsg(channel_, &tmp, 1) == TRUE );
				}
				bool port::ready(void) {
					return  UCanReady(channel_) == TRUE;
				}
				void port::reset(void) {
					UCanReset(channel_, 0);
				}
				void port::poll(void) {
					UcanPoll(channel_);
				}
			}
		}
	}
}