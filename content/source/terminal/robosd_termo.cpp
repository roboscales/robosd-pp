#include "terminal/robosd_termo.hpp"
#include "core/robosd_string.hpp"
#include "core/robosd_ring_buf.hpp"
#include <stdarg.h>

#if ROBO_APP_TERMINAL_ENABLED == 1
#ifndef TERMO_STREAMER_OUT_BUF_SIZE_BITS
#define TERMO_STREAMER_OUT_BUF_SIZE_BITS 7
#endif

namespace robo{
	namespace termo{
		ring_t<TERMO_STREAMER_OUT_BUF_SIZE_BITS> out_buf_;
		class core{
			friend class command;
			friend class itf;
			node root_cmd_;
			char input_buffers_[ROBOSD_TERMINAL_HISTORY_DEPTH][ROBOSD_TERMINAL_BUFFER_SIZE + 1];
			command * context_buffer_[ROBOSD_TERMINAL_CONTEXT_SIZE];
			char prompt_[ROBOSD_TERMINAL_PROMPT_SIZE + 1];
			int cur_buf_index_;
			int cur_context_;
			int cur_index_;
			command * command_context_;
			::robo::net::iserial * serial_ = 0;
			bool echo_ = true;
			bool nopromt_ = false;
			bool terminated_ = true;
			//bool external_echo_;
			command::list execs_;

			//void add_command_(command *);
			void start_of_line_(void);
			typedef enum { NULLMATCH, FULLMATCH, PARTMATCH, UNMATCH, MATCH, AMBIG } match;
			match strstart_(const char *s1, const char *s2);
			match parse_command_(char **_str, command * & _result_cmd);
			match exec_command_line_(char *_str);
			match help_command_line_(char *_str);
			match complete_command_line_(char *_str);
			void process_(char c); // обработка очередного символа
			void set_prompt_(const char *str);
			void exec_(const char * cmd);
			void stop_(void);
			void run_(void);
			void poll_(void);
			void top_(void){
				command_context_ = &root_cmd_;
				cur_context_ = 0;
				start_of_line_();
			}

			void echo_put_(char _ch){
				if (echo_){
					itf::put(_ch);
				}
			}

			void echo_puts_(const char * _s){
				if (echo_){
					itf::prints(_s);
				}
			}
			void put_nl_(void){
				if (out_buf_.space() > 2){
					out_buf_.put('\n');
					out_buf_.put('\r');
				}
			}
			core(void) 
				: root_cmd_("@root", "", "<CR>", nullptr)
				, serial_(&robo::net::serial_dummy::instance())				
				{
			}
			void connect_to_(::robo::net::iserial * _serial){
				if (serial_){
					serial_->reset();
				}
				if (_serial){
					serial_ = _serial;
					serial_->reset();
					top_();
					terminated_ = false;
					itf::events().connect();
					exec_("w\r\n");
				}
				else{
					serial_ = &robo::net::serial_dummy::instance();
					itf::events().disconnect();
					out_buf_.clear();
					terminated_ = false;
				}
			}
			static core & instance_(void){
				static core instance_;
				return instance_;
			}
		};
		void itf::printf(const char * _format, ...){
			va_list args;
			va_start(args, _format);
			{
				string::format_stream(out_buf_,_format,args);
			}
			va_end(args);
		};
		void itf::prints(const char * _s){
			if (_s){
				size_t space = out_buf_.space();
				while ((*_s) && (space--)){
					out_buf_.put(*_s++);
				}
			}
		};
		void itf::put(char  _ch){
			if( out_buf_.space() > 0){
				out_buf_.put(_ch);
			}
		};
		delegat::delegat(command & _command) 
			: ref_(*this), owner(_command) {
			ref_.attach_to(_command.delegats_);
		}
		command::command(
			const char * _name
			, const char * _note
			, const char * _usage
			, command * _parent
		) 
		: ref_(*this)
		, exec_ref_(*this)
		, name_(_name)
		, note_(_note)
		, usage_(_usage)
		, parent_(_parent)
		{
			if (parent_){
				ref_.attach_to(parent_->childs_);
			}
		}
		command::~command(){
			while (childs_.count()){
				command & ch = childs_.last()->owner();
				ch.ref_.dettach();
			}
		}
		bool command::begin_(void){
			bool ret = begin();
			for (delegat::ref * rf = delegats_.first(); rf; rf = rf->next()){
				rf->owner().execute();
			}
			return ret;
		}
		node::node(const char * _name, const char * _note, const char * _usage, command * _parent) 
			: command(_name, _note, _usage, _parent	){}

