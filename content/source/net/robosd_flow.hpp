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
				bool put( uint8_t _data);
				//template<typename S> void put(const S& _data) { put((uint8_t*)&_data, (uint8_t) (sizeof(S) / sizeof(uint8_t))); };
				//template<typename S> void put(const S& _data, size_t _offset) { put((uint8_t*)&_data, _offset, sizeof(S) / sizeof(uint8_t)); };
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
			
			class rout_record;
	
			class performer {
				friend class port;
				friend class machine;
				friend class rout_record;
				public:
					typedef  ::robo::list::unique <performer,int> map;
					typedef  map::ref ref;
					enum class request { idle = 0, get = 1, put = 2 };
					enum class kind_t { frontend, backend };
				private:
					ref ref_;
					port * port_;
					rout_record* rout_record_;
					request request_ = request::idle;
					kind_t kind_;
				protected:
					msg* in_msg = nullptr;
					size_t max_size(void);
					virtual void execute(void) = 0;
					virtual void begin(void) {};
					bool put_answer(const uint8_t* _data, size_t _size);
					bool put_answer(msg* _msg);
				public:
					msg* msg_query(void);
					performer(cstr _command_path, kind_t _kind);
					static performer * find(cstr _path);
					static performer* find(int _ix);
			};
			
			class port;
			
			class rout_record {
				friend class performer;
				friend class port;
				friend class machine;
				private:
					typedef  ::robo::list::unsorted <rout_record> list;
					typedef  list::ref ref;
					ref ref_;
					int port_ix_;
					int command_ix_;
					uint8_t suba_;
					uint8_t answ_suba_;
				public:
					rout_record(
						cstr  _port_path
						, cstr  _command_path
						, uint8_t _suba
						, uint8_t _answ_suba
					);
			};	
				
			class port {
				friend class performer;
				friend class msg;
				friend class machine;
			private:
				typedef  ::robo::list::unique <port, int> map;
				typedef  map::ref ref;
				ref ref_;
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

				port(cstr _path, size_t _max_size, size_t _suba_count);
				virtual ~port(void);
				static port* find(cstr _path);
				static port* find(int _ix);

			};

			class machine {
				friend class  port;
				friend class  performer;
				friend class  rout_record;
				performer::map frontend_list_;
				performer::map backend_list_;
				performer::map performers_;
				port::map ports_;
				rout_record::list rout_records_;
				static machine& instance_(void);
				static void run_(performer* _performer);
			public:
				static void begin(void);
				static void frontend_poll(void);
				static void backend_poll(void);
			};

			template <typename D> class port_t : public port, private D {
			public:
				port_t() :port(D::path, D::packet_size, D::suba_count) {
					for (int i = 0; i < D::msg_pool_size; i++) {
						new msg_a<D::packet_size>(*this);
					}
				}

				void on_receive(uint32_t _id, const uint8_t* _data, size_t _size) {
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
									uint8_t query_len = _data[0];
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
					cstr _local_path
					, kind_t _kind
					, iserial& _remote
					, iserial& _local
				)
					: performer(
						_local_path
						, _kind
					)
					, remote_(_remote)
					, local_(_local) {
						bool ret = _local.begin(_local_path);
					ROBO_APP_ASSERT(ret);
				}
				net::iserial& local(void){return local_;}
			protected:
				virtual void execute(void);
			};

			struct snapshot {
				virtual size_t size() = 0;
				virtual const uint8_t * data()= 0;				
				virtual void update() = 0;
			};

			struct goal {
				virtual void applay(const uint8_t * data, size_t _size) = 0;
			};

			class snapshot_proto : public performer {
				friend class goal_proto;
				snapshot & snapshot_;
				size_t actual_size_ = 0;
				const uint8_t* actual_ = nullptr;
				size_t declared_ = 0;
				uint8_t page_ = 0;
				size_t page_count_=0;
				size_t page_size_=0;
			public:
				snapshot_proto(
					cstr _command_path
					, kind_t _kind
					, snapshot& _snapshot
				);
				void update_and_post(void);
			protected:
				virtual void execute(void);
				virtual void begin(void);
			private:
				void post_page_(uint8_t _page);
			};

			class goal_proto : public performer {
				goal & goal_;
				snapshot_proto & snapshot_proto_;
			public:
				goal_proto(
					cstr _command_path
					, kind_t _kind
					, goal& _goal
					, snapshot_proto & _snapshot_proto
				);
			protected:
				virtual void execute(void);
				virtual void begin(void);
			private:
			};

			template <typename D, unsigned SA, unsigned SB, typename G = void > class hardware_serial_proto_t
				: protected hardware_bridge_t<D, SA, SB, G>
				, public serial_proto {
			public:
				hardware_serial_proto_t(
					cstr _local_path
					, kind_t _kind
				) : hardware_bridge_t<D,SA, SB, G>(), serial_proto(
					_local_path
					, _kind
					, hardware_bridge_t<D,SA, SB, G>::A
					, hardware_bridge_t<D,SA, SB, G>::B
				) {

				}
			};
			template <unsigned SA, unsigned SB, typename G = void > class serial_proto_t
				: protected bridge_t<SA, SB, G>
				, public serial_proto {
			public:
				serial_proto_t(
					cstr _local_path
					, cstr  _remote_path
					, kind_t _kind
				) : bridge_t<SA, SB, G>(), serial_proto(
					_local_path
					, _kind
					, bridge_t<SA, SB, G>::A
					, bridge_t<SA, SB, G>::B
				) {
					bridge_t<SA, SB, G>::B.begin(_remote_path);
				}
				
			};
		}
	}
}

