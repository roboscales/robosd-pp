#ifndef __robo_can_h
#define __robo_can_h
#include "core/robosd_common.hpp"
#include "core/robosd_list.hpp"
#include "core/robosd_delegat.hpp"
namespace robo {
	namespace net {
		class ROBO_EXPORT ican {
		public:
			typedef list::unique<ican, int> map;
			typedef map::ref ref;
			enum class event {
				init
				, deinit
				, connect
				, disconnect
				, fault
			};
			typedef delegat::base<void, ican&, uint32_t, uint8_t*, uint8_t   > on_receive_f;
			typedef delegat::base<void, ican&, event> on_event_f;
		private:
			ref ref_;

		protected:
			on_receive_f * on_receive;
			on_event_f * on_event;
		public:
			void set_on_receive(on_receive_f* _on_receive);
			void set_on_event(on_event_f* _on_event);

			virtual bool open(bool _owned_view = false) = 0;
			virtual void close(void) = 0;
			virtual bool send(uint32_t _id, uint8_t* _buf, uint8_t  _len) = 0;
			virtual bool ready(void) = 0;
			virtual void reset(void) = 0;
			virtual void pool(void) = 0;

			ican(void);
			bool reg(cstr _caption);
			void unreg(void);
			static ican* find(cstr _caption);
			static ican* query(cstr _caption);
			static ican& query_ref(cstr _caption);
			void  release(void);
			static void forall(lambda<void(ican&)>& _operator);
		};

		class ROBO_EXPORT can_dummy :public ican {
		private:
			can_dummy(void) :ican() {}
		public:
			virtual bool open(bool _owned_view = false) { return false; }
			virtual void close(void) {}
			virtual bool send(uint32_t /*_id*/, uint8_t* /*_buf*/, uint8_t  /*_len*/) { return false; }
			virtual bool ready(void) { return false; }
			virtual void reset(void) {}
			virtual void pool(void) {}
			static can_dummy& instance(void);
		};
	}
}

#endif
