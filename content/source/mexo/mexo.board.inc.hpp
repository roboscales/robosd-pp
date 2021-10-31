#ifndef BOARD_FREEMASTER_CONNECT_TYPE
#include "mexo/mexo.board.common.hpp"
#define BOARD_FREEMASTER_CONNECT_TYPE BOARD_FREEMASTER_CONNECT_TYPE_ABONENT
#define BOARD_TERMO_CONNECT_TYPE BOARD_TERMO_CONNECT_TYPE_ABONENT
#endif

#ifndef  BOARD_PROTO_SWITCH_ENABLED
#define BOARD_PROTO_SWITCH_ENABLED 0
#endif

#ifndef BOARD_TYPE_NONE
#define BOARD_TYPE_NONE 0
#endif
 
#ifndef BOARD_FREEMASTER_CONNECT_TYPE
#define BOARD_FREEMASTER_CONNECT_TYPE BOARD_TYPE_NONE
#endif

#ifndef BOARD_TERMO_CONNECT_TYPE
#define BOARD_TERMO_CONNECT_TYPE BOARD_TYPE_NONE
#endif

#if BOARD_FREEMASTER_CONNECT_TYPE != BOARD_FREEMASTER_CONNECT_TYPE_NONE
#include "freemaster/robosd_fm.hpp"
#endif

#if BOARD_TERMO_CONNECT_TYPE != BOARD_TERMO_CONNECT_TYPE_NONE
#include "terminal/robosd_termo.hpp"
#endif

#include "mexo/mexo.hpp"

#include "mexo/vartable.hpp"

namespace mexo{
	namespace board{

#if BOARD_FREEMASTER_CONNECT_TYPE == BOARD_FREEMASTER_CONNECT_TYPE_ABONENT
//абонент  freemaster
#ifndef BOARD_FREEMASTER_ABONENT_LOCK_US
#define BOARD_FREEMASTER_ABONENT_LOCK_US 100000
#endif 

#ifndef BOARD_FREEMASTER_ABONENT_SILENS_US
#define BOARD_FREEMASTER_ABONENT_SILENS_US 10000000
#endif 
			::robo::freemaster::abonent freemaster_abonent(
				BOARD_FREEMASTER_ABONENT_LOCK_US
				, BOARD_FREEMASTER_ABONENT_SILENS_US
			);
#endif
#if BOARD_TERMO_CONNECT_TYPE !=  BOARD_TERMO_CONNECT_TYPE_NONE
			extern "C" int statprint_(void * /*param*/,char const * _format, ...){		
				va_list args;
				va_start(args, _format);
				::robo::termo::itf::printf(_format,args);
				::robo::termo::itf::printf("\r");
				va_end(args);
				return 0;
			}
#endif

#if BOARD_TERMO_CONNECT_TYPE == BOARD_TERMO_CONNECT_TYPE_ABONENT
//абонент  terminal
#ifndef BOARD_TERMO_ABONENT_LOCK_US
#define BOARD_TERMO_ABONENT_LOCK_US 100000
#endif
#ifndef BOARD_TERMO_ABONENT_SILENS_US
#define BOARD_TERMO_ABONENT_SILENS_US 10000000
#endif

			::robo::termo::abonent termo_abonent(
					(const uint8_t *)"***"
					, 3
					, BOARD_TERMO_ABONENT_LOCK_US
					, BOARD_TERMO_ABONENT_SILENS_US
			);

#endif
		
#if BOARD_FREEMASTER_CONNECT_TYPE == BOARD_FREEMASTER_CONNECT_TYPE_ABONENT \
		|| BOARD_TERMO_CONNECT_TYPE == BOARD_TERMO_CONNECT_TYPE_ABONENT
#define BOARD_SWITCH_PORT_ENABLED  1
#else
#define BOARD_SWITCH_PORT_ENABLED  0
#endif

#if BOARD_SWITCH_PORT_ENABLED ==1
#ifndef BOARD_SWITCH_KEY_TIMEOUT_US
#define BOARD_SWITCH_KEY_TIMEOUT_US 1000000
#endif
#ifndef BOARD_SWITCH_PORT_TYPE
#define BOARD_SWITCH_PORT_TYPE ::robo::net::proto::switcher::port::PACKET
#endif
#ifndef BOARD_SWITCH_PORT_PATH
#define BOARD_SWITCH_PORT_PATH RT("")
#endif
			::robo::net::proto::switcher::port switch_port_(BOARD_SWITCH_PORT_TYPE);
#endif

