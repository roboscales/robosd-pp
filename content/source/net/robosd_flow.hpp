#ifndef robosd_net_flow_hpp
#define robosd_net_flow_hpp
#include "core/robosd_common.hpp"
#include "core/robosd_list.hpp"
#include "core/robosd_tran.h"
#if ROBO_APP_NET_FLOW_ENABLED==1
namespace robo {
	namespace net {
		namespace flow {
			class port;

			class msg {
				friend class port;
				friend class machine;
				friend class performer;
				port& port_;
				size_t size_ = 0;
				uint8_t* data_;

			public:
				typedef  ::robo::list::unsorted <msg> list;
				typedef  list::ref ref;
			private:
				ref ref_;
				void release_(void);
			public:
				const uint8_t* data(void) { return data_; }
				size_t size(void) { return size_; }
				size_t max_size(void);
				void release(void);
				bool put(const uint8_t* _data, size_t _size);
				msg(port& _port, uint8_t* _data);
				virtual ~msg(void) {};

			};

			template <size_t sz> class msg_a : public msg {
				uint8_t memo[sz];
			public:
				msg_a(port& _port) : msg(_port, memo) {};
			};

			class performer {
			public:
				friend class port;
				friend class machine;
				typedef  ::robo::list::unsorted <performer> list;
				typedef  list::ref ref;
				enum class request { idle = 0, get = 1, put = 2 };
				enum class kind_t { fronteend, backend };
			private:
				ref ref_;
				port& port_;
				uint8_t command_;
				uint8_t suba_;
				uint8_t answ_suba_;
				request request_ = request::idle;
				kind_t kind_;
			protected:
				msg* in_msg = nullptr;
				size_t max_size(void);
				virtual void execute(void) = 0;
				bool send(const uint8_t* _data, size_t _size);
			public:
				performer(
					port& _port
					, uint8_t _command
					, uint8_t _suba
					, uint8_t _answ_suba
					, kind_t _kind
				);
			};

			class port {
				friend class performer;
				friend class msg;
				msg::list pool_;
				size_t max_size_;
				size_t suba_count_;
				performer** performer_index_ = nullptr;
				msg** outcomm_ = nullptr;
			protected:
				bool put(uint8_t _suba, msg* _msg);
			public:
				msg* get(uint8_t _suba);
				/** у каждого порта потенциально свой пул пакетов*/
				msg* query(void);

				size_t max_size(void) { return max_size_; };
				size_t suba_count(void) { return suba_count_; };

				port(size_t _max_size, size_t _suba_count);
				virtual ~port(void);
			};

			class machine {
				friend class  port;
				performer::list frontend_list_;
				performer::list backend_list_;
				static machine& instance_(void);
				static void run_(performer* _performer);
			public:
				static void frontend_poll(void);
				static void backend_poll(void);
			};

			template <size_t sz> class port_t : public port {
			public:
				port_t(size_t _suba_count) :port(sz, _suba_count) {
					for (int i = 0; i < _suba_count; i++) {
						new msg_a<sz>(*this);
					}
				}
			};
		}
	}
}
#endif
#endif