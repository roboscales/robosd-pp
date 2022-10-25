#include "mexo/mexo.env.common.hpp"

#include "mexo/mexo.hpp"
#include "mexo/math.hpp"

#if ENV_FREEMASTER_CONNECT_TYPE != ENV_FREEMASTER_CONNECT_TYPE_NONE
#include "freemaster/robosd_fm.hpp"
#endif


#if ENV_TERMO_CONNECT_TYPE != ENV_TERMO_CONNECT_TYPE_NONE
#include "terminal/robosd_termo.hpp"
#endif


#include "mexo/vartree.hpp"


#if ENV_NET_FLOW_TYPE == ENV_NET_FLOW_TYPE_DEFAULT
#include "net/robosd_flow.hpp"
#endif


namespace mexo{
	namespace env {
		#if ENV_FREEMASTER_CONNECT_TYPE == ENV_FREEMASTER_CONNECT_TYPE_ABONENT
		//абонент  freemaster
		#ifndef ENV_FREEMASTER_ABONENT_LOCK_US
		#define ENV_FREEMASTER_ABONENT_LOCK_US 100000
		#endif 

		#ifndef ENV_FREEMASTER_ABONENT_SILENS_US
		#define ENV_FREEMASTER_ABONENT_SILENS_US 10000000
		#endif 
		::robo::freemaster::abonent freemaster_abonent(
			ENV_FREEMASTER_ABONENT_LOCK_US
			, ENV_FREEMASTER_ABONENT_SILENS_US
		);
		#endif
		#if ENV_TERMO_CONNECT_TYPE !=  ENV_TERMO_CONNECT_TYPE_NONE
		extern "C" int statprint_(void* /*param*/, char const* _format, ...) {
			va_list args;
			va_start(args, _format);
			::robo::termo::itf::printf(_format, args);
			::robo::termo::itf::printf("\r");
			va_end(args);
			return 0;
		}
		#endif

		#if ENV_TERMO_CONNECT_TYPE == ENV_TERMO_CONNECT_TYPE_ABONENT
		//абонент  terminal
		#ifndef ENV_TERMO_ABONENT_LOCK_US
		#define ENV_TERMO_ABONENT_LOCK_US 100000
		#endif
		#ifndef ENV_TERMO_ABONENT_SILENS_US
		#define ENV_TERMO_ABONENT_SILENS_US 10000000
		#endif

		::robo::termo::abonent termo_abonent(
			(const uint8_t*)"***"
			, 3
			, ENV_TERMO_ABONENT_LOCK_US
			, ENV_TERMO_ABONENT_SILENS_US
		);

		#endif

		#if ENV_FREEMASTER_CONNECT_TYPE == ENV_FREEMASTER_CONNECT_TYPE_ABONENT \
		|| ENV_TERMO_CONNECT_TYPE == ENV_TERMO_CONNECT_TYPE_ABONENT
		#ifdef ENV_SWITCH_PORT0_SERIAL_PATH  
		#define ENV_SWITCH_PORT0_ENABLED  1
		#else
		#define ENV_SWITCH_PORT0_ENABLED  0
		#endif
		#ifdef ENV_SWITCH_PORT1_SERIAL_PATH  
		#define ENV_SWITCH_PORT1_ENABLED  1
		#else
		#define ENV_SWITCH_PORT1_ENABLED  0
		#endif
		#else
		#define ENV_SWITCH_PORT0_ENABLED  0
		#endif

		#if ENV_SWITCH_PORT0_ENABLED ==1 || ENV_SWITCH_PORT1_ENABLED ==1
		#ifndef ENV_SWITCH_KEY_TIMEOUT_US
		#define ENV_SWITCH_KEY_TIMEOUT_US 1000000
		#endif

		#ifndef ENV_SWITCH_PORT0_TYPE
		#define ENV_SWITCH_PORT0_TYPE ::robo::net::proto::switcher::port::PACKET
		#endif


		#ifndef ENV_SWITCH_PORT1_SERIAL_TYPE
		#define ENV_SWITCH_PORT1_SERIAL_TYPE ::robo::net::proto::switcher::port::SERIAL
		#endif


