#ifndef __robo_net_serial_h
#define __robo_net_serial_h
#include "core/robosd_common.hpp"
#include "core/robosd_list.hpp"
#include "core/robosd_string.hpp"
#include "core/robosd_delegat.hpp"
#include  "robosd_ring_buf.hpp"
#include "net/robosd_net_link.hpp"

namespace robo {
	namespace net {
		class ROBO_EXPORT iserial: public link {
			typedef ::robo::list::unique<iserial, int> list;
			typedef list::ref ref;
			static list& list_(void);
		public:
			virtual size_t available(void) = 0;
			virtual size_t get(uint8_t* _data, size_t _max_size) = 0;
			virtual bool put(const uint8_t* _data, size_t _size) = 0;
			virtual size_t get(uint8_t & _data) = 0;
			virtual bool  put(uint8_t) = 0;
			virtual void reset(void) = 0;
			virtual ~iserial(void){};
			iserial(void):link(){};
			virtual bool  puts(const char *_s);
			
		};

		class ROBO_EXPORT serial_dummy :public iserial {
		private:
			serial_dummy(void) :iserial() {}
		public:
			virtual size_t available(void);
			virtual size_t space(void);
			virtual size_t space_max(void);
			virtual size_t get(uint8_t* _data, size_t _max_size);
			virtual bool put(const uint8_t* _data, size_t _size);
			virtual size_t get(uint8_t & _data);
			virtual bool  put(uint8_t);
			virtual void reset(void);
			static serial_dummy& instance(void);
		};

		/*class ROBO_EXPORT port {
			virtual size_t incomm_space(void) = 0;
			virtual void receive(uint8_t* _data, size_t _size) = 0;
			virtual size_t send_need(void) = 0;
			virtual size_t get_send_buf(uint8_t* _data, size_t _size) = 0;
		};*/

		template <class D, unsigned S, class G > class ROBO_EXPORT receiver_t : public  iserial, public D {
			ring_t<S> ring_;
		public:

			virtual size_t available(void) {
				G g__;
				return ring_.count();
			}
			virtual size_t space_max(void) {
				return ring_.size();
			}
			virtual size_t space(void) {
				G g__;
				return ring_.space();
			}
			virtual size_t get(uint8_t* _data, size_t _max_size) {
				G g__;
				return ring_.get(_data, _max_size);
			}
			virtual bool put(const uint8_t* _data, size_t _size) {
				return false;
			}
			virtual size_t get(uint8_t& _data) {
				G g__;
				return ring_.get(&_data,1);
			}
			virtual bool  put(uint8_t _data) {				
				return false;
			}
			virtual void reset(void) {
				G g__;
				ring_.clear();
			}
			void on_receive(const uint8_t* _data, size_t _size) {
				G g__;
				if (!ring_.put(_data, _size)) {
					//D::fault(); to do крепко подумать
				}
			}
		};
		#if 0
		template <class D, unsigned SB, typename G > class ROBO_EXPORT sender_t: private D {
			ring_t<SB> ring_;
		public:
			virtual size_t available(void) {
				return G g__;
				return ring_.available();
			}
			virtual size_t space(void) {
				G g__;
				return ring_.space();
			}
			virtual size_t space_max(void) {
				return ring_.size();
			}
			virtual size_t get(uint8_t* _data, size_t _max_size) {
				G g__;
				return ring_.get(_data, _max_size);
			}
			virtual bool put(const uint8_t* _data, size_t _size) {
				bool tmp;
				{
					G g__;
					tmp = ring_.put(_data, _size);
					D::try_send(*this);
					return tmp;
				}
			}

			virtual size_t get(uint8_t& _data) {
				G g__;
				return ring_.get(_data);
			}

			virtual bool  put(uint8_t _data) {
				bool tmp; 
				{
					G g__;
					if (ring_.space() > 0) {
						ring_.put(_data);
						tmp = true;
					}
					else {
						tmp = false;
					}
					D::try_send(*this);
					return tmp;
				}
			}
		};
		#endif

