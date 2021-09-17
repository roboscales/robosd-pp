#ifndef robosd_net_flow_hpp
#define robosd_net_flow_hpp
#include "core/robosd_common.hpp"
#include "core/robosd_list.hpp"
#include "core/robosd_tran.h"
#include "net/robosd_serial.hpp"
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
				bool put(const uint8_t* _data, size_t _offset, size_t _size);
				bool put(iserial& _serial, size_t _offset, size_t _size);
				void set_size(size_t _size);
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
				enum class kind_t { frontend, backend };
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
				bool put_answer(const uint8_t* _data, size_t _size);
				bool put_answer(msg* _msg);
			public:
				msg* msg_query(void);
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

			template <typename D> class port_t : public port, private D {
			public:
				port_t() :port(D::packet_size, D::suba_count) {
					for (int i = 0; i < D::msg_pool_size; i++) {
						new msg_a<D::packet_size>(*this);
					}
				}

				void on_receive(unsigned _id, uint8_t* _data, size_t _size) {
					typename  D::id_t id;
					id.value = _id;
					if (id.slave == 0) {
						if (id.request == 1) {
							::robo::net::flow::msg* pmsg = query();
							if (pmsg) {
								pmsg->put(_data, _size);
								put(id.suba, pmsg);
							}
						}
						else {
							if (_size > 0) {
								//длина запрошенных данных в первом и единственном байте
								id.slave = 1;
								::robo::net::flow::msg* out_msg = get(id.suba);
								if (out_msg) {
									size_t query_len = _data[0];
									if (out_msg->size() == query_len) {
										D::send(id.value, out_msg->data(), query_len);
										put(id.suba, nullptr);
									}
									out_msg->release();
								}
							}
						}
					}
				}
			};

			class serial_proto : public performer {
				size_t declared_count_ = 0;
				net::iserial& remote_;
				net::iserial& local_;
			public:
				serial_proto(
					port& _port
					, uint8_t _command
					, uint8_t _suba
					, uint8_t _answ_suba
					, kind_t _kind
					, iserial& _remote
					, iserial& _local
				)
					: performer(
						_port
						, _command
						, _suba
						, _answ_suba
						, _kind
					)
					, remote_(_remote)
					, local_(_local) {}
					net::iserial& local(void){return local_;}
			protected:

				virtual void execute(void);
			};
			template <unsigned SA, unsigned SB, typename G = void > class serial_proto_t
				: protected bridge_t<SA, SB, G>
				, public serial_proto {
			public:
				serial_proto_t(
					port& _port
					, uint8_t _command
					, uint8_t _suba
					, uint8_t _answ_suba
					, kind_t _kind
				) : bridge_t<SA, SB, G>(), serial_proto(
					_port
					, _command
					, _suba
					, _answ_suba
					, _kind
					, bridge_t<SA, SB, G>::A
					, bridge_t<SA, SB, G>::B
				) {

				}
			};
		}
	}
}

#define FLOW_ROUTE_RECORD_A( P, C , K)  FLOW_ROUTE_RECORD_A_(P, C, K)
#define	 FLOW_ROUTE_RECORD_A_( P, C, K)	class P##_##C : public  ::robo::net::flow::performer { \
protected:	\
	virtual void execute(void);	\
public:	\
	P##_##C() :performer(\
		P\
		, P##_##C##_FLOW_CMD_ID \
		, P##_##C##_SUBA \
		, P##_##C##_SUBA_ANSW \
		, kind_t::P##_##C##_KIND \
	) {} \
} P##_##C##_;

#define FLOW_ROUTE_RECORD_B( P, C , K, T)  FLOW_ROUTE_RECORD_B_(P, C, K, T)
#define	 FLOW_ROUTE_RECORD_B_( P, C, K, T)	class P##_##C : public  ::robo::net::flow::performer { \
protected:	\
	virtual void execute(void){  T } \
public:	\
	P##_##C() :performer(\
		P\
		, P##_##C##_FLOW_CMD_ID \
		, P##_##C##_SUBA \
		, P##_##C##_SUBA_ANSW \
		, kind_t::P##_##C##_KIND \
	) {} \
} P##_##C##_;


#define FLOW_SERIAL_ROUTE_RECORD(NI,NO, P, C , K)  FLOW_SERIAL_ROUTE_RECORD_(NI,NO,P, C, K)

#define FLOW_SERIAL_ROUTE_RECORD_(NI,NO,P, C, K) ::robo::net::flow::serial_proto_t<NI, NO, ::robo::system::guard> P##_##C##_( \
	P\
	, P##_##C##_FLOW_CMD_ID \
	, P##_##C##_SUBA \
	, P##_##C##_SUBA_ANSW \
	, ::robo::net::flow::performer::kind_t::P##_##C##_KIND \
) ;
#endif
#endif