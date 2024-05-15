#ifndef robosd_net_tcp_client_hpp
#define robosd_net_tcp_client_hpp
#include "net/robosd_ip_client.hpp"
#include <Ws2tcpip.h>
namespace win {

	class ROBO_EXPORT ip{
		class ROBO_EXPORT core {
			friend class ip;
			core(void);
			~core(void);
			int used_ = 0;
			static core& instance_(void);
			void enter_(void);
			void leave_(void);
			bool exists_ = false;
		};
	public:
		ip(void);
		~ip(void);
		bool exists(void);
	};

	class ROBO_EXPORT tcp_poll_client {
	
		robo::net::iclient::statuses status_ = robo::net::iclient::statuses::disconnected;
		SOCKET remote_ = INVALID_SOCKET;
		size_t recv_need_ = 0;
		size_t recv_count_ = 0;
		uint8_t* recv_buffer_ = nullptr;
		size_t send_need_ = 0;
		size_t send_count_ = 0;
		const uint8_t* send_buffer_ = nullptr;

		struct sockaddr_in  servo_addr_in_ = {};
		int servo_addr_size_ = sizeof(servo_addr_in_);
		int servo_port_=0;
		#if ROBO_UNICODE_ENABLED == 1
		char* servo_address_ = nullptr;
		#else
		const char *  servo_address_ = nullptr;
		#endif
		ip ip_;
	public:
		~tcp_poll_client();
		inline const SOCKET& socket(void) { return remote_; }
		robo::net::iclient::statuses status(void) {
			return status_;
		}
		virtual void driver_refuse_connect() = 0;
		virtual void driver_abort_session() = 0;
		void open(void);
		void close(void);
		bool begin_receive(uint8_t* _data, size_t _size);
		size_t received(void);
		bool begin_send(const uint8_t* _data, size_t _size);
		size_t sended(void);
		size_t space(void) { return 0xFFFF; }
		size_t space_max(void) { return 0xFFFF; }
		void reset(void) {};
		void poll() {};
		void setup(int _servo_port, const robo::string& _adress);
	};
}
/*
namespace win {
	class ROBO_EXPORT tcp_client : public ::robo::net::tcp_client {
		SOCKET remote_ = INVALID_SOCKET;
		
		WSAEVENT receive_event_ = NULL;
		uint8_t* receive_buf_ = nullptr;
		uint16_t receive_buf_size_ = 0;
	protected:
		inline const SOCKET& socket(void) { return remote_; }
		virtual bool  do_post(const uint8_t* _data, size_t _len);
		bool try_receive(robo::time_us_t _timeout);
		virtual void reset(void) {}
		virtual size_t space(void) { return 0xFFFF; };
		virtual size_t max_space(void) { return 0xFFFF; };
		tcp_client(void);
		virtual ~tcp_client(void);
		virtual bool begin(const robo::string _alis);
		virtual void finish(void);
		virtual void stop(void);
	};

	class ROBO_EXPORT  tcp_client_thread :public tcp_client {
	private:

		bool terminate_ = false;
		std::thread* recever_thread_ = nullptr;
	public:
		tcp_client_thread(void);
		bool start(void);
		virtual void stop(void);
		virtual void finish(void);
	};

	class ROBO_EXPORT  tcp_client_poll :public tcp_client {
		bool backend_;
	protected:
		virtual void poll(void);
	public:
		tcp_client_poll(bool _backend);
		void start(void);
		virtual void stop(void);
	};
};*/
#endif