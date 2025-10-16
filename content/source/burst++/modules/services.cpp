#include "burst++/burst.hpp"
#include "burst++/modules/services.hpp"

#if SERVICE_FREEMASTER_CONNECT_TYPE != SERVICE_FREEMASTER_CONNECT_TYPE_NONE
#include "freemaster/robosd_fm.hpp"
#endif


#if SERVICE_TERMO_CONNECT_TYPE != SERVICE_TERMO_CONNECT_TYPE_NONE
#include "terminal/robosd_termo.hpp"
#endif

#include "burst++/vartree.hpp"


#if SERVICE_NET_FLOW_TYPE == SERVICE_NET_FLOW_TYPE_DEFAULT
#include "net/robosd_flow.hpp"
#include "burst++/modules/service.proto.hpp"
#endif



namespace burst{
	namespace service {
		#if SERVICE_FREEMASTER_CONNECT_TYPE == SERVICE_FREEMASTER_CONNECT_TYPE_ABONENT
		//абонент  freemaster
		#ifndef SERVICE_FREEMASTER_ABONENT_LOCK_US
		#define SERVICE_FREEMASTER_ABONENT_LOCK_US 100000
		#endif 

		#ifndef SERVICE_FREEMASTER_ABONENT_SILENS_US
		#define SERVICE_FREEMASTER_ABONENT_SILENS_US 10000000
		#endif 
		::robo::freemaster::abonent freemaster_abonent(
			SERVICE_FREEMASTER_ABONENT_LOCK_US
			, SERVICE_FREEMASTER_ABONENT_SILENS_US
		);
		#endif
		#if SERVICE_TERMO_CONNECT_TYPE !=  SERVICE_TERMO_CONNECT_TYPE_NONE
		extern "C" int statprint_(void* /*param*/, char const* _format, ...) {
			va_list args;
			va_start(args, _format);
			::robo::termo::itf::printf(_format, args);
			::robo::termo::itf::printf("\r");
			va_end(args);
			return 0;
		}
		#endif

		#if SERVICE_TERMO_CONNECT_TYPE == SERVICE_TERMO_CONNECT_TYPE_ABONENT
		//абонент  terminal
		#ifndef SERVICE_TERMO_ABONENT_LOCK_US
		#define SERVICE_TERMO_ABONENT_LOCK_US 100000
		#endif
		#ifndef SERVICE_TERMO_ABONENT_SILENS_US
		#define SERVICE_TERMO_ABONENT_SILENS_US 10000000
		#endif

		::robo::termo::abonent termo_abonent(
			(const uint8_t*)"***"
			, 3
			, SERVICE_TERMO_ABONENT_LOCK_US
			, SERVICE_TERMO_ABONENT_SILENS_US
		);

		#endif

		#if SERVICE_FREEMASTER_CONNECT_TYPE == SERVICE_FREEMASTER_CONNECT_TYPE_ABONENT \
		|| SERVICE_TERMO_CONNECT_TYPE == SERVICE_TERMO_CONNECT_TYPE_ABONENT
		#ifdef SERVICE_SWITCH_PORT0_SERIAL_PATH  
		#define SERVICE_SWITCH_PORT0_ENABLED  1
		#else
		#define SERVICE_SWITCH_PORT0_ENABLED  0
		#endif
		#ifdef SERVICE_SWITCH_PORT1_SERIAL_PATH  
		#define SERVICE_SWITCH_PORT1_ENABLED  1
		#else
		#define SERVICE_SWITCH_PORT1_ENABLED  0
		#endif
		#else
		#define SERVICE_SWITCH_PORT0_ENABLED  0
		#endif

		#if SERVICE_SWITCH_PORT0_ENABLED ==1 || SERVICE_SWITCH_PORT1_ENABLED ==1
		#ifndef SERVICE_SWITCH_KEY_TIMEOUT_US
		#define SERVICE_SWITCH_KEY_TIMEOUT_US 1000000
		#endif

		#ifndef SERVICE_SWITCH_PORT0_TYPE
		#define SERVICE_SWITCH_PORT0_TYPE ::robo::net::proto::switcher::port::PACKET
		#endif


