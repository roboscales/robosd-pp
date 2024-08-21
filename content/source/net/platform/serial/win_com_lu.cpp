#include "net/platform/serial/win_com_lu.hpp"
#include <stdio.h>

namespace robo{

	namespace net{
		void win_com_lu::start_receive(uint8_t* _data, size_t _size) {
			if (hCommPort_ != INVALID_HANDLE_VALUE  && !rx_) {
				rx_ = true;
				if (reader_) {
					reader_->join();
					delete reader_;
				}
				reader_ = new std::thread([ this, _data, _size] {
					DWORD dwRead;
					
					if (::ReadFile(this->hCommPort_, _data, _size, &dwRead,NULL)) {
						
						if (dwRead == 0) {
							refuse();
						}
						else {
							on_receive(_data, dwRead);
						}
					}
					else {
						refuse();
					}
					rx_ = false;
				});
			}
			else {
				refuse();
			}
		}

		void win_com_lu::start_transmit(const uint8_t* _data, size_t _size) {
			if (hCommPort_ != INVALID_HANDLE_VALUE && !tx_) {
				tx_ = true;
				if (writer_) {
					writer_->join();
					delete writer_;
				}
				writer_ = new std::thread([&] {
					DWORD dwWrite;
					if (::WriteFile(hCommPort_, _data, _size, &dwWrite, NULL)) {
						if (dwWrite == _size) {
							confirm();
						}
						else {
							refuse();
						}
					}
					else {
						refuse();
					}
					tx_ = false;
					});
			}
			else {
				refuse();
			}
		}
		bool   win_com_lu::win_com_lu::begin(robo::cstr _comm) {
			name_ = _comm;
			return begin_();
		}
		bool   win_com_lu :: win_com_lu::begin_(void) {
			
			hCommPort_ =
				CreateFile(
					name_.c_str(),
					GENERIC_READ | GENERIC_WRITE,
					0,
					NULL,
					OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL,
					NULL);

			if (hCommPort_ == INVALID_HANDLE_VALUE) {
				robo_errlog(RT("open port %s error"), name_.c_str());
				return false;
			}

			SetCommMask(hCommPort_, EV_RXCHAR);
			SetupComm(hCommPort_, 1500, 1500);

			COMMTIMEOUTS CommTimeOuts;
			CommTimeOuts.ReadIntervalTimeout = 10;
			CommTimeOuts.ReadTotalTimeoutMultiplier = 0;
			CommTimeOuts.ReadTotalTimeoutConstant = 10;
			CommTimeOuts.WriteTotalTimeoutMultiplier = 0;
			CommTimeOuts.WriteTotalTimeoutConstant = 10;

			if (!SetCommTimeouts(hCommPort_, &CommTimeOuts)) {
				CloseHandle(hCommPort_);
				hCommPort_ = INVALID_HANDLE_VALUE;
				robo_errlog(RT("open port %s error"), name_.c_str());
				return false;
			}

			DCB ComDCM;

			memset(&ComDCM, 0, sizeof(ComDCM));
			ComDCM.DCBlength = sizeof(DCB);
			GetCommState(hCommPort_, &ComDCM);
			ComDCM.BaudRate = 115200;
			ComDCM.ByteSize = 8;
			ComDCM.Parity = NOPARITY;
			ComDCM.StopBits = ONESTOPBIT;
			ComDCM.fAbortOnError = TRUE;
			ComDCM.fDtrControl = DTR_CONTROL_DISABLE;
			ComDCM.fRtsControl = RTS_CONTROL_DISABLE;
			ComDCM.fBinary = TRUE;
			ComDCM.fParity = FALSE;
			ComDCM.fInX = FALSE;
			ComDCM.fOutX = FALSE;
			ComDCM.XonChar = 0;
			ComDCM.XoffChar = (unsigned char)0xFF;
			ComDCM.fErrorChar = FALSE;
			ComDCM.fNull = FALSE;
			ComDCM.fOutxCtsFlow = FALSE;
			ComDCM.fOutxDsrFlow = FALSE;
			ComDCM.XonLim = 128;
			ComDCM.XoffLim = 128;

			if (!SetCommState(hCommPort_, &ComDCM)) {
				CloseHandle(hCommPort_);
				hCommPort_ = INVALID_HANDLE_VALUE;
				robo_errlog(RT("open port %s error"), name_.c_str());
				return false;
			}
			if (hCommPort_ == INVALID_HANDLE_VALUE) {
				return false;
			}
			else {
				return true;
			}
		}
		
		void   win_com_lu::finish(void) {
			if (hCommPort_ == INVALID_HANDLE_VALUE) {
				auto tmp = hCommPort_;
				hCommPort_ = INVALID_HANDLE_VALUE;
				CloseHandle(tmp);
			}
			if (writer_) {
				writer_->join();
				delete writer_;
			}
			if (reader_) {
				reader_->join();
				delete reader_;
			}
		}

		//typedef libx::channel::Serial<win_com_driver, COM_DRIVER_BUF_SIZE, BYTE, true> ComLink;


		win_com_lu::win_com_lu(void)
            :
            hCommPort_(INVALID_HANDLE_VALUE)
         {
		}
		win_com_lu::~win_com_lu(){
			finish();
		}
		void win_com_lu::cancel(void) {
		/*	if (hCommPort_ == INVALID_HANDLE_VALUE) {
				auto tmp = hCommPort_;
				hCommPort_ = INVALID_HANDLE_VALUE;
				CloseHandle(tmp);
			}
			begin_();*/
		}
	}
}
