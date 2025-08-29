#ifndef robo_net_canopen_power402_hpp
#define robo_net_canopen_power402_hpp

#include "core/robosd_system.hpp"

namespace robo {
	namespace net {
		namespace canopen {
			namespace power402 {
				class machine {
				public:
					enum class statuses { disabled, fault, enabled };
					enum class states {
						start = 0
						, not_ready_to_switch_on //Not Ready to Switch On 
						/*
							К приводу подано низкое напряжение (например, ≈ 15 В, 5 В).
							Привод инициализируется или выполняет самотестирование.
							В этом состоянии необходимо включить тормоз, если он имеется. Функция привода отключена.
						*/
						, switch_on_disabled//Switch On Disabled
						/*
							Инициализация привода завершена. Параметры привода настроены. Параметры привода могут быть изменены.
							Возможно, к приводу не подается высокое напряжение (например, по соображениям безопасности).
							Функция привода отключена.
						*/
						, ready_to_switch_on //Ready to Switch On
						/*
							Возможно, на привод подано высокое напряжение. Параметры привода могут быть изменены. Функция привода отключена.
						*/
						, switched_on //Switched On
						/*
							На привод подано высокое напряжение. Усилитель мощности готов.
							Параметры привода могут быть изменены. Функция привода отключена
						*/
						, operation_enable //Operation Enable
						/*
							Неисправностей обнаружено не было.
							Функция привода включена, и на двигатель подано питание. Параметры привода могут быть изменены.
							(Это соответствует нормальной работе привода)
						*/
						, quiq_stop_active //Quick Stop Activ
						/*
							Параметры привода могут быть изменены. Выполняется функция быстрой остановки.
							Функция привода включена, и на двигатель подается питание.
							Если код опции quick stop переключен на 5 (оставаться в состоянии QUICK STOP ACTIVE),
							вы не сможете оставить состояние QUICK STOP АКТИВНЫМ, но вы можете перейти в состояние
							OPERATION ENABLE с помощью команды "Включить операцию’.
						*/
						, fault_reaction_active
						/*
							Параметры привода могут быть изменены. В приводе произошла неисправность.
							Выполняется функция быстрой остановки.
							Функция привода включена, и на двигатель подается питание.
						*/
						, fault
						/*
							Параметры привода могут быть изменены. В приводе произошла неисправность.
							Включение/выключение высокого напряжения зависит от области применения. Функция привода отключена.
						 */
					};

					enum class commands {
						none = 0
						, shutdown
						, switch_on
						, disable_voltage
						, quick_stop
						, disable_operation
						, enable_operation
						, fault_reset
					};

