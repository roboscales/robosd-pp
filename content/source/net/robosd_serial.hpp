#ifndef __robo_net_serial_h
#define __robo_net_serial_h
#include "core/robosd_common.hpp"
#include "core/robosd_list.hpp"
#include "core/robosd_string.hpp"
#include "core/robosd_lambda.hpp"
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

		template <unsigned SA, unsigned SB, typename G > class bridge_t {
			ring_t<SA> ring_a_;
			ring_t<SB> ring_b_;

		public:
			class incomm : public  iserial {
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

			class outcomm : public  iserial {
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

		template <unsigned SA, unsigned SB > class bridge_t<SA,SB,void> {
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
					return owner_.ring_a_.size();
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
		
		template <typename D,unsigned SA, unsigned SB, typename G > class hardware_bridge_t: protected bridge_t<SA,SB,G>, public net::iserial{
		typedef bridge_t<SA,SB,G> bridge;
		public:
			void on_receive(uint8_t _data){
				if( ! bridge::A.put(_data)){
					//D::fault(); to do крепко подумать
				}
			}
			void on_receive(uint8_t* _data, size_t _max_size){
				if( ! bridge::A.put(_data,_max_size) ){
					//D::fault(); to do крепко подумать
				}
			}
			
			virtual size_t available(void) {
				return bridge::B.count();
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
					//D::fault(); to do крепко подумать
					return 0;
				}
			}
			virtual bool  put(uint8_t _data) {
				bool tmp = bridge::B.put(_data);
				D::try_send(bridge::A);
				if(!tmp){
					//D::fault(); to do крепко подумать
				}
				return tmp;
			}
			void on_confirm(void){
				D::try_send(bridge::A);
			}
			void on_refuse(void){
				D::try_send(bridge::A);
				//D::fault(); to do крепко подумать
			}
		};
		
	}
}


#endif
