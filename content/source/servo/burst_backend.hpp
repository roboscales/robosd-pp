#ifndef burst_backend_hpp
#define burst_backend_hpp
#include "servo/burst_frontend.hpp"
#include "servo/robosd_backend.hpp"
namespace burst {
	namespace backend {
		class devagent : public robo::backend::devagent {
		protected:
			class varindex : public ::robo::backend::vartable {
			public:
				class descriptor {
				public:
					typedef ::robo::list::unsorted<descriptor> queue;
					//typedef robo::delegat::ref<void, bool, ivar* > confirm_d;
					typedef ::robo::backend::vartable::ivar::performer  performer;
				private:
					robo::string name_;
					robo::string type_;
					::robo::backend::vartable::record record_;
					queue::ref ref_;
					performer* performer_ = nullptr;
					ivar* var_ = nullptr;
					int requery_count_ = 3;
					varindex& varindex_;
				public:
					ivar* var(void) { return var_; }
					robo::cstr name(void) const { return name_.c_str(); }
					descriptor(varindex& _varindex, robo::cstr _name, performer* _performer = nullptr);
					void setup_recprd(robo::cstr _type, uint16_t _address, uint16_t _length);
					void confirm(void);
					void refuse(void);
				};
			private:
				descriptor::queue index_;
				descriptor::queue request_;
				descriptor* current_descriptor_ = nullptr;
				enum { packet_offset = 4 };
			public:
				class proto : public ::robo::backend::vartable::proto {
				public:
					virtual result reques_desc(robo_tran_t& _tran, const descriptor* _desc) = 0;
					virtual result confirm_desc(const robo_tran_t& _tran, descriptor* desc) = 0;
				};
			protected:
				virtual vartable::query_result query(robo_tran_t& _tran);
				void confirm(const robo_tran_t& _tran);
				virtual bool do_ready(void);
				virtual bool do_exchange_need(void);
				bool query_a(robo::cstr _name, varindex::descriptor::performer* _prtformer = nullptr);
			public:
				template <typename ... Args> bool query(robo::cstr _name, Args...arg) {
					return query_a(_name, varindex::ivar::answer::autonum::fabric::create(arg...));
					return false;
				}
				varindex(devagent& _agent, proto& _proto, priority _priority, const record* const _records, size_t _count);
			};

		private:
			class echo : public stream {
				robo::time_us_t last_ = 0;
				robo::time_us_t period_ = 0;
				bool show_enable_ = false;
				enum class state { idle, request } state_ = state::idle;
			protected:
				bool do_load(void);
			public:
				echo(devagent& _agent);
				virtual bool exchange_need(void);
				virtual query_result query(robo_tran_t& _tran);
				virtual void confirm(const robo_tran_t& _tran);
			} echo_;

			class proto : public varindex::proto {
				uint8_t index_ = 0;
				uint8_t len_ = 0;
				typedef ::robo::backend::vartable::ivar::status op;
				op op_ = op::clean;
				enum class step { put = 1, get = 2, idle = 0, desc_put = 3, desc_get = 4 } step_ = step::idle;
			public:
				void reset(void) {
					index_ = 0;
					len_ = 0;
					op_ = op::clean;
					step_ = step::idle;
				}
				virtual result request(robo_tran_t& _tran, ::robo::backend::vartable::ivar* _var);
				virtual result confirm(const robo_tran_t& _tran, ::robo::backend::vartable::ivar* _var);
				virtual result reques_desc(robo_tran_t& _tran, const varindex::descriptor* _desc);
				virtual result confirm_desc(const robo_tran_t& _tran, varindex::descriptor* _desc);
			};

