#include "servo/mexo_backend.hpp"
#include "mexo/vartree.hpp"

namespace mexo {
	namespace backend {

		bool devagent::echo::do_load(void) {
			ROBO_LBREAKN(stream::do_load());
			ROBO_LBREAKN(robo::ini::load(current_path(), common_path(), RT("echo_period_us"), period_));
			ROBO_LBREAKN(robo::ini::load(current_path(), common_path(), RT("echo_show_enable"), show_enable_));
			return true;
		}

		bool devagent::echo::exchange_need(void) {
			return period_ > 0;
		}

		devagent::echo::echo(devagent& _agent) 
			: stream(RT("echo"), _agent, priority::lo) {
		}

		devagent::echo::query_result devagent::echo::query(robo_tran_t& _tran) {
			switch (state_) {
			case state::idle:
			{
				//todo включить в таск
				robo::time_us_t now = robo::system::env::time_us();
				if (now - last_ > period_) {
					last_ = now;
				}
				else {
					return query_result::none;
				}
			}
			_tran.header.command = mexo::front::dev::flow_command_ix::echo;
			_tran.request = ROBO_TRAN_REQUEST_PUT;
			_tran.size_actual =  sizeof(last_);
			*((robo::time_us_t*)_tran.data) = last_;
			return query_result::success;
			case state::request:
			_tran.header.command = mexo::front::dev::flow_command_ix::echo;
			_tran.request = ROBO_TRAN_REQUEST_GET;
			_tran.size_actual = sizeof(last_);			
			return query_result::success;
			}
			return query_result::none;
		}

		void devagent::echo::confirm(const robo_tran_t& _tran) {
			if (_tran.status == ROBO_TRAN_COMPLETE) {
				switch (state_) {
				case state::idle:
				state_ = state::request;
				break;
				case state::request:
				state_ = state::idle;
				if (show_enable_) {
					int tm = (int)(*((robo::time_us_t*)_tran.data));
					int err = tm - (int)last_;
					robo_detaillog(6, robo::log::mask::disabled, "echo %d (err: %d)", tm, err);
				}
				}
			}
			else {
				state_ = state::idle;
			}
		}

		devagent::varindex::descriptor::descriptor(varindex& _varindex, robo::cstr _name, confirm_d* _confirm)
			: ref_(*this)
			, varindex_(_varindex)
			, confirm_(_confirm)
			, lambda_(nullptr){
			robo::system::guard g__;
			ref_.attach_to(varindex_.request_);
			record_ = {};
			name_ = _name;
			record_.name = name_.c_str();
		}
		devagent::varindex::descriptor::descriptor(varindex& _varindex, robo::cstr _name, lambda * _lambda)
			: ref_(*this)
			, varindex_(_varindex)
			, confirm_(nullptr)
			, lambda_(_lambda)
			{
			robo::system::guard g__;
			ref_.attach_to(varindex_.request_);
			record_ = {};
			name_ = _name;
			record_.name = name_.c_str();
		}

		void devagent::varindex::descriptor::setup_recprd(robo::cstr _type, uint16_t _address, uint16_t _length) {
			type_ = _type;
			record_.type = type_.c_str();
			record_.address = _address;
			record_.length = _length;
		}

		void devagent::varindex::descriptor::confirm(void) {
			fabric* f = fabric::find(record_.type);
			ROBO_VBREAKN_F((f != nullptr), "invalid fabric: '%s' ", record_.type);
			var_ = dynamic_cast<ivar*>(f->create(varindex_, record_));
			ROBO_VBREAKN_F((var_ != nullptr), "error create var '%s' with type '%s' ", record_.name, record_.type);

			ref_.attach_to(varindex_.index_);
			if (confirm_) {
				var_->query(confirm_);
			}
			else if (lambda_) {
				var_->query(lambda_);
			} else
			{
				var_->query();
			}

		}

		void devagent::varindex::descriptor::refuse(void) {
			if (requery_count_ == 0) {
				if (confirm_) {
					(*confirm_)(var_, false);
				}
			}
			else {
				requery_count_--;
				ref_.attach_to(varindex_.request_);
			}
		}

		devagent::varindex::query_result devagent::varindex::query(robo_tran_t& _tran) {
			if (current_descriptor_ == nullptr) {
				current_descriptor_ = request_.pop();
			}
			if (current_descriptor_ == nullptr) {
				return vartable::query(_tran);;
			}
			else {
				switch (proto_cast<proto>().reques_desc(_tran, current_descriptor_)) {
				case proto::result::success:
				return query_result::success;
				break;
				case proto::result::repeat:
				return query_result::repeat;
				break;
				default:
				ROBO_ALARM_F("invalid proto for %s/%s", own_agent().alias(), current_descriptor_->name());
				current_descriptor_->refuse();
				current_descriptor_ = nullptr;
				return query_result::none;
				}
			}
		}
		
