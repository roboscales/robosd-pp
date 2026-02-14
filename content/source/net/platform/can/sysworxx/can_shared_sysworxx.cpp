#include "net/platform/can/sysworxx/can_shared_sysworxx.hpp"
#include "core/robosd_string.hpp"
#include "core/robosd_ini.hpp"
#include "CanItf.h"
namespace robo {
	namespace net {
		namespace can{
			namespace sysworxx {
				class ROBO_EXPORT shared {
					int bitrate_;
					int channel_;
					string name_;
					bool extended_;
					int link_count_ = 0;
					bool open_(port::ref & _ref) {
						if (link_count_ == 0) {
							name_ = RT("sysworxx shared");
							channel_ = -1;
							ROBO_JAMPN(ini::load(name_, RT("bitrate_presc"), bitrate_), error);
							ROBO_JAMPN(ini::load(name_, RT("channel"), channel_), error);
							ROBO_JAMPN(ini::load(name_, RT("extended"), extended_), error);
							ROBO_JAMPN_F(UCanInit(on_read_, USBCAN_CHANNEL_CH0, USBCAN_ANY_MODULE, bitrate_, channel_, 0x7FF) == TRUE, error, "Failed to open channel %d with bitrate (%d)", channel_, bitrate_);
						}
						link_count_++;
						_ref.attach_to(ports_);
						return true;
					error:
						close_();
						ROBO_LBREAK();

					}
					port::list ports_;
					void on_read__(BYTE bChannel_p, tCanMsgStruct* pCanMsg_p) {
						for (auto* it = ports_.first(); it; it = it->next()) {
							auto& can = it->owner();
							can.receive(pCanMsg_p->m_dwID, pCanMsg_p->m_bData, (DWORD)pCanMsg_p->m_bDLC);
						}

					}
					shared(void) {

					}
					static void on_read_(BYTE bChannel_p, tCanMsgStruct* pCanMsg_p) {
						instance_().on_read__(bChannel_p, pCanMsg_p);
					}
					virtual void close_(void) {
						link_count_--;
						if (link_count_ <= 0) {
							if (channel_ > 0) {
								UCanShutDown(channel_);
							}
							link_count_ = 0;
						}
					}
					virtual bool send_(uint32_t _id, const uint8_t* _buf, uint8_t  _len) {
						tCanMsgStruct tmp = {};
						if (extended_) {
							tmp.m_dwID = _id & 0x1FFFFFFF;
							tmp.m_bFF = USBCAN_MSG_FF_EXT;
						}
						else {
							tmp.m_dwID = _id & 0x7FF;
							tmp.m_bFF = USBCAN_MSG_FF_STD;
						}
						tmp.m_bDLC = _len;
						std::copy_n(_buf, _len, tmp.m_bData);
						ROBO_LRET(UCanWriteMsg(channel_, &tmp, 1) == TRUE);
					}
					virtual bool ready_(void) {
						return  UCanReady(channel_) == TRUE;
					}
					virtual void reset_(void) {
						UCanReset(channel_, 0);
					}
					virtual void poll_(void) {
						UcanPoll(channel_);
					}
					static inline shared & instance_(void) {
						static shared instance_;
						return instance_;
					}
				public:
					static bool open(port::ref& _ref) {
						ROBO_LRET(instance_().open_(_ref));
					}
					static void close(void) {
						instance_().close_();
					}
					static bool send(uint32_t _id, const uint8_t* _buf, uint8_t  _len) {
						ROBO_LRET(instance_().send_(_id,_buf,_len));
					}
					static bool ready(void) {
						return  instance_().ready_();
					}
					static void reset(void) {
						instance_().reset_();
					}
					static void poll(void) {
						instance_().poll_();
					}

				};
				bool port::open(bool/* _owned_view */ ) {
					ROBO_LRET(shared::open(ref_));
				}
				void port::close(void) {
					shared::close();
					ref_.dettach();
				}
				bool port::send(uint32_t _id, const uint8_t* _buf, uint8_t  _len) {
					ROBO_LRET(shared::send(_id,_buf,_len));
				}
				bool port::ready(void) {
					return shared::ready();
				}
				void port::reset(void) {
					shared::reset();
				}
				void port::poll(void) {
					shared::poll();
				}
				void port::receive(uint32_t _id , const uint8_t* _data, uint8_t _size) {
					if (on_receive) {
						(*on_receive)(*this, _id, _data, _size);
					}

				}
			}
		}
	}
}