			class flow_serial : public robo::backend::devagent::tunnel
			{
				enum class state { none, query_size, normal } state_ = state::none;
				//int command_ = -1;
				size_t remote_out_count_ = 0;
				size_t remote_in_space_ = 0;
				//robo::time_us_t last_request_us_ = 0;
				//robo::time_us_t request_period_us_ = 0;
				void decode_count_(uint8_t  _data);
				void reset_(void);
			protected:
				virtual bool exchange_need(void);
				virtual query_result query(robo_tran_t& _tran);
				virtual void confirm(const robo_tran_t& _tran);
				virtual bool do_load(void);
				virtual void do_clean(void);
			public:
				flow_serial(robo::cstr _name, devagent& _agent, priority _priority);
				virtual ~flow_serial(void);
			} * * flow_serials_ = nullptr;
			int flow_serials_count_ = 0;

		protected:
			typedef ::robo::quest quest;

			static auto answer_(quest* _quest) {
				return 
					[_quest](varindex::ivar* _var, bool _result) {
					if (_result) {
						_quest->confirm();
					}
					else {
						_quest->refuse();
					}
				};
			}
			static auto post_var_(quest* _quest, varindex::ivar * _v, ::robo::cstr _value) {
				if (!_v->post(
					_value
					, answer_(_quest)
				)) {
					_quest->refuse();
				}
			}
			robo::quest::reaction reacton_(::robo::quest::result _r, ::robo::string * _sv, ::robo::string* _vv=nullptr) {
				if (_r == robo::quest::result::success) {
					if(_vv != nullptr ){
						if (_sv != nullptr) {
							robo_detaillog(6, robo::log::mask::disabled, "\t\tquest post var  %s/%s = %s  - success", display_alias(), _sv->c_str(), _vv->c_str() );
							delete _sv;
						}
						delete _vv;
					}
					else {
						if (_sv != nullptr) {
							robo_detaillog(6, robo::log::mask::disabled, "\t\tquest query var %s/%s  - success", display_alias(), _sv->c_str());
							delete _sv;
						}
					}
					return robo::quest::reaction::normal;
				}
				else {
					if (_vv != nullptr) {
						if (_sv != nullptr) {
							robo_errlog("\t\tquest post var  %s/%s =%s - refused, canceled or termibated (%d) ", display_alias(), _sv->c_str(), _vv->c_str() , (int)_r);
							delete _sv;
						}
						delete _vv;
					}
					else {
						if (_sv != nullptr) {
							robo_errlog("\t\tquest var query %s/%s  - refused, canceled or termibated (%d) ", display_alias(), _sv->c_str(), (int)_r);
							delete _sv;
						}
					}
					return robo::quest::reaction::terminate;
				}
			}

			quest* var_query_quest(::robo::cstr _var, quest* _owner, quest* _sema = nullptr) {
				::robo::string* sv = new ::robo::string(_var);
				return ::robo::quest::create(
					_owner
					, _sema
					, ::robo::quest::answer_fabric::create( [this,sv](::robo::quest::result _r)->robo::quest::reaction {
						return this->reacton_(_r, sv);
					})
					, ::robo::quest::request_fabric::create( [this, sv](::robo::quest* _quest) {

						robo_detaillog(6, robo::log::mask::disabled, "\t\tquest: %s/%s - start query", this->display_alias(), sv->c_str());

						if (!vars.query(sv->c_str(), 
							 [this, _quest, sv](varindex::ivar* _var, bool _result) {
							if (_result) {
								_quest->confirm();
							}
							else {
								_quest->refuse();
							}
						}
												
						)
						) {
							_quest->refuse();
						}
					})
					);
			}
			