		#if ENV_SWITCH_PORT0_ENABLED == 1
		::robo::net::proto::switcher::port switch_port0_(ENV_SWITCH_PORT0_TYPE);
		#endif
		#if ENV_SWITCH_PORT1_ENABLED == 1
		::robo::net::proto::switcher::port switch_port1_(ENV_SWITCH_PORT1_TYPE);
		#endif
		#endif

		::robo::time_us_t  time_us = 0;
		::mexo::machine::slot::simple start(
			::mexo::machine::slot::kind::start
			, [] {
				#if ENV_SWITCH_PORT0_ENABLED == 1
				switch_port0_.connect(::robo::net::iserial::query<::robo::net::iserial>(ENV_SWITCH_PORT0_SERIAL_PATH), ENV_SWITCH_KEY_TIMEOUT_US);
				#endif
				#if ENV_SWITCH_PORT1_ENABLED == 1
				switch_port1_.connect(::robo::net::iserial::query<::robo::net::iserial>(ENV_SWITCH_PORT1_SERIAL_PATH), ENV_SWITCH_KEY_TIMEOUT_US);
				#endif
				#if ENV_FREEMASTER_CONNECT_TYPE == ENV_FREEMASTER_CONNECT_TYPE_ABONENT
				::robo::freemaster::connect( /*::robo::net::iserial::query(serial0_PATH)*/ &freemaster_abonent);
				#endif
				#if ENV_FREEMASTER_CONNECT_TYPE == ENV_FREEMASTER_CONNECT_TYPE_DIRRECT
				::robo::freemaster::connect( ENV_FREEMASTER_SERIAL );
				#endif
				#if ENV_TERMO_CONNECT_TYPE == ENV_TERMO_CONNECT_TYPE_ABONENT
				::robo::termo::itf::connect(&termo_abonent);
				::robo::termo::itf::set_prompt(">");
				#endif
				#if ROBO_APP_MEXO_VAR_ENABLED
				var::record::create(::mexo::var::types::const_uint32, time_us, RT("tm_us"));
				#endif

			});


		::mexo::machine::slot::simple frontend_pool_ (
			::mexo::machine::slot::kind::frontend
			,	[]{
				static volatile robo::time_us_t g_time_us_t = 0;
				static time_us_t tick_prev_us = 0;
				#if ENV_FREEMASTER_CONNECT_TYPE != ENV_FREEMASTER_CONNECT_TYPE_NONE
				robo::freemaster::poll();
				#endif
				#if ENV_TERMO_CONNECT_TYPE != ENV_TERMO_CONNECT_TYPE_NONE
				robo::termo::itf::poll();
				#endif
				#if ENV_SWITCH_PORT0_ENABLED  == 1 || ENV_SWITCH_PORT1_ENABLED  == 1
				g_time_us_t = robo::system::env::realtime_us();
				robo::net::proto::switcher::core::poll(g_time_us_t - tick_prev_us);
				time_us =  tick_prev_us = g_time_us_t;
				#endif

			}
		);

		#if ENV_FREEMASTER_CONNECT_TYPE != ENV_FREEMASTER_CONNECT_TYPE_NONE
		::mexo::machine::slot::simple backend_(
			::mexo::machine::slot::kind::backend
			,	&robo::freemaster::recorder	
		);	
		#endif

		#if ENV_TERMO_CONNECT_TYPE != ENV_TERMO_CONNECT_TYPE_NONE		
		class imexo_cmd : public ::robo::termo::node {
		public:
			typedef enum  {
				UNKNOWN
				#if ENV_NET_FLOW_COMMAND_ENABLED == 1
				, ADDR_GET
				#endif
				#if ENV_SETTINGS_STORE_ENABLE == 1
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
				#if ENV_NET_FLOW_COMMAND_ENABLED == 1
				case	ADDR_GET:
				{
									//uint8_t addr = mexo_net_flow_get_addr();
									//itf::printf("flow address : 0x%x%x ", (addr >> 4), addr & 0xF);
				}
				return false;
				#endif
												
				#if ENV_SETTINGS_STORE_ENABLE == 1
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
							
						::robo::termo::itf::printf(RT8("used: %d (%d)\n\r"), ms.used.size,ms.used.count);
						::robo::termo::itf::printf(RT8("total: %d payload: %d (%d)\n\r"), ms.total.size,ms.total.payload,ms.total.count);
							
						#if ENV_TERMO_PRINT_TYPE == ENV_TERMO_PRINT_TYPE_KEIL
						__heapstats(&statprint_,nullptr);
						#endif
						#endif
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
				RT8("ENV")
				, RT8("ENV commands")
				, RT8("addr <CR>")
				, robo::termo::itf::root()
			);
		static imexo_cmd reset(
				imexo_cmd::kind::RESET
				, RT8("reset")
				, RT8("ENV CPU reset")
				, RT8("<CR>")
				, &root
			);
		static imexo_cmd memo(
				imexo_cmd::kind::MEMO
				, RT8("memo")
				, RT8("ENV memory usage")
				, RT8("<CR>")
				, &root
			);

