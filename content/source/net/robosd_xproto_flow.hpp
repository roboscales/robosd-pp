#ifndef robosd_net_xproto_flaw_hpp
#define robosd_net_xproto_flaw_hpp
#include "core/robosd_app.hpp"
#include "core/robosd_log.hpp"
#include "net/robosd_can.hpp"
#include "net/robosd_can_flow_bus.hpp"
#include "net/robosd_can_flow_module.hpp"
#include "..\srt-actuator\source\common\srt-actuator.proto.hpp"
namespace robo {
	namespace net {
		namespace can {
			namespace flow {
				namespace xproto {

					class ROBO_EXPORT instance {
						using map = list::unique<instance, uint16_t>;
						using ref = map::ref;
						ref ref_;
						static map& tables_(void);
					protected:
						instance(cstr _instance_name);
						virtual void set_handle(::robo::net::ican& _ican) = 0;
					public:
						virtual bool is_flow_id(uint16_t _id) = 0;
						virtual void do_can_receive(uint32_t _id, const uint8_t* _data, uint8_t _len) = 0;
						virtual uint16_t encode_id(uint32_t _id, const uint8_t* _data) = 0;
						virtual void recode_id(uint32_t _id, uint16_t & _new_id, uint8_t & _header) = 0;

						//virtual canfest_table* create(void) = 0;
						static instance* attach(cstr _instance_name, ::robo::net::ican& _ican);
					};

					template<typename D> class ROBO_EXPORT phys_t : public ::robo::net::can::flow::phys_t<D>
					{
						using B = ::robo::net::can::flow::phys_t<D>;
						instance* instance_ = nullptr;
					protected:
						virtual void do_can_receive(::robo::net::ican& _ican, uint32_t _id, const uint8_t* _data, uint8_t _len) {
							ROBO_APP_ASSERT(instance_);
							if (instance_->is_flow_id(_id)) {
								if (_len > 1) {
									B::do_can_receive(_ican, instance_->encode_id(_id, _data), _data + 1, _len - 1);
								}
							}
							else {
								instance_->do_can_receive(_id, _data, _len);
							}
						}

						virtual bool do_send(ican& _ican, uint32_t _id, const uint8_t* _data, uint8_t _len) {
							ROBO_APP_ASSERT(instance_);
							if (_len > 7) return false;
							uint16_t id;
							uint8_t data[8];
							instance_->recode_id(_id, id, data[0]);
							uint8_t* d = data + 1;
							const uint8_t* s = _data;
							for (int i = 0; i < _len; ++i, ++d, ++s) *d = *s;


							ROBO_LRET( B::do_send(_ican, id, data , _len + 1) );
						}
						robo::string proto_name;
						virtual bool do_load(robo::cstr _current, robo::cstr _common) {
							ROBO_LBREAKN(B::do_load(_current, _common));
							ROBO_LBREAKN(proto_name.load(_common, _current, RT("proto")));
							return true;
						}
						virtual bool do_open(void) {
							ROBO_LBREAKN(B::do_open());
							instance_ = instance::attach(proto_name, B::can_instance);
							ROBO_LBREAKN(instance_);
							return true;
						}
						virtual void do_close(void) {
							instance_ = nullptr;
							B::do_close();
						}
						
					};
				}
			}
		}
	}
}
#endif