			quest* var_post_quest(::robo::cstr _var, ::robo::cstr _value, quest* _owner,quest* _sema = nullptr) {

				::robo::string* sv = new ::robo::string(_var);
				::robo::string* vv = new ::robo::string(_value);
				//*sv = _var;
				//delete sv;
				return ::robo::quest::create(
					_owner
					, _sema
					, ::robo::quest::answer_fabric::create( [this, sv, vv](::robo::quest::result _r)->robo::quest::reaction {
						return this->reacton_(_r, sv, vv);
					}
					)
					, ::robo::quest::request_fabric::create( [this, sv, vv](::robo::quest* _quest) {

						robo_detaillog(6, robo::log::mask::disabled, "\t\tquest: %s/%s=%s - start to post", this->display_alias(), sv->c_str(), vv->c_str());
						varindex::ivar* v = dynamic_cast<varindex::ivar*>(vars.find_var(sv->c_str()));
						if (v) {
							post_var_(_quest, v, vv->c_str());
						}
						else {
							if (!vars.query(sv->c_str(), 
											
							 [this, _quest, vv](varindex::ivar* _var, bool _result) {
								if (_result) {
									post_var_(_quest, _var, vv->c_str());
								}
								else {
									_quest->refuse();
								}
							}
							
							)) {
								_quest->refuse();
							}
						}
					})

				);
			}
			

			class post_vars_quest {
				//using vat_answer_fabric = ::robo::delegat::autonum_fabric< void, varindex::ivar*, bool >;
				devagent& devagent_;
				enum { bufsz = ROBO_STRING_BUFFER_SIZE };
				robo::char_t keys_[bufsz] = {};
				robo::char_t* begin_ch_ = nullptr;
				robo::char_t* current_ch_ = nullptr;
				size_t counter_ = 0;
				robo::string value_;
				robo::string section_common_sturtup_;
				robo::string section_startup_;
				robo::cstr section_ = nullptr;
				quest* quest_ = nullptr;
				bool begin_( robo::cstr _sect) {
					section_ = _sect;
					::robo::system::ini::load_data(keys_, bufsz, section_, nullptr, counter_);
					ROBO_LBREAKN_F( ( counter_ < bufsz - 2), "var's list for %s is oversized (%u)", devagent_.display_alias(), counter_);
					begin_ch_ = current_ch_ = keys_;
					return true;
				}
				enum class result { fault, complete, next};
				
				auto post_confirm_() {
					return
						[this](varindex::ivar* _var, bool _result) {
						if (_result) {
							robo_detaillog(6, robo::log::mask::disabled, "\t\t\tquest %s/%s=%s post complete ", this->devagent_.display_alias(), _var->name(), this->value_.c_str());
							if (counter_ > 0) {
								begin_ch_ = current_ch_ + 1;
								current_ch_++;
								counter_--;
							}
							this->run_();
						}
						else {
							robo_errlog("\t\t\tquest  %s/%s=%s post refuse ", this->devagent_.display_alias(), _var->name(), this->value_.c_str());
							this->refuse_();
						}
					};
					
				}
				result  post_var_(varindex::ivar* _var, ::robo::cstr _value) {
					_var->set_paranoic_put(true);
					robo_detaillog(6, robo::log::mask::disabled, "\t\t\tquest %s/%s=%s - start to posting", devagent_.display_alias(), _var->name(), _value);
					return
						_var->post(
							_value,
						post_confirm_()
					) ? result::next : result::fault;

				}
				result  post_var_(::robo::cstr _var, ::robo::cstr _value) {
					varindex::ivar* v = dynamic_cast<varindex::ivar*>(devagent_.vars.find_var(_var));
					if (v) {
						return post_var_(v, _value);
					}
					else {
						return devagent_.vars.query(_var, 						
							[this, _value](varindex::ivar* _var, bool _result) {
							if (_result) {
								post_var_(_var, _value);
							}
							else {
								this->refuse_();
							}
						}
						) ? result::next : result::fault;
					}
				}