		template <unsigned SA, unsigned SB, typename G > class ROBO_EXPORT bridge_t {
			ring_t<SA> ring_a_;
			ring_t<SB> ring_b_;

		public:
			class ROBO_EXPORT incomm : public  iserial {
				bridge_t& owner_;
			public:
				virtual size_t space_max(void) {
					return owner_.ring_a_.size();
				}
				virtual size_t available(void) {
					G g__;
					return owner_.ring_a_.count();
				}
				virtual size_t space(void) {
					G g__;
					return owner_.ring_b_.space();
				}
				virtual size_t get(uint8_t* _data, size_t _max_size) {
					G g__;
					return owner_.ring_a_.get(_data, _max_size);
				}
				virtual bool put(const uint8_t* _data, size_t _size) {
					G g__;
					return owner_.ring_b_.put(_data, _size);
				}
				virtual size_t get(uint8_t & _data) {
					G g__;
					if (owner_.ring_a_.count() > 0) {
						_data =  owner_.ring_a_.get();
						return 1;
					}
					else {
						return 0;
					}
				}
				virtual bool  put(uint8_t _data) {
					G g__;
					if (owner_.ring_b_.space() > 0) {
						owner_.ring_b_.put(_data);
						return true;
					}
					else {
						return false;
					}
				}
				incomm(bridge_t& _owner) : iserial(), owner_(_owner) {}
				virtual void reset(void) {
					G g__;
					owner_.ring_a_.clear();
				}
			} A;

			class ROBO_EXPORT outcomm : public  iserial {
				bridge_t& owner_;
			public:
				virtual size_t available(void) {
					G g__;
					return owner_.ring_b_.count();
				}				
				virtual size_t space(void) {
					G g__;
					return owner_.ring_a_.space();
				}
				virtual size_t space_max(void) {
					return owner_.ring_a_.size();
				}
				virtual size_t get(uint8_t* _data, size_t _max_size) {
					G g__;
					return owner_.ring_b_.get(_data, _max_size);
				}
				virtual bool put(const uint8_t* _data, size_t _size) {
					G g__;
					return owner_.ring_a_.put(_data, _size);
				}
				virtual size_t get(uint8_t & _data) {
					G g__;
					if (owner_.ring_b_.count() > 0) {
						_data = owner_.ring_b_.get();
						return 1;
					}
					else {
						return 0;
					}
				}
				virtual bool  put(uint8_t _data) {
					G g__;
					if (owner_.ring_a_.space() > 0) {
						owner_.ring_a_.put(_data);
						return true;
					}
					else {
						return false;
					}
				}
				outcomm(bridge_t& _owner)
					: iserial()
					, owner_(_owner) {}
				virtual void reset(void) {
					G g__;
					owner_.ring_b_.clear();
				}
			} B;

			bridge_t(void)
				: A(*this)
				, B(*this) {}

		};

		template <unsigned SA, unsigned SB > class ROBO_EXPORT bridge_t<SA,SB,void> {
			ring_t<SA> ring_a_;
			ring_t<SB> ring_b_;
		protected:
			virtual uint8_t unsave_get(void) {
				return ring_a_.get();
			}
		public:
			class incomm : public  iserial {
				bridge_t& owner_;
			protected:
			public:
				virtual size_t available(void) {
					return owner_.ring_a_.count();
				}
				virtual size_t space_max(void) {
					return owner_.ring_b_.size();
				}
				virtual size_t space(void) {
					return owner_.ring_b_.space();
				}
				virtual size_t get(uint8_t* _data, size_t _max_size) {
					return owner_.ring_a_.get(_data, _max_size);
				}
				virtual bool put(const uint8_t* _data, size_t _size) {
					return owner_.ring_b_.put(_data, _size);
				}
				virtual size_t get(uint8_t & _data) {
					if (owner_.ring_a_.count() > 0) {
						_data = owner_.ring_a_.get();
						return 1;
					}
					else {
						return 0;
					}
				}
				virtual bool  put(uint8_t _data) {
					if (owner_.ring_b_.space() > 0) {
						owner_.ring_b_.put(_data);
						return true;
					}
					else {
						return false;
					}
				}
				incomm(bridge_t& _owner) : iserial(), owner_(_owner) {}
				virtual void reset(void) {
					owner_.ring_a_.clear();
				}
			} A;

			class outcomm : public  iserial {
				bridge_t& owner_;
			public:
				virtual size_t available(void) {
					return owner_.ring_b_.count();
				}
				virtual size_t space_max(void) {
					return owner_.ring_a_.size();
				}
				virtual size_t space(void) {
					return owner_.ring_a_.space();
				}
				virtual size_t get(uint8_t* _data, size_t _max_size) {
					return owner_.ring_b_.get(_data, _max_size);
				}
				virtual bool put(const uint8_t* _data, size_t _size) {
					return owner_.ring_a_.put(_data, _size);
				}
				virtual size_t get(uint8_t & _data) {
					if (owner_.ring_b_.count() > 0) {
						_data =  owner_.ring_b_.get();
						return 1;
					}
					else {
						return 0;
					}
				}
				virtual bool  put(uint8_t _data) {
					if (owner_.ring_a_.space() > 0) {
						owner_.ring_a_.put(_data);
						return true;
					}
					else {
						return false;
					}
				}
				outcomm(bridge_t& _owner)
					: iserial()
					, owner_(_owner) {}
				virtual void reset(void) {
					owner_.ring_b_.clear();
				}
			} B;

			bridge_t(void)
				: A(*this)
				, B(*this) {}


		};
		
