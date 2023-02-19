#include "net/platform/serial/win_com.hpp"
#include <stdio.h>

namespace robo{

	namespace net{


		win_com::guard::guard(win_com & _instance) :instance(_instance){
			EnterCriticalSection(&(instance.cs));
		}
		win_com::guard::~guard(void){
			LeaveCriticalSection(&(instance.cs));
		}
		void win_com::raise(void){
			SetEvent(hSend);
		}


		void win_com::performer_(void){
			HANDLE ev[3] = { hSend, hReceive, hTerminate };

			while (true){
				while (!connected()){
					connect();
					if (!connected()){
						if (WaitForSingleObject(hTerminate, reconnect_pause_ms_) == WAIT_TIMEOUT){
							events.reconnect();
						}
						else{
							break;
						}
					}
					else{
						break;
					}
				};
				if (WaitForSingleObject(hTerminate, 0) == WAIT_TIMEOUT){
					if (!listen()){
						disconnect();
						continue;
					}

					switch (WaitForMultipleObjects(3, ev, FALSE, INFINITE)){
					case  0:
						if (!send()){
							disconnect();
						}
						break;
					case 1:
						if (!receive()){
							disconnect();
						}
						break;
					case 2:
						disconnect();
						return;
					default:
						disconnect();
						WaitForSingleObject(hTerminate, INFINITE);
						return;
					}
					Sleep(0);
				}
				else{
					return;
				}
			}
		}

		DWORD WINAPI win_com::performer(LPVOID _instance){
			win_com * instance = (win_com *)_instance;
			instance->performer_();
			return 0;
		}

		bool win_com::start(void){
			DWORD id;
			hThread = CreateThread(
				NULL //IN LPSECURITY_ATTRIBUTES lpThreadAttributes,
				, 0 //IN SIZE_T dwStackSize,
				, performer // IN LPTHREAD_START_ROUTINE lpStartAddress,
				, this //IN LPVOID lpParameter,
				, CREATE_SUSPENDED //IN DWORD dwCreationFlags,
				, &id//OUT LPDWORD lpThreadId
				);
			if (hThread == NULL){
				stop();
				return false;
			}
			ResetEvent(hTerminate);
			ResumeThread(hThread);
			return true;
		}
		void win_com::stop(void){
			if (hThread != NULL){
				SetEvent(hTerminate);
				if (WaitForSingleObject(hThread, 1000) != WAIT_OBJECT_0){
					TerminateThread(hThread, -1);
				}
				else{
					CloseHandle(hThread);
				}
				hThread = NULL;
			}
		}




		//typedef libx::channel::Serial<win_com_driver, COM_DRIVER_BUF_SIZE, BYTE, true> ComLink;


        win_com::win_com(void)
            :
            outcomQueue(*this)
            , hCommPort(INVALID_HANDLE_VALUE)
            , dwError(0)
            , incomQueue(*this)
        {
			hSendFinish = ::CreateEvent(NULL, FALSE, FALSE, NULL);
            osReader = { ROBO_ZEROS_STRUCT };
			hTerminate = CreateEvent(NULL, TRUE, FALSE, NULL);
			hReceive = CreateEvent(NULL, FALSE, FALSE, NULL);
			hReceiveComplete = CreateEvent(NULL, FALSE, FALSE, NULL);
			hThread = NULL;
			InitializeCriticalSection(&cs);
			hSend = CreateEvent(NULL, FALSE, FALSE, NULL);
		}
		win_com::~win_com(){
			stop();
			CloseHandle(hSendFinish);
			CloseHandle(hTerminate);
			CloseHandle(hReceive);
			CloseHandle(hReceiveComplete);
		}
		bool win_com::send(void){
			HANDLE ev[2] = { hSendFinish, hTerminate };
			if (hCommPort){
				while (1){
					Sleep(1);
					dwToWrite = (DWORD)outcomQueue.get(writeBuf, COM_DRIVER_BUF_SIZE);
					if (dwToWrite == 0) {
						ResetEvent(hSend);
						return TRUE;
					}
					dwWritten = 0;
                    osWrite = { ROBO_ZEROS_STRUCT };

					// Create this write operation's OVERLAPPED structure's hEvent.
					osWrite.hEvent = hSendFinish;

					// Issue write.flow_outcom_buffer_readCount
					if (!::WriteFile(hCommPort, writeBuf, dwToWrite, &dwWritten, &osWrite)) {
						if (::GetLastError() != ERROR_IO_PENDING) {
							return false;
						}
					}


                    if (WaitForMultipleObjects(2, ev, FALSE, 50) == 0){
                        /*BOOL ret =*/ GetOverlappedResult(hCommPort, &osWrite, &dwWritten, FALSE);
						{
							//win_com_driver::Guard __g(this);
							/*printf("COM <<< CAN:\n");
							for (int i = 0; i < dwWritten; i++){
							printf("\t0x%x", writeBuf[i]);
							}
							printf("\n");*/
						}
						
					}
					else {
						return false;
					}
				}
			}
			return false;
		}