				result next_(void) {
					while (counter_ > 0) {
						if (*current_ch_ == 0) {
							const robo::cstr r = RT("#");
							if (*begin_ch_ != r[0]) {
								robo::string value;
								if (!value_.load(section_, begin_ch_)) {
									return result::fault;
								}
								else {
									return post_var_(begin_ch_, value_.c_str());
								}
							}
							begin_ch_ = current_ch_ + 1;
						}
						current_ch_++;
						counter_--;
						//return counter_ == 0 ? result::complete : result::next;
					}
					return result::complete;
				}
				enum class status { none, common, specific, complete, panic } status_ = status::none;
				void run_(void) {
					switch (status_) {
					case status::none:
						if (!begin_(section_common_sturtup_)) {
							refuse_();
							break;
						}
						else {
							status_ = status::common;
						}
					case status::common:
						switch (next_()) {
						case result::complete:
							if (!begin_(section_startup_)) {
								refuse_();
								break;
							}else {
								status_ = status::specific;
								run_();// todo говнокод
							}
							break;
						case result::fault:
							refuse_();
							break;
						case result::next:
							break;
						}
						break;
					case status::specific:
						switch (next_()) {
						case result::complete:
							confirm_();
							break;
						case result::fault:
							refuse_();
							break;
							case result::next:
							break;
						}
						break;
					case status::complete:
						break;
					case status::panic:
						break;
					}
				}

				void confirm_(void) {
					if (quest_ != nullptr) {
						quest_->confirm();
					}
				}

				void refuse_(void) {
					status_ = status::panic;
					if (quest_ != nullptr) {
						quest_->refuse();
					}
				}


				post_vars_quest(devagent& _devagent): devagent_(_devagent){

				}
			public:
				static quest * begin(devagent& _devagent, quest * _owner, quest * _sema) {
					post_vars_quest* inst = new post_vars_quest(_devagent);
					quest* q = nullptr;
					ROBO_JAMPN_F(inst->section_common_sturtup_.load(_devagent.current_path(), _devagent.defaults_path(), RT("common_startup")), fault, "var's section 'common_startup' for %s isn't found", _devagent.display_alias());
					ROBO_JAMPN_F(inst->section_startup_.load(_devagent.current_path(), _devagent.defaults_path(), RT("startup")), fault, "var's section 'startup' for %s isn't found", _devagent.display_alias());
					q = quest::create(_owner, _sema
						, quest::answer_fabric::create( [inst](::robo::quest::result _r)->robo::quest::reaction {
							if (_r == robo::quest::result::success) {
								robo_detaillog(6, robo::log::mask::disabled, "\t\tquest: 'startup vars load for %s' - complete", inst->devagent_.display_alias());
								delete inst;
								return robo::quest::reaction::normal;
							}
							else {
								robo_errlog("\t\tquest: 'startup vars load for %s' - refused, canceled or termibated (%d) ", inst->devagent_.display_alias(), (int)_r);
								delete inst;
								return robo::quest::reaction::terminate;
							}
						}
						)
						, quest::request_fabric::create( [inst](quest* _q) { inst->run_();  } )
					);
					inst->quest_ = q;
					return q;
				fault:
					if(q != nullptr)
						delete q;
					if (inst != nullptr)
						delete inst;
					return nullptr;
				}
			};
			bool post_startup_vars_(quest * _owner, quest * _sema, robo::cstr _sect) {
				const size_t N = ROBO_STRING_BUFFER_SIZE;
				robo::char_t keys[N];

				robo::string startup;

				ROBO_LBREAKN_F(startup.load(current_path(), defaults_path(), _sect), "var's section %s for %s isn't found", _sect, display_alias());

				size_t sz = 0;
				::robo::system::ini::load_data(keys, N, startup.c_str(), nullptr, sz);
				ROBO_LBREAKN_F((sz < N - 2), "var's list for %s is oversized (%u)", display_alias(), sz);

				robo::char_t* bg = keys;
				robo::char_t* ptr = keys;
				while (sz > 0) {
					if (*ptr == 0) {
						robo::cstr r = RT("#");
						if (*bg != r[0]) {
							robo::string value;
							if (!value.load(startup, bg)) {
								return false;
							}
							else {
								var_post_quest(bg, value.c_str(), _owner, _sema);
							}
						}
						bg = ptr + 1;
					}
					ptr++;
					sz--;
				}

				return true;
			}