		#if ENV_NET_FLOW_COMMAND_ENABLED == 1
		static ::robo::termo::node flow(
				"flow"
				, "flow settings commands"//const char * note; 
				, "flow <CR>" //const char * usage;                 
				, &root
			);
		static ::robo::termo::node flow_addr(
				"addr"
				, "flow address commands"//const char * note; 
				, "addr <CR>" //const char * usage;                 
				, &flow
			);

		class iflow_addr_set : public node {
			uint8_t addr_;
		protected:
			virtual bool setup(void){
				if (ENV::mexo_net_flow_set_addr(addr_)){
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
				, "ENV settings commands"
				, "<CR>"
				, &mexo_cmd_class
		};

		static termo_mexo_command_t settings_reset =
		{
			{
				{ 0 }
				, "reset"
					, "ENV settings reset"
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
					, "ENV settings load"
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
					, "ENV settings save"
					, "<CR>"
					, &mexo_cmd_class
			}
			, TERMO_MEXO_CMD_SETTINGS_SAVE
		};
		#endif
			
		#ifndef ROBO_TERMO_VT_SHOW_PATH_BUFFER_SIZE 
		#define ROBO_TERMO_VT_SHOW_PATH_BUFFER_SIZE 50
		#endif
		#if ROBO_APP_MEXO_VAR_ENABLED == 1
		namespace vartree{
			::robo::termo::node root(
				RT8("vt")
				, RT8("vartree commands set")
				, RT8("vt <CR>")
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
					current_node_ = ::mexo::node::root().first_on_path(path_ptr_, path_sz_);
					if(current_node_){
						current_var = current_node_->vars.first();
						::robo::termo::itf::printf(RT8("vartree\n\r"));
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
							current_node_ = current_node_->next_on_path(path_ptr_, path_sz_);
							if (current_node_) {
								current_var = current_node_->vars.first();
								return true;
							}
							else {
								return false;
							}
						}
					}			
												
					return false;
				}
				bool parse_long_arg(const char * _arg, const char * _val){
					::robo::termo::itf::printf(RT8("argument ""%s"" is not support"), _arg);
					return false;
				}
				bool parse_arg(char _arg, const char * _val){
					::robo::termo::itf::printf(RT8("argument ""%c"" is not support"), _arg);
					return false;
				}
				bool parse_opt(char _opt){
					::robo::termo::itf::printf(RT8("option ""%c"" is not support"), _opt);
					return false;

				}
				bool parse_long_opt(const char * _opt){
					::robo::termo::itf::printf(RT8("option ""%s"" is not support"), _opt);
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
					#if ROBO_UNICODE_ENABLED == 1
					robo::string nm;
					nm.format(RT("%s%s\t%d\t%x\n\r")
							  , path, current_var->name
							  , (int)current_var->desc.len
							  , uint32_t(((FMSTR_ADDRESS_OFFSET_TYPE)current_var->addr) - FMSTR_ADDRESS_OFFSET));
					nm.ascii([](const char* _buf) {::robo::termo::itf::printf(_buf); });
					#else
					::robo::termo::itf::printf(RT8("%s%s\t%d\t%x\n\r")
											   , path, current_var->name
											   , (int)current_var->desc.len
											   , uint32_t(((FMSTR_ADDRESS_OFFSET_TYPE)current_var->addr) - FMSTR_ADDRESS_OFFSET)
					);
					#endif
				}
			public:
				show_fml(void) :show(
					RT8("fml")
					, RT8("vartree freemaster list show")
					, RT8("fml  <CR>")
					)
				{

				}
			}show_fm_;
			class show_val:public show{
			protected:
				virtual void printf(void){				
					#if ROBO_UNICODE_ENABLED == 1
					char_t tmp[20];
					current_var->sprintf(tmp,20);
					robo::string nm;
					nm.format(RT("%30s%-10s%s\n\r")
						,path,current_var->name
						,tmp
					);
					nm.ascii([](const char* _buf) {::robo::termo::itf::printf(_buf); });
					#else
					char_t tmp[20];
					current_var->sprintf(tmp, 20);
					::robo::termo::itf::printf(RT8("%30s%-10s%s\n\r")
											   , path, current_var->name
											   , tmp
					);
					#endif
				}
			public:
				show_val(void) :show(
					RT8("vl")
					, RT8("vartree values show")
					, RT8("vl  <CR>")
					)
				{

				}
			}show_val_;
				
