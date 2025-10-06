// tcp_server_hardcore.cpp
#include "net/platform/serial/linux_serialovtcp.hpp"
//#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>
#include "core/robosd_log.hpp"
#include "core/robosd_ini.hpp"
namespace robo{
	namespace net{
		serialovtcp::serialovtcp() 
		{
		}

		serialovtcp::~serialovtcp() {
			stop();
		}

		// Системные вызовы
		int serialovtcp::create_socket() {
			int sockfd = socket(AF_INET, SOCK_STREAM, 0);
			if (sockfd < 0) {
				robo_errlog("socket() failed: %s",strerror(errno))
				return -1;
			}
			
			// Устанавливаем SO_REUSEADDR
			int optval = 1;
			if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
				robo_errlog("setsockopt(SO_REUSEADDR) failed: %s",strerror(errno))
				close(sockfd);
				return -1;
			}
			
			return sockfd;
		}

		int serialovtcp::set_socket_nonblocking(int fd) {
			int flags = fcntl(fd, F_GETFL, 0);
			if (flags < 0) {
				robo_errlog("fcntl(F_GETFL) failed: %s",strerror(errno))
				return -1;
			}
			
			if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
				robo_errlog("fcntl(F_SETFL) failed: %s",strerror(errno))
				return -1;
			}
			