					union controlworld_s {
						uint16_t value;
						struct {
							uint16_t switch_on : 1;//0 - Switch on
							uint16_t enable_voltage : 1;//1 - Disable Voltage
							uint16_t quick_stop : 1;//2 - Quick Stop
							uint16_t enable_operation : 1;//3 - Enable Operation
							uint16_t operation_mode_specific : 1;//4-6 - Operation mode specific
							uint16_t fault_reset : 1;//7 - Reset Fault(only effective for faults)
							uint16_t halt : 1;//8 - Pause / halt
							uint16_t reserved : 2;//9-10 - reserved
							uint16_t manufacturer_specific : 5;//11 - 15 - Manufacturer - specific
						};
					};
					union statusword_S {
						uint16_t value;
						struct {
							uint16_t ready_to_switch_on : 1; //0
							uint16_t switched_on : 1; //1
							uint16_t operation_enabled : 1; //2
							uint16_t fault : 1; //3
							uint16_t voltage_enabled : 1;//4
							uint16_t quick_stop : 1;//5
							uint16_t switch_on_disabled : 1;//6 
							uint16_t warning : 1;//7
							uint16_t specific : 1;//8
							uint16_t remote : 1;//9
							uint16_t targetreached : 1;//10
							uint16_t internal_limit_active : 1;//11
							uint16_t operation_mode_specific : 2;//12 - 13
							uint16_t manufacturer_specific : 2;//14-15
						};
					};
				protected:
					virtual void panic(const char* _function, states _state) = 0;
					virtual void do_switch_on(void) = 0;
					virtual void do_switch_off(void) = 0;
					virtual void do_enable_operation(void) = 0;
					virtual void do_disable_operation(void) = 0;
					virtual void do_terminate(void) = 0;
					virtual void do_quick_stop(void) = 0;
					virtual void do_write_object(uint32_t _object_id, uint32_t _subindex,  const uint8_t* _data, size_t _size) = 0;
					virtual void do_read_object(uint32_t _object_id, uint32_t _subindex, uint8_t* _data, size_t _size) = 0;
					/*High voltage is applied to the drive when this bit is set to 1.*/
					virtual uint16_t status_high_voltage_enabled_get(void) { return 1;  };
/*
	A drive warning is present if bit 7 is set. The cause means no error but a state that has to be mentioned, 
	e.g. temperature limit, job refused. The status of the drive does not change. The cause of this warning 
	may be found by reading the fault code parameter. The bit is set and reset by the device.
*/
					virtual uint16_t status_warning_get(void) { return 0; };
/*
	If bit 9 is set, then parameters may be modified via the CAN-network, and the drive executes the content of a command message. 
	If the bit remote is reset, then the drive is in local mode and will not execute the command message. 
	The drive may transmit messages containing valid actual values like a position actual value, depending on the actual drive configuration. 
	The drive will accept accesses via SDO in local mode.
*/
					virtual uint16_t status_remote_get(void) { return state_> states::not_ready_to_switch_on; };
/*
	If bit 10 is set by the drive, then a set-point has been reached. The set-point is dependent on the operating mode. 
	The description is situated in the chapter of the special mode. The change of a target value by software alters this bit.
	If quick stop option code is 5, 6, 7 or 8, this bit must be set, when the quick stop operation is finished and the drive is halted.
	If halt occurred and the drive has halted then this bit is set too.
*/
					virtual uint16_t status_target_reached_get(void) { return 0; }
					virtual uint16_t status_internal_limit_active_get(void) { return 0; }
					virtual uint16_t status_specific_get(void) { return 0; }
					virtual uint16_t status_manufacturer_get(void) { return 0; }
				public:
					virtual void write_object(uint32_t _object_id, uint32_t _subindex, const uint8_t* _data, size_t _size) {
						robo::system::critical c_;
						do_write_object(_object_id, _subindex, _data, _size);
					}
					virtual void read_object(uint32_t _object_id, uint32_t _subindex, uint8_t* _data, size_t _size) {
						robo::system::critical c_;
						do_read_object(_object_id, _subindex, _data, _size);
					}
					template <typename T> void write_object(uint32_t _object_id, uint32_t _subindex, const T& _src) {
						write_object(_object_id, _subindex,(const uint8_t * ) & _src, sizeof(T));
					}
					template <typename T> void read_object(uint32_t _object_id, uint32_t _subindex,T& _dst) {
						read_object(_object_id, _subindex,(uint8_t * ) & _dst, sizeof(T));
					}
#define READ_OBJECT(T) READ_OBJECT_(T)
#define READ_OBJECT_(T)\
				T read_object_##T##(uint32_t _object_id,uint32_t _subindex) {\
					T tmp;\
					read_object(_object_id, _subindex, tmp);\
					return tmp;\
				}
					//READ_OBJECT(uint8_t);
					READ_OBJECT(uint16_t);
					READ_OBJECT(uint32_t);
					READ_OBJECT(uint64_t);
					READ_OBJECT(int8_t);
					READ_OBJECT(int16_t);
					READ_OBJECT(int32_t);
					READ_OBJECT(int64_t);
					READ_OBJECT(float);
					READ_OBJECT(double);
#undef READ_OBJECT_
#undef READ_OBJECT
					/*
						State Transition 0: START -> NOT READY TO SWITCH ON
						Event: Reset.
						Action: The drive self-tests and/or self-initializes.
					*/
					void reset(void) {
						state_ = states::not_ready_to_switch_on;
						robo_infolog("%u ds402::reset", (unsigned)system::time_us());
					}

