#ifndef w5500_hpp
#define w5500_hpp
#include <stdint.h>

#define __GP_REGISTER8(name, address)             \
  static inline void write##name(uint8_t _data) { \
    instance.write(address, 0x04, _data);                  \
  }                                               \
  static inline uint8_t read##name() {            \
    return instance.read(address, 0x00);                   \
  }

#define __GP_REGISTER16(name, address)            \
  static void write##name(uint16_t _data) {       \
    instance.write(address,  0x04, _data >> 8);            \
   instance. write(address+1, 0x04, _data & 0xFF);         \
  }                                               \
  static uint16_t read##name() {                  \
    uint16_t res = instance.read(address, 0x00);           \
    res = (res << 8) + instance.read(address + 1, 0x00);   \
    return res;                                   \
  }

#define __GP_REGISTER_N(name, address, size)      \
  static uint16_t write##name(const uint8_t (&_buff)[size]) {   \
    return instance.write(address, 0x04, &_buff[0], size);     \
  }                                               \
  static uint16_t read##name( uint8_t (&_buff)[size]) {    \
    return instance.read(address, 0x00, &_buff[0], size);      \
  }
	

#define __SOCKET_REGISTER8(name, address)                    \
  static inline void write##name(SOCKET _s, uint8_t _data) { \
    writeSn(_s, address, _data);                             \
  }                                                          \
  static inline uint8_t read##name(SOCKET _s) {              \
    return readSn(_s, address);                              \
  }

#define __SOCKET_REGISTER16(name, address)                   \
  static void write##name(SOCKET _s, uint16_t _data) {       \
    writeSn(_s, address,   _data >> 8);                      \
    writeSn(_s, address+1, _data & 0xFF);                    \
  }                                                          \
  static uint16_t read##name(SOCKET _s) {                    \
    uint16_t res = readSn(_s, address);                      \
    res = (res << 8) + readSn(_s, address + 1);              \
    return res;                                              \
  }

#define __SOCKET_REGISTER_N(name, address, size)             \
  static uint16_t write##name(SOCKET _s, const uint8_t (&_buff)[size]) {   \
    return writeSn(_s, address, &_buff[0], size);                \
  }                                                          \
  static uint16_t read##name(SOCKET _s, uint8_t (&_buff)[size]) {    \
    return readSn(_s, address, &_buff[0], size);                 \
  }
    