			return 0;
		}

		int serialovtcp::accept_connection() {
			struct sockaddr_in client_addr;
			socklen_t client_len = sizeof(client_addr);
			
			int client_fd = accept(server_socket_, (struct sockaddr*)&client_addr, &client_len);
			if (client_fd < 0) {
				if (errno != EAGAIN && errno != EWOULDBLOCK) {
					robo_errlog("accept() failed: %s",strerror(errno))
				}
				return -1;
			}
			
			// Сохраняем адрес клиента
			char client_ip[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
			client_address_ = std::string(client_ip) + ":" + std::to_string(ntohs(client_addr.sin_port));
			
			// Устанавливаем неблокирующий режим
			if (set_socket_nonblocking(client_fd) < 0) {
				close(client_fd);
				return -1;
			}
			
			// Отключаем алгоритм Нейгла для низкой задержки
			int optval = 1;
			setsockopt(client_fd, IPPROTO_TCP, O_NDELAY, &optval, sizeof(optval));
			
			return client_fd;
		}

		void serialovtcp::close_socket(int fd) {
			if (fd >= 0) {
				close(fd);
			}
		}

		bool serialovtcp::setup_server_socket() {
			server_socket_ = create_socket();
			if (server_socket_ < 0) {
				return false;
			}
			
			// Устанавливаем неблокирующий режим
			if (set_socket_nonblocking(server_socket_) < 0) {
				close_socket(server_socket_);
				return false;
			}
			
			// Привязываем сокет к порту
			struct sockaddr_in server_addr;
			memset(&server_addr, 0, sizeof(server_addr));
			server_addr.sin_family = AF_INET;
			server_addr.sin_addr.s_addr = INADDR_ANY;
			server_addr.sin_port = htons(port_);
			
			if (::bind(server_socket_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
				robo_errlog("bind() failed: %s",strerror(errno))
				close_socket(server_socket_);
				return false;
			}
			
			// Начинаем слушать
			if (listen(server_socket_, BACKLOG) < 0) {
				robo_errlog("listen() failed: %s",strerror(errno))
				close_socket(server_socket_);
				return false;
			}
			
			return true;
		}
		bool serialovtcp::open(cstr _name){
			ROBO_LBREAKN(robo::ini::load(_name, RT("port"), port_));
			ROBO_LBREAKN(start());
			return true;
		}
		bool serialovtcp::start() {
			ROBO_LBREAKN_F(is_running_==false, RT("Server is already running"))
			ROBO_LBREAKN(setup_server_socket())
			
			is_running_ = true;
			
			// Запускаем основной поток сервера
			server_thread_ = std::thread(&serialovtcp::server_loop, this);
			robo_infolog("TCP serial server started on port %d",port_);
			return true;
		}

		void serialovtcp::stop() {
			if (!is_running_) return;
			
			is_running_ = false;
			
			// Закрываем сокеты чтобы выйти из блокирующих вызовов
			close_socket(client_socket_);
			close_socket(server_socket_);
			
			// Ждем завершения потоков
			if (server_thread_.joinable()) {
				server_thread_.join();
			}
			if (client_handler_thread_.joinable()) {
				client_handler_thread_.join();
			}
			if (read_thread_.joinable()) {
				read_thread_.join();
			}
			if (write_thread_.joinable()) {
				write_thread_.join();
			}
			
			// Очищаем буферы
			{
				std::lock_guard<std::mutex> lock(input_mutex_);
				input_buffer_.clear();
			}
			{
				std::lock_guard<std::mutex> lock(output_mutex_);
				std::queue<std::vector<uint8_t>> empty;
				std::swap(output_queue_, empty);
				output_queue_size_ = 0;
			}
			
			client_connected_ = false;
			client_address_.clear();
			
			robo_infolog("TCP serial server (port %d) stopped ",port_);
		}

		void serialovtcp::server_loop() {
			struct pollfd fds[1];
			fds[0].fd = server_socket_;
			fds[0].events = POLLIN;
			
			while (is_running_) {
				int ret = ::poll(fds, 1, 100); // Таймаут 100мс
				
				if (ret < 0) {
					if (errno != EINTR) {
						robo_errlog("poll() failed: %s",strerror(errno))
						break;
					}
					continue;
				}
				
				if (ret > 0) {
					if (fds[0].revents & POLLIN) {
						// Новое подключение
						int new_client = accept_connection();
						if (new_client >= 0) {
							if (client_connected_) {
								robo_infolog("Rejecting new connection (port %d) - single client mode", port_)
								close_socket(new_client);
							} else {
								client_socket_ = new_client;
								client_connected_ = true;
								
								robo_infolog("Client (addr %s, port %d) connected - single client mode", client_address_.c_str(), port_)
								
								// Очищаем буферы при новом подключении
								{
									std::lock_guard<std::mutex> lock(input_mutex_);
									input_buffer_.clear();
								}
								{
									std::lock_guard<std::mutex> lock(output_mutex_);
									std::queue<std::vector<uint8_t>> empty;
									std::swap(output_queue_, empty);
									output_queue_size_ = 0;
								}
								
								// Запускаем потоки для работы с клиентом
								read_thread_ = std::thread(&serialovtcp::read_loop, this);
								write_thread_ = std::thread(&serialovtcp::write_loop, this);
							}
						}
					}
				}
			}
		}

		void serialovtcp::read_loop() {
			uint8_t buffer[4096];
			struct pollfd fds[1];
			fds[0].fd = client_socket_;
			fds[0].events = POLLIN;
			
			while (is_running_ && client_connected_) {
				int ret = ::poll(fds, 1, 50); // Таймаут 50мс
				
				if (ret < 0) {
					if (errno != EINTR) {
						robo_errlog("poll() in read_loop failed: %s",strerror(errno))
						break;
					}
					continue;
				}
				
				if (ret > 0) {
					if (fds[0].revents & POLLIN) {
						ssize_t bytes_read = recv(client_socket_, buffer, sizeof(buffer), 0);
						
						if (bytes_read > 0) {
							add_to_input_buffer(buffer, bytes_read);
						} else if (bytes_read == 0) {
							// Клиент отключился
							robo_infolog("Client (port %d) disconnected ", port_);
							break;
						} else {
							if (errno != EAGAIN && errno != EWOULDBLOCK) {
								robo_errlog("recv() failed: %s",strerror(errno))
								break;
							}
						}
					}
					
					if (fds[0].revents & (POLLHUP | POLLERR)) {
						robo_infolog("Client (port %d) connection lost", port_);
						break;
					}
				}
			}
			
			// Очищаем состояние клиента
			client_connected_ = false;
			close_socket(client_socket_);
			client_socket_ = -1;
			client_address_.clear();
		}

		void serialovtcp::write_loop() {
			struct pollfd fds[1];
			fds[0].fd = client_socket_;
			fds[0].events = POLLOUT;
			
			while (is_running_ && client_connected_) {
				// Проверяем есть ли данные для отправки
				std::vector<uint8_t> data_to_send;
				{
					std::lock_guard<std::mutex> lock(output_mutex_);
					if (!output_queue_.empty()) {
						data_to_send = std::move(output_queue_.front());
						output_queue_.pop();
						output_queue_size_ -= data_to_send.size();
					}
				}
				
				if (data_to_send.empty()) {
					// Нет данных для отправки - небольшая пауза
					std::this_thread::sleep_for(std::chrono::milliseconds(10));
					continue;
				}
				
				// Отправляем данные
				const uint8_t* data_ptr = data_to_send.data();
				size_t data_remaining = data_to_send.size();
				size_t total_sent = 0;
				
				while (data_remaining > 0 && is_running_ && client_connected_) {
					// Ждем пока сокет станет доступен для записи
					int ret = ::poll(fds, 1, 100);
					if (ret <= 0) continue;
					
					if (fds[0].revents & POLLOUT) {
						ssize_t bytes_sent = send(client_socket_, data_ptr, 
												 std::min(data_remaining, MAX_SINGLE_WRITE), 
												 MSG_NOSIGNAL);
						
						if (bytes_sent > 0) {
							data_ptr += bytes_sent;
							data_remaining -= bytes_sent;
							total_sent += bytes_sent;
						} else if (bytes_sent < 0) {
							if (errno != EAGAIN && errno != EWOULDBLOCK) {
								robo_errlog("send() failed: %s", strerror(errno) )
								break;
							}
						}
					}
					
					if (fds[0].revents & (POLLHUP | POLLERR)) {
						robo_infolog("Client connection lost during write (port %d)", port_ )
						break;
					}
				}
				
				if (total_sent > 0) {
					//std::cout << "Sent " << total_sent << " bytes to client" << std::endl;
				}
			}
		}

		void serialovtcp::add_to_input_buffer(const uint8_t* data, size_t size) {
			std::lock_guard<std::mutex> lock(input_mutex_);
			
			// Ограничиваем размер буфера
			if (input_buffer_.size() + size > MAX_BUFFER_SIZE) {
				size_t overflow = (input_buffer_.size() + size) - MAX_BUFFER_SIZE;
				if (overflow < input_buffer_.size()) {
					input_buffer_.erase(input_buffer_.begin(), 
									   input_buffer_.begin() + overflow);
				} else {
					input_buffer_.clear();
				}
			}
			
			input_buffer_.insert(input_buffer_.end(), data, data + size);
			input_cv_.notify_all();
		}

		size_t serialovtcp::get_from_input_buffer(uint8_t* data, size_t max_size) {
			std::lock_guard<std::mutex> lock(input_mutex_);
			
			if (input_buffer_.empty() || max_size == 0) {
				return 0;
			}
			
			size_t bytes_to_copy = std::min(max_size, input_buffer_.size());
			std::copy(input_buffer_.begin(), input_buffer_.begin() + bytes_to_copy, data);
			input_buffer_.erase(input_buffer_.begin(), input_buffer_.begin() + bytes_to_copy);
			
			return bytes_to_copy;
		}

		// Реализация интерфейса
		size_t serialovtcp::available(void) {
			std::lock_guard<std::mutex> lock(input_mutex_);
			return input_buffer_.size();
		}

		size_t serialovtcp::get(uint8_t* _data, size_t _max_size) {
			if (!_data || _max_size == 0) return 0;
			return get_from_input_buffer(_data, _max_size);
		}
		
		size_t serialovtcp::get(uint8_t & _data){
			return get(&_data,1);
		}

		bool serialovtcp::put(const uint8_t* _data, size_t _size) {
			if (!_data || _size == 0 || !client_connected_) {
				return false;
			}
			
			// Проверяем доступное пространство
			if (space() < _size) {
				robo_errlog("Not enough space to send data (size %d)",_size)
				return false;
			}
			
			// Копируем данные в очередь отправки
			std::vector<uint8_t> data_to_send(_data, _data + _size);
			
			{
				std::lock_guard<std::mutex> lock(output_mutex_);
				output_queue_.push(std::move(data_to_send));
				output_queue_size_ += _size;
			}
			
			return true;
		}

		bool  serialovtcp::put(uint8_t _data){
			return put(&_data,1);
		}

		void serialovtcp::reset(void) {
			stop();
			start();
		}

		size_t serialovtcp::space() {
			return MAX_BUFFER_SIZE - output_queue_size_.load();
		}

		size_t serialovtcp::space_max() {
			return MAX_SINGLE_WRITE;
		}

		bool serialovtcp::is_running() const {
			return is_running_;
		}

		bool serialovtcp::is_client_connected() const {
			return client_connected_;
		}

		std::string serialovtcp::get_client_info() const {
			return client_address_;
		}
	}
}