					/*State Transition 1: NOT READY TO SWITCH ON -> SWITCH ON DISABLED
						Event: The drive has self-tested and/or initialized successfully.
						Action: Activate communication.
					*/
					void initialized_successfully(void) {
						ROBO_APP_ASSERT(state_ == states::not_ready_to_switch_on);
						state_ = states::ready_to_switch_on;
						robo_infolog("%u ds402::initialized_successfully", (unsigned)system::time_us());
					}

				private:
					/*
						State Transition 2: SWITCH ON DISABLED -> READY TO SWITCH ON
						Event: 'Shutdown' command received from host.
						Action: None

						State Transition 6: SWITCHED ON -> READY TO SWITCH ON
						Event: 'Shutdown' command received from host.
						Action: The power section is switched off.

						State Transition 8: OPERATION ENABLE -> READY TO SWITCH ON
						Event: 'Shutdown' command received from host.
						Action: The power section is switched off immediately, and the motor is free to rotate if unbraked.

					*/
					void shutdown_(void) {
						switch (state_) {
						case states::operation_enable:
							do_disable_operation();
						case states::switched_on:
							do_switch_off();
						case states::switch_on_disabled:
							state_ = states::ready_to_switch_on;
							break;
						default:
							panic(ROBO_APP_PROC_NAME, state_);
						}
						robo_infolog("%u ds402::shutdown_", (unsigned)system::time_us());
					}

					/*
						State Transition 3: READY TO SWITCH ON -> SWITCHED ON
						Event: 'Switch On' command received from host.
						Action: The power section is switched on if it is not already switched on.
					*/
					void switch_on_(void) {
						if (state_ == states::ready_to_switch_on) {
							state_ = states::switched_on;
							do_switch_on();
						}
						else {
							panic(ROBO_APP_PROC_NAME, state_);
						}
						robo_infolog("%u ds402::switch_on_", (unsigned)system::time_us());
					}

					/*
						State Transition 4: SWITCHED ON -> OPERATION ENABLE
						Event: 'Enable Operation' command received from host.
						Action: The drive function is enabled.

						State Transition 16: QUICK STOP ACTIVE -> OPERATION ENABLE
						Event : 'Enable Operation' command received from host.This transition is possible if the Quick - Stop - Option - Code is 5, 6, 7 or 8 ( Chapter 10.3.5).
						Action : The drive function is enabled.
					*/
					void enable_operation_(void) {
						switch (state_) {
						case states::switched_on:
						case states::quiq_stop_active:
							state_ = states::operation_enable;
							do_enable_operation();
							break;
						default:
							panic(ROBO_APP_PROC_NAME, state_);
						}
						robo_infolog("%u ds402::enable_operation_", (unsigned)system::time_us());
					}

