#ifndef mexo_backend_hpp
#define mexo_backend_hpp
#include "servo/mexo_frontend.hpp"
#include "servo/robosd_backend.hpp"
namespace mexo {
	namespace backend {
		class devagent : public robo::backend::devagent {

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



			class varindex : public ::robo::backend::vartable {
			public:
				class descriptor {
				public:
					typedef ::robo::list::unsorted<descriptor> queue;
					//typedef robo::delegat::base<void, bool, ivar* > confirm_d;
					typedef ::robo::backend::vartable::ivar::delegat confirm_d;
				private:
					robo::string name_;
					robo::string type_;
					::robo::backend::vartable::record record_;
					queue::ref ref_;
					confirm_d* confirm_ = nullptr;
					const lambda& _lambda;
					ivar* var_ = nullptr;
					int requery_count_ = 3;
					varindex& varindex_;
				public:
					ivar* var(void) { return var_; }
					robo::cstr name(void) const { return name_.c_str(); }
					descriptor(varindex& _varindex, robo::cstr _name, confirm_d* _confirm = nullptr);
					descriptor(varindex& _varindex, robo::cstr _name, const lambda & _lambda);
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
			public:
				void query(robo::cstr _name, varindex::descriptor::confirm_d* _confirm = nullptr);
				varindex(devagent& _agent, proto& _proto, priority _priority, const record* const _records, size_t _count);
			};

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
			virtual bool do_load(void) {
				ROBO_LBREAKN(robo::backend::devagent::do_load());
				if ( robo::ini::try_load(current_path(), common_path(), RT("flow_serial_count"), flow_serials_count_) ) {
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
		public:
			typedef mexo::common::devagent::action_s action_s;
			typedef mexo::common::devagent::feedback_s feedback_s;
			proto proto_;
			varindex vars_;
			devagent(robo::cstr _name, robo::backend::boardagent& _boardagent, action_s& _goal, feedback_s& _feedback)
				:robo::backend::devagent(_name, _boardagent, _goal.agent, _feedback.agent)
				, echo_(*this)
				, vars_(*this, proto_, varindex::priority::normal, nullptr, 0) {
/*				robo::delegat::slambda<void, bool, varindex::ivar*>* d
					= new robo::delegat::slambda<void, bool, varindex::ivar*>(
						[](bool _result, varindex::ivar* _var) {
							if (_result) {
								robo_detaillog(1, robo::log::mask::disabled, "var '%s' query success ", _var->name());
							}
							else {
								robo_errlog("var query fail ");
							}
						});*/
					vars_.query(RT("hps.mo_enco.native"), [](bool _result, varindex::ivar* _v) {
				});
			}
		};
		class servo : public robo::backend::servo {
			robo::backend::router defrout;
		public:
			servo(robo::cstr _name, robo::app::module& _module)
				: robo::backend::servo(_name, _module), defrout(RT("defrout"), _module) {}
		};
	}
}
#endif

