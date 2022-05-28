#include "net\platform\can\vscom\robosd_vscom_can.hpp"
#include "core\robosd_log.hpp"
#include "core\robosd_ini.hpp"

namespace robo {
	namespace net {
		namespace can {
			namespace vscom {
				port::port(void) : ican() {}
				bool port::open_(void) {
					VSCAN_API_VERSION version;
					enum { sz = 255 };
					char buf[sz];
					string com;

					/* configure the default debug options*/
					ROBO_JAMPN_F(VSCAN_Ioctl(0, VSCAN_IOCTL_SET_DEBUG_MODE, VSCAN_DEBUG_MODE_CONSOLE) == VSCAN_ERR_OK, error, "Failed to set debug mode");
					ROBO_JAMPN_F(VSCAN_Ioctl(0, VSCAN_IOCTL_SET_DEBUG, VSCAN_DEBUG_LOW) == VSCAN_ERR_OK, error, "Failed to set debug level");
					ROBO_JAMPN_F(VSCAN_Ioctl(0, VSCAN_IOCTL_GET_API_VERSION, &version) == VSCAN_ERR_OK, error, "Failed to get API version");
					robo_infolog("VSCAN-API Version %d.%d.%d\n", version.Major, version.Minor, version.SubMinor);

					ROBO_JAMPN(com.load(name_, RT("comm")), error);
					buf[snprintf(buf, sz - 1, "%ws", com.c_str())] = 0;
					int btr;
					ROBO_JAMPN(ini::load(name_, RT("bitrate_kBit"), btr), error);

					switch (btr) {
					case 20: bitrate_ = VSCAN_SPEED_20K; break;
					case 50: bitrate_ = VSCAN_SPEED_50K; break;
					case 100: bitrate_ = VSCAN_SPEED_100K; break;
					case 125: bitrate_ = VSCAN_SPEED_125K; break;
					case 250: bitrate_ = VSCAN_SPEED_250K; break;
					case 500: bitrate_ = VSCAN_SPEED_500K; break;
					case 800: bitrate_ = VSCAN_SPEED_800K; break;
					case 1000: bitrate_ = VSCAN_SPEED_1M; break;
					default:
					ROBO_JAMP_F(error, "Wrong bitrate %ld", btr);
					}

					handle_ = VSCAN_Open(buf, VSCAN_MODE_NORMAL);

					ROBO_JAMPN_F(handle_ > 0, error, "VSCAN_Open failed for %S", buf);

					ROBO_JAMPN_F((VSCAN_Ioctl(handle_, VSCAN_IOCTL_SET_SPEED, bitrate_) == VSCAN_ERR_OK), error, "Setting CAN speed failed %ld", bitrate_);

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
					if (handle_ > 0) {
						VSCAN_Close(handle_);
						handle_ = -1;
					}

				}
				bool port::send(uint32_t _id, const uint8_t* _buf, uint8_t  _len) {
					VSCAN_MSG msg = {};
					DWORD rv;
					msg.Id = _id;
					std::copy_n(_buf, _len, msg.Data);
					msg.Size = _len;
					ROBO_LBREAKN_F(VSCAN_Write(handle_, &msg, 1, &rv) == VSCAN_ERR_OK, "VSCAN_Write failed");
					ROBO_LBREAKN_F(VSCAN_Flush(handle_) == VSCAN_ERR_OK, "VSCAN_Flush failed");
					return true;
				}
				bool port::ready(void) {
					DWORD flags;
					ROBO_LBREAKN_F(VSCAN_Ioctl(handle_, VSCAN_IOCTL_GET_FLAGS, &flags) == VSCAN_ERR_OK, "VSCAN_Ioctl VSCAN_IOCTL_GET_FLAGS failed");
					return flags == VSCAN_ERR_OK;

				}
				void port::reset(void) {
					close();
					open_();

				}
				void port::poll(void) {
					VSCAN_MSG msg;
					DWORD rv;
					ROBO_ALARMN_F(VSCAN_Read(handle_, &msg, 1, &rv) == VSCAN_ERR_OK, "VSCAN_Read failed");
					if (rv > 0) {
						if (on_receive)
							(*on_receive)(*this, msg.Id, msg.Data, msg.Size);
					}
				}
			}
		}
	}
}