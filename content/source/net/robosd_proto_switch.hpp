#ifndef __robosd_proto_switch_h
#define __robosd_proto_switch_h
#include "core/robosd_common.hpp"

#ifndef APP_PROTO_SWITCH_ENABLED 
#define APP_PROTO_SWITCH_ENABLED 0
#endif

#if ROBO_APP_PROTO_SWITCH_ENABLED ==1
#include "core/robosd_list.hpp"
#include "net/robosd_serial.hpp"

#ifndef ROBO_PROTO_SWITCH_MARKER_MIN_LEN 
#define ROBO_PROTO_SWITCH_MARKER_MIN_LEN 3
#endif

#ifndef ROBO_PROTO_SWITCH_MARKER_MAX_LEN 
#define ROBO_PROTO_SWITCH_MARKER_MAX_LEN 7
#endif
namespace robo{
	namespace net{
		namespace proto{
			namespace switcher{
				struct ROBO_EXPORT marker{
					const uint8_t * value;
					const size_t len;
					marker(const uint8_t * _value, size_t _len) : value(_value), len(_len){}
				};
				struct ROBO_EXPORT key{
					size_t len;
					uint8_t value[ROBO_PROTO_SWITCH_MARKER_MAX_LEN];
					bool operator == (const marker & _marker);
					key(void);
					struct ROBO_EXPORT buffer{
						size_t len;
						uint8_t * memo;
						uint8_t value[ROBO_PROTO_SWITCH_MARKER_MAX_LEN];
						void operator = (const key & _key);
						buffer() : len(0), memo(0) {}
					};
				};
				class abonent;
				class ROBO_EXPORT port {
					friend class core;
					friend class abonent;
				public:
					typedef enum { SERIAL, PACKET } type;
				private:
					typedef robo::list::unsorted<port> list;
					typedef list::ref ref;
					ref ref_;
					key key_;
					type type_;
					iserial * serial_;
					struct {
						time_us_t key_reset;
						time_us_t tick;
					} timeouts_;
					abonent * abonent_;
					void disconnect_(void);
					void poll_(time_us_t _period);
				public:
					port(
						type _type
					);
					port(
						type _type
						, iserial* _serial
						, time_us_t _key_reset_us
					);
					void connect(
						iserial * _serial
						, time_us_t _key_reset_us
					);
					~port(void);
				};
				class ROBO_EXPORT abonent: public iserial{
					friend class core;
					friend class port;
					typedef robo::list::unsorted<abonent> list;
					typedef list::ref ref;
					ref ref_;
					struct {
						time_us_t lock;
						time_us_t silence;
						time_us_t tick;
					} timeouts_;
					marker marker_;
					port * port_;
					void disconnect_(void);
					void poll_(time_us_t _period);
					bool terminate_;
					
					key::buffer key_;

				protected:
					iserial * serial();

					virtual void start(void) = 0;
					virtual void stop(void) = 0;
				public:
					virtual size_t available(void);
					virtual size_t space(void);
					virtual size_t get(uint8_t* _data, size_t _max_size);
					virtual bool put(const uint8_t* _data, size_t _size);
					virtual size_t get(uint8_t & _data);
					virtual bool  put(uint8_t);
					virtual void reset(void);
					virtual size_t space_max(void);
					abonent(
						const uint8_t * _marker
						, size_t _markerlen
						, time_us_t _lock_us
						, time_us_t _silence_us
					);
					virtual ~abonent(void);
					void disconnect(void);
					void alive(void);
				};
				class ROBO_EXPORT core{
					friend class abonent;
					friend class port;
					struct ROBO_EXPORT ports{
						port::list busy; //порты, занятые абонентами
						port::list ready; //незанятые порты
					} ports_;

					abonent::list abonents_; //все абоненты
					bool recognize_(port * _port);
					void poll_(time_us_t _period);
					core(void);
					~core(void);
					static core & instance_(void);
					public:
						static void poll(time_us_t _period){ instance_().poll_(_period); }
				};

			}
		}
	}
}
#endif
#endif
