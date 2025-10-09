#ifndef linux_serialovtcp_hpp
#define linux_serialovtcp_hpp

#include "net/platform/serial/linux_serialovtcp.hpp"

#include <cstdint>
#include <cstddef>
#include <string>
#include <atomic>
#include <thread>
#include <mutex>
#include <queue>
#include <vector>
#include <functional>
#include "core/robosd_common.hpp"
#include "net/robosd_serial.hpp"
#include <condition_variable>
namespace robo{
	namespace net{
		class serialovtcp: public robo::net::iserial {
		public:
			// Конструктор/деструктор
			serialovtcp();
			virtual ~serialovtcp();
			
			// Реализация требуемого интерфейса
			virtual size_t available(void); // количество байт, доступных для чтения
			virtual size_t get(uint8_t* _data, size_t _max_size); // получить байты в буфер
			virtual bool put(const uint8_t* _data, size_t _size); // записать байты
			virtual void reset(void); // перезапустить
			virtual size_t space(); // количество байт, которые можно отправить
			virtual size_t space_max(); // максимальное количество байт для разовой отправки
			virtual size_t get(uint8_t & _data); 
			virtual bool  put(uint8_t _data);
			// Дополнительные методы управления
			bool open(cstr _name);
			bool start();
			void stop();
			bool is_running() const;
			bool is_client_connected() const;
			std::string get_client_info() const;

		private:
			// Системные вызовы
			int create_socket();
			int set_socket_nonblocking(int fd);
			int accept_connection();
			void close_socket(int fd);
			
			// Рабочие потоки
			void server_loop();
			void client_handler_loop();
			void read_loop();
			void write_loop();
			
			// Вспомогательные методы
			bool setup_server_socket();
			void cleanup();
			void add_to_input_buffer(const uint8_t* data, size_t size);
			size_t get_from_input_buffer(uint8_t* data, size_t max_size);
			
			// Состояние сервера
			std::atomic<bool> is_running_{false};
			std::atomic<bool> client_connected_{false};
			
			std::atomic<int> server_socket_{-1};
			std::atomic<int> client_socket_{-1};
			uint16_t port_=0;
			std::string client_address_;
			// Буферы данных
			std::vector<uint8_t> input_buffer_;
			mutable std::mutex input_mutex_;
			std::condition_variable input_cv_;
			
			std::queue<std::vector<uint8_t>> output_queue_;
			mutable std::mutex output_mutex_;
			std::atomic<size_t> output_queue_size_{0};
			std::atomic<bool> write_pending_{false};
			
			// Потоки
			std::thread server_thread_;
			std::thread client_handler_thread_;
			std::thread * read_thread_ = nullptr;
			std::thread *write_thread_ = nullptr;
			void close_client_connection(void);
			// Константы
			static inline const size_t MAX_BUFFER_SIZE = 64 * 1024; // 64KB
			static inline const size_t MAX_SINGLE_WRITE = 16 * 1024; // 16KB
			static inline const int BACKLOG = 1; // Только один клиент в очереди
		};
	}
}
#endif // linux_serialovtcp_hpp