		void devagent::varindex::confirm(const robo_tran_t& _tran) {
			if (current_descriptor_ != nullptr) {
				switch (proto_cast<proto>().confirm_desc(_tran, current_descriptor_)) {
				case proto::result::success:
				current_descriptor_->confirm();
				current_descriptor_ = nullptr;
				break;
				case proto::result::repeat:
				break;
				default:
				ROBO_ALARM_F("invalid proto for %s/%s", own_agent().alias(), current_descriptor_->name());
				current_descriptor_->refuse();
				current_descriptor_ = nullptr;
				}
			}
			else {
				vartable::confirm(_tran);
			}
		}
		
		bool devagent::varindex::do_ready(void) {
			return request_.count() == 0 && current_descriptor_ == nullptr; 
		}
		
		bool devagent::varindex::do_exchange_need(void) {
			return request_.count() != 0 || current_descriptor_ != nullptr;
		}
		
		void devagent::varindex::query(robo::cstr _name, varindex::descriptor::confirm_d* _confirm) {
			ivar* v = dynamic_cast<ivar*>(find_var(_name));
			if (v == nullptr) {
				new descriptor(*this, _name, _confirm);
			}
			else {
				v->query(_confirm);
			}
		}

		devagent::varindex::varindex(devagent& _agent, proto& _proto, priority _priority, const record* const _records, size_t _count)
			: vartable(_agent, _proto, _priority, _records, _count) {

		}

		devagent::proto::result devagent::proto::request(robo_tran_t& _tran, ::robo::backend::vartable::ivar* _var) {
			if (step_ == step::idle) {
				ROBO_JAMPN_F(_var->length() <= _tran.size_max - 2, fail, "invalid var size %s", _var->name());
				ROBO_JAMPN_F(_var->addr() < 255, fail, "invalid var index ");
				index_ = (uint8_t)_var->addr();
				len_ = (uint8_t)_var->length();
				op_ = _var->actual_status();
				step_ = step::put;
				_tran.data[1] = index_;
				_tran.request = ROBO_TRAN_REQUEST_PUT;
				_tran.header.command = mexo::front::dev::flow_command_ix::var;
				if (op_ == op::put) {
					_tran.data[0] = mexo::var::request::put;
					_var->encode(_tran.data + 2);
					_tran.size_actual = 2 + len_;
					return result::success;
				}
				else if (op_ == op::get) {
					_tran.data[0] = mexo::var::request::get;
					_tran.size_actual = 2;
					return result::repeat;

				}
			}
			else if (step_ == step::get) {
				_tran.request = ROBO_TRAN_REQUEST_GET;
				_tran.size_actual = 2 + len_;
				return result::success;
			}
			robo_errlog("error proto series (%s)", _var->name());
		fail:
			reset();
			return result::fail;
		}
		devagent::proto::result devagent::proto::confirm(const robo_tran_t& _tran, ::robo::backend::vartable::ivar* _var) {
			if (_tran.status == ROBO_TRAN_COMPLETE) {
				ROBO_JAMPN_F(((_tran.data[0] & mexo::var::error_mask) == 0), fail, "invalid var answer (%s , %d)", _var->name(), (int)(_tran.data[0] & mexo::var::error_mask));
				ROBO_JAMPN_F(((_tran.data[1]) == index_), fail, "invalid var index (%s , %d,%d) ", _var->name(), (int)(_tran.data[1]));
				if (op_ == op::put) {
					if (step_ == step::put) {
						ROBO_JAMPN_F((_tran.data[0] == mexo::var::request::put), fail, "invalid var  operaton (%s, %d) ", _var->name(), _tran.data[0]);
						reset();
						return result::success;
					}
				}
				else {
					if (op_ == op::get) {
						ROBO_JAMPN_F((_tran.data[0] == mexo::var::request::get), fail, "invalid var operaton (%s, %d) ", _var->name(), _tran.data[0]);
						if (step_ == step::put) {
							step_ = step::get;
							return result::repeat;
						}
						else {
							ROBO_JAMPN_F((len_ + 2 == _tran.size_actual), fail, "invalid var  size (%s, %d,%d) ", _var->name(), len_ + 2, _tran.size_actual);
							_var->decode(_tran.data + 2);
							reset();
							return result::success;
						}
					}
				}
			}
			robo_errlog("error proto series (%s)", _var->name());
		fail:
			reset();
			return result::fail;
		}
		devagent::proto::result devagent::proto::reques_desc(robo_tran_t& _tran, const varindex::descriptor* _desc) {
			if (step_ == step::idle) {
				index_ = 0;
				len_ = 0;
				op_ = op::clean;
				step_ = step::desc_put;
				_tran.request = ROBO_TRAN_REQUEST_PUT;
				_tran.data[0] = mexo::var::request::index;
				*(int32_t*)(_tran.data + 1) = (uint32_t)robo::hash(_desc->name());
				_tran.size_actual = 5;
				_tran.header.command = _tran.header.command = mexo::front::dev::flow_command_ix::var;
				return result::repeat;
			}
			else if (step_ == step::desc_get) {
				step_ = step::desc_get;
				_tran.request = ROBO_TRAN_REQUEST_GET;
				_tran.size_actual = 4;
				_tran.header.command = 0x02;
				return result::success;
			}
			robo_errlog("error proto series (%s)", _desc->name());
			reset();
			return result::fail;
		}
		devagent::proto::result devagent::proto::confirm_desc(const robo_tran_t& _tran, varindex::descriptor* _desc) {
			if (_tran.status == ROBO_TRAN_COMPLETE) {
				ROBO_JAMPN_F(((_tran.data[0] & mexo::var::error_mask) == 0), fail, "invalid desc answer (%s, %d)", _desc->name(), (int)(_tran.data[0] & mexo::var::error_mask));
				if (step_ == step::desc_put) {
					ROBO_JAMPN_F((_tran.data[0] == mexo::var::request::index), fail, "invalid desc operation  (%s, %d) ", _desc->name(), _tran.data[0]);
					step_ = step::desc_get;
					return result::repeat;
				}
				else if (step_ == step::desc_get) {
					ROBO_JAMPN_F((4 == _tran.size_actual), fail, "invalid desc  size (%s, %d,%d) ", _desc->name(), 4, _tran.size_actual);
					mexo::var::descriptor d;
					d.bytes[0] = _tran.data[2];
					d.bytes[1] = _tran.data[3];
					_desc->setup_recprd(type_name(d), _tran.data[1], d.len);
					reset();
					return result::success;
				}
			}
			robo_errlog("error proto series (%s)", _desc->name());
		fail:
			reset();
			return result::fail;
		}