		#ifndef SERVICE_SWITCH_PORT1_SERIAL_TYPE
		#define SERVICE_SWITCH_PORT1_SERIAL_TYPE ::robo::net::proto::switcher::port::SERIAL
		#endif


		#if SERVICE_SWITCH_PORT0_ENABLED == 1
		::robo::net::proto::switcher::port switch_port0_(SERVICE_SWITCH_PORT0_TYPE);
		#endif
		#if SERVICE_SWITCH_PORT1_ENABLED == 1
		::robo::net::proto::switcher::port switch_port1_(SERVICE_SWITCH_PORT1_TYPE);
		#endif
		#endif


	#if SERVICE_NET_FLOW_SERIAL0_ENABLED
	#ifndef SERVICE_NET_FLOW_SERIAL0_PATH
	#define SERVICE_NET_FLOW_SERIAL0_PATH RT("s0")
	//qqq
	#endif
	#endif
		time_us_t  time_us = 0;
		board::slot::simple start(
			board::slot::kind::start
			, [] {
				#if SERVICE_SWITCH_PORT0_ENABLED == 1
				switch_port0_.connect(::robo::net::iserial::query<::robo::net::iserial>(SERVICE_SWITCH_PORT0_SERIAL_PATH), SERVICE_SWITCH_KEY_TIMEOUT_US);
				#endif
				#if SERVICE_SWITCH_PORT1_ENABLED == 1
				switch_port1_.connect(::robo::net::iserial::query<::robo::net::iserial>(SERVICE_SWITCH_PORT1_SERIAL_PATH), SERVICE_SWITCH_KEY_TIMEOUT_US);
				#endif
				#if SERVICE_FREEMASTER_CONNECT_TYPE == SERVICE_FREEMASTER_CONNECT_TYPE_ABONENT
				::robo::freemaster::connect( /*::robo::net::iserial::query(serial0_PATH)*/ &freemaster_abonent);
				#endif
				#if SERVICE_FREEMASTER_CONNECT_TYPE == SERVICE_FREEMASTER_CONNECT_TYPE_DIRRECT
				::robo::freemaster::connect( SERVICE_FREEMASTER_SERIAL );
				#endif
				#if SERVICE_TERMO_CONNECT_TYPE == SERVICE_TERMO_CONNECT_TYPE_ABONENT
				::robo::termo::itf::connect(&termo_abonent);
				::robo::termo::itf::set_prompt(">");
				#endif
				#if SERVICE_TERMO_CONNECT_TYPE == SERVICE_TERMO_CONNECT_TYPE_DIRRECT
				::robo::termo::itf::connect(SERVICE_TERMO_SERIAL);
				::robo::termo::itf::set_prompt(">");
				#endif
				#if ROBO_APP_BURST_VARTREE_ENABLED
				burst::var::reg(burst::var::const_uint32, time_us, RT("tm_us"));
				#endif

			});


		board::slot::simple frontend_pool(
			board::slot::kind::frontend
			,	[]{
				#if SERVICE_FREEMASTER_CONNECT_TYPE != SERVICE_FREEMASTER_CONNECT_TYPE_NONE
				#if SERVICE_FREEMASTER_MANUAL_POOL_ENABLED == 0
				robo::freemaster::poll();
				#endif
				#endif
				static volatile robo::time_us_t g_time_us_t = 0;
				static time_us_t tick_prev_us = 0;
				#if SERVICE_TERMO_CONNECT_TYPE != SERVICE_TERMO_CONNECT_TYPE_NONE
				robo::termo::itf::poll();
				#endif
				#if SERVICE_SWITCH_PORT0_ENABLED  == 1 || SERVICE_SWITCH_PORT1_ENABLED  == 1
				g_time_us_t = robo::system::time_us();
				robo::net::proto::switcher::core::poll(g_time_us_t - tick_prev_us);
				time_us =  tick_prev_us = g_time_us_t;
				#endif
			}
		);

		#if SERVICE_FREEMASTER_CONNECT_TYPE != SERVICE_FREEMASTER_CONNECT_TYPE_NONE
		board::slot::simple backend (
			board::slot::kind::backend
			,	&robo::freemaster::recorder	
		);	
		#endif