			::mexo::machine::slot::simple start(
			::mexo::machine::slot::kind::start
			, [] {
					#if BOARD_SWITCH_PORT_ENABLED == 1
					switch_port_.connect(::robo::net::iserial::query(BOARD_SWITCH_PORT_PATH),BOARD_SWITCH_KEY_TIMEOUT_US);
					#endif
					#if BOARD_FREEMASTER_CONNECT_TYPE == BOARD_FREEMASTER_CONNECT_TYPE_ABONENT
					::robo::freemaster::connect( /*::robo::net::iserial::query(serial0_PATH)*/ &freemaster_abonent );
					#endif
					#if BOARD_TERMO_CONNECT_TYPE == BOARD_TERMO_CONNECT_TYPE_ABONENT
					::robo::termo::itf::connect(&termo_abonent);
					::robo::termo::itf::set_prompt(">");
					#endif
			});

			::mexo::machine::slot::simple frontend_pool_ (
				::mexo::machine::slot::kind::frontend
				,	[]{
					static volatile robo::time_us_t g_time_us_t = 0;
					static time_us_t tick_prev_us = 0;
					#if BOARD_FREEMASTER_CONNECT_TYPE != BOARD_FREEMASTER_CONNECT_TYPE_NONE
					robo::freemaster::poll();
					#endif
					#if BOARD_TERMO_CONNECT_TYPE != BOARD_TERMO_CONNECT_TYPE_NONE
					robo::termo::itf::poll();
					#endif
					g_time_us_t = robo::system::env::realtime_us();
					#if BOARD_SWITCH_PORT_ENABLED  == 1
					robo::net::proto::switcher::core::poll(g_time_us_t - tick_prev_us);
					#endif
					tick_prev_us = g_time_us_t;
				}
			);

			#if BOARD_FREEMASTER_CONNECT_TYPE != BOARD_FREEMASTER_CONNECT_TYPE_NONE
			::mexo::machine::slot::simple backend_(
				::mexo::machine::slot::kind::backend
				,	&robo::freemaster::recorder	
			);	
			#endif
			#if BOARD_TERMO_CONNECT_TYPE != BOARD_TERMO_CONNECT_TYPE_NONE
			class imexo_cmd : public ::robo::termo::node {
			public:
				typedef enum  {
					UNKNOWN
	#if BOARD_MEXO_NET_FLOW_ENABLED
					, ADDR_GET
	#endif
	#if ROBO_APP_MEXO_SETTINGS_STORE_ENABLE == 1
					, SETTINGS_SAVE
					, SETTINGS_LOAD
					, SETTINGS_RESET
	#endif
					, RESET
					, MEMO
				} kind;
			private:
				kind kind_;
			protected:
				bool begin(){
					switch (kind_){
	#if BOARD_MEXO_NET_FLOW_ENABLED
					case	ADDR_GET:
					{
										uint8_t addr = mexo_net_flow_get_addr();
										itf::printf("flow address : 0x%x%x ", (addr >> 4), addr & 0xF);
					}
					return false;
	#endif
												
	#if MEXO_SETTINGS_STORE_ENABLE == 1
					case	TERMO_MEXO_CMD_SETTINGS_SAVE:
						if (mexo_settings_save() == ROBO_SUCCESS){
							termo_abonent_printf("success");
						}
						else{
							termo_abonent_printf("error");
						}
						return ROBO_SUCCESS;
					case	TERMO_MEXO_CMD_SETTINGS_LOAD:
						if (mexo_settings_load() == ROBO_SUCCESS){
							termo_abonent_printf("success");
						}
						else{
							termo_abonent_printf("error");
						}
						return ROBO_SUCCESS;
					case	TERMO_MEXO_CMD_SETTINGS_RESET:
						if (mexo_settings_reset() == ROBO_SUCCESS){
							termo_abonent_printf("success");
						}
						else{
							termo_abonent_printf("error");
						}
	#endif
					case	RESET:
						break;
					case	MEMO:
						{
	#if ROBO_APP_ALLOC_ENABLED ==1							
							const ::system::mem::stat & ms = ::system::get_mem_statistic();
							::robo::termo::itf::printf(RT("used: %d (%d)\n\r"), ms.used.size,ms.used.count);
							::robo::termo::itf::printf(RT("total: %d payload: %d (%d)\n\r"), ms.total.size,ms.total.payload,ms.total.count);							
	#endif
							__heapstats(&statprint_,nullptr);
						}
						break;
					case	UNKNOWN:
						::robo::termo::itf::prints("unknown command\n\r");
						break;
					
					}
					return false;
				}
			public:
				imexo_cmd(kind _kind, const char * _name, const char * _note, const char * _usage, command * _parent) \
					: node(_name, _note, _usage, _parent), kind_(_kind){
				}
			};
      static ::robo::termo::node root(
				"board"
				, "board commands"
				, "addr <CR>"
				, robo::termo::itf::root()
			);
      static imexo_cmd reset(
				imexo_cmd::kind::RESET
				, "reset"
				, "board CPU reset"
				, "<CR>"
				, &root
			);
      static imexo_cmd memo(
				imexo_cmd::kind::MEMO
				, "memo"
				, "board memory usage"
				, "<CR>"
				, &root
			);

