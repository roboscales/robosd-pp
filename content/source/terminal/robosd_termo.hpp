#ifndef __robosd_termo_hpp
#define __robosd_termo_hpp
#include "core/robosd_common.hpp"
#ifndef ROBO_APP_TERMINAL_ENABLED
#define ROBO_APP_TERMINAL_ENABLED 0
#endif 
#if ROBO_APP_TERMINAL_ENABLED == 1
#include "core/robosd_lambda.hpp"
#include "core/robosd_list.hpp"
#include "net/robosd_serial.hpp"
#include "net/robosd_proto_switch.hpp"

#ifndef ROBOSD_TERMINAL_ARG_COUNT_MAX 
#define ROBOSD_TERMINAL_ARG_COUNT_MAX 16
#endif

#ifndef ROBOSD_TERMINAL_READ_BUF_LEN 
#define ROBOSD_TERMINAL_READ_BUF_LEN 254
#endif
#ifndef ROBOSD_TERMINAL_WRITE_BUF_LEN 
#define ROBOSD_TERMINAL_WRITE_BUF_LEN 256
#endif
#ifndef ROBOSD_TERMINAL_BUFFER_SIZE
#define ROBOSD_TERMINAL_BUFFER_SIZE 256
#endif
#ifndef ROBOSD_TERMINAL_CONTEXT_SIZE
#define ROBOSD_TERMINAL_CONTEXT_SIZE 16
#endif
#ifndef ROBOSD_TERMINAL_HISTORY_DEPTH
#define ROBOSD_TERMINAL_HISTORY_DEPTH 16
#endif

#ifndef ROBOSD_TERMINAL_PROMPT_SIZE
#define ROBOSD_TERMINAL_PROMPT_SIZE 16
#endif

#ifndef ROBOSD_TERMINAL_ARG_LEN_MAX
#define ROBOSD_TERMINAL_ARG_LEN_MAX 50
#endif


#ifndef ROBOSD_TERMINAL_TOPCHAR
#define ROBOSD_TERMINAL_TOPCHAR '/'
#endif

#ifndef ROBOSD_TERMINAL_GLOBAL_LOCK_ENABLED
#define ROBOSD_TERMINAL_GLOBAL_LOCK_ENABLED 0
#endif
#ifndef ROBOSD_TERMINAL_CORE_LOCK_ENABLED
#define ROBOSD_TERMINAL_CORE_LOCK_ENABLED 0
#endif

namespace robo{
	namespace termo{
		class command;
		
		class ROBO_EXPORT delegat{
			friend class core;
			friend class command;
		public:
			typedef ::robo::list::unsorted<delegat> list;
			typedef list::ref ref;
		private:
			::robo::net::iserial * serial_ = 0;
			ref ref_;
		protected:
			command & owner;
			virtual void execute() = 0;
		public:
			delegat(command & _command);
		};

		class ROBO_EXPORT command{
			friend class core;
			friend class delegat;
			typedef ::robo::list::unsorted<command> list;
			typedef list::ref ref;
			ref ref_;
			ref exec_ref_;
			const char * name_;                  // command input name, not 0 
			const char * note_;                  // short help string, can be 0 
			const char * usage_;                  // help string, can be 0 
			list childs_;
			delegat::list delegats_;
			command * parent_ ;
			bool parse_arg_(int argc, const char * _arg[]);
			void exec_(char *_arg);
			void display_child_help_(void);	
			bool begin_(void);
		protected:
			virtual bool begin(void) = 0;
			virtual bool loop(void) = 0;
			virtual bool parse_long_arg(const char * _arg, const char * _val) = 0;
			virtual bool parse_arg(char _arg, const char * _val) = 0;
			virtual bool parse_opt(char _opt) = 0;
			virtual bool parse_long_opt(const char * _opt) = 0;
		public:
			const char * name(){return name_;};  
			command(
				const char * _name
				, const char * _note
				, const char * _usage
				, command * _parent
			);
			virtual ~command();
		};
		

		class node :public command{
		protected:
			virtual bool begin(void);
			virtual bool loop(void);
			virtual bool parse_long_arg(const char * _arg, const char * _val);
			virtual bool parse_arg(char _arg, const char * _val);
			virtual bool parse_opt(char _opt);
			virtual bool parse_long_opt(const char * _opt);
		public:
			node(
				const char * _name
				, const char * _note
				, const char * _usage
				, command * _parent
				);
		};
		class ROBO_EXPORT itf{
		public:
			class ievents{
			public:
				lambda< void ( void ) > connect;
				lambda< void ( void ) > disconnect;
				lambda< void ( void ) > alive;
				lambda< void ( void ) > stop;
				ievents(void){
					connect = []()-> void{};
					disconnect = []()-> void{};
					alive = []()-> void{};
					stop = []()-> void{};				
				}
			};
			static ievents & events();
			static void stop(void);
			static void top(void);
			static void set_prompt(const char * _str); //Формат приглашения
			static void poll(void); //Если очередная команда выполнена- заглянет во входной поток и обработает очередной символ. Вызывать из прерывания, например.
			static void exec(const char * _cmd); //Эмуляция пользовательского ввода
			static void new_line(void); //Перевод строки в терминале
			static void printf(const char * _format, ...);
			static void printfa(const char * _format, va_list _args);
			static void prints(const char * _s);
			static void put(char  _ch);
			static command * root();
			static void connect(::robo::net::iserial * _serial);
			static void connect(const robo::cstr _str );
			static bool busy(void);
			static void echo_off(void);
			static size_t get(uint8_t & _fata);
			static void serial_enter(void);
			static void serial_leave(void);
		};
#if ROBO_APP_PROTO_SWITCH_ENABLED ==1
		class ROBO_EXPORT abonent : public robo::net::proto::switcher::abonent {
		protected:
			virtual void stop(void);
			virtual void start(void);
		public:
			abonent(
				const uint8_t * _marker
				, size_t _markerlen
				, time_us_t _lock_us
				, time_us_t _silence_us
				);
		};
#endif
	}
}
#endif
#endif