		bool node::begin(void){
			return false;
		}
		bool node::loop(void){
			itf::prints("execute is not support");
			return false;
		}
		bool node::parse_long_arg(const char * _arg, const char * _val){
			itf::prints("argument """);
			itf::prints(_arg);
			itf::prints(""" is not support");
			return false;

		}
        bool node::parse_arg(char _arg, const char * /* _val*/ ){
			itf::prints("argument """);
			itf::put(_arg);
			itf::prints(""" is not support");
			return false;

		}
		bool node::parse_opt(char _opt){
			itf::prints("option """);
			itf::put(_opt);
			itf::prints(""" is not support");
			return false;
		}
		bool node::parse_long_opt(const char * _opt){
			itf::prints("option """);
			itf::prints(_opt);
			itf::prints(""" is not support");
			return false;

		}

		class ihelp_cmd: public node {
		protected:
			virtual bool begin(void){
				itf::prints(
					"?            display help on given or available commands\n\r"
					"<TAB>        auto-completion\n\r"
					"<ENTER>      execute command line\n\r"
					"CTRL-P       recall previous input line\n\r"
					"CTRL-N       recall next input line\n\r"
					"CTRL-Z       torgle external screen output\n\r"
					"<any>        treat as input character\n\r"
				);
				return false;
			}
		public:
			ihelp_cmd() : node( "help", "display help for current note", "<CR>", itf::root() ){}
		} help_cmd;

		class iwellcom_cmd : public node {
		protected:
			virtual bool begin(void){
				itf::prints(
					"\nRobosd terminal++ 3.0.alpha\n\r"
					"\tAndrei Iusupov <a.n.jusupov@gmail.com>\n\r\tFebruary 2020\n\r"
				);
				return false;
			}
		public:
			iwellcom_cmd() : node("wellcom", "show wellcom", "<CR>", itf::root()){}
		} wellcom_cmd;

		class iexit_cmd : public node {
		protected:
			virtual bool begin(void){
				itf::prints(
					"Good by! Robosd terminal closed.\n\r"
				);
				itf::stop();
				return false;
			}
		public:
			iexit_cmd() : node("exit", "exit from terminal", "<CR>", itf::root()){}
		} exit_cmd;

		class imarker_cmd : public node {
		protected:
			virtual bool begin(void){
				itf::top();
				return false;
			}
		public:
			imarker_cmd() : node("***", "fake command after marker received", "<CR>", itf::root()){}
		} marker_cmd;

		class itest_cmd : public command {
		protected:
			virtual bool begin(void){
				itf::prints("test options:\n\r");
				return true;
			}
			virtual bool loop(void){
				itf::prints("\tend of options\n\r");
				return false;
			}
			virtual bool parse_long_arg(const char * _arg, const char * _val){
				itf::printf("\t%s:%s\n\r", _arg, _val);
				return true;
			}
			virtual bool parse_arg(char _arg, const char * _val){
				itf::printf("\t[%c]:%s\n\r", _arg, _val);
				return true;
			}
			virtual bool parse_opt(char _opt){
				itf::printf("\tset[%c]\n\r", _opt);
				return true;
			}
			virtual bool parse_long_opt(const char * _opt){
				itf::printf("\tset[%s]\n\r", _opt);
				return true;
			}
		public:
			itest_cmd() : command("test", "test to command string"
				, "test [ <--,-><option><value> ]  [...] ... <CR>", itf::root()){}
		} test_cmd;

		void core::process_(char c){
			char *line = input_buffers_[cur_buf_index_];
			switch (c){
			case 0x1a:
				if (!echo_){
					echo_ = true;
					itf::prints("\n\r external echo on");
				}
				else
				{
					itf::prints("\n\r external echo OFF");
					echo_ = false;
				}
				break;
			case '\n':
				break;
			case '\r':
				put_nl_();
				while (*line && *line == ' ') line++;
				if (*line) // not empty line 
				{
					cur_buf_index_ = (cur_buf_index_ + 1) % ROBOSD_TERMINAL_HISTORY_DEPTH;
					cur_index_ = 0;
					input_buffers_[cur_buf_index_][0] = 0;
					exec_command_line_(line);
				}
				else {
					start_of_line_();
				}
				break;
			case ROBOSD_TERMINAL_TOPCHAR:
				top_();
				break;

			case 8: //backspace
			case 127:
				if (cur_index_ > 0)
				{
					cur_index_--;
					line[cur_index_] = 0;
				}
				echo_puts_("\b \b");
				break;
			case 16: // CTRL-P: back in history 
				{
					int prevline = (cur_buf_index_ + ROBOSD_TERMINAL_HISTORY_DEPTH - 1) % ROBOSD_TERMINAL_HISTORY_DEPTH;
					if (input_buffers_[prevline][0])
					{
						line = input_buffers_[prevline];
						// fill the rest of the line with spaces 
						if (echo_){
							while (cur_index_-- > 0)
								itf::prints("\b \b");
							itf::prints(line);
						}
						cur_index_ = std::strlen(line);
						cur_buf_index_ = prevline;
					}
				}
				break;
			case 14:
			case 40:// CTRL-N: next in history 
				{
					int nextline = (cur_buf_index_ + 1) % ROBOSD_TERMINAL_HISTORY_DEPTH;
					if (input_buffers_[nextline][0])
					{
						line = input_buffers_[nextline];
						// fill the rest of the line with spaces 
						if (echo_){
							while (cur_index_-- > 0)
								itf::prints("\b \b");
							itf::prints(line);
						}
						cur_index_ = std::strlen(line);
						cur_buf_index_ = nextline;
					}
				}
				break;
			case '?': // display help 
				echo_put_('?');
				help_command_line_(line);
				cur_buf_index_ = (cur_buf_index_ + 1) % ROBOSD_TERMINAL_HISTORY_DEPTH;
				cur_index_ = 0;
				input_buffers_[cur_buf_index_][0] = 0;
				start_of_line_();
				break;
			case '!':
			case 9: // TAB: autocompletion 
				if (complete_command_line_(line))
				{
					start_of_line_();
					itf::prints(line);
				}
				cur_index_ = std::strlen(line);
				break;
			default: // any input character 
				if (cur_index_ < ROBOSD_TERMINAL_BUFFER_SIZE)
				{
					line[cur_index_++] = c;
					line[cur_index_] = 0;
					echo_put_(c);
				}
			}
		}

		/* verify if the non-spaced part of s2 is included at the begining
		* of s1.
		* return FULLMATCH if s2 equal to s1, PARTMATCH if s1 starts with s2
		* but there are remaining chars in s1, UNMATCH if s1 does not start with
		* s2
		*/

		core::match	core::strstart_(const char *s1, const char *s2){
			while(*s1 && *s1==*s2) { s1++; s2++; }
			if(*s2==' ' || *s2==0)
			{
				if(*s1==0)
					return FULLMATCH; //  full match
				else
					return PARTMATCH; // partial match
			}
			else
				return UNMATCH;     // no match
		}

		/*
		* check commands at given level with input string.
		* _cmd: point to first command at this level, return matched cmd
		* _str: point to current unprocessed input, return next unprocessed
		*/
		
		core::match	core::parse_command_(char **_str, command  * & _result_cmd){
			char *str=*_str;
			command * cmd;
			command * ctx =  _result_cmd;
			command * matched_cmd = nullptr;
			// first eliminate first blanks
			while(*str==' ') str++;
			if(!*str)
			{
				*_str=str;
				return NULLMATCH; // end of input
			}

		// first pass: count matches
			for (command::ref * r = ctx->childs_.first(); r; r = r->next())
			{
				cmd = &r->owner();
				switch (strstart_(cmd->name_, str)){
				case FULLMATCH:
				// found full match
					while (*str && *str != ' ') str++;
					while (*str == ' ') str++;
					*_str = str;
					_result_cmd = cmd;
					return MATCH;

				case PARTMATCH:
					if (matched_cmd)
					{
						return AMBIG;
					}
					else
					{
						matched_cmd = cmd;
					}
				default:
					break;
				}
			}

			if(matched_cmd)
			{
				while(*str && *str!=' ') str++;
				while(*str==' ') str++;
				_result_cmd = matched_cmd;
				*_str=str;
				return MATCH;
			}
			else
				return UNMATCH;
		}
		bool command::parse_arg_(int argc, const char * _arg[]){
			const char * __arg;
			static char _name[ROBOSD_TERMINAL_ARG_LEN_MAX];
			for (int i = 1; i < argc; i++){
				__arg = _arg[i];
				if (*__arg == '-'){
					__arg++;
					char * __name = _name;
					char c = *__arg;
					int cl = 0;
					while (c && c != '-' &&  c != ' ' && cl<49) {
						*__name++ = c;
						cl++;
						c = *++__arg;
						if (cl == ROBOSD_TERMINAL_ARG_LEN_MAX-1) 
							return false;
					};
					*__name = 0;
					while (*__arg && *__arg == ' ') __arg++;
					if (*__arg == 0){
						if ( !parse_long_opt(_name)) return false;
					}
					else
					{
						if (!parse_long_arg(_name, __arg)) return false;
					}
				}
				else
				{
					char n = __arg[0];
					__arg++;
					while (*__arg && *__arg == ' ') __arg++;
					if (*__arg == 0){
						if (!parse_opt(n)) return false;
					}
					else
					{
						if (!parse_arg(n, __arg) ) return false;
					}
				}
			}
			return true;
		}

		void command::exec_(char *_arg){
			static const char *argv[ROBOSD_TERMINAL_ARG_COUNT_MAX];
			static char arg_buffer[ROBOSD_TERMINAL_BUFFER_SIZE + 1];
			int argc = 0;
			enum { FETCH_FIRST, FETCH_BEGIN, FETCH_DELIM, FETCH_ARG } fetch_status = FETCH_FIRST;
			// copy command line to preserve it for history 
			for (int i = 0; i<ROBOSD_TERMINAL_BUFFER_SIZE; i++)
				arg_buffer[i] = _arg[i];
			_arg = arg_buffer;

			// cut into arguments 
			argv[argc++] = name_;
			if (*_arg){
				do{
					switch (*_arg){
					case '-':
						switch (fetch_status)
						{
						case FETCH_FIRST:
						case FETCH_BEGIN:
							fetch_status = FETCH_DELIM;
							*_arg = 0;
							break;
						case FETCH_DELIM:
							argv[argc++] = _arg;
							fetch_status = FETCH_ARG;
							break;
						case FETCH_ARG:
							break;
						}
						break;
					case ' ':
						switch (fetch_status)
						{
						case FETCH_FIRST:
						case FETCH_BEGIN:
							break;
						case FETCH_DELIM:
							//ошибка 
							itf::printf("\n\rarg ""%s"" parsing  error: \n\r", _arg);
							return;
						case FETCH_ARG:
							// после первого пробела будем ждать
							fetch_status = FETCH_BEGIN;
							break;
						}
						break;
					default:
						switch (fetch_status)
						{
						case FETCH_FIRST:
							//ошибка
							itf::printf("\n\rarg ""%s"" parsing  error: \n\r", _arg);
							return;
						case FETCH_BEGIN:
							fetch_status = FETCH_ARG;
							break;
						case FETCH_DELIM:
							argv[argc++] = _arg;
							fetch_status = FETCH_ARG;
							break;
						case FETCH_ARG:
							break;
						}
					}
				} while (*(++_arg) && argc < ROBOSD_TERMINAL_ARG_COUNT_MAX);
			}
			if ( parse_arg_( argc, argv)){
				if (begin_()){
					exec_ref_.attach_to(core::instance_().execs_ );					
					return;
				}
			}
			itf::new_line();
		}
		// try to execute the current command line 
		core::match core::exec_command_line_(char *_str){
			char *str = _str;
			command * cmd = command_context_;
			int _cur_context = cur_context_;
			while (1)
			{
				switch (parse_command_(&str, cmd)){
				case MATCH:
					// found unique match 
					if (cmd->childs_.count() == 0) // no sub-command, execute 
					{
						cmd->exec_(str);
						return NULLMATCH;
					}
					else
					{
						context_buffer_[_cur_context++] = cmd;
						if (*str == 0) // no more input, this is a context 
						{
							cur_context_ = _cur_context;
							command_context_ = cmd;
							start_of_line_();
							return NULLMATCH;
						}
						continue;
					}
				case AMBIG:
					itf::printf("\n\rambiguity: %s\n\r", str);
					return NULLMATCH;
				case UNMATCH:
					itf::printf("\n\rno match: %s\n\r", str);
					return NULLMATCH;
				default:
					itf::printf("\n\rno match: %s\n\r", str);
					return NULLMATCH;
				}
			}
		}
		// display help for list of commands
		void command::display_child_help_(void){
			command * cm;
			int len = 0;
			itf::prints("\n\r");
			for ( command::ref * r = childs_.first(); r; r = r->next() ){
				int _len = std::strlen(r->owner().name_);
				if (len<_len)
					len = _len;
			}
			for (command::ref * r = childs_.first(); r; r = r->next()){
				cm = &r->owner();
				itf::prints(cm->name_);
				if (cm->note_)
				{
					for (int i = std::strlen(cm->name_); i < len + 2; i++)
						itf::put(' ');
					itf::prints(cm->note_);
				}
				itf::prints("\n\r");
			}
		}
		// try to display help for current comand line
		core::match core::help_command_line_(char *_str){
			char *str = _str;
			command * cmd = command_context_;
			while (1)
			{
				switch (parse_command_(&str, cmd)){
				case MATCH:
					if (*str == 0) // found unique match or empty line 
					{
						if (cmd->childs_.count() > 0){
							cmd->display_child_help_();
							return NULLMATCH;
						}
						else  // no sub-command, show single help 
						{
							itf::printf("\n\rnote: %s\n\r", cmd->note_);
							itf::printf("\n\rusage: %s\n\r", cmd->usage_);
						}
						return NULLMATCH;
					}
					break;
				case AMBIG:
					itf::printf("\n\rambiguity: %s\n\r", str);
					return NULLMATCH;
				case UNMATCH:
					itf::printf("\n\rno match: %s\n\r", str);
					return NULLMATCH;
				case NULLMATCH:
					if (cmd->childs_.count() > 0){
						cmd->display_child_help_();
					}
					return NULLMATCH;
				default:
					break;
				}
			}
		}
		// try to complete current command line
		core::match core::complete_command_line_(char *_str)	{
			char *str = _str;
			command * cmd = command_context_;

			while (1)
			{
				match ret;
				int common_len = ROBOSD_TERMINAL_BUFFER_SIZE;
				int _str_len;
				int i;
				char *__str = str;

				ret = parse_command_(&str, cmd);
				for (_str_len = 0; __str[_str_len] && __str[_str_len] != ' '; _str_len++);
				if (ret == MATCH && *str)
				{
					if (cmd->childs_.count() == 0){
						return NULLMATCH;
					}
				}
				else if (ret == AMBIG || ret == MATCH || ret == NULLMATCH)
				{
					command * cm;
					command *  matched_cmd = 0;
					int nb_match = 0;
					command::ref * rf = 0;
					for (rf = &(cmd->ref_); rf; rf = rf->next()){
						match r;
						cm = &rf->owner();
						r = strstart_(cm->name_, __str);
						if (r == FULLMATCH)
						{
							for (i = _str_len; cmd->name_[i]; i++)
								process_(cmd->name_[i]);
							if (*(str - 1) != ' ')
								process_(' ');
							if (cmd->childs_.count() ==0)
							{
								return NULLMATCH;
							}
							else{
								break;
							}
						}
						else if (r == PARTMATCH)
						{
							nb_match++;
							if (!matched_cmd)
							{
								matched_cmd = cm;
								common_len = std::strlen(cm->name_);
							}
							else
							{
								for (i = _str_len; cm->name_[i] && i<common_len &&
									cm->name_[i] == matched_cmd->name_[i]; i++);
								if (i<common_len)
									common_len = i;
							}
						}
					}
					if (rf)
						continue;
					if (matched_cmd)
					{
						if (_str_len == common_len)
						{
							put_nl_();
							command::ref * rf = 0;
							for (rf = &(cmd->ref_); rf; rf = rf->next()){
								cm = &rf->owner();
								match r = strstart_(cm->name_, __str);
								if (r == FULLMATCH || r == PARTMATCH)
								{
									itf::prints(cm->name_);
									put_nl_();
								}
							}
							return FULLMATCH;
						}
						else
						{
							for (i = _str_len; i<common_len; i++)
								process_(matched_cmd->name_[i]);
							if (nb_match == 1)
								process_(' ');
						}
					}
					return NULLMATCH;
				}
				else // UNMATCH 
				{
					return NULLMATCH;
				}
			}
		}

		void core::start_of_line_(){
			if (!nopromt_){
				// display start of new line 
				put_nl_();
				for (int i = 0; i < cur_context_; ++i){
					itf::put('\\');
					itf::prints(context_buffer_[i]->name_);
				}
				itf::prints(prompt_);
				cur_index_ = 0;
			}
		}

		void core::set_prompt_(const char *str){
			int i;
			for (i = 0; str[i] && i < ROBOSD_TERMINAL_PROMPT_SIZE; i++)
				prompt_[i] = str[i];
			prompt_[i] = 0;
		}

		void core::exec_(const char * cmd){
			bool tmp = echo_;
			bool tmp2 = nopromt_;
			echo_ = false;
			nopromt_ = true;
			while (*cmd) {
				process_(*cmd);
				cmd++;
			}
			echo_ = tmp;
			nopromt_ = tmp2;
		}
		void core::stop_(void){
			terminated_ = true;
			itf::events().stop();
		}

		void core::poll_(void){

			size_t sz = serial_->available();
			if (sz){
				while (sz--){
					process_(serial_->get());
				}
				itf::events().alive();
			}

			command::ref * r = execs_.first();
			if (r){
				command * cm = &r->owner();
				if ( ! cm->loop()){
					cm->exec_ref_.dettach();
					start_of_line_();
				}
			}

			sz = out_buf_.count();
			size_t av = serial_->space();
			if (sz > av) sz = av;
			while (sz--){
				serial_->put(out_buf_.get());
			}

			if (terminated_){
				if ((out_buf_.count() == 0) && (execs_.count()==0)){			
					connect_to_(0);
				}
			}
		}
		itf::ievents & itf::events(){
			static  ievents instance_;
			return instance_;
		}
		void itf::stop(void){
			core::instance_().stop_();
		}
		void itf::top(void){
			core::instance_().top_();
		}
		void itf::set_prompt(const char * _str){ //Формат приглашения
			core::instance_().set_prompt_(_str);
		}
		void itf::poll(void){ //Если очередная команда выполнена- заглянет во входной поток и обработает очередной символ. Вызывать из прерывания, например.
			core::instance_().poll_();
		}
		void itf::exec(const char * _cmd){ //Эмуляция пользовательского ввода
			core::instance_().exec_(_cmd);
		}
		void itf::new_line(void){ //Перевод строки в терминале
			core::instance_().start_of_line_();
		}		
		command * itf::root(){
			return &(core::instance_().root_cmd_);
		}
		void itf::connect(::robo::net::iserial * _serial){
			core::instance_().connect_to_(_serial);
		}
		bool itf::busy(void){
			return out_buf_.count() > 0;
		}
#if ROBO_APP_PROTO_SWITCH_ENABLED ==1

		abonent::abonent(
			const uint8_t * _marker
			, size_t _markerlen
			, time_us_t _lock_us
			, time_us_t _silence_us
		):  robo::net::proto::switcher::abonent(
			_marker
			, _markerlen
			, _lock_us
			, _silence_us
		){
		};
		void abonent::stop(void){
			itf::connect(0);
		}
		void abonent::start(void){
			itf::connect(this);
			itf::events().alive = [this]{ 
				alive(); 
			};
		}
#endif
	}
}
#endif
