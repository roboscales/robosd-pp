#ifndef __win_com_lu_h
#define __win_com_lu_h
#include "net/robosd_net_link.hpp"
#include "net/robosd_queue.hpp"
#include "net/robosd_serial.hpp"
#include "core/robosd_string.hpp"
#include "core/robosd_lambda.hpp"
#include <windows.h>
#include <thread>
#include <atomic>
namespace robo{

	namespace net{

		class ROBO_EXPORT win_com_lu {
		public:

			void start_receive( uint8_t * _data, size_t _size);
			virtual void on_receive(const uint8_t* _data, size_t _size) = 0;
			virtual void refuse(void) = 0;
			void start_transmit(const uint8_t* _data, size_t _size);
			virtual void confirm(void) = 0;

			//void raise();
			//robo::net::queue<win_com_lu, COM_DRIVER_BUF_BITS, BYTE, true> outcomQueue;

		private:
			std::string name_;
			HANDLE hCommPort_ = INVALID_HANDLE_VALUE;
			std::thread* reader_ = nullptr;
			std::thread* writer_ = nullptr;
			std::atomic_bool rx_ = false;
			std::atomic_bool tx_ = false;
		public:
			bool read_busy() { return rx_;  }
			bool write_busy() { return tx_; }
			win_com_lu(void);
			~win_com_lu(void);
			bool begin(robo::cstr _comm);
			bool begin_(void);
			void finish(void);
			void cancel(void);
		protected:
	
		public:

		};
	}
}

#endif