					/*
						State Transition 5: OPERATION ENABLE -> SWITCHED ON
						Event: 'Disable Operation' command received from host.
						Action: The drive operation will be disabled.
					*/
					void disable_operation_(void) {
						if (state_ == states::operation_enable) {
							state_ = states::switched_on;
							do_disable_operation();
						}
						else {
							panic(ROBO_APP_PROC_NAME, state_);
						}
						robo_infolog("%u ds402::disable_operation_", (unsigned)system::time_us());
					}
					/*
						State Transition 7: READY TO SWITCH ON->SWITCH ON DISABLED
						Event : 'Quick Stop' and ‘Disable Voltage’ command received from host.
						Action : None

						State Transition 10: SWITCHED ON -> SWITCH ON DISABLED
						Event: 'Disable Voltage' or 'Quick Stop' command received from host.
						Action: The power section is switched off immediatly, and the motor is free to rotate if unbraked.

						State Transition 11: OPERATION ENABLE -> QUICK STOP ACTIVE
						Event: 'Quick Stop' command received from host.
						Action: The quick stop function is executed.


					*/
					void quick_stop_(void) {
						switch (state_) {
						case states::switched_on:
							do_switch_off();
						case states::ready_to_switch_on:
							state_ = states::switch_on_disabled;
							break;
						case  states::operation_enable:
							state_ = states::quiq_stop_active;
							do_quick_stop();
							do_disable_operation();
							do_switch_off();
							break;
						default:
							panic(ROBO_APP_PROC_NAME, state_);
						}
						robo_infolog("%u ds402::quick_stop_", (unsigned)system::time_us());
					}
				public:
					/*
						State Transition 12: QUICK STOP ACTIVE->SWITCH ON DISABLED
						Event : 'Quick Stop' is completed or 'Disable Voltage' command received from host.
						This transition is possible, if the Quick - Stop - Option - Code is different 5 (stay in the state ‘Quick Stop Active’).
						Action : The power section is switched off.
					*/
					void quick_stop_complete(void) {
						if (state_ == states::quiq_stop_active) {
							state_ = states::switch_on_disabled;
						}
						else {
							panic(ROBO_APP_PROC_NAME, state_);
						}
						robo_infolog("%u ds402::quick_stop_complete", (unsigned)system::time_us());
					}
				private:
					/*
						State Transition 7: READY TO SWITCH ON->SWITCH ON DISABLED
						Event : 'Quick Stop' and ‘Disable Voltage’ command received from host.
						Action : None

						State Transition 9: OPERATION ENABLE -> SWITCH ON DISABLED
						Event: 'Disable Voltage' command received from host.
						Action: The power section is switched off immediately, and the motor is free to rotate if unbraked.

						State Transition 10: SWITCHED ON -> SWITCH ON DISABLED
						Event: 'Disable Voltage' or 'Quick Stop' command received from host.
						Action: The power section is switched off immediatly, and the motor is free to rotate if unbraked.


						State Transition 12: QUICK STOP ACTIVE -> SWITCH ON DISABLED
						Event: 'Quick Stop' is completed or 'Disable Voltage' command received from host.
						This transition is possible, if the Quick-Stop-Option-Code is different 5 (stay in the state ‘Quick Stop Active’).
						Action: The power section is switched off.

					*/
					void disable_voltage_(void) {
						switch (state_) {
						case  states::operation_enable:
						case states::switched_on:
						case states::ready_to_switch_on:
						case  states::quiq_stop_active:
							do_terminate();
							state_ = states::switch_on_disabled;
							break;
						default:
							panic(ROBO_APP_PROC_NAME, state_);
						}
						robo_infolog("%u ds402::disable_voltage_", (unsigned)system::time_us());
					}
				public:
					/*
						State Transition 13: All states -> FAULT REACTION ACTIVE
						A fault has occurred in the drive.
						Action: Execute appropriate fault reaction.
					*/
					void fault(void) {
						switch (state_) {
						case  states::operation_enable:
							do_disable_operation();
						case states::switched_on:
							do_switch_off();
						default:
							state_ = states::fault_reaction_active;
						}
						robo_infolog("%u ds402::fault", (unsigned)system::time_us());
					}

					/*
						State Transition 14: FAULT REACTION ACTIVE -> FAULT
						Event: The fault reaction is completed.
						Action: The drive function is disabled. The power section may be switched off.

					*/
					void fault_reaction_completed(void) {
						if (state_ == states::fault_reaction_active) {
							state_ = states::fault;
						}
						else {
							panic(ROBO_APP_PROC_NAME, state_);
						}
						robo_infolog("%u ds402::fault_reaction_completed", (unsigned)system::time_us());
					}
				private:
					/*
						State Transition 15: FAULT -> SWITCH ON DISABLED
						Event: 'Fault Reset' command received from host.
						Action: A reset of the fault condition is carried out if no fault exists currently on the drive.
						After leaving the state Fault the Bit 'Fault Reset' of the controlword has to be cleared by the host.
					*/
					void fault_reset_(void) {
						if (state_ == states::fault) {
							state_ = states::switch_on_disabled;
						}
						else {
							panic(ROBO_APP_PROC_NAME, state_);
						}
						robo_infolog("%u ds402::fault_reset_", (unsigned)system::time_us());
					}