		template <typename D,unsigned SA, unsigned SB, typename G > class ROBO_EXPORT hardware_bridge_t: protected bridge_t<SA,SB,G>, public net::iserial, private D {
		public:
			typedef bridge_t<SA,SB,G> bridge;
			void on_receive(uint8_t _data){
				if( ! bridge::A.put(_data)){
					//D::fault(); to do крепко подумать
				}
			}
			void on_receive(const uint8_t* _data, size_t _max_size){
				if( ! bridge::A.put(_data,_max_size) ){
					//D::fault(); to do крепко подумать
				}
			}
			
			virtual size_t available(void) {
				return bridge::B.available();
			}
			virtual size_t space(void) {
				return bridge::B.space();
			}
			virtual size_t get(uint8_t* _data, size_t _max_size) {
				return bridge::B.get(_data, _max_size);
			}
			virtual bool put(const uint8_t* _data, size_t _size) {
				bool tmp = bridge::B.put(_data, _size);
				D::try_send(bridge::A);
				return tmp;
			}
			virtual size_t get(uint8_t & _data) {
				if (bridge::B.get(_data) > 0) {
					return 1;					
				}
				else {
					return 0;
				}
			}
			/*void try_send_(){				
				uint8_t * buffer = D::tx_buffer;
				size_t sz = bridge::A.get(buffer,D::buffer_size);
				if(sz>0){
					D::try_send(buffer, sz);
				}
			}*/
			virtual bool  put(uint8_t _data) {
				bool tmp = bridge::B.put(_data);
				D::try_send(bridge::A);
				if(!tmp){
					reset();
					//D::fault(); to do крепко подумать
				}
				return tmp;
			}
			void on_confirm(void){
				D::try_send(bridge::A);
			}
			void on_refuse(void){
				reset();
//				D::try_send(bridge::A);
				//D::fault(); to do крепко подумать
			}
			virtual void reset(void){
				bridge::A.reset();
				bridge::B.reset();
			}
			virtual size_t space_max(void){
				return bridge::A.space_max();
			}

		};
		