			class show_records:public show{
			protected:
				virtual void printf(void){						
					#if ROBO_UNICODE_ENABLED == 1
					robo::string nm;
					nm.format(RT("%20s%10s\t%p\t%8x\t%d\t%d\t%d\t%d\n\r")
						,path,current_var->name
						,current_var->addr
						, (unsigned int)current_var->key
						, (int)current_var->desc.len
						, (int)current_var->desc.bsign
						, (int)current_var->desc.bconst
						, (int)current_var->desc.real
					);
					nm.ascii([](const char* _buf) {::robo::termo::itf::printf(_buf); });
					#else
					::robo::termo::itf::printf(RT8("%20s%10s\t%p\t%8x\t%d\t%d\t%d\t%d\n\r")
											   , path, current_var->name
											   , current_var->addr
											   , (unsigned int)current_var->key
											   , (int)current_var->desc.len
											   , (int)current_var->desc.bsign
											   , (int)current_var->desc.bconst
											   , (int)current_var->desc.real
					);
					#endif
				}
			public:
				show_records(void) :show(
					RT8("rl")
					, RT8("vartree record list show")
					, RT8("rl  <CR>")
					)
				{

				}
			}show_records_;
		}
		#endif
		#endif	
		}
	}
	#if ENV_NET_FLOW_TYPE == ENV_NET_FLOW_TYPE_DEFAULT
	
	#ifndef ENV_NET_FLOW_PORT_PATH
	#error ENV_NET_FLOW_PORT_PATH isn't defined'
	#endif 

	#ifndef ENV_NET_FLOW_ECHO_ENABLED
	#define ENV_NET_FLOW_ECHO_ENABLED 0
	#endif 
	#if ENV_NET_FLOW_ECHO_ENABLED == 1

	#ifndef ENV_NET_FLOW_ECHO_SUBA
	#define ENV_NET_FLOW_ECHO_SUBA 0x01
	#endif

	#ifndef ENV_NET_FLOW_ECHO_SUBA_ANSW
	#define ENV_NET_FLOW_ECHO_SUBA_ANSW 0x01
	#endif

	#ifndef ENV_NET_FLOW_ECHO_PERFORMER_PATH
	#define ENV_NET_FLOW_ECHO_PERFORMER_PATH RT("echo")
	#endif


	class flow_echo_performer : public ::robo::net::flow::performer{ 
	public:
		flow_echo_performer(void) : performer(ENV_NET_FLOW_ECHO_PERFORMER_PATH, ::robo::net::flow::performer::kind_t::frontend) {}
		virtual void execute(void) {
			static uint8_t old_data[8];
			static size_t old_sz;
			if (in_msg) {
				put_answer(in_msg->data(), in_msg->size());
				old_sz = in_msg->size();
				std::copy_n(in_msg->data(), old_sz, old_data);
			}
			else {
				put_answer(old_data, old_sz);
			}
		} 
	}; 
	
	flow_echo_performer flow_echo_performer_;
	::robo::net::flow::rout_record flow_echo_rout_record_(
		 ENV_NET_FLOW_PORT_PATH
		, ENV_NET_FLOW_ECHO_PERFORMER_PATH
		, ENV_NET_FLOW_ECHO_SUBA
		, ENV_NET_FLOW_ECHO_SUBA_ANSW
	);
	#endif

	#ifndef ENV_NET_FLOW_SERIAL0_ENABLED
	#define ENV_NET_FLOW_SERIAL0_ENABLED 0
	#endif 

	#if ENV_NET_FLOW_SERIAL0_ENABLED
	