					commands decode_command_(uint16_t _cvv) {
						if (_cvv == 0) {
							 return commands::none;
						}
						controlworld_s _cv;
						_cv.value = _cvv;
						if (_cv.fault_reset) {
							return commands::fault_reset;
						}
						if (_cv.enable_voltage == 0) {
							return commands::disable_voltage;
						}
						if (_cv.quick_stop == 0) {
							return commands::quick_stop;
						}
						auto tmp = _cv.value & 0b1111;
						switch (tmp) {
						case 0b0110:
						case 0b1110:
							return commands::shutdown;
						case 0b0111:
							if (state_ == states::operation_enable) {
								return commands::disable_operation;
							}
							else {
								return commands::switch_on;
							}
						case 0b1111:
							return commands::enable_operation;

						}
						return commands::none;
					}
					statuses status_ = statuses::disabled;
					states state_ = states::start;
					commands command_ = commands::none;
				public:
					struct objects {
						enum {
							  controlword	= 0x6040
							, statusword	= 0x6041
						};
					};
					machine(

					) {

					}
					void poll(void) {
						auto cmd = decode_command_(read_object_uint16_t(objects::controlword,0));
						if (cmd!= commands::none) {
							if (command_ != cmd) {
								robo_infolog("%u ds402::command %d (old: %d) is received", (unsigned)system::time_us(), (int)cmd, (int)command_);
								command_ = cmd;
								switch (command_) {
								case commands::disable_operation:
									disable_operation_();
									break;
								case commands::disable_voltage:
									disable_voltage_();
									break;
								case commands::enable_operation:
									enable_operation_();
									break;
								case commands::fault_reset:
									fault_reset_();
									break;
								case commands::quick_stop:
									quick_stop_();
									break;
								}
							}
							write_object(objects::controlword, 0, (uint16_t)0);
						}
						statusword_S statusword;
						statusword.value = read_object_uint16_t(objects::statusword,0) & 0b00000000;

						switch ( state_ ) {
						case states::start:
						case states::not_ready_to_switch_on:
						case states::switch_on_disabled:
							break;
						case states::ready_to_switch_on:
							statusword.ready_to_switch_on = 1;
							statusword.quick_stop = 1;
							break;

						case states::switched_on:
							statusword.ready_to_switch_on = 1;
							statusword.switched_on = 1;
							statusword.quick_stop = 1;
							break;

						case states::operation_enable:
							statusword.ready_to_switch_on = 1;
							statusword.switched_on = 1;
							statusword.operation_enabled = 1;							
							statusword.quick_stop = 1;
							break;

						case states::quiq_stop_active:
							statusword.ready_to_switch_on = 1;
							statusword.switched_on = 1;
							statusword.operation_enabled = 1;
							statusword.quick_stop = 0;
							break;

						case states::fault_reaction_active:
							statusword.ready_to_switch_on = 1;
							statusword.switched_on = 1;
							statusword.operation_enabled = 1;
							statusword.fault = 1;
							break;
						case states::fault:
							break;
						}
						
						statusword.voltage_enabled = status_high_voltage_enabled_get();
						statusword.warning = status_warning_get();
						statusword.remote = status_remote_get();
						statusword.targetreached = status_target_reached_get();
						statusword.internal_limit_active = status_internal_limit_active_get();
						statusword.specific = status_specific_get();
						statusword.manufacturer_specific = status_manufacturer_get();

						write_object(objects::statusword, 0, statusword.value);

					}
					bool isfault(void) {
						return state_ == states::fault || state_ == states::fault_reaction_active;
					}
				};
			}
		}
	}
}

#endif