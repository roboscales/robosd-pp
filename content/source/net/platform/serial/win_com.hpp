#ifndef __win_com_h
#define __win_com_h

#include "net/robosd_queue.hpp"
#include "net/robosd_serial.hpp"
#include "core/robosd_string.hpp"
#include "core/robosd_lambda.hpp"
#include <windows.h>
#ifndef COM_DRIVER_BUF_BITS 
#define COM_DRIVER_BUF_BITS 12
#endif 

#define COM_DRIVER_BUF_SIZE (1<<COM_DRIVER_BUF_BITS)

namespace robo{

	namespace net{

		class ROBO_EXPORT win_com : public robo::net::iserial{
		public:
			friend class ROBO_EXPORT guard;
			class ROBO_EXPORT guard{
				win_com & instance;
			public:
				guard(win_com & _instance);
				~guard();
			};
			void raise();
			robo::net::queue<win_com, COM_DRIVER_BUF_BITS, BYTE, true> outcomQueue;

		private:
			CRITICAL_SECTION cs;
			HANDLE hSend;
			HANDLE hCommPort;
			// создаем объект ядра "событие" со сбросом вручную (в свободном состоянии)
			HANDLE hSendFinish;
			OVERLAPPED lpOverlapped;	// структура для задания режима асинхронного чтения
			DWORD dwError;
			HANDLE hTerminate;
			HANDLE hReceive;
			HANDLE hReceiveComplete;
			HANDLE hThread;
			DWORD dwToWrite;
			DWORD dwWritten;
			OVERLAPPED osWrite;
			BYTE writeBuf[COM_DRIVER_BUF_SIZE];
			virtual bool send();

			BYTE readBuf[COM_DRIVER_BUF_SIZE];
			DWORD dwRead = 0;
			OVERLAPPED osReader;
			bool listen(void);

			bool receive(void);
			void disconnect(void);
			void performer_(void);
			static DWORD WINAPI performer(LPVOID _instance);
			robo::net::queue<win_com, COM_DRIVER_BUF_BITS, BYTE, false> incomQueue;
			unsigned reconnect_pause_ms_;
		public:
			robo::string comm;
			//IncomBuffer incomBuffer;
			win_com(void);
			~win_com(void);
			struct ROBO_EXPORT ievents{
				lambda<void(void)> reconnect;
				lambda<void(void)> connected;
				lambda<void(void)> disconnected;
				ievents(){
					reconnect = []()-> void{};
					connected = []()-> void{};
					disconnected = []()-> void{};
				}
			}	events;
		protected:
			bool connect(void);
			bool start(void);
			virtual void service(void){};
		public:
			void stop(void);
			void connect(cstr _comm, unsigned _reconnect_pause_ms = 1000);
			virtual size_t available(void);
			virtual size_t space(void);
			virtual size_t space_max(void);
			virtual size_t get(uint8_t* _data, size_t _max_size);
			virtual bool put(const uint8_t* _data, size_t _size);
			virtual size_t get(uint8_t& _data);
			virtual bool  put(uint8_t);
			virtual void reset(void);
			bool connected(void);
			bool wait(void);
			bool active(void);
		};
	}
}

#endif