		#if SERVICE_TERMO_CONNECT_TYPE != SERVICE_TERMO_CONNECT_TYPE_NONE		
		class imexo_cmd : public ::robo::termo::node {
		public:
			typedef enum  {
				UNKNOWN
				#if SERVICE_NET_FLOW_COMMAND_ENABLED == 1
				, ADDR_GET
				#endif
				#if SERVICE_SETTINGS_STORE_ENABLE == 1
				, SETTINGS_SAVE
				, SETTINGS_LOAD
				, SETTINGS_RESET
				#endif
				#ifndef SERVICE_RESET_ENABLE
				#define SERVICE_RESET_ENABLE 0
				#endif
				#if SERVICE_RESET_ENABLE == 1
				, RESET
				#endif
				#if ROBO_APP_ALLOC_ENABLED ==1	
				, MEMO
				#endif
			} kind;
		private:
			kind kind_;
		protected:
			bool begin(){
				switch (kind_){
				#if SERVICE_NET_FLOW_COMMAND_ENABLED == 1
				case	ADDR_GET:
				{
									//uint8_t addr = mexo_net_flow_get_addr();
									//itf::printf("flow address : 0x%x%x ", (addr >> 4), addr & 0xF);
				}
				return false;
				#endif
												
				#if SERVICE_SETTINGS_STORE_ENABLE == 1
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
				#if SERVICE_RESET_ENABLE == 1
				case	RESET:
					break;
				#endif
				#if ROBO_APP_ALLOC_ENABLED ==1							
				case	MEMO:
					{
						const robo::system::mem::stat & ms = robo::system::get_mem_statistic();
							
						::robo::termo::itf::printf(RT8("used: %d (%d)\n\r"), ms.used.size,ms.used.count);
						::robo::termo::itf::printf(RT8("total: %d payload: %d (%d)\n\r"), ms.total.size,ms.total.payload,ms.total.count);
							
						#if SERVICE_TERMO_PRINT_TYPE == SERVICE_TERMO_PRINT_TYPE_KEIL
						__heapstats(&statprint_,nullptr);
						#endif
					}
					break;
				#endif
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
		#if SERVICE_NET_FLOW_COMMAND_ENABLED == 1  || SERVICE_SETTINGS_STORE_ENABLE == 1 || SERVICE_RESET_ENABLE == 1 ||  ROBO_APP_ALLOC_ENABLED ==1	
		
		static ::robo::termo::node root(
				RT8("burst")
				, RT8("burst commands")
				, RT8("addr <CR>")
				, robo::termo::itf::root()
			);
		#endif
		#if SERVICE_RESET_ENABLE == 1
		static imexo_cmd reset(
				imexo_cmd::kind::RESET
				, RT8("reset")
				, RT8("CPU reset")
				, RT8("<CR>")
				, &root
			);
		#endif
		#if ROBO_APP_ALLOC_ENABLED ==1	
		static imexo_cmd memo(
				imexo_cmd::kind::MEMO
				, RT8("memo")
				, RT8("memory usage")
				, RT8("<CR>")
				, &root
			);
		#endif

		#if SERVICE_NET_FLOW_COMMAND_ENABLED == 1
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
				if (SERVICE::mexo_net_flow_set_addr(addr_)){
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

		#if SERVICE_SETTINGS_STORE_ENABLE == 1

		static termo_command_t settings =
		{
			{ 0 }
			, "settings"
				, "SERVICE settings commands"
				, "<CR>"
				, &mexo_cmd_class
		};

		static termo_mexo_command_t settings_reset =
		{
			{
				{ 0 }
				, "reset"
					, "SERVICE settings reset"
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
					, "SERVICE settings load"
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
					, "SERVICE settings save"
					, "<CR>"
					, &mexo_cmd_class
			}
			, TERMO_MEXO_CMD_SETTINGS_SAVE
		};
		#endif
			

		#if ROBO_APP_BURST_VARTREE_ENABLED == 1
		namespace var{
			::robo::termo::node root(
				RT8("vt")
				, RT8("vartree commands set")
				, RT8("vt <CR>")
				, ::robo::termo::itf::root()
			);
			class showpro {
				robo::char_t* path_ = nullptr;
				robo::char_t* path_ptr_ = nullptr;
				int* path_stack_buffer_ = nullptr;
				int path_sz_ = 0;
				int* path_stack_top_ = 0;
				int path_stack_level_ = 0;
				void end_(void) {
					if (path_) {
						delete[] path_;
						path_ = nullptr;
					}
					if (path_stack_buffer_) {
						delete[] path_stack_buffer_;
						path_stack_buffer_ = nullptr;
					}
				}
			protected:
				burst::var::ref_s** pref = nullptr;
				burst::var::ref_s* ref = nullptr;
				void perform(void) {
					using namespace burst::var;
					int sz;
					ref = *pref;
					switch (ref->tag) {
					case burst::var::tags::push:
						if (path_sz_) {
							if (path_stack_level_ < stack_size) {
								sz = (int)robo::system::sprintf(path_ptr_, path_sz_, RT(".%s"), ((record_s*)(ref))->name);
								*(path_stack_top_) = sz;
								path_ptr_[sz] = 0;
								path_ptr_ += sz;
								path_sz_ -= sz;
								path_stack_top_++;
								path_stack_level_++;
							}
							else {
								end_();
								return;
							}
						}
						break;
					case tags::pop:
						path_stack_top_--;
						path_stack_level_--;
						sz = *(path_stack_top_);
						path_ptr_ -= sz;
						*(path_ptr_) = 0;
						path_sz_ += sz;
						break;
					default:
						sz = (int)robo::system::sprintf(path_ptr_, path_sz_, RT(".%s"), ((record_s*)(ref))->name);
						*(path_stack_top_) = sz;
						path_ptr_[sz] = 0;
						printf((burst::var::record_s*)ref, path_ + 1);
						
					}
				}

				virtual void printf(burst::var::record_s* _rec, robo::cstr _path) = 0;

				bool do_begin(void) {
					using namespace burst::var;
					pref = burst::var::first();
					if (pref) {
						path_ = new robo::char_t[path_size + 1];
						path_stack_buffer_ = new int[stack_size];

						path_ptr_ = path_;
						path_sz_ = path_size;
						path_stack_top_ = path_stack_buffer_;
						path_stack_level_ = 0;

						perform();
						if (pref != burst::var::last()) {
							return true;
						}
						else {
							end_();
							return false;
						}
					}
					else {
						return false;
					}
				}
				bool do_perform(void) {
					pref++;
					perform();
					if (pref != burst::var::last()) {
						return true;
					}
					else {
						end_();
						return false;
					}
				}
				
				bool do_loop(void) {
					if (::robo::termo::itf::busy()) {
						return true;
					}
					return do_perform();
				}
			};
			class show :public ::robo::termo::command, public showpro {
			protected:

				virtual bool begin(void){
					return showpro::do_begin();
				}
				virtual bool loop(void){
					return showpro::do_loop();
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
			class show_fml :public show {
			protected:
				virtual void printf(burst::var::record_s* _rec, robo::cstr _path) {
#if ROBO_UNICODE_ENABLED == 1
					robo::string nm;
					nm.format(RT("%s\t%d\t%x\n\r")
						, _path
						, (int)_rec->desc.len
						, uint32_t(((FMSTR_ADDRESS_OFFSET_TYPE)_rec->addr) - FMSTR_ADDRESS_OFFSET));
					nm.ascii([](const char* _buf) {::robo::termo::itf::printf(_buf); });
#else
					::robo::termo::itf::printf(RT8("%s\t%d\t%x\n\r")
						, _path
						, (int)_rec->desc.len
						, uint32_t(((FMSTR_ADDRESS_OFFSET_TYPE)_rec->addr) - FMSTR_ADDRESS_OFFSET)
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
				virtual void printf(burst::var::record_s* _rec, robo::cstr _path){
					#if ROBO_UNICODE_ENABLED == 1
					robo::char_t tmp[20];
					sprintf(*_rec, tmp, 20);
					robo::string nm;
					nm.format(RT("%30s\t%s\n\r")
						,_path
						,tmp
					);
					nm.ascii([](const char* _buf) {::robo::termo::itf::printf(_buf); });
					#else
					robo::char_t tmp[20];
					sprintf(*_rec,tmp, 20);
					::robo::termo::itf::printf(RT8("%30s\t%s\n\r")
											   , _path
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
				virtual void printf(burst::var::record_s* _rec, robo::cstr _path){
					#if ROBO_UNICODE_ENABLED == 1
					robo::string nm;
					nm.format(RT("%20s\t%p\t%8x\t%d\t%d\t%d\t%d\n\r")
						,_path
						,_rec->addr
						, (unsigned int)_rec->key
						, (int)_rec->desc.len
						, (int)_rec->desc.bsign
						, (int)_rec->desc.bconst
						, (int)_rec->desc.real
					);
					nm.ascii([](const char* _buf) {::robo::termo::itf::printf(_buf); });
					#else
					::robo::termo::itf::printf(RT8("%20s\t%p\t%8x\t%d\t%d\t%d\t%d\n\r")
											   , _path 
											   , _rec->addr
											   , (unsigned int)_rec->key
											   , (int)_rec->desc.len
											   , (int)_rec->desc.bsign
											   , (int)_rec->desc.bconst
											   , (int)_rec->desc.real
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

			class showto_fml_stream :public  showpro {
				void (*do_print_)(const char* _buf) = nullptr;

				virtual void printf(burst::var::record_s* _rec, robo::cstr _path) {
					robo::string nm;
					nm.format(RT("%s\t%d\t%x\n\r")
						, _path
						, (int)_rec->desc.len
						, uint32_t(((FMSTR_ADDRESS_OFFSET_TYPE)_rec->addr) - FMSTR_ADDRESS_OFFSET));
					nm.ascii([this](const char* _buf) { this->do_print_(_buf); });
				}
			public:
				void run(void (*_do_print)(const char* _buf)) {
					do_print_ = _do_print;
					if (do_begin()) while (do_loop());
					do_print_ = nullptr;
				}
				void fast_run(void (*_do_print)(const char *_buf)) {
					do_print_ = _do_print;
					if (do_begin())
						while (do_perform())
							;
					do_print_ = nullptr;
				}

				showto_fml_stream(void)
				{

				}
			}showto_fml_stream_;
		}
			
		void varlist(void (*_do_print)(const char* _buf)) {
			var::showto_fml_stream_.fast_run(_do_print);
		}
		#endif
		#endif
		}
	}
	#if SERVICE_NET_FLOW_TYPE == SERVICE_NET_FLOW_TYPE_DEFAULT
	
	#ifndef SERVICE_NET_FLOW_PORT_PATH
	#error SERVICE_NET_FLOW_PORT_PATH isn't defined'
	#endif 

	#ifndef SERVICE_NET_FLOW_ECHO_ENABLED
	#define SERVICE_NET_FLOW_ECHO_ENABLED 0
	#endif 
	#if SERVICE_NET_FLOW_ECHO_ENABLED == 1

	#ifndef SERVICE_NET_FLOW_ECHO_SUBA
	#define SERVICE_NET_FLOW_ECHO_SUBA burst::proto::flow::suba::echo
	#endif

	#ifndef SERVICE_NET_FLOW_ECHO_SUBA_ANSW
	#define SERVICE_NET_FLOW_ECHO_SUBA_ANSW burst::proto::flow::ans_suba::echo
	#endif

	#ifndef SERVICE_NET_FLOW_ECHO_PERFORMER_PATH
	#define SERVICE_NET_FLOW_ECHO_PERFORMER_PATH RT("echo")
	#endif


	class flow_echo_performer : public ::robo::net::flow::performer{ 
	public:
		flow_echo_performer(void) : performer(SERVICE_NET_FLOW_ECHO_PERFORMER_PATH, ::robo::net::flow::performer::kind_t::frontend) {}
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
		 SERVICE_NET_FLOW_PORT_PATH
		, SERVICE_NET_FLOW_ECHO_PERFORMER_PATH
		, SERVICE_NET_FLOW_ECHO_SUBA
		, SERVICE_NET_FLOW_ECHO_SUBA_ANSW
	);
	#endif

	#ifndef SERVICE_NET_FLOW_SERIAL0_ENABLED
	#define SERVICE_NET_FLOW_SERIAL0_ENABLED 0
	#endif 

	#if SERVICE_NET_FLOW_SERIAL0_ENABLED
	
	#ifndef SERVICE_NET_FLOW_SERIAL0_SUBA
	#define SERVICE_NET_FLOW_SERIAL0_SUBA burst::proto::flow::suba::serial0
	#endif

	#ifndef SERVICE_NET_FLOW_SERIAL0_SUBA_ANSW
	#define SERVICE_NET_FLOW_SERIAL0_SUBA_ANSW burst::proto::flow::ans_suba::serial0
	#endif

	#ifndef SERVICE_NET_FLOW_SERIAL0_INPUT_BITS
	#define SERVICE_NET_FLOW_SERIAL0_INPUT_BITS 10
	#endif

	#ifndef SERVICE_NET_FLOW_SERIAL0_OUTPUT_BITS
	#define SERVICE_NET_FLOW_SERIAL0_OUTPUT_BITS 4
	#endif

	#ifndef SERVICE_NET_FLOW_SERIAL0_PATH
	#define SERVICE_NET_FLOW_SERIAL0_PATH RT("s0")
	#endif

	#ifndef SERVICE_NET_FLOW_SERIAL0_PERFORMER_PATH
	#define SERVICE_NET_FLOW_SERIAL0_PERFORMER_PATH RT("s0p")
	#endif

	::robo::net::flow::serial_proto_t<SERVICE_NET_FLOW_SERIAL0_INPUT_BITS, SERVICE_NET_FLOW_SERIAL0_OUTPUT_BITS, void>  
		flow_serial0_performer_(SERVICE_NET_FLOW_SERIAL0_PERFORMER_PATH,SERVICE_NET_FLOW_SERIAL0_PATH, ::robo::net::flow::performer::kind_t::frontend);

	::robo::net::flow::rout_record flow_serial0_rout_record_(
		 SERVICE_NET_FLOW_PORT_PATH
		, SERVICE_NET_FLOW_SERIAL0_PERFORMER_PATH
		, SERVICE_NET_FLOW_SERIAL0_SUBA
		, SERVICE_NET_FLOW_SERIAL0_SUBA_ANSW
	);
	
	#endif

	#if ROBO_APP_BURST_VARTREE_ENABLED == 1
	#ifndef SERVICE_NET_FLOW_VAR_ENABLED
	#define SERVICE_NET_FLOW_VAR_ENABLED 0
	#endif 
	#if SERVICE_NET_FLOW_VAR_ENABLED == 1

	#ifndef SERVICE_NET_FLOW_VAR_SUBA
	#define SERVICE_NET_FLOW_VAR_SUBA burst::proto::flow::suba::var
	#endif

	#ifndef SERVICE_NET_FLOW_VAR_SUBA_ANSW
	#define SERVICE_NET_FLOW_VAR_SUBA_ANSW burst::proto::flow::ans_suba::var
	#endif

	#ifndef SERVICE_NET_FLOW_VAR_PERFORMER_PATH
	#define SERVICE_NET_FLOW_VAR_PERFORMER_PATH RT("var")
	#endif

	class flow_var_performer : public ::robo::net::flow::performer {
	public:
		flow_var_performer(void) : performer(SERVICE_NET_FLOW_VAR_PERFORMER_PATH, ::robo::net::flow::performer::kind_t::frontend) {}
		virtual void execute(void) {
			static uint8_t answer[8];
			if (in_msg) {
				put_answer(answer, ::burst::var::proto(in_msg->data(), answer));
			}
		}
	};
	flow_var_performer flow_var_performer_;
	::robo::net::flow::rout_record flow_var_rout_record_(
		 SERVICE_NET_FLOW_PORT_PATH
		, SERVICE_NET_FLOW_VAR_PERFORMER_PATH
		, SERVICE_NET_FLOW_VAR_SUBA
		, SERVICE_NET_FLOW_VAR_SUBA_ANSW
	);
	#endif
	#endif
	#endif



#if SERVICE_RELAY_PROTO_ENABLED==1
		//абонент  freemaster
		#ifndef SERVICE_RELAY_PROTO_ABONENT_LOCK_US
		#define SERVICE_RELAY_PROTO_ABONENT_LOCK_US 100000
		#endif 

		#ifndef SERVICE_RELAY_PROTO_ABONENT_SILENS_US
		#define SERVICE_RELAY_PROTO_ABONENT_SILENS_US 1000000
		#endif 

		uint8_t relaye_proto_abonent_marker[3] = { 0xaa,0xaa };

		robo::net::proto::switcher::abonent(
			abonent_marker,
			2,
			SERVICE_RELAY_PROTO_ABONENT_LOCK_US
			, SERVICE_RELAY_PROTO_ABONENT_SILENS_US
		) relaye_proto_abonent;
#endif

namespace burst{
	namespace service {
		namespace rs485{
			
			namespace A{
				#if SERVICE_RS485_A_TAG == SERVICE_RS485_TAG_MODBUS_RTU_SLAVE
				modbus_rtu_servo_t modbus_rtu_servo;
				void prf::modbus_rtu_slave::on_receive(const uint8_t * _data, uint16_t _length){		
					modbus_rtu_servo.on_receive(_data,_length);
				}
				void prf::modbus_rtu_slave::on_receive(uint16_t _length){		
					modbus_rtu_servo.on_receive(_length);
				}
				#endif
				#if SERVICE_RS485_A_TAG == SERVICE_RS485_TAG_MODBUS_RTU_MASTER
				#ifndef SERVICE_RS485_A_MODBUS_RTU_MASTER_US
				#define SERVICE_RS485_A_MODBUS_RTU_MASTER_US 20000
				#endif
				dispetcher_t dispetcher(SERVICE_RS485_A_MODBUS_RTU_MASTER_US);
				void prf::modbus_rtu_master::confirm(){
						dispetcher.confirm();
				}
				void prf::modbus_rtu_master::refuse(){
					dispetcher.refuse();
				}
				#endif
				
				#if SERVICE_RS485_A_TAG == SERVICE_RS485_TAG_SERIAL

				serial_t serial_;
				robo::net::iserial & serial = serial_;
			
				void prf::serial::refuse(void){
					serial_.on_refuse();
				}
				void prf::serial::confirm(void){
					serial_.on_confirm();
				}
				void prf::serial::receive(const uint8_t * data, uint16_t _length){
					serial_.on_receive(data,_length);
				}
				void prf::serial::receive(uint16_t _length){
					serial_.on_receive(rx_buffer,_length);
				}

				#endif
			}
			
			namespace B{
				#if SERVICE_RS485_B_TAG == SERVICE_RS485_TAG_MODBUS_RTU_SLAVE
				modbus_rtu_servo_t modbus_rtu_servo;
				void prf::modbus_rtu_slave::on_receive(const uint8_t * _data, uint16_t _length){		
					modbus_rtu_servo.on_receive(_data,_length);
				}
				void prf::modbus_rtu_slave::on_receive(uint16_t _length){		
					modbus_rtu_servo.on_receive(_length);
				}
				#endif
				#if SERVICE_RS485_B_TAG == SERVICE_RS485_TAG_MODBUS_RTU_MASTER
				#ifndef SERVICE_RS485_B_MODBUS_RTU_MASTER_US
				#define SERVICE_RS485_B_MODBUS_RTU_MASTER_US 20000
				#endif
				dispetcher_t dispetcher(SERVICE_RS485_B_MODBUS_RTU_MASTER_US);
				void prf::modbus_rtu_master::confirm(){
						dispetcher.confirm();
				}
				void prf::modbus_rtu_master::refuse(){
					dispetcher.refuse();
				}
				#endif
				
				#if SERVICE_RS485_B_TAG == SERVICE_RS485_TAG_SERIAL

				serial_t serial_;
				robo::net::iserial & serial = serial_;
			
				void prf::serial::refuse(void){
					serial_.on_refuse();
				}
				void prf::serial::confirm(void){
					serial_.on_confirm();
				}
				void prf::serial::receive(const uint8_t * data, uint16_t _length){
					serial_.on_receive(data,_length);
				}
				void prf::serial::receive(uint16_t _length){
					serial_.on_receive(rx_buffer,_length);
				}
				#endif
			}
			
			namespace C{
				#if SERVICE_RS485_C_TAG == SERVICE_RS485_TAG_MODBUS_RTU_SLAVE
				modbus_rtu_servo_t modbus_rtu_servo;
				void prf::modbus_rtu_slave::on_receive(const uint8_t * _data, uint16_t _length){		
					modbus_rtu_servo.on_receive(_data,_length);
				}
				void prf::modbus_rtu_slave::on_receive(uint16_t _length){		
					modbus_rtu_servo.on_receive(_length);
				}
				#endif
				#if SERVICE_RS485_C_TAG == SERVICE_RS485_TAG_MODBUS_RTU_MASTER
				#ifndef SERVICE_RS485_C_MODBUS_RTU_MASTER_US
				#define SERVICE_RS485_C_MODBUS_RTU_MASTER_US 20000
				#endif
				dispetcher_t dispetcher(SERVICE_RS485_C_MODBUS_RTU_MASTER_US);
				void prf::modbus_rtu_master::confirm(){
						dispetcher.confirm();
				}
				void prf::modbus_rtu_master::refuse(){
					dispetcher.refuse();
				}
				#endif
				
				#if SERVICE_RS485_C_TAG == SERVICE_RS485_TAG_SERIAL

				serial_t serial_;
				robo::net::iserial & serial = serial_;
				void prf::serial::refuse(void){
					serial_.on_refuse();
				}
				void prf::serial::confirm(void){
					serial_.on_confirm();
				}
				void prf::serial::receive(const uint8_t * data, uint16_t _length){
					serial_.on_receive(data,_length);
				}
				void prf::serial::receive(uint16_t _length){
					serial_.on_receive(rx_buffer,_length);
				}

				#endif			
			}
			
			
			burst::board::slot::simple begin(
				burst::board::slot::kind::begin
				, [] {
					#if SERVICE_RS485_A_TAG == SERVICE_RS485_TAG_MODBUS_RTU_MASTER
					A::dispetcher.begin();
					#endif
					#if SERVICE_RS485_B_TAG == SERVICE_RS485_TAG_MODBUS_RTU_MASTER
					B::dispetcher.begin();
					#endif
					#if SERVICE_RS485_C_TAG == SERVICE_RS485_TAG_MODBUS_RTU_MASTER
					C::dispetcher.begin();
					#endif

					#if SERVICE_RS485_A_TAG == SERVICE_RS485_TAG_SERIAL
					A::serial_.begin(SERVICE_RS485_A);
					#endif
					#if SERVICE_RS485_B_TAG == SERVICE_RS485_TAG_SERIAL
					B::serial_.begin(SERVICE_RS485_B);
					#endif
					#if SERVICE_RS485_C_TAG == SERVICE_RS485_TAG_SERIAL
					C::serial_.begin(SERVICE_RS485_C);
					#endif					
					}
				);
					
			burst::board::slot::simple start(
			burst::board::slot::kind::start
			, [] {
				
				#if SERVICE_RS485_A_TAG == SERVICE_RS485_TAG_MODBUS_RTU_SLAVE
				A::prf::modbus_rtu_slave::start_receive();
				#endif
				#if SERVICE_RS485_B_TAG == SERVICE_RS485_TAG_MODBUS_RTU_SLAVE
				B::prf::modbus_rtu_slave::start_receive();
				#endif
				#if SERVICE_RS485_C_TAG == SERVICE_RS485_TAG_MODBUS_RTU_SLAVE
				C::prf::modbus_rtu_slave::start_receive();
				#endif				
				
				
				#if SERVICE_RS485_A_TAG == SERVICE_RS485_TAG_SERIAL
				A::prf::serial::begin_receive();
				#endif
				#if SERVICE_RS485_B_TAG == SERVICE_RS485_TAG_SERIAL
				B::prf::serial::begin_receive();
				#endif
				#if SERVICE_RS485_C_TAG == SERVICE_RS485_TAG_SERIAL
				C::prf::serial::begin_receive();
				#endif		
			}
			);
			burst::board::slot::simple frontend_loop(
			burst::board::slot::kind::frontend
			, [] {
				#if SERVICE_RS485_A_TAG == SERVICE_RS485_TAG_MODBUS_RTU_MASTER
				A::dispetcher.poll();
				#endif		
				#if SERVICE_RS485_B_TAG == SERVICE_RS485_TAG_MODBUS_RTU_MASTER
				B::dispetcher.poll();
				#endif		
				#if SERVICE_RS485_C_TAG == SERVICE_RS485_TAG_MODBUS_RTU_MASTER
				C::dispetcher.poll();
				#endif		
			}
			);
		}
	}
}