			#if BOARD_MEXO_NET_FLOW_ENABLED == 1
      static node flow(
				"flow"
				, "flow settings commands"//const char * note; 
				, "flow <CR>" //const char * usage;                 
				, &root
			);
      static node flow_addr(
				"addr"
				, "flow address commands"//const char * note; 
				, "addr <CR>" //const char * usage;                 
				, &flow
			);

			class iflow_addr_set : public node {
				uint8_t addr_;
			protected:
				virtual bool setup(void){
					if (board::mexo_net_flow_set_addr(addr_)){
						itf::printf("success");
					} else {
						itf::printf("error");
					}
					return false;
				}
				virtual bool parse_long_arg(const char * _arg, const char * _val){
					if (ROBO_STD_STR_EQ(_arg, "address") == ROBO_TRUE){
						addr_ = atoi(_val);
						return true;
					}
					else{
						return false;
					}
				}
				virtual bool parse_arg(char _arg, const char * _val){
					if (_arg == 'a'){
						addr_ = atoi(_val);
						return true;
					}
					else{
						return false;
					}
				}
			public:
				iflow_addr_set() :node(
					"set"
					, " set flow address command "//const char * note; 
					, "set [-a | --address] address <CR>" //const char * usage;  
					, &flow_addr
					){}
			} flow_addr_set;

			imexo_cmd flow_addr_get(
				imexo_cmd::kind::ADDR_GET
				, "get"
				, "get current flow address command "//const char * note; 
				, "<CR>" //const char * usage;                 
				, &flow_addr
			);
			#endif

			#if MEXO_SETTINGS_STORE_ENABLE == 1

				static termo_command_t settings =
				{
					{ 0 }
					, "settings"
						, "board settings commands"
						, "<CR>"
						, &mexo_cmd_class
				};

				static termo_mexo_command_t settings_reset =
				{
					{
						{ 0 }
						, "reset"
							, "board settings reset"
							, "<CR>"
							, &mexo_cmd_class
					}
					, TERMO_MEXO_CMD_SETTINGS_RESET
				};

				static termo_mexo_command_t settings_load =
				{
					{
						{ 1 }
						, "load"
							, "board settings load"
							, "<CR>"
							, &mexo_cmd_class
					}
					, TERMO_MEXO_CMD_SETTINGS_LOAD
				};

				static termo_mexo_command_t settings_save =
				{
					{
						{ 2 }
						, "save"
							, "board settings save"
							, "<CR>"
							, &mexo_cmd_class
					}
					, TERMO_MEXO_CMD_SETTINGS_SAVE
				};
	#endif
	
			
			
			
			
			#ifndef ROBO_TERMO_VT_SHOW_PATH_BUFFER_SIZE 
			#define ROBO_TERMO_VT_SHOW_PATH_BUFFER_SIZE 50
			#endif