#define FLOW_PERFORMER_CUSTOM_RECORD( C,P, E) FLOW_PERFORMER_CUSTOM_RECORD_( C,P, E )
#define FLOW_PERFORMER_CUSTOM_RECORD_(C,P, E) \
class C##_##P##_performer_  : public ::robo::net::flow::performer { \
public:\
	C##_##P##_performer_ (void) : performer(C##_PATH, ::robo::net::flow::performer::kind_t:: C##_KIND) {}\
	virtual void execute(void) { E 	} \
}; \
C##_##P##_performer_ C##_##P##_performer__; \
::robo::net::flow::rout_record C##_##P##_rout_record_( \
	P##_PATH \
	, C##_PATH \
	, C##_##P##_SUBA \
	, C##_##P##_SUBA_ANSW \
);

#define FLOW_SERIAL_PERFORMER_RECORD( C,P, NI,NO ) FLOW_SERIAL_PERFORMER_RECORD_( C,P, NI,NO )
#define FLOW_SERIAL_PERFORMER_RECORD_(C,P, NI,NO) \
::robo::net::flow::serial_proto_t<NI, NO, void>  C##_##P##_proto_(C##_PATH,C##_PATH, ::robo::net::flow::performer::kind_t:: C##_KIND);\
::robo::net::flow::rout_record C##_##P##_rout_record_( \
	P##_PATH \
	, C##_PATH \
	, C##_##P##_SUBA \
	, C##_##P##_SUBA_ANSW \
);

#define FLOW_PERFORMER_RECORD( C, P ) FLOW_PERFORMER_RECORD_( C, P )
#define FLOW_PERFORMER_RECORD_( C, P ) \
::robo::net::flow::rout_record C##_##P##_rout_record_( \
	P##_PATH \
	, C##_PATH \
	, C##_##P##_SUBA \
	, C##_##P##_SUBA_ANSW \
);


#define FLOW_PERFORMER_RECORD( C, P ) FLOW_PERFORMER_RECORD_( C, P )
#define FLOW_PERFORMER_RECORD_( C, P ) \
::robo::net::flow::rout_record C##_##P##_rout_record_( \
	P##_PATH \
	, C##_PATH \
	, C##_##P##_SUBA \
	, C##_##P##_SUBA_ANSW \
);

#define FLOW_PERFORMER_RECORD_C( C ) FLOW_PERFORMER_RECORD_C_( C )
#define FLOW_PERFORMER_RECORD_C_( C ) \
::robo::net::flow::rout_record C##_rout_record_( \
	C##_PORT_PATH \
	, C##_PATH \
	, C##_SUBA \
	, C##_SUBA_ANSW \
);

#define FLOW_PERFORMER_CUSTOM_RECORD_C( C, E) FLOW_PERFORMER_CUSTOM_RECORD_C_( C, E )
#define FLOW_PERFORMER_CUSTOM_RECORD_C_(C, E) \
class C##_performer_  : public ::robo::net::flow::performer { \
public:\
	C##_performer_ (void) : performer(C##_PATH, ::robo::net::flow::performer::kind_t:: C##_KIND) {}\
	virtual void execute(void) { E 	} \
}; \
C##_performer_ C##_performer__; \
::robo::net::flow::rout_record C##_rout_record_( \
	C##_PORT_PATH \
	, C##_PATH \
	, C##_SUBA \
	, C##_SUBA_ANSW \
);

#define FLOW_SERIAL_PERFORMER_RECORD_C( C, NI,NO ) FLOW_SERIAL_PERFORMER_RECORD_C_( C, NI,NO )
#define FLOW_SERIAL_PERFORMER_RECORD_C_(C, NI,NO) \
::robo::net::flow::serial_proto_t<NI, NO, void>  C##_proto_(C##_PATH,C##_PATH, ::robo::net::flow::performer::kind_t:: C##_KIND);\
::robo::net::flow::rout_record C##_rout_record_( \
	C##_PORT_PATH \
	, C##_PATH \
	, C##_SUBA \
	, C##_SUBA_ANSW \
);

#define FLOW_SERIAL_FRONTEND_PERFORMER( C, IN, ON ) FLOW_SERIAL_FRONTEND_PERFORMER_( C, IN, ON )
#define FLOW_SERIAL_FRONTEND_PERFORMER_( C, IN, ON ) \
::robo::net::flow::serial_proto_t<IN,ON, ::robo::system::guard>  C##_proto_(C##_PATH, ::robo::net::flow::performer::kind_t::frontend);

#define FLOW_SERIAL_BACKEND_PERFORMER( C, IN, ON ) FLOW_SERIAL_BACKEND_PERFORMER_( C, IN, ON )
#define FLOW_SERIAL_BACKEND_PERFORMER_( C, IN, ON ) \
::robo::net::flow::serial_proto_t<IN,ON, ::robo::system::guard>  C##_proto_(C##_PATH, ::robo::net::flow::performer::kind_t::backend);


#endif
#endif