namespace robo{
	namespace prf{
		template<class D > class w5500_t {
		public:
			using SOCKET = typename D::SOCKET;
			enum {
				MAX_SOCK_NUM = 8
			};
			struct IPPROTO{
				enum {
					IP   = 0,
					ICMP = 1,
					IGMP = 2,
					GGP  = 3,
					TCP  = 6,
					PUP  = 12,
					UDP  = 17,
					IDP  = 22,
					ND   = 77,
					RAW  = 255,
				};
			};
			
			struct SnMR {
				enum{
					CLOSE  = 0x00,
					TCP    = 0x01,
					UDP    = 0x02,
					IPRAW  = 0x03,
					MACRAW = 0x04,
					PPPOE  = 0x05,
					ND     = 0x20,
					MULTI  = 0x80
				};
			}	;

			struct SnIR {
				enum{
					SEND_OK = 0x10,
					TIMEOUT = 0x08,
					RECV    = 0x04,
					DISCON  = 0x02,
					CON     = 0x01
				};
			};

			struct SnSR {
				enum{
					CLOSED      = 0x00,
					INIT        = 0x13,
					LISTEN      = 0x14,
					SYNSENT     = 0x15,
					SYNRECV     = 0x16,
					ESTABLISHED = 0x17,
					FIN_WAIT    = 0x18,
					CLOSING     = 0x1A,
					TIME_WAIT   = 0x1B,
					CLOSE_WAIT  = 0x1C,
					LAST_ACK    = 0x1D,
					UDP         = 0x22,
					IPRAW       = 0x32,
					MACRAW      = 0x42,
					PPPOE       = 0x5F
				};
			};
			enum SockCMD {
				Sock_OPEN      = 0x01,
				Sock_LISTEN    = 0x02,
				Sock_CONNECT   = 0x04,
				Sock_DISCON    = 0x08,
				Sock_CLOSE     = 0x10,
				Sock_SEND      = 0x20,
				Sock_SEND_MAC  = 0x21,
				Sock_SEND_KEEP = 0x22,
				Sock_RECV      = 0x40
			};

			static w5500_t instance;
			__GP_REGISTER8 (MR,     0x0000);    // Mode
			__GP_REGISTER_N(GAR,    0x0001, 4); // Gateway IP address
			__GP_REGISTER_N(SUBR,   0x0005, 4); // Subnet mask address
			__GP_REGISTER_N(SHAR,   0x0009, 6); // Source MAC address
			__GP_REGISTER_N(SIPR,   0x000F, 4); // Source IP address
			__GP_REGISTER8 (IR,     0x0015);    // Interrupt
			__GP_REGISTER8 (IMR,    0x0016);    // Interrupt Mask
			__GP_REGISTER16(RTR,    0x0019);    // Timeout address
			__GP_REGISTER8 (RCR,    0x001B);    // Retry count
			__GP_REGISTER_N(UIPR,   0x0028, 4); // Unreachable IP address in UDP mode
			__GP_REGISTER16(UPORT,  0x002C);    // Unreachable Port address in UDP mode
			__GP_REGISTER8 (PHYCFGR,     0x002E);    // PHY Configuration register, default value: 0b 1011 1xxx
		
			__SOCKET_REGISTER8(SnMR,        0x0000)        // Mode
			__SOCKET_REGISTER8(SnCR,        0x0001)        // Command
			__SOCKET_REGISTER8(SnIR,        0x0002)        // Interrupt
			__SOCKET_REGISTER8(SnSR,        0x0003)        // Status
			__SOCKET_REGISTER16(SnPORT,     0x0004)        // Source Port
			__SOCKET_REGISTER_N(SnDHAR,     0x0006, 6)     // Destination Hardw Addr
			__SOCKET_REGISTER_N(SnDIPR,     0x000C, 4)     // Destination IP Addr
			__SOCKET_REGISTER16(SnDPORT,    0x0010)        // Destination Port
			__SOCKET_REGISTER16(SnMSSR,     0x0012)        // Max Segment Size
			__SOCKET_REGISTER8(SnPROTO,     0x0014)        // Protocol in IP RAW Mode
			__SOCKET_REGISTER8(SnTOS,       0x0015)        // IP TOS
			__SOCKET_REGISTER8(SnTTL,       0x0016)        // IP TTL
			__SOCKET_REGISTER16(SnTX_FSR,   0x0020)        // TX Free Size
			__SOCKET_REGISTER16(SnTX_RD,    0x0022)        // TX Read Pointer
			__SOCKET_REGISTER16(SnTX_WR,    0x0024)        // TX Write Pointer
			__SOCKET_REGISTER16(SnRX_RSR,   0x0026)        // RX Free Size
			__SOCKET_REGISTER16(SnRX_RD,    0x0028)        // RX Read Pointer
			__SOCKET_REGISTER16(SnRX_WR,    0x002A)        // RX Write Pointer (supported?)

			struct cs{
				cs(void){ D::cs_off(); }
				~cs(void){ D::cs_on(); }
			};
		  static const uint16_t SSIZE = D::bufsz*1024; // Max Tx buffer size
			static const uint16_t RSIZE = D::bufsz*1024; // Max Rx buffer size

			typedef const uint8_t (& cip_t)[4];
			typedef uint8_t (& ip_t)[4];
			
			struct phy {
				struct regs {
					enum {
					/* PHYCFGR register value */
						RST =   	~(1<<7)  //< For PHY reset, must operate AND mask.
						, OPMD =  (1<<6)   // Configre PHY with OPMDC value
						, OPMDC_ALLA  =  (7<<3)
						, OPMDC_PDOWN =  (6<<3)
						, OPMDC_NA =  (5<<3)
						, OPMDC_100FA =  (4<<3)
						, OPMDC_100F =  (3<<3)
						, OPMDC_100H =  (2<<3)
						, OPMDC_10F =  (1<<3)
						, OPMDC_10H =  (0<<3)           					
						, DPX_FULL =  (1<<2)
						, DPX_HALF =  (0<<2)
						, SPD_100 =  (1<<1)
						, SPD_10 =  (0<<1)
						, LINK_ON =  1
						, LINK_OFF =  0
					};
				};
				enum class modes{
					ALLA  =  7
					, PDOWN =  6
					, NA =  5
					, M100FA =  4
					, M100F =  3
					, M100H =  2
					, M10F =  1
					, M10H =  0
				};
				enum class confbys{
					HW = 0     ///< Configured PHY operation mode by HW pin
					,SW = 1     ///< Configured PHY operation mode by SW register   
				};
				
				
				
				struct config{
					modes mode;
					confbys by;       ///< set by @ref PHY_CONFBY_HW or @ref PHY_CONFBY_SW
					
					void reset(void){
						uint8_t tmp = w5500_t::readPHYCFGR();
						tmp &= regs::RST;
						w5500_t::writePHYCFGR(tmp);
						tmp =  w5500_t::readPHYCFGR();
						tmp |= ~regs::RST;
						w5500_t::writePHYCFGR(tmp);
					}

					void get(void){
						uint8_t tmp = 0;
						tmp = w5500_t::readPHYCFGR();
						by   = (tmp & regs::OPMD) ? confbys::SW : confbys::HW;
						mode = (modes) ((tmp & regs::OPMDC_ALLA )>>3);
					}
					
					bool set(void){
						uint8_t tmp = 0;
						if(by == confbys::SW)
							tmp |= regs::OPMD;
						tmp |= (((uint8_t) mode) << 3);
						w5500_t::writePHYCFGR(tmp);
						store_s s = {mode,by};
						reset();
						get();
						return mode == s.mode && by == s.by;	
					}
					
					struct store_s{
						modes mode;
						confbys by;
					} ;
					
					bool powerdown(store_s & _s)	{
						get();
						_s.mode = mode;
						_s.by = by;
						mode =  modes::PDOWN;
						by = confbys::SW;						
						return set();							
					}
					bool powernorm(const store_s & _s)	{
						mode = _s.mode;
						by = _s.by;
						return set();
					}				
				};

				struct status{
					enum class duplexs{
						HALF = 0     
						,FULL = 1     
					};
					enum class speeds{
						M10 = 0     
						,M100 = 1     
					};
					enum class links{
						OFF = 0     
						,ON = 1    
					};
					duplexs duplex;
					speeds speed;
					links link;
					void get(void){
						uint8_t tmp = w5500_t::readPHYCFGR();
						duplex = ( (tmp & regs::DPX_FULL) == regs::DPX_FULL ) ? duplexs::FULL:duplexs::HALF;
						speed = ( (tmp & regs::SPD_100) == regs::SPD_100 ) ? speeds::M100:speeds::M10;
						link = ( (tmp & regs::LINK_ON) == regs::LINK_ON ) ? links::ON:links::OFF;
					}
				};
			};

			
			static void begin(void)
			{				
				typename phy::config config;
				typename phy::config::store_s store;
				//config.powerdown(store);
				//config.powernorm(store);
				

				swReset();


				for (int i=0; i<MAX_SOCK_NUM; i++) {
					uint8_t cntl_byte = (0x0C + (i<<5));
					instance.write( 0x1E, cntl_byte, D::bufsz); //0x1E - Sn_RXBUF_SIZE
					instance.write( 0x1F, cntl_byte, D::bufsz); //0x1F - Sn_TXBUF_SIZE
				}

				typename phy::status status;
				status.get();

			}

			static uint16_t space( SOCKET s)
			{
				uint16_t val=0, val1=0;
				do {
					val1 = readSnTX_FSR(s);
					if (val1 != 0)
						val = readSnTX_FSR(s);
				} 
				while (val != val1);
				return val;
			}

			static uint16_t available(SOCKET s)
			{
				uint16_t val=0,val1=0;
				do {
					val1 = readSnRX_RSR(s);
					if (val1 != 0)
						val = readSnRX_RSR(s);
				} 
				while (val != val1);
				return val;
			}

			static void send_data_processing(SOCKET s, const uint8_t *data, uint16_t len)
			{
				// This is same as having no offset in a call to send_data_processing_offset
				send_data_processing_offset(s, 0, data, len);

			}

			static void send_data_processing_offset(SOCKET s, uint16_t data_offset, const uint8_t *data, uint16_t len)
			{
				uint16_t ptr = readSnTX_WR(s);
				uint8_t cntl_byte = (0x14+(s<<5));
				ptr += data_offset;
				instance.write(ptr, cntl_byte, data, len);
				ptr += len;
				writeSnTX_WR(s, ptr);
			}

			static void recv_data_processing(SOCKET s, uint8_t *data, uint16_t len, uint8_t peek)
			{
				uint16_t ptr;
				ptr = readSnRX_RD(s);

				instance.read_data(s, ptr, data, len);
				if (!peek)
				{
					ptr += len;
					writeSnRX_RD(s, ptr);
				}
			}

			static void read_data(SOCKET s, volatile uint16_t src, volatile uint8_t *dst, uint16_t len)
			{
				uint8_t cntl_byte = (0x18+(s<<5));
				instance.read((uint16_t)src , cntl_byte, (uint8_t *)dst, len);
			}

			uint8_t write(uint16_t _addr, uint8_t _cb, uint8_t _data)
			{
				cs cs__;
				D::write(_addr >> 8);
				D::write(_addr & 0xFF);
				D::write(_cb);
				D::write(_data);
				return 1;
			}

			uint16_t write(uint16_t _addr, uint8_t _cb, const uint8_t *_buf, uint16_t _len)
			{
				cs cs__;
				D::write(_addr >> 8);
				D::write(_addr & 0xFF);
				D::write(_cb);
				for (uint16_t i=0; i<_len; ++i){
					D::write(_buf[i]);
				}
				return _len;
			}

			uint8_t read(uint16_t _addr, uint8_t _cb)
			{
				cs cs__;
				D::write(_addr >> 8);
				D::write(_addr & 0xFF);
				D::write(_cb);
				return D::read();
			}

			static uint16_t read(uint16_t _addr, uint8_t _cb, uint8_t *_buf, uint16_t _len)
			{ 
				cs cs__;
				D::write(_addr >> 8);
				D::write(_addr & 0xFF);
				D::write(_cb);
				for (uint16_t i=0; i<_len; i++){
					_buf[i] = D::read();
				}
				return _len;
			}

			static void execCmdSn(SOCKET s, SockCMD _cmd) {
				// Send command to socket
				writeSnCR(s, _cmd);
				// Wait for command to complete
				while (readSnCR(s));
			}


			static uint8_t readVersion(void)
			{
				cs cs__;
				D::write( 0x00 );
				D::write( 0x39 );
				D::write( 0x01);
				return D::read();
			}
			
			static uint8_t readSn(SOCKET _s, uint16_t _addr) {
				uint8_t cntl_byte = (_s<<5)+0x08;
				return instance.read(_addr, cntl_byte);
			}

			static uint8_t writeSn(SOCKET _s, uint16_t _addr, uint8_t _data) {
				uint8_t cntl_byte = (_s<<5)+0x0C;
				return instance.write(_addr, cntl_byte, _data);
			}

			static uint16_t readSn(SOCKET _s, uint16_t _addr, uint8_t *_buf, uint16_t _len) {
				uint8_t cntl_byte = (_s<<5)+0x08;
				return instance.read(_addr, cntl_byte, _buf, _len );
			}

			static uint16_t writeSn(SOCKET _s, uint16_t _addr, const uint8_t *_buf, uint16_t _len) {
				uint8_t cntl_byte = (_s<<5)+0x0C;
				return instance.write(_addr, cntl_byte, _buf, _len);
			}

			static void getGatewayIp(ip_t  _ip) {
				readGAR(_ip);
			}

			static void setGatewayIp(cip_t  _ip) {
				writeGAR(_ip);
			}

			static void getSubnetMask(ip_t  _ip) {
				readSUBR(_ip);
			}

			static void setSubnetMask(cip_t  _ip) {
				writeSUBR(_ip);
			}

			static void getMACAddress(uint8_t (&_addr) [6]) {
				readSHAR(_addr);
			}

			static void setMACAddress(const  uint8_t (&_addr) [6]) {
				writeSHAR(_addr);
			}

			static void getIPAddress(ip_t  _ip) {
				readSIPR(_ip);
			}

			static void setIPAddress(cip_t  _ip) {
				writeSIPR(_ip);
			}

			static void setRetransmissionTime(uint16_t _timeout) {
				writeRTR(_timeout);
			}

			void setRetransmissionCount(uint8_t _retry) {
				writeRCR(_retry);
			}

			static void setPHYCFGR(uint8_t _val) {
				writePHYCFGR(_val);
			}

			uint8_t getPHYCFGR() {
			//  readPHYCFGR();
				return read(0x002E, 0x00);
			}

			static void swReset() {
				writeMR( (readMR() | 0x80) );
			}
			//==================================
			struct socket{
				uint16_t avalable(void){ return w5500_t::available(instance); }
				uint16_t local_port;
				SOCKET instance =MAX_SOCK_NUM;  // socket ID for Wiz5100
				/**
				 * @brief	This Socket function initialize the channel in perticular mode, and set the port and wait for w5500 done it.
				 * @return 	1 for success else 0.
				 */
				bool  bind(uint8_t _protocol, uint16_t _port, uint8_t _flag){
					SOCKET s =MAX_SOCK_NUM;
					for (int i = 0; i < MAX_SOCK_NUM; ++i) {
						uint8_t st = w5500_t::readSnSR(i);
						if (st == (SnSR::CLOSED || st == SnSR::FIN_WAIT) ) {
							s = i;
							break;
						}
					}
					
					if (s == MAX_SOCK_NUM){
						return false;
					}
					
					return bind_(s,_protocol,_port,_flag);
				}
				bool  bind(SOCKET _s, uint8_t _protocol, uint16_t _port, uint8_t _flag){					
					if(_s<0){
						return false;
					}
					if(_s>=MAX_SOCK_NUM){
						return false;
					}
					return bind_(_s,_protocol,_port,_flag);
				}
				
				bool  bind_(SOCKET _s, uint8_t _protocol, uint16_t _port, uint8_t _flag){
					uint8_t st = w5500_t::readSnSR(_s);
					if ( ! (st == SnSR::CLOSED || st == SnSR::FIN_WAIT  )) {
						return false;
					}	
					if(instance != MAX_SOCK_NUM){
						close_(instance);
					}

					instance = _s;
					
					if ((_protocol == SnMR::TCP) || (_protocol == SnMR::UDP) || (_protocol == SnMR::IPRAW) || (_protocol == SnMR::MACRAW) || (_protocol == SnMR::PPPOE))
					{
						w5500_t::writeSnMR(instance, _protocol | _flag);
						local_port = _port;
						w5500_t::writeSnPORT(instance, _port);					
						w5500_t::execCmdSn(instance, Sock_OPEN);
						return true;
					} else {
						return false;
					}
				}


				/**
				 * @brief	This function close the instance and parameter is "s" which represent the instance number
				 */
				void close_(SOCKET _instance){
					w5500_t::execCmdSn(_instance, Sock_CLOSE);
					w5500_t::writeSnIR(_instance, 0xFF);
				}
				void close(void){
					if(instance != MAX_SOCK_NUM){
						close(instance);
					}
				}

				/**
				 * @brief	This function established  the connection for the channel in passive (server) mode. This function waits for the request from the peer.
				 * @return	1 for success else 0.
				 */
				bool listen(void){
					if(instance == MAX_SOCK_NUM) return false;
					if (w5500_t::readSnSR(instance) != SnSR::INIT)	return false;
					w5500_t::execCmdSn(instance, Sock_LISTEN);
					return true;
				}


				/**
				 * @brief	This function established  the connection for the channel in Active (client) mode. 
				 * 		This function waits for the untill the connection is established.
				 * 		
				 * @return	1 for success else 0.
				 */
				bool connect(uint8_t * addr, uint16_t port)
				{
					if(instance == MAX_SOCK_NUM) return false;
					if 
						(
					((addr[0] == 0xFF) && (addr[1] == 0xFF) && (addr[2] == 0xFF) && (addr[3] == 0xFF)) ||
						((addr[0] == 0x00) && (addr[1] == 0x00) && (addr[2] == 0x00) && (addr[3] == 0x00)) ||
						(port == 0x00) 
						) 
						return false;

					// set destination IP
					w5500_t::writeSnDIPR(instance, addr);
					w5500_t::writeSnDPORT(instance, port);
					w5500_t::execCmdSn(instance, Sock_CONNECT);

					return true;
				}



				/**
				 * @brief	This function used for disconnect the instance and parameter is "s" which represent the instance number
				 * @return	1 for success else 0.
				 */
				void disconnect(void){
					if(instance != MAX_SOCK_NUM){
						w5500_t::execCmdSn(instance, Sock_DISCON);
					}
				}
				protected:

				/**
				 * @brief	This function used to send the data in TCP mode
				 * @return	1 for success else 0.
				 */
				uint16_t send(const uint8_t * buf, uint16_t len)
				{
					uint8_t status=0;
					uint16_t ret=0;
					uint16_t freesize=0;
					if(instance == MAX_SOCK_NUM) return 0;
					if (len > w5500_t::SSIZE) 
						ret = w5500_t::SSIZE; // check size not to exceed MAX size.
					else 
						ret = len;

					freesize = w5500_t::space(instance);
					if(freesize >= ret){
						status = w5500_t::readSnSR(instance);
						if ((status != SnSR::ESTABLISHED) || (status == SnSR::CLOSE_WAIT)){
							close();
							return 0;
						}
					} else {
							return 0;
					}						

					// copy data
					w5500_t::send_data_processing(instance, (uint8_t *)buf, ret);
					w5500_t::execCmdSn(instance, Sock_SEND);

					return ret;
				}


				/**
				 * @brief	This function is an application I/F function which is used to receive the data in TCP mode.
				 * 		It continues to wait for data as much as the application wants to receive.
				 * 		
				 * @return	received data size for success else -1.
				 */
				bool recv(uint8_t *buf, int16_t len)
				{
					// Check how much data is available
					int16_t ret = w5500_t::available (instance);
					if ( ret == 0 )
					{
						// No data available.
						uint8_t status = w5500_t::readSnSR(instance);
						if ( status == SnSR::LISTEN || status == SnSR::CLOSED || status == SnSR::CLOSE_WAIT )
						{
							// The remote end has closed its side of the connection, so this is the eof state
							return false;
						}
						else
						{
							// The connection is still up, but there's no data waiting to be read
							close(instance);
							return false;
						}
					}
					else if (ret > len)
					{
						ret = len;
					}

					w5500_t::recv_data_processing(instance, buf, ret);
					w5500_t::execCmdSn(instance, Sock_RECV);
					return ret;
				}


				/**
				 * @brief	Returns the first byte in the receive queue (no checking)
				 * 		
				 * @return
				 */
				uint16_t peek(uint8_t *buf)
				{
					w5500_t::recv_data_processing(instance, buf, 1, 1);

					return 1;
				}


				/**
				 * @brief	This function is an application I/F function which is used to send the data for other then TCP mode. 
				 * 		Unlike TCP transmission, The peer's destination address and the port is needed.
				 * 		
				 * @return	This function return send data size for success else -1.
				 */
				bool sendto(cip_t _addr, uint16_t _port, const uint8_t * _buf, uint16_t _len)	{
					if (_len > w5500_t::SSIZE) return 0;
						
					if
						(
					((_addr[0] == 0x00) && (_addr[1] == 0x00) && (_addr[2] == 0x00) && (_addr[3] == 0x00)) ||
						((_port == 0x00)) ||(_port == 0)
						) 
					{
						return false;
					}
					else
					{
						w5500_t::writeSnDIPR(instance, _addr);
						w5500_t::writeSnDPORT(instance, _port);

						// copy data
						w5500_t::send_data_processing(instance, _buf, _len);
						w5500_t::execCmdSn(instance, Sock_SEND);
						
						return true;

					}
				}


				/**
				 * @brief	This function is an application I/F function which is used to receive the data in other then
				 * 	TCP mode. This function is used to receive UDP, IP_RAW and MAC_RAW mode, and handle the header as well. 
				 * 	
				 * @return	This function return received data size for success else -1.
				 */
				uint16_t recvfrom(uint8_t *buf, ip_t _ip, uint16_t & _port)
				{
					uint8_t head[8];
					uint16_t data_len=0;
					uint16_t ptr=0;

					ptr = w5500_t::readSnRX_RD(instance);
					switch (w5500_t::readSnMR(instance) & 0x07){
						case SnMR::UDP :
							w5500_t::read_data(instance, ptr, head, 0x08);
							ptr += 8;
							// read peer's IP address, port number.
							_ip[0] = head[0];
							_ip[1] = head[1];
							_ip[2] = head[2];
							_ip[3] = head[3];
							_port = head[4];
							_port = (_port << 8) + head[5];
							data_len = head[6];
							data_len = (data_len << 8) + head[7];
							w5500_t::read_data(instance, ptr, buf, data_len); // data copy.
							ptr += data_len;
							w5500_t::writeSnRX_RD(instance, ptr);
							break;
#if 0
						case SnMR::IPRAW :
							w5500_t::read_data(s, ptr, head, 0x06);
							ptr += 6;

							addr[0] = head[0];
							addr[1] = head[1];
							addr[2] = head[2];
							addr[3] = head[3];
							data_len = head[4];
							data_len = (data_len << 8) + head[5];

							w5500_t::read_data(s, ptr, buf, data_len); // data copy.
							ptr += data_len;

							w5500_t::writeSnRX_RD(s, ptr);
							break;

						case SnMR::MACRAW:
							w5500_t::read_data(s, ptr, head, 2);
							ptr+=2;
							data_len = head[0];
							data_len = (data_len<<8) + head[1] - 2;

							w5500_t::read_data(s, ptr, buf, data_len);
							ptr += data_len;
							w5500_t::writeSnRX_RD(s, ptr);
							break;
#endif
						default :
							break;
					}
					w5500_t::execCmdSn(instance, Sock_RECV);
					return data_len;
				}

				/**
				 * @brief	Wait for buffered transmission to complete.
				 */
				void flush(void) {
					// TODO
				}
#if 0
				uint16_t igmpsend(SOCKET s, const uint8_t * buf, uint16_t len)
				{
					uint8_t status=0;
					uint16_t ret=0;

					if (len > w5500_t::SSIZE) 
						ret = w5500_t::SSIZE; // check size not to exceed MAX size.
					else 
						ret = len;

					if (ret == 0)
						return 0;

					w5500_t::send_data_processing(s, (uint8_t *)buf, ret);
					w5500_t::execCmdSn(s, Sock_SEND);

					while ( (w5500_t::readSnIR(s) & SnIR::SEND_OK) != SnIR::SEND_OK ) 
					{
						status = w5500_t::readSnSR(s);
						if (w5500_t::readSnIR(s) & SnIR::TIMEOUT)
						{
							/* in case of igmp, if send fails, then socket closed */
							/* if you want change, remove this code. */
							close(s);
							return 0;
						}
					}

					w5500_t::writeSnIR(s, SnIR::SEND_OK);
					return ret;
				}
#endif



			//===================================
			};
			
			
			class udp: public socket {
			private:

				uint16_t port_ = 0; // local port to listen on
				uint16_t offset_; // offset into the packet being sent
				uint16_t remaining_; // remaining bytes of incoming packet yet to be processed

			public:
				udp(){};  // Constructor
					
				virtual bool bind(uint16_t _port, uint8_t flags){	// initialize, start listening on specified port. Returns 1 if successful, 0 if there are no sockets available to use
					port_ = _port;
					remaining_ = 0;
					return socket::bind(SnMR::UDP, _port, flags);
				}
				virtual bool bind(SOCKET _s, uint16_t _port, uint8_t flags){	// initialize, start listening on specified port. Returns 1 if successful, 0 if there are no sockets available to use
					port_ = _port;
					remaining_ = 0;
					return socket::bind(_s, SnMR::UDP, _port, flags);
				}
				using socket::sendto;
				using socket::recvfrom;

				
#if 0
				
				// Start building up a packet to send to the remote host specific in host and port
				// Returns 1 if successful, 0 if there was a problem resolving the hostname or port
				virtual int beginPacket(const char *host, uint16_t port);
				// Finish off this packet and send it
				// Returns 1 if the packet was sent successfully, 0 if there was an error
				virtual int endPacket();
				// Write a single byte into the packet
				virtual size_t write(uint8_t);
				// Write size bytes from buffer into the packet
				virtual size_t write(const uint8_t *buffer, size_t size);
				
				using Print::write;

				// Start processing the next available incoming packet
				// Returns the size of the packet in bytes, or 0 if no packets are available
				virtual int parsePacket();
				// Number of bytes remaining in the current packet
				virtual int available();
				// Read a single byte from the current packet
				virtual int read();
				// Read up to len bytes from the current packet and place them into buffer
				// Returns the number of bytes read, or 0 if none are available
				virtual int read(unsigned char* buffer, size_t len);
				// Read up to len characters from the current packet and place them into buffer
				// Returns the number of characters read, or 0 if none are available
				virtual int read(char* buffer, size_t len) { return read((unsigned char*)buffer, len); };
				// Return the next byte from the current packet without moving on to the next byte
				virtual int peek();
				virtual void flush();	// Finish reading the current packet

				// Return the IP address of the host who sent the current incoming packet
				virtual IPAddress remoteIP() { return _remoteIP; };
				// Return the port of the host who sent the current incoming packet
				virtual uint16_t remotePort() { return _remotePort; };
				#endif
			};

			
		};
		template<class D > w5500_t<D> w5500_t<D>::instance;
	}
}
#endif