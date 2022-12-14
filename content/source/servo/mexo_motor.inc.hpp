#include "servo/mexo_backend.hpp"
#include "core/robosd_convert.hpp"
#include "core/robosd_common.hpp"
#ifndef MOTOR_CURRENT_MEASSURY_ENABLED
#define MOTOR_CURRENT_MEASSURY_ENABLED 0
#endif
#ifndef MOTOR_POSTITION_MEASSURY_ENABLED
#define MOTOR_POSTITION_MEASSURY_ENABLED 0
#endif
#ifndef MOTOR_DEV_ERROR_VAR_NAME
#define MOTOR_DEV_ERROR_VAR_NAME "joint.dev.error"
#endif
namespace mexo {
	namespace backend {
		namespace motor {
			template <class servo, class devs_s, class snapshot_s, class goal_s> class machine_t : public mexo::backend::devagent {
			private:
				servo& servo_;
				friend class snapshot;
				void update_snapshot_(void) {
					feedback_s& fb = feedback<machine_t <servo, devs_s, snapshot_s, goal_s>>();
					snapshot_.snapshot0_s::decode(fb, converters);
					//
					//todo завтра здесь!
					((devagent::feedback_s&)fb).agent.state.remote.status = ::robo::common::devagent::state_s::remote_s::statuses::ready;
					//feedback_.state.remote.status
				}
				size_t encode_goal_(uint8_t* _memo, size_t _mem_size) {
					return snapshot_.goal0_s::encode(goal<machine_t <servo, devs_s, snapshot_s, goal_s>>(), converters, _memo, _mem_size);
				}
				class snapshot : public robo::backend::devagent::datamap, public snapshot_s, public goal_s {
					using machine = machine_t <servo, devs_s, snapshot_s, goal_s>;
					friend class machine;
					virtual void on_complete(void) {
						((machine&)own_agent()).update_snapshot_();
					}
					virtual size_t encode(uint8_t* _data, size_t _max_size) {
						return ( (machine&)own_agent()).encode_goal_(_data, _max_size);
					}
					virtual devagent::stream::query_result first_query(robo_tran_t& _tran) {
						robo::backend::devagent::datamap::first_query(_tran);
						if (((machine&)own_agent()).actual_status(((machine&)own_agent()).actual_command()) == statuses::dirrect) {
							_tran.size_actual = encode(_tran.data, _tran.size_max);
							_tran.header.command = ::mexo::front::dev::flow_command_ix::goal;
						}
						return query_result::repeat;
					}
				public:
					snapshot(machine& _agent)
						: robo::backend::devagent::datamap(RT("snapshot"), _agent, priority::hi, snapshot::memo, nullptr, snapshot::mem_size, ::mexo::front::dev::flow_command_ix::snapshot) {}
				} snapshot_;

			public:
				typedef typename devs_s::action_s action_s;
				typedef typename devs_s::feedback_s feedback_s;
				typedef typename devs_s::config_s config_s;
				typedef typename devs_s content_s;
				class converters_s : public ::robo::app::node {
				public:
					using machine = machine_t <servo, content_s, snapshot_s, goal_s>;
					::robo::converter voltage;
					#if MOTOR_CURRENT_MEASSURY_ENABLED  == 1
					::robo::converter current;
					#endif
					#if MOTOR_POSTITION_MEASSURY_ENABLED == 1
					::robo::converter speed;
					::robo::converter position;
					#endif
					converters_s(machine & _owner)
						: ::robo::app::node(RT("co"), &_owner)
						, voltage(RT("v"), this)
						#if MOTOR_CURRENT_MEASSURY_ENABLED  == 1
						, current(RT("c"), this)
						#endif
						#if MOTOR_POSTITION_MEASSURY_ENABLED == 1
						, speed(RT("sp"), this)
						, position(RT("po"), this)
						#endif
					{

					}
				} converters;
			protected:
				virtual bool do_start(void) {
					ROBO_LBREAKN(mexo::backend::devagent::do_start());
					if (this->exchabge_enabled()) {
						snapshot_.start();
					}
					return true;
				}
				
				virtual bool do_load(void) {
					ROBO_LBREAKN(mexo::backend::devagent::do_load());
					if (feedback < ::robo::common::devagent::feedback_s >().state.local == state_s::locals::configure) {
						using q = ::robo::quest;
						q* finish = quest_configure(servo_.config_finish_quest());
						q* commit = var_post_quest(finish, RT(MOTOR_DEV_ERROR_VAR_NAME), RT("0"));
						q* startup = var_post_quest(nullptr, RT(MOTOR_DEV_ERROR_VAR_NAME), RT("66"));
						post_vars_quest::begin(*this, commit, startup);						

						::robo::quest::post();
					}
					return true;
				}
			public:
				machine_t(robo::cstr _name, robo::backend::boardagent& _boardagent, action_s& _goal, feedback_s& _feedback) :
					mexo::backend::devagent(_name, _boardagent, (devagent::action_s &)_goal, (devagent::feedback_s&)_feedback)
					, servo_((servo &)_boardagent.owner() )
					, converters(*this)
					, snapshot_(*this)
				{
				}
			};

			template <class servo, class content_s, class snapshot_s, class goal_s> class agent_t {
			public:
				robo::backend::boardagent board;
				robo::backend::devagent_t<machine_t<servo, content_s, snapshot_s, goal_s> >  devagent;
				agent_t(::robo::cstr _name, servo& _servo, content_s & _ctn)
					: board(_name, _servo)
					, devagent(RT("agent"), board, _ctn) {
				}
			};

		}
	}
}
#undef MOTOR_CURRENT_MEASSURY_ENABLED
#undef MOTOR_POSTITION_MEASSURY_ENABLED
#undef MOTOR_DEV_ERROR_VAR_NAME