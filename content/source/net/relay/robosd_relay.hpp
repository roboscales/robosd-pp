#ifndef __robo_net_reley_hpp
#define __robo_net_reley_hpp


#include "core/robosd_common.hpp"
#include "net/robosd_serial.hpp"
#include "core/robosd_list.hpp"

#ifndef ROBO_APP_NET_RELEY_ENABLED
#define  ROBO_APP_NET_RELEY_ENABLED 0
#endif

#if  ROBO_APP_NET_RELEY_ENABLED == 1
namespace robo{
	namespace net{
		namespace relay{
			const int UNKMOWN_PORT_ID = 0xFFFF;
			class ROBO_EXPORT channel;
			class ROBO_EXPORT abonent;
			class ROBO_EXPORT machine;
			class ROBO_EXPORT decoder;
			class ROBO_EXPORT encoder;
			class ROBO_EXPORT endpoint;

			class ROBO_EXPORT packet{
				friend class machine;
			protected:
				uint8_t * data_;
				size_t size_;
				channel & channel_;
				endpoint & endpoint_;
				packet(channel & _channel, endpoint & _endpoint, uint8_t * _data, size_t _size);
				packet(channel & _channel, endpoint & _endpoint, size_t _size);
				packet(channel & _channel, endpoint & _endpoint, uint8_t _msg, uint8_t * _data, size_t _size);
				virtual ~packet(void);
			public:
				inline  const uint8_t *   data(void) { return  data_; }
				inline size_t size(void) { return size_; }
				int  endpoint_id(void);
				int  channel_id(void);
				//inline channel & endpoint_channel(void) { return channel_; }
				//void use(list & _list);
				//static void relese(packet * &  _packet);				
			};
			
			class ROBO_EXPORT in : public packet{
			public:
				typedef robo::list::unsorted<in> list;
				typedef robo::list::ref<in> ref;
				friend class machine;
			private:
				ROBO_REDECLARE_NEW
				ref ref_;
				in(channel & _channel, endpoint & _endpoint, robo_byte_p _data, robo_size_t _size);
			protected:
				virtual ~in(void){};
			public:
				void confirm(void);
				void refuse(void);
			};

			class ROBO_EXPORT out : public packet{
			public:
				typedef robo::list::queue<out> queue;
				typedef robo::list::ref<out> ref;
				friend class machine;
				friend class channel;
				operator ref * (){ return &ref_; }
			private:
				ROBO_REDECLARE_NEW
				ref ref_;
				out(channel & _channel, endpoint & _endpoint, robo_byte_p _data, robo_size_t _size);
				out(channel & _channel, endpoint & _endpoint,  robo_size_t _size);
				out(channel & _channel, endpoint & _endpoint, uint8_t _msg, robo_byte_p _data, robo_size_t _size);
				
			protected:
				virtual ~out(void){};
			public:
				void confirm(void);
				void refuse(void);
			};


			class ROBO_EXPORT channel{
				friend class decoder;
				friend class encoder;
				friend class machine;
				friend class packet;
				friend class in;
				friend class out;
			public:
				typedef robo::list::map<channel> map;
				typedef robo::list::ref<channel> ref;
			private:
				ref ref_;
				encoder * encoder_;
				decoder * decoder_;
				out * sended_;
				out::queue outcom_;
			protected:
                virtual void confirm(in * /*_in*/){}
                virtual void refuse(in * /*_in*/){}
			public:
				int id(void) { return ref_.id();  }
				channel(int _id);
				virtual ~channel(void){}
				void poll(void);
                bool ready() {return  outcom_.count()==0;};

			};


			class ROBO_EXPORT decoder{
			protected:
				channel & channel_;			
			public:
				decoder(channel & _channel);
				virtual ~decoder(void);
				virtual void poll(void) = 0;
			};

			class ROBO_EXPORT encoder{
				friend class channel;
			protected:
				channel & channel_;
				virtual robo_result_t process(void) = 0;
				virtual void send(out * _out) = 0;
			public:
				encoder(channel & _channel);
				virtual ~encoder(void);
			};

			class ROBO_EXPORT endpoint {
			public:
				friend class in;
				friend class out;
				friend class machine;
				friend class packet;
				typedef robo::list::map<endpoint> map;
				typedef robo::list::ref<endpoint> ref;
			private:
				in::list incom_;
				ref ref_;
			protected:
                virtual void confirm(out * /*_in*/){}
                virtual void refuse(out * /*_in*/){}
				virtual robo_result_t process(in * _in) = 0;
			public:
				endpoint(void);
				bool attach(int _id);
				void detach(void);
				endpoint(int _id);
				virtual ~endpoint(void);
				void reset(void);
				void poll();
			};

			class ROBO_EXPORT machine {
				friend class channel;
				friend class endpoint;
				channel::map channels_;
				endpoint::map endpoints_;
				//abonent * resolve_(uint8_t endpoint_channel_);
				bool receive_(channel & _channel, int _endpoint_id, robo_byte_p _data, robo_size_t _size);
				bool post_(int _channel, endpoint & _endpoint, const robo_byte_p _data, robo_size_t _size);
				bool post_(int _channel, endpoint & _endpoint, uint8_t _msg, robo_byte_p _data, robo_size_t _size);
				bool post_(out * _out);
				out * query_(int _channel, endpoint & _endpoint, robo_size_t _size);
				static machine & instance(void);
				void poll_(void);
				void release_(packet * &  _packet);
                bool ready_(int _channel);
            public:
				static bool receive(channel & _channel, int _endpoint_id, robo_byte_p _data, robo_size_t _size){
					return instance().receive_(_channel, _endpoint_id, _data, _size);
				}
				static bool post(int _channel, endpoint & _endpoint, const robo_byte_p _data, robo_size_t _size){
					return instance().post_(_channel, _endpoint, _data, _size);
				}
				static bool post(int _channel, endpoint & _endpoint, uint8_t _msg, robo_byte_p _data, robo_size_t _size){
					return instance().post_(_channel, _endpoint, _msg ,_data, _size);
				}
				static bool post(out * _out){
					return instance().post_(_out);
				}
				static void poll(void){
					instance().poll_();
				}
				static void release(packet * &  _packet){
					instance().release_(_packet);
				}
				static out * query(int _channel, endpoint & _endpoint, robo_size_t _size){
					return instance().query_(_channel, _endpoint, _size);
				}
                static bool ready(int _channel){
                    return instance().ready_(_channel);
                }
                //void complete(packet * _packet);
				//void refuse(packet * _packet);
			};
		}
	}
} 

#endif

#endif
