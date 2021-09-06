#include "net/robosd_flow.hpp"
#include "core/robosd_system.hpp"
#include <algorithm>
#if ROBO_APP_NET_FLOW_ENABLED==1
namespace robo {
	namespace net {
		namespace flow {
			msg::msg(port& _port, uint8_t* _data)
				: ref_(*this)
				, port_(_port)
				, data_(_data) {
				ref_.attach_to(port_.pool_);
			}

			performer::performer(
				port& _port
				, uint8_t _command
				, uint8_t _suba
				, uint8_t _answ_suba
				, kind_t _kind
			)
				: ref_(*this)
				, port_(_port)
				, command_(_command)
				, suba_(_suba)
				, answ_suba_(_answ_suba)
				, kind_(_kind) {
				ROBO_APP_ASSERT(port_.performer_index_[suba_] == 0);
				port_.performer_index_[suba_] = this;
			}
			bool performer::send(const uint8_t* _data, size_t _size) {
				msg* m = port_.query();
				ROBO_LBREAKN(m);
				ROBO_LBREAKN(m->put(_data, _size));
				msg** p = port_.outcomm_ + answ_suba_;
				{
					guard__;
					if (*p != nullptr) {
						m->release_();
						return false;
					}
					else {
						*p = m;
					}
				}
				return true;
			}
			size_t performer::max_size(void) {
				return port_.max_size();
			}
			port::port(size_t _max_size, size_t _suba_count)
				: max_size_(_max_size)
				, suba_count_(_suba_count) {
				if (suba_count_ > 0) {
					outcomm_ = new msg * [suba_count_];
					performer_index_ = new performer * [suba_count_];
					std::fill_n(outcomm_, suba_count_, nullptr);
					std::fill_n(performer_index_, suba_count_, nullptr);
				}
			}

			msg* port::query(void) {
				guard__;
				return pool_.pop();
			};
			void msg::release(void) {
				guard__;
				ref_.attach_to(port_.pool_);
			}
			void msg::release_(void) {
				ref_.attach_to(port_.pool_);
			}

			size_t msg::max_size(void) {
				return port_.max_size_;
			}


			bool msg::put(const uint8_t* _data, size_t _size) {
				if (_size <= port_.max_size()) {
					std::copy_n(_data, _size, data_);
					size_ = _size;
					return true;
				}
				else {
					return false;
				}
			}

			bool port::put(uint8_t _suba, msg* _msg) {
				if (_suba < suba_count_) {
					performer* pfr = performer_index_[_suba];
					if (pfr) {
						guard__;
						if (pfr->request_ == performer::request::idle) {
							if (pfr->kind_ == performer::kind_t::backend) {
								pfr->ref_.attach_to(machine::instance_().backend_list_);
							}
							else {
								pfr->ref_.attach_to(machine::instance_().frontend_list_);
							}
							pfr->in_msg = _msg;
							pfr->request_ = (_msg != nullptr) ? performer::request::put : performer::request::get;
						}
						else {
							_msg->release();
						}
					}
				}
				return true;
			}

			msg* port::get(uint8_t _suba) {
				if (_suba < suba_count_) {
					msg** p = outcomm_ + _suba;
					{
						guard__;
						msg* tmp = *p;
						*p = nullptr;
						return tmp;
					}
				}
				else return nullptr;
			}

			port::~port(void) {
				if (outcomm_ != nullptr)
					delete[] outcomm_;
				if (performer_index_ != nullptr)
					delete[] performer_index_;
			}

			machine& machine::instance_(void) {
				static machine instance__;
				return instance__;
			}

			void machine::run_(performer* _pfr) {
				_pfr->execute();
				if (_pfr->in_msg) {
					_pfr->in_msg->release_();
					_pfr->in_msg = nullptr;
				}
				_pfr->request_ = performer::request::idle;

			}
			void machine::frontend_poll(void) {
				performer* pfr;
				{
					critical__;
					pfr = instance_().frontend_list_.pop();
				}
				if (pfr) {
					run_(pfr);
				}
			}
			void machine::backend_poll(void) {
				performer* pfr;
				{
					guard__;
					pfr = instance_().backend_list_.pop();
				}
				if (pfr) {
					run_(pfr);
				}
			}

		}
	}
}
#endif