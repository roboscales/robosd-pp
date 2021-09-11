#ifndef __robo_net_serial_h
#define __robo_net_serial_h
#include "core/robosd_common.hpp"
#include "core/robosd_list.hpp"
#include "core/robosd_string.hpp"
#include "core/robosd_lambda.hpp"
#include  "robosd_ring_buf.hpp"

namespace robo {
	namespace net {
		class ROBO_EXPORT iserial {
			typedef ::robo::list::unique<iserial, int> list;
			typedef list::ref ref;
			static list& list_(void);
			ref ref_;
		public:
			virtual size_t available(void) = 0;
			virtual size_t space(void) = 0;
			virtual size_t get(uint8_t* _data, size_t _max_size) = 0;
			virtual bool put(const uint8_t* _data, size_t _size) = 0;
			virtual uint8_t get(void) = 0;
			virtual bool  put(uint8_t) = 0;
			virtual void reset(void) = 0;
			virtual ~iserial(void);
			iserial(void);
			bool reg(cstr _caption);
			void unreg(void);
			static iserial* find(cstr _caption);
			static iserial* query(cstr _caption);
			static iserial& query_ref(cstr _caption);
			void  release(void);
			static void forall(lambda<void(iserial&)>& _operator);
		};

		class ROBO_EXPORT serial_dummy :public iserial {
		private:
			serial_dummy(void) :iserial() {}
		public:
			virtual size_t available(void);
			virtual size_t space(void);
			virtual size_t get(uint8_t* _data, size_t _max_size);
			virtual bool put(const uint8_t* _data, size_t _size);
			virtual uint8_t get(void);
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

		template <unsigned SA, unsigned SB, typename G = void > class bridge_t {
			ring_t<SA> ring_a_;
			ring_t<SB> ring_b_;
		public:
			class incomm : public  iserial {
				bridge_t& owner_;
			public:
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
				virtual uint8_t get(void) {
					G g__;
					if (owner_.ring_a_.count() > 0) {
						return owner_.ring_a_.get();
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
				virtual size_t get(uint8_t* _data, size_t _max_size) {
					G g__;
					return owner_.ring_b_.get(_data, _max_size);
				}
				virtual bool put(const uint8_t* _data, size_t _size) {
					G g__;
					return owner_.ring_a_.put(_data, _size);
				}
				virtual uint8_t get(void) {
					G g__;
					if (owner_.ring_b_.count() > 0) {
						return owner_.ring_b_.get();
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

	}
}


#endif