			namespace vartable{
				::robo::termo::node root(
					RT("vt")
					, RT("vartable commands set")
					, RT("vt <CR>")
					, ::robo::termo::itf::root()
				);

				class show :public ::robo::termo::command{
					::mexo::node * current_node_ = nullptr;
					char_t * path_ptr_ = nullptr;
					size_t path_sz_ = 0;					
				protected:
					static inline char_t path[ROBO_TERMO_VT_SHOW_PATH_BUFFER_SIZE];
					::mexo::var::record::ref *  current_var = nullptr;
					virtual void printf(void) = 0;
						/*{						
						::robo::termo::itf::printf(RT("%s%s\t%p\t%d\t%d-%d\n\r")
							,path_,current_var_->name
						,current_var_->addr
						, (int)current_var_->desc.len
						, current_var_->key
						);
					}*/
					virtual bool begin(void){
						path_ptr_ = path;
						path_sz_ = ROBO_TERMO_VT_SHOW_PATH_BUFFER_SIZE;
						current_node_ = ::mexo::node::root().first_on_path(path_ptr_,path_sz_);
						if(current_node_){
							current_var = current_node_->vars.first();
							::robo::termo::itf::printf(RT("vartable\n\r"));
							if(current_var){
								printf();
								current_var=current_var->next();
							}
							return true;
						}	else {
							return false;
						}							
					}
					virtual bool loop(void){
						if(::robo::termo::itf::busy()) {
							return true;
						}
						if(current_node_){
							if(current_var){
									printf();
									current_var=current_var->next();
									return true;
							}else {
								current_node_ = current_node_->next_on_path(path_ptr_,path_sz_);
								if(current_node_){
									current_var = current_node_->vars.first();
									return true;
								} else {
									return false;
								}
							}
						}			
												
						return false;
					}
					bool parse_long_arg(const char * _arg, const char * _val){
						::robo::termo::itf::printf(RT("argument ""%s"" is not support"), _arg);
						return false;
					}
					bool parse_arg(char _arg, const char * _val){
						::robo::termo::itf::printf(RT("argument ""%c"" is not support"), _arg);
						return false;
					}
					bool parse_opt(char _opt){
						::robo::termo::itf::printf(RT("option ""%c"" is not support"), _opt);
						return false;

					}
					bool parse_long_opt(const char * _opt){
						::robo::termo::itf::printf(RT("option ""%s"" is not support"), _opt);
						return false;
					}
				protected:
					show(
						const char * _name
						, const char * _note
						, const char * _usage
					) : command(_name,_note,_usage,&root){
					}					
				};
				class show_fml:public show{
				protected:
					virtual void printf(void){						
						::robo::termo::itf::printf(RT("%s%s\t%p\t%d\n\r")
							,path,current_var->name
							,current_var->addr
							, (int)current_var->desc.len
						);
					}
				public:
					show_fml(void) :show(
						RT("fml")
						, RT("vartable freemaster list show")
						, RT("fml  <CR>")
						)
					{

					}
				}show_fm_;
				class show_val:public show{
				protected:
					virtual void printf(void){				
						char_t tmp[20];
						current_var->sprintf(tmp,20);
						::robo::termo::itf::printf(RT("%30s%-10s%s\n\r")
							,path,current_var->name
							,tmp
						);
					}
				public:
					show_val(void) :show(
						RT("vl")
						, RT("vartable values show")
						, RT("vl  <CR>")
						)
					{

					}
				}show_val_;
				
				class show_records:public show{
				protected:
					virtual void printf(void){						
						::robo::termo::itf::printf(RT("%20s%10s\t%p\t%8x\t%d\t%d\t%d\t%d\n\r")
							,path,current_var->name
							,current_var->addr
							, (unsigned int)current_var->key
							, (int)current_var->desc.len
							, (int)current_var->desc.bsign
							, (int)current_var->desc.bconst
							, (int)current_var->desc.real
						);
					}
				public:
					show_records(void) :show(
						RT("rl")
						, RT("vartable record list show")
						, RT("rl  <CR>")
						)
					{

					}
				}show_records_;
			}
#endif	

	}	
}