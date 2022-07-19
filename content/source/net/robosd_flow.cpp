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


			performer::performer(cstr _command_path, kind_t _kind) 
				: ref_(*this, hash(_command_path,0) )
				, kind_(_kind)
				, port_(nullptr)
				, rout_record_(nullptr){
				ROBO_APP_ASSERT( ref_.attach_to( machine::instance_().performers_ ) )
			}
			
			bool performer::put_answer(const uint8_t* _data, size_t _size) {
				msg* m = port_->query();
				ROBO_LBREAKN(m);
				ROBO_LBREAKN(m->put(_data, _size));
				msg** p = port_->outcomm_ + rout_record_->answ_suba_;
				{
					guard__;
					if (*p != nullptr) {
						(*p)->release_();
					}
					*p = m;
				}
				return true;
			}
			bool performer::put_answer(msg* _m) {
				msg** p = port_->outcomm_ + rout_record_->answ_suba_;
				{
					guard__;
					if (*p != nullptr) {
						(*p)->release_();
					}
					*p = _m;
				}
				return true;
			}
			msg* performer::msg_query(void) {
				return port_->query();
			}
			size_t performer::max_size(void) {
				return port_->max_size();
			}

			static performer::map& performers_(void) {
				static performer::map performers__;
				return performers__;
			}

			performer* performer::find(cstr _path) {
				performer* tmp = machine::instance_().performers_.find(hash(_path,0));
				ROBO_APP_ASSERT(tmp != nullptr);
				return tmp;
			}

			performer* performer::find( int _ix) {
				performer* tmp = machine::instance_().performers_.find(_ix);
				ROBO_APP_ASSERT(tmp != nullptr);
				return tmp;
			}

			port::port(cstr _path, size_t _max_size, size_t _suba_count)
				: ref_(*this, hash(_path))
				, max_size_(_max_size)
				, suba_count_(_suba_count) {
				if (suba_count_ > 0) {
					outcomm_ = new msg * [suba_count_];
					performer_index_ = new performer * [suba_count_];
					std::fill_n(outcomm_, suba_count_, nullptr);
					std::fill_n(performer_index_, suba_count_, nullptr);
				}
				ref_.attach_to( machine::instance_().ports_);
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

			bool msg::put(const uint8_t* _data, size_t _offset, size_t _size) {
				if (_size + _offset <= size_) {
					std::copy_n(_data, _size, data_ + _offset);
					return true;
				}
				else {
					return false;
				}
			}

			bool msg::put(iserial& _serial, size_t _offset, size_t _size) {
				if (_size + _offset <= size_) {
					_serial.get(data_ + _offset, _size);
					return true;
				}
				else {
					return false;
				}
			}

			void msg::set_size(size_t _size) {
				ROBO_APP_ASSERT(_size <= port_.max_size());
				size_ = _size;
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
							return true;;
						}
					} 
				}
				if (_msg)
					_msg->release();
				return false;
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
			port * port::find(cstr _path) {
				port* tmp = machine::instance_().ports_.find(hash(_path, 0));
				ROBO_APP_ASSERT(tmp != nullptr);
				return tmp;
			}
			port* port::find(int _ix) {
				port* tmp = machine::instance_().ports_.find( _ix );
				ROBO_APP_ASSERT(tmp != nullptr);
				return tmp;
			}
			machine& machine::instance_(void) {
				static machine instance__;
				return instance__;
			}

			void machine::run_(performer* _pfr) {
				_pfr->execute();
				if (_pfr->in_msg) {
					_pfr->in_msg->release();
					_pfr->in_msg = nullptr;
				}
				_pfr->request_ = performer::request::idle;

			}
			void machine::frontend_poll(void) {
				if(instance_().frontend_list_.count()>0){
					performer* pfr;
					{
						guard__;
						pfr = instance_().frontend_list_.pop();
					}
					if (pfr) {
						run_(pfr);
					}
				}
			}
			void machine::backend_poll(void) {
				if(instance_().backend_list_.count()>0){
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

			void machine::begin(void) {
				for (rout_record::ref* pr = instance_().rout_records_.first(); pr; pr = pr->next()) {
					rout_record& r = pr->owner();
					port* p = port::find(r.port_ix_);
					ROBO_APP_ASSERT(p != nullptr);
					ROBO_APP_ASSERT(r.suba_ > 0);
					ROBO_APP_ASSERT(r.suba_ < p->suba_count_);
					ROBO_APP_ASSERT(p->performer_index_[r.suba_] == nullptr);
					performer* prf = performer::find(r.command_ix_);
					ROBO_APP_ASSERT(prf != nullptr);
					p->performer_index_[r.suba_] = prf;
					prf->port_ = p;
					prf->rout_record_ = &r;
					prf->begin();
				}
				for (performer::ref* pr = instance_().performers_.first(); pr; pr = pr->next()) {
					performer& p = pr->owner();
					ROBO_APP_ASSERT(p.port_ != nullptr);
					ROBO_APP_ASSERT(p.rout_record_ != nullptr);
				}
			}

			rout_record::rout_record(
				cstr  _port_path
				, cstr  _command_path
				, uint8_t _suba
				, uint8_t _answ_suba
			):
				ref_(*this)
				, suba_(_suba)
				, answ_suba_(_answ_suba) {
				port_ix_ = hash(_port_path);
				command_ix_ = hash(_command_path);
				ref_.attach_to(machine::instance_().rout_records_);
			}

			void serial_proto::execute(void) {

				size_t out_available;
				size_t out_total = 0;
				size_t in_size = in_msg ? in_msg->size() : 0;
				size_t out_max_size = max_size();
				const uint8_t* in_data = in_msg->data();
				size_t in_space = remote_.space();
				if (in_size) {
					size_t in_data_size = in_size - 1;
					uint8_t cmd = *in_data;
					if (cmd == 0xBB) {
						if (in_space >= in_data_size) {
							if (in_data_size) {
								remote_.put(in_msg->data() + 1, in_data_size);
							}
							return;
						}
					}
					else if (cmd == 0xAA) {
						//remote_.reset();
						//local_.reset();
						declared_count_ = 0;
					}
				}

				out_available = remote_.available();
				if (out_available >= declared_count_) {
					out_total = declared_count_;
					msg* m = msg_query();
					if (m) {
						if (out_total > 0) {
							ROBO_VBREAKN(m);
							m->set_size(out_total + 1);
							m->put(remote_, 1, out_total);
						}
						else {
							m->set_size(1);
						}
						declared_count_ = out_available - declared_count_;
						if (in_space > 0xF)
							in_space = 0xF;
						if (declared_count_ > 0xF)
							declared_count_ = 0xF;
						if (declared_count_ > out_max_size - 1)
							declared_count_ = out_max_size - 1;
						uint8_t header = (uint8_t)((in_space << 4) + declared_count_);
						m->put(&header, 0, 1);
						put_answer(m);
						return;
					}
				}
				remote_.reset();
				local_.reset();
				declared_count_ = 0;
			}
			snapshot_proto::snapshot_proto(
				cstr _command_path
				, kind_t _kind
				, snapshot& _snapshot
			)
				: performer(
					_command_path
					, _kind
				)
				, snapshot_(_snapshot) {
			}
			void snapshot_proto::update_and_post(void) {
				snapshot_.update();
				if (snapshot_.size() <= max_size()) {
					put_answer(snapshot_.data(), snapshot_.size());
				}
				else {
					size_t sz = max_size() - 1;
					msg* m = msg_query();
					ROBO_VBREAKN(m != nullptr);
					m->set_size(sz + 1);
					page_ = 0;
					m->put(page_);
					m->put(snapshot_.data(), 1, sz);
					actual_size_ = snapshot_.size() - sz;
					actual_ = snapshot_.data() + sz;
				}
			}
			void snapshot_proto::begin(void) {
				if (snapshot_.size() <= max_size()) {
					page_count_ = 1;
					page_size_ = max_size();
				}
				else {
					page_size_ = max_size() - 1;
					page_count_ = (snapshot_.size() / (max_size() - 1));
					if (page_count_ * page_size_ < snapshot_.size()) page_count_++;
				}
			}
			void snapshot_proto::execute(void) {
				if (in_msg != nullptr) {
					if (in_msg->size() == 1) {
						post_page_(in_msg->data()[0]);
					}
				}
				else {
					if (page_ < page_count_) {
						post_page_(page_ + 1);
					}
				}
			}

			void snapshot_proto::post_page_(size_t _page) {
				if (page_ == 0) {
					update_and_post();
				}
				else {
					size_t sz = max_size() - 1;
					size_t offset = _page * sz;
					size_t out_sz = snapshot_.size() - offset;
					if (out_sz > sz) {
						out_sz = sz;
					}
					msg* m = msg_query();
					ROBO_VBREAKN(m != nullptr)

					page_ = _page;
					m->set_size(out_sz + 1);
					m->put(page_);
					m->put(snapshot_.data() + offset, 1, out_sz);

					offset += out_sz;
					actual_size_ = snapshot_.size() - offset;
					actual_ = snapshot_.data() + offset;
				}
			}
			
		}

	}
}

#endif