		bool win_com::listen(void){
			if (osReader.hEvent) return true;
//			bool fRes = false;
            osReader = { ROBO_ZEROS_STRUCT };

			// Create the overlapped event. Must be closed before exiting
			// to avoid a handle leak.
			osReader.hEvent = hReceive;

			// Issue read operation.
			if (::ReadFile(hCommPort, readBuf, 1, &dwRead, &osReader) == FALSE){
				if (::GetLastError() != ERROR_IO_PENDING) {
					// WriteFile failed, but isn't delayed. Report error and abort.
					return false;
				}
			}
			return true;
		}

		void win_com::disconnect(void){
			if (hCommPort != INVALID_HANDLE_VALUE){
				CloseHandle(hCommPort);
				hCommPort = INVALID_HANDLE_VALUE;
				events.disconnected();
			}
		}

		bool win_com::connect(void){
			hCommPort = ::CreateFile(
				comm.c_str(),
				GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL,
				OPEN_EXISTING,
				FILE_FLAG_OVERLAPPED,
				NULL);
			if (hCommPort == INVALID_HANDLE_VALUE){
				dwError = GetLastError();
				return false;
			}
			else {
				events.connected();
				return true;
			}
		}

		void win_com::connect(cstr  _comm, unsigned _reconnect_pause_ms){
			reconnect_pause_ms_ = _reconnect_pause_ms;
			comm = _comm;
			stop();
			start();
		}

		bool win_com::connected(void){
			return hCommPort != INVALID_HANDLE_VALUE;
		}

		bool win_com::receive(void) {
			if (::GetOverlappedResult(hCommPort, &osReader, &dwRead, FALSE)) {
				guard __g(*this);
				BYTE * ptr = readBuf;
				DWORD cnt = dwRead;
				while (cnt--){
					incomQueue.put(*ptr++);
				}
				service();
				osReader.hEvent = 0;
				SetEvent(hReceiveComplete);
				return true;
			}
			else{
				osReader.hEvent = 0;
				return false;
			}
		}

		size_t win_com::get(uint8_t & _tmp){
			if (incomQueue.avalable()) {
				_tmp = incomQueue.get();
				return 1;
			}
			else {
				return 0;
			}
		}

		bool  win_com::put(uint8_t _data){
			return outcomQueue.put(_data);
		}

		size_t  win_com::available(void){
			return incomQueue.count();
		}
		
		size_t win_com::space(void){
			return outcomQueue.space();
		}

		size_t win_com::space_max(void) {
			return outcomQueue.size();
		}

		size_t win_com::get(uint8_t * _buf, size_t _max_size){
			return incomQueue.get(_buf, _max_size);
		}

		bool  win_com::put(const uint8_t * _buf, size_t _size){
			return outcomQueue.put(_buf,_size);
		}

		void win_com::reset(void){
			guard __g(*this);
			stop();
			outcomQueue.clear();
			incomQueue.clear();
			connect(comm);
		}
		bool win_com::wait(void){
			HANDLE ev[2] = { hReceiveComplete, hTerminate };
			if (WaitForMultipleObjects(2, ev, FALSE, INFINITE) == 0){
				return true;
			}
			else {
				return false;
			}
		}
		bool win_com::active(void){
			return (WaitForSingleObject(hTerminate, 0) == WAIT_TIMEOUT);
		}
	}
}