			quest* post_startup_vars(robo::cstr _sect, quest* _owner, quest* _sema = nullptr) {

				//todo
				//список строк заканивается двумя нулями
				quest* end_load = ::robo::quest::create(
					_owner
					, nullptr
					, [this,_sect](::robo::quest::result _r)->robo::quest::reaction {
						if (_r == robo::quest::result::success) {
							robo_detaillog(6, robo::log::mask::disabled, "\t\tquest: 'load startup var's ' for %s/%s  - success", display_alias(),_sect);
							return robo::quest::reaction::normal;
						}
						else {
							robo_errlog("\t\tquest: 'load startup var's' for  %s/%s  - refused, canceled or termibated (%d) ", display_alias(), _sect, (int)_r);
							return robo::quest::reaction::terminate;
						}
					}
					);
				quest* begin_load = ::robo::quest::create(
					end_load
					, _sema
					, [this, _sect](::robo::quest::result _r)->robo::quest::reaction {
						if (_r == robo::quest::result::success) {
							robo_detaillog(6, robo::log::mask::disabled, "\tquest: 'begin load startup var's ' for %s/%s  - success", display_alias(), _sect);
							return robo::quest::reaction::normal;
						}
						else {
							robo_errlog("\tquest: 'begin load startup var's' for  %s/%s  - refused, canceled or termibated (%d) ", display_alias(), _sect, (int)_r);
							return robo::quest::reaction::terminate;
						}
					}
				);
				if (!post_startup_vars_(end_load, begin_load, _sect)) {
					begin_load->terminate();
					if(_sema)
						_sema->terminate();
					return nullptr;
				}
				else {
					return begin_load;
				}

			}
			virtual bool do_load(void) {

				ROBO_LBREAKN(robo::backend::devagent::do_load());
				if ( robo::ini::try_load(current_path(), defaults_path(), RT("flow_serial_count"), flow_serials_count_) ) {
					if (flow_serials_count_ > 0) {
						flow_serials_ = new flow_serial * [flow_serials_count_];
						ROBO_APP_ASSERT(flow_serials_!=nullptr);
						flow_serial** pf = flow_serials_;
						robo::string key;
						for (int i = 0; i < flow_serials_count_; ++i, ++pf) {
							*pf = nullptr;
						}
						pf = flow_serials_;
						for (int i = 0; i < flow_serials_count_; ++i, ++pf) {
							key.format(RT("flow_serial_%d"), i);
							flow_serial* f = new flow_serial(key,*this,devagent::stream::priority::lo);
							ROBO_APP_ASSERT(f != nullptr);
							*pf = f;
						}
					}
				}
				return true;
			}

			virtual void do_clean(void) {
				if (flow_serials_ != nullptr) {
					flow_serial** pf = flow_serials_;
					for (int i = 0; i < flow_serials_count_; ++i, ++pf) {
						if (*pf != nullptr) delete* pf;
					}
					delete[] flow_serials_;
					flow_serials_ = nullptr;
				}
				robo::backend::devagent::do_clean();
			}
		protected:
			//to do сделать иерархию
			//typedef mexo::common::devagent::action_s action_s;
			//typedef mexo::common::devagent::feedback_s feedback_s;
			typedef robo::common::devagent::feedback_s feedback_s;
			typedef robo::common::devagent::action_s action_s;
			proto varproto;
			varindex vars;
			devagent(robo::cstr _name, robo::backend::boardagent& _boardagent, action_s& _goal, feedback_s& _feedback)
				:robo::backend::devagent(_name, _boardagent, _goal, _feedback)
				, echo_(*this)
				, vars(*this, varproto, varindex::priority::hi, nullptr, 0) {

			}
		};
		class servo_s : public robo::backend::servo_s {
			robo::backend::router defrout;
		public:
			virtual ::robo::quest* config_finish_quest() { return nullptr; };
			servo_s(robo::cstr _name, robo::app::module& _module)
				: robo::backend::servo_s(_name, _module), defrout(RT("defrout"), _module) {}
		};
	}
}
#endif