	#ifndef ENV_NET_FLOW_SERIAL0_SUBA
	#define ENV_NET_FLOW_SERIAL0_SUBA 0x0F
	#endif

	#ifndef ENV_NET_FLOW_SERIAL0_SUBA_ANSW
	#define ENV_NET_FLOW_SERIAL0_SUBA_ANSW 0x0F
	#endif

	#ifndef ENV_NET_FLOW_SERIAL0_INPUT_BITS
	#define ENV_NET_FLOW_SERIAL0_INPUT_BITS 10
	#endif

	#ifndef ENV_NET_FLOW_SERIAL0_OUTPUT_BITS
	#define ENV_NET_FLOW_SERIAL0_OUTPUT_BITS 4
	#endif

	#ifndef ENV_NET_FLOW_SERIAL0_PATH
	#define ENV_NET_FLOW_SERIAL0_PATH RT("s0")
	#endif

	#ifndef ENV_NET_FLOW_SERIAL0_PERFORMER_PATH
	#define ENV_NET_FLOW_SERIAL0_PERFORMER_PATH RT("s0p")
	#endif

	::robo::net::flow::serial_proto_t<ENV_NET_FLOW_SERIAL0_INPUT_BITS, ENV_NET_FLOW_SERIAL0_OUTPUT_BITS, void>  
		flow_serial0_performer_(ENV_NET_FLOW_SERIAL0_PERFORMER_PATH,ENV_NET_FLOW_SERIAL0_PATH, ::robo::net::flow::performer::kind_t::frontend);

	::robo::net::flow::rout_record flow_serial0_rout_record_(
		 ENV_NET_FLOW_PORT_PATH
		, ENV_NET_FLOW_SERIAL0_PERFORMER_PATH
		, ENV_NET_FLOW_SERIAL0_SUBA
		, ENV_NET_FLOW_SERIAL0_SUBA_ANSW
	);
	
	#endif

	#if ROBO_APP_MEXO_VAR_ENABLED == 1
	#ifndef ENV_NET_FLOW_VAR_ENABLED
	#define ENV_NET_FLOW_VAR_ENABLED 0
	#endif 
	#if ENV_NET_FLOW_VAR_ENABLED == 1

	#ifndef ENV_NET_FLOW_VAR_SUBA
	#define ENV_NET_FLOW_VAR_SUBA 0x0B
	#endif

	#ifndef ENV_NET_FLOW_VAR_SUBA_ANSW
	#define ENV_NET_FLOW_VAR_SUBA_ANSW 0x0B
	#endif

	#ifndef ENV_NET_FLOW_VAR_PERFORMER_PATH
	#define ENV_NET_FLOW_VAR_PERFORMER_PATH RT("var")
	#endif

	class flow_var_performer : public ::robo::net::flow::performer {
	public:
		flow_var_performer(void) : performer(ENV_NET_FLOW_VAR_PERFORMER_PATH, ::robo::net::flow::performer::kind_t::frontend) {}
		virtual void execute(void) {
			static uint8_t answer[8];
			if (in_msg) {
				put_answer(answer, ::mexo::var::machine::proto(in_msg->data(), answer));
			}
		}
	};
	flow_var_performer flow_var_performer_;
	::robo::net::flow::rout_record flow_var_rout_record_(
		 ENV_NET_FLOW_PORT_PATH
		, ENV_NET_FLOW_VAR_PERFORMER_PATH
		, ENV_NET_FLOW_VAR_SUBA
		, ENV_NET_FLOW_VAR_SUBA_ANSW
	);
	#endif
	#endif
	#endif



#if ENV_RELAY_PROTO_ENABLED==1
		//абонент  freemaster
		#ifndef ENV_RELAY_PROTO_ABONENT_LOCK_US
		#define ENV_RELAY_PROTO_ABONENT_LOCK_US 100000
		#endif 

		#ifndef ENV_RELAY_PROTO_ABONENT_SILENS_US
		#define ENV_RELAY_PROTO_ABONENT_SILENS_US 1000000
		#endif 

		uint8_t relaye_proto_abonent_marker[3] = { 0xaa,0xaa };

		robo::net::proto::switcher::abonent(
			abonent_marker
			2,
			ENV_RELAY_PROTO_ABONENT_LOCK_US
			, ENV_RELAY_PROTO_ABONENT_SILENS_US
		) relaye_proto_abonent;
#endif
