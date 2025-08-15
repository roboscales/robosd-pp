#ifndef __robo_can_h
#define __robo_can_h
#include "core/robosd_common.hpp"
#include "core/robosd_list.hpp"
#include "core/robosd_delegat.hpp"
#include "core/robosd_string.hpp"
#include "core/robosd_app.hpp"

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
			typedef delegat::ref<void, ican&, uint32_t, const uint8_t*, uint8_t   > on_receive_f;
			typedef delegat::ref<void, ican&, event> on_event_f;
		private:
			ref ref_;

		protected:
			on_receive_f * on_receive;
			on_event_f * on_event;
		public:
			virtual void set_on_receive(on_receive_f* _on_receive);
			virtual void set_on_receive(void (*_simple)(ican&, uint32_t, const uint8_t*, uint8_t));
			virtual void set_on_event(on_event_f* _on_event);
			virtual void set_on_event(void (*_simple)( ican&, event) );

			//virtual bool open(bool _owned_view = false) = 0;
			virtual void close(void) = 0;
			virtual bool send(uint32_t _id, const  uint8_t* _buf, uint8_t  _len) = 0;
			virtual bool ready(void) = 0;
			virtual void reset(void) = 0;
			virtual void poll(void) = 0;
			ican(void);
			virtual bool reg(cstr _caption);
			virtual void unreg(void);
			static ican* find(cstr _caption);
			static ican* query(cstr _caption);
			static ican& query_ref(cstr _caption);
			virtual void  release(void);
			static void forall(lambda<void(ican&)>& _operator);
		};

		class ROBO_EXPORT can_dummy :public ican {
		private:
			can_dummy(void) :ican() {}
		public:
			virtual bool open(bool _owned_view = false) { return false; }
			virtual void close(void) {}
			virtual bool send(uint32_t /*_id*/, const uint8_t* /*_buf*/, uint8_t  /*_len*/) { return false; }
			virtual bool ready(void) { return false; }
			virtual void reset(void) {}
			virtual void poll(void) {}
			static can_dummy& instance(void);
		};
	
		//class ROBO_EXPORT can_phys : public  app::node {
		//protected: 
			//can_phys(cstr _name, app::node* _owner) : app::node(_name, _owner) {}
		//};
		#if ROBO_APP_MODULE_ENABLED
		template<typename P, cstr N> class ROBO_EXPORT cans_module_t
			: public robo::app::module {
			cans_module_t(void)
				: robo::app::module(N) {}

			P ** cans_ = nullptr;
			int can_count_ = 0;

		protected:
			/*
			robo::net::can_instanceflow_bus::packet pk;
			robo::net::can_instanceflow_bus::packet res;
			*/
			virtual void backend_loop(void) {
				P** p = cans_;
				for (int i = 0; i < can_count_; ++i, ++p) {
					(*p)->poll();
				}
			}
			virtual void frontend_loop(void) {
			}
			virtual bool do_load(void) {
				ROBO_LBREAKN(robo::app::module::do_load());
				ROBO_LBREAKN(robo::ini::load(current_path(), RT("channels_count"), can_count_));
				if (can_count_ > 0) {
					cans_ = new P * [can_count_];
					P ** b = cans_;
					for (int i = 0; i < can_count_; ++i, ++b) {
						(*b) = nullptr;
					}

					b = cans_;
					for (int i = 0; i < can_count_; ++i, ++b) {
						robo::string name(RT("channel-%d"), i + 1);
						(*b) = new P(name,this);
						ROBO_LBREAKN((*b) != nullptr);
					}
				}
				return true;
			}

			virtual void do_clean(void) {
				if (cans_ != nullptr) {
					P ** b = cans_;
					for (int i = 0; i < can_count_; ++i, ++b) {
						if ((*b) != nullptr) delete (*b);
					}
					delete[] cans_;
					cans_ = nullptr;
				}				
			}
		public:
			static cans_module_t& instance(void) {
				static cans_module_t instance_;
				return instance_;
			}
		};
		#endif
	}
}

#endif
