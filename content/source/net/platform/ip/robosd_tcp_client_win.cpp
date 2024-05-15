#include "net/platform/ip/robosd_tcp_client_win.hpp"
#include "core/robosd_log.hpp"
#include <WinSock2.h>
#pragma comment(lib,"wsock32.lib")

namespace win {
	ip::core::core(void) {}
	ip::core::~core(void) {}
	ip::core& ip::core::instance_(void) {
		static core instance__;
		return instance__;
	}
	ip::ip(void) { 
		robo::system::guard g__;
		core::instance_().enter_();
	}
	ip::~ip(void) {
		robo::system::guard g__;
		core::instance_().leave_();
	}

	void ip::core::enter_(void) {
		used_++;
		if (used_ == 1) {
			WORD wVersionRequested;
			WSADATA wsaData;
			DWORD err;
			wVersionRequested = MAKEWORD(2, 0);
			err = WSAStartup(wVersionRequested, &wsaData);

			if (err != 0) {
				ROBO_VBREAK_F(RT("Socket2.0 failed to initialize with error %d"), err);
			}
			//Confirm version
			if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 0) {
				err = WSAGetLastError();
				WSACleanup();
				ROBO_VBREAK_F(RT("Socket2.0 failed to initialize with error %d"), err);
			}
			exists_ = true;
		}
	};
	void ip::core::leave_(void) {
		used_--;
		if (used_ == 0) {
			WSACleanup();
			exists_ = false;
		}
	}
	bool ip::exists(void) {
		robo::system::guard g__;
		return core::instance_().exists_;
	};


	tcp_poll_client::~tcp_poll_client(void) {
		#if ROBO_UNICODE_ENABLED == 1
		if (servo_address_) delete[] servo_address_;
		#endif
	}

	void tcp_poll_client::setup(int _servo_port, const robo::string& _adress) {
		servo_port_ = _servo_port;
		#if ROBO_UNICODE_ENABLED == 1
		auto sz = _adress.length() + 1;
		servo_address_ = new char[sz];
		_adress.ascii(servo_address_, sz);
		//robo::string nm();
		//nm.ascii();
		#else
		servo_address_ = _adress.c_str();
		#endif
		//servo_address_ = _servo_address;
	}

	void tcp_poll_client::open(void){
		if (!ip_.exists()) {
			driver_refuse_connect();
			ROBO_VBREAK_F(RT("WSA IS NOT EXISTS"));
		}

		int err;
		
		

		//2. Create socket
		remote_ = ::socket(
			AF_INET,           //internetwork: UDP, TCP, etc
			SOCK_STREAM,        //TCP
			0                  //protocol
		);

		if (remote_ == INVALID_SOCKET) {
			driver_refuse_connect();
			err = WSAGetLastError();
			ROBO_VBREAK_F(RT("Socket creation failed error %d"), err);
		}

		//3. Set the address sent to
		//SOCKADDR_IN addrto;            //Address to	
		memset(&servo_addr_in_, 0, servo_addr_size_);
		servo_addr_in_.sin_family = AF_INET;

		servo_addr_in_.sin_addr.S_un.S_addr = inet_addr(servo_address_);
		//inet_pton(AF_INET, servo_address_, &(servo_addr_in_.sin_addr));

		//The port number must be the same as the port number bound to the server
		servo_addr_in_.sin_port = htons(servo_port_);




		//Apply for link server
		if (::connect(remote_, (SOCKADDR*)&servo_addr_in_, sizeof(SOCKADDR)) != 0) {
		
			err = WSAGetLastError();
			closesocket(remote_);
			remote_ = INVALID_SOCKET;
			driver_refuse_connect();
			ROBO_VBREAK_F(RT("connect failed with error %d"),err);
		}

		// Set to non-blocking mode.
		unsigned long ul = 1;
		if (ioctlsocket(remote_, FIONBIO, (unsigned long*)&ul) != 0) {
			err = WSAGetLastError();
			closesocket(remote_);
			remote_ = INVALID_SOCKET;
			driver_refuse_connect();
			ROBO_VBREAK_F(RT("non-blocking mode failed with error %d"), err);
		}
		else {
			status_ = robo::net::iclient::statuses::connected;
		}
	}

	void tcp_poll_client::close(void){
		if (remote_ != INVALID_SOCKET) {
			closesocket(remote_);
			remote_ = INVALID_SOCKET;
			status_ = robo::net::iclient::statuses::disconnected;
		}
	}
	bool tcp_poll_client::begin_receive(uint8_t* _data, size_t _size){
		recv_need_ = _size;
		recv_buffer_ = _data;
		recv_count_ = 0;
		return true;
	}
	size_t tcp_poll_client::received(void) {
		int cnt = recv(remote_, (char*)recv_buffer_, (int)recv_need_,0);
		if (cnt < 0) {
			robo_errlog(RT("recv failed with error % d"), WSAGetLastError());
			driver_abort_session();
			return 0;
		}
		if (cnt > 0) {
			recv_need_ -= cnt;
			recv_buffer_ += cnt;
			recv_count_ += cnt;
		}
		return recv_count_;
	}

	bool tcp_poll_client::begin_send(const uint8_t* _data, size_t _size){
		send_need_ = _size;
		send_buffer_ = _data;
		send_count_ = 0;
		return true;
	}
	size_t tcp_poll_client::sended(void){
		int cnt = send(remote_, (char*)send_buffer_, (int)send_need_,0);
		if (cnt != send_need_) {
			robo_errlog(RT("recv failed with error % d"), WSAGetLastError());
			driver_abort_session();
			return 0;
		}
		return send_need_;
	}
}