		void devagent::flow_serial::decode_count_(uint8_t _data) {
			remote_out_count_ = _data & 0xF;
			remote_in_space_ = (_data >> 4);
		}
		void devagent::flow_serial::reset_(void) {
			remote_out_count_ = 0;
			remote_in_space_ = 0;
			state_ = state::none;
		}
		bool devagent::flow_serial::exchange_need() {
			//todo
			return port_ != nullptr;
		};

		devagent::stream::query_result devagent::flow_serial::query(robo_tran_t& _tran) {
			//todo подумать
			_tran.header.command = _tran.header.command = mexo::front::dev::flow_command_ix::serial_1;
			switch (state_) {
			case state::none:
				_tran.size_actual = 1;
				_tran.request = ROBO_TRAN_REQUEST_PUT;
				_tran.data[0] = 0xAA;
				return query_result::success;
			case state::query_size:
				_tran.size_actual = 1;
				_tran.request = ROBO_TRAN_REQUEST_GET;
				return query_result::success;
			case state::normal:
				if (remote_out_count_ > 0) {
					size_t get_count = remote_out_count_ + 1;
					if (get_count > _tran.size_max) {
						get_count = _tran.size_max;
					}
					remote_out_count_ -= (get_count - 1);
					_tran.size_actual = get_count;
					_tran.request = ROBO_TRAN_REQUEST_GET;
					return query_result::success;
				}
				else {
					size_t put_count = port_->available();
					if (put_count > 0 && remote_in_space_ > 0) {
						if (put_count > remote_in_space_) {
							put_count = remote_in_space_;
						}
						if (put_count > _tran.size_max - 1) {
							put_count = _tran.size_max - 1;
						}
						remote_in_space_ -= put_count;
						_tran.size_actual = put_count + 1;
						//пошел на принцип оставил так 0xBB
						_tran.data[0] = 0xBB;
						if (port_->get(_tran.data + 1, put_count) > 0) {
							_tran.request = ROBO_TRAN_REQUEST_PUT;
							return query_result::success;
						}
					}
				}

				_tran.data[0] = 0;
				_tran.size_actual = 1;
				_tran.request = ROBO_TRAN_REQUEST_GET; //
				state_ = state::query_size;
				return query_result::success;
			}
			return query_result::none;
		}

		void  devagent::flow_serial::confirm(const robo_tran_t& _tran) {
			if (_tran.status == ROBO_TRAN_COMPLETE) {
				switch (state_) {
				case state::none:
					if (_tran.request == ROBO_TRAN_REQUEST_PUT) {
						state_ = state::query_size;
						return;
					}
					break;
				case state::query_size:
					if (_tran.request == ROBO_TRAN_REQUEST_GET && _tran.size_actual == 1) {
						decode_count_(_tran.data[0]);
						state_ = state::normal;
						return;
					}
					break;
				case state::normal:
					if (_tran.request == ROBO_TRAN_REQUEST_GET) {
						if (_tran.size_actual >= 1) {
							decode_count_(_tran.data[0]);
							size_t get_count = _tran.size_actual - 1;
							if ( port_->put(_tran.data + 1, get_count) ) {
								return;
							}
						}
					}
					else {
						return;
					}
					break;
				}
			}
			robo_errlog("freemaster proto is broke");
			reset_();
		}

		devagent::flow_serial::flow_serial(robo::cstr _name, devagent& _agent, priority _priority)
			: ::robo::backend::devagent::tunnel(_name, _agent, _priority){
		}

		devagent::flow_serial::~flow_serial(void) {
		}

		bool devagent::flow_serial::do_load(void) {
			ROBO_LBREAKN(robo::backend::devagent::tunnel::do_load());
			//ROBO_LBREAKN(ini::load(RT("flow"), RT(""),);
			return true;
		}

		void devagent::flow_serial::do_clean(void) {
			robo::backend::devagent::tunnel::do_clean();
		}

	}
}