		template <typename D,unsigned SA, unsigned SB, typename G > class ROBO_EXPORT half_duplex_t: protected bridge_t<SA,SB,G>, public net::iserial{
		typedef bridge_t<SA,SB,G> bridge;
		public:
			enum class panic{ rx_overflow, tx_refuse};
			::robo::time_us_t last_tx_ = 0;
		private:
			bool busy_ = false;
//			::robo::delegat::ref<void> & on_receive_;			
			::robo::delegat::ref<void,panic> & on_panic_;
			void try_send_(){
				if( !busy_) {
					uint8_t * buffer = D::tx_buffer;
					size_t sz = bridge::A.get(buffer,D::buffer_size);
					if(sz>0){
						busy_ = true;
						if(D::try_send(buffer, sz)){
							last_tx_ = D::time_us();
							return;
						} else{
							busy_ = false;
							on_panic_(panic::tx_refuse);
						}
					}
					D::begin_receive();
				}
			}
			::robo::delegat::dummy<void,panic> pd_;
		public:
			half_duplex_t(const char * _name=nullptr, ::robo::delegat::ref<void,panic> * _on_panic=nullptr): 
				on_panic_(_on_panic?*_on_panic:pd_){
					if(_name){
						iserial::begin(_name);						
					}
			}
			void on_receive(uint8_t _data){
				if( ! bridge::A.put(_data)){
					on_panic_(panic::rx_overflow);
				}else{
					last_tx_ = D::time_us();
				}
			}
			void on_receive(const uint8_t* _data, size_t _max_size){
				if( ! bridge::A.put(_data,_max_size) ){
					on_panic_(panic::rx_overflow);
				} else {
							last_tx_ = D::time_us();
				}
			}
			void on_confirm(void){
				busy_ = false;
				try_send_();				
			}
			void on_refuse(void){
				busy_ = false;
				on_panic_(panic::tx_refuse);
				reset();
			}
			
			void watchdog(void){
				robo::time_us_t now = D::time_us();
				if(now - last_tx_ > D::watchdog_us){
					reset();
					last_tx_ = now;
				}
			}
			
			virtual size_t available(void) {
				return bridge::B.available();
			}
			virtual size_t space(void) {
				return bridge::B.space();
			}
			virtual size_t get(uint8_t* _data, size_t _max_size) {
				return bridge::B.get(_data, _max_size);
			}
			virtual bool put(const uint8_t* _data, size_t _size) {
				if( bridge::B.put(_data, _size)){
					try_send_();
					return true;
				} else{
					on_panic_(panic::tx_refuse);
					return false;
				}
			}
			virtual size_t get(uint8_t & _data) {
				if (bridge::B.get(_data) > 0) {
					return 1;					
				}
				else {
					return 0;
				}
			}
			virtual bool  put(uint8_t _data) {
				if( bridge::B.put(_data)){
					try_send_();
					return true;
				} else{
					on_panic_(panic::tx_refuse);
					return false;
				}
			}
			virtual void reset(void){
				G g__;
				bridge::A.reset();
				bridge::B.reset();
				D::begin_receive();
			}
			void start(void){
				reset();
			}
			virtual size_t space_max(void){
				return bridge::A.space_max();
			}

		};
		
		
		template <typename D,unsigned SA, unsigned SB, typename G > class ROBO_EXPORT hardware_serial_t: protected bridge_t<SA,SB,G>, public net::iserial{
		typedef bridge_t<SA,SB,G> bridge;
			enum { rx_buffer_size =  1<<SA };
			enum { tx_buffer_size =  1<<SB };
			static inline uint8_t tx_buffer[tx_buffer_size] = {};
			static inline uint8_t rx_buffer[rx_buffer_size] = {};

		public:
			void begin_receive(void){
				D::begin_receive(rx_buffer,rx_buffer_size);
			}
			void on_receive(size_t _size){
				bridge::A.put(rx_buffer,_size);
				D::begin_receive(rx_buffer,rx_buffer_size);
			}
			void try_send(void){
				size_t sz = bridge::A.get(tx_buffer,tx_buffer_size);
				if(sz>0){
					D::send(tx_buffer,sz);
				} 
			}
			void on_confirm(void){
				try_send();
			}
			void on_refuse(void){
				//todo подумать
				reset();
				D::begin_receive(rx_buffer,rx_buffer_size);
			}

			virtual void reset(void){
				bridge::A.reset();
				bridge::B.reset();
			}
			
			virtual size_t available(void) {
				return bridge::B.available();
			}
			virtual size_t space(void) {
				return bridge::B.space();
			}
			virtual size_t get(uint8_t* _data, size_t _max_size) {
				return bridge::B.get(_data, _max_size);
			}
			virtual bool put(const uint8_t* _data, size_t _size) {
				bool tmp = bridge::B.put(_data, _size);
				try_send();
				return tmp;
			}
			virtual bool  puts(const char *_s){
				bool tmp = bridge::B.puts(_s);
				try_send();
				return tmp;
			}
			
			virtual size_t get(uint8_t & _data) {
				if (bridge::B.get(_data) > 0) {
					return 1;					
				}
				else {
					return 0;
				}
			}
			
			virtual bool  put(uint8_t _data) {
				bool tmp = bridge::B.put(_data);
				if(!tmp){
					reset();
				} else {
					try_send();
				}
				return tmp;
			}
			
			virtual size_t space_max(void){
				return bridge::A.space_max();
			}

		};
		
	}
}


#endif
