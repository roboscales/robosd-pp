#include "net/platform/can/emulator/emu_can.hpp"
#include "core/robosd_string.hpp"
#include "core/robosd_ini.hpp"
#include "im/edev/edev.hpp"
namespace robo {
	namespace net {
		namespace emu_can {
			struct packet {
				union {
					struct {
						uint16_t id;
						uint8_t size;
						struct {
							uint8_t state : 2;
							uint8_t send_count : 6;
						};
					};
					uint32_t header;
				};
				union {
					uint32_t data32[2];
					uint8_t data[8];
				};
			};

			struct port::shared {
				packet out;
				packet received;
				uint8_t used;
				uint8_t unused[2];
			};
			struct port::ether {
				shared shareds[ROBO_CAN_CONNECTION_COUNT];
				uint8_t changed;
			};

			enum { SHARED_UNUSED = 0, SHARED_USED = 1 };
			enum { NORM = 0, CHANGED = 1 };
			enum { PACKET_EMPTY = 0, PACKET_TRANSMIT = 1, PACKET_REFUSE = 2, PACKET_COMPLETE = 3 };

			int refuse_count = 0;
			int receive_count = 0;
			int fault_count = 0;
			int tick_count = 0;
			int send_count = 0;
			int confirm_count = 0;
			system::shared* shared_memo_ = nullptr;
			int shared_used_ = 0;
			bool port::open(void) {
				string name;
				ROBO_LBREAKN( name.format( RT(ROBO_CAN_CHANNEL_SHARED_FILE_NAME), channel) );
				if (shared_memo_ == nullptr) {
					shared_memo_ = new system::shared;
					shared_used_ = 1;
					ROBO_LBREAKN(shared_memo_->open(name.c_str(), sizeof(ether)));
				}
				else {
					shared_used_++;
				}
				ether_ = (ether *)(shared_memo_->memo() );
				{
					system::shared::guard g__(*shared_memo_);

					shared* sh = ether_->shareds;
					for (int i = 0; i < ROBO_CAN_CONNECTION_COUNT; ++i, ++sh) {
						if (sh->used == SHARED_UNUSED) {
							shared_ = sh;
							sh->used = SHARED_USED;
							ether_->changed = CHANGED;
							return true;
						}
					}
				}
				ROBO_LBREAK_F("Any empty slot is not found");
			}

			void port::close(void) {
				if (ether_) {
					if (shared_) {
						shared_->used = SHARED_UNUSED;
						shared_ = 0;
					}
					ether_->changed = CHANGED;
					ether_ = nullptr;
				}
				if (shared_used_ == 1) {
					shared_used_ = 0;
					shared_memo_->close();
					delete shared_memo_;
					shared_memo_ = nullptr;
				}
				else {
					shared_used_--;
				}
			}

			bool port::send(uint16_t _id, uint8_t* _buf, uint8_t  _len) {
				if (shared_) {
					if (shared_->out.state != PACKET_EMPTY) {
						return false;
					}
					shared_->out.id = _id;
					shared_->out.size = _len;
					shared_->out.send_count = repeat_max_count + 1;
					std::copy_n(shared_->out.data, _len, _buf);
					shared_->out.state = PACKET_TRANSMIT;
					return true;
				}
				else {
					return false;
				}
			}
			bool port::ready(void) {
				return shared_->out.state == PACKET_EMPTY;
			}
			void port::reset(void) {
				close();
				open();
			}
			void port::pool(void) {
				switch (shared_->out.state) {
				case PACKET_REFUSE:
					shared_->out.state = PACKET_EMPTY;
					(*on_event)(*this, event::fault);
				break;
				case PACKET_COMPLETE:
					shared_->out.state = PACKET_EMPTY;
					confirm_count++;
				break;
				}
				switch (shared_->received.state) {
				case PACKET_REFUSE:
					shared_->received.state = PACKET_EMPTY;
					(* on_event)(*this, event::fault);
				break;
				case PACKET_COMPLETE:
					shared_->received.state = PACKET_EMPTY;
					(*on_receive)(*this, shared_->received.id, shared_->received.data, shared_->received.size);
				break;
				}
			}

			class agent :public edev::agent {
				port master_port_;
				enum { SIMPLE = 0, EMULATOR = 1 } mode_ = SIMPLE;
				//	std::queue<emu_can_shared_p> queue_;
				int channel_ = 0;
				float baudrate_ = 1000000.f;
				float busyPeriod = 0.f;
				double busyTime_ = 0.;
				double showTime_ = 0.;
				float showPeriod_ = 0.f;

				int test_ports_count_ = 0;
				int test_send_prescale_ = 0;
				int test_send_tick_ = 0;
				port** test_ports_ = nullptr;
				int test_ports_show_period_us_ = 0;
				int test_send_freq_ = 0;
				int test_repeat_max_count_ = 0;
				port::shared* winner_ = nullptr;
				int nodes_count_ = 0;
				port::shared* index_[ROBO_CAN_CONNECTION_COUNT];
				void test_on_event__(ican& /*_can*/, ican::event _ev) {
					if (_ev == ican::event::fault) {
						refuse_count++;
					}
				}
				void test_on_receive__(ican& /*_can*/, uint16_t /*_id*/, uint8_t*/*_msg*/, uint8_t/*_size*/) {
					receive_count++;
				}
				delegat::smember<agent, void, ican&, uint16_t, uint8_t*, uint8_t >on_receive_;
				delegat::smember<agent, void, ican&, ican::event > on_event_;
			protected:
				virtual void do_background_run(double _time) {
					if (_time >= busyTime_) {
						busyTime_ = _time + busyPeriod;

						port::ether* ether_ = master_port_.ether_;
						if (ether_->changed == CHANGED) {
							port::shared* shared_ = ether_->shareds;
							nodes_count_ = 0;
							for (int i = 0; i < ROBO_CAN_CONNECTION_COUNT; ++i, ++shared_) {
								if (shared_->used == SHARED_USED) {
									if (shared_->out.state != PACKET_EMPTY) {
										shared_->out.state = PACKET_REFUSE;
									}
									index_[nodes_count_++] = shared_;
								}
							}
							winner_ = nullptr;
							ether_->changed = NORM;
						}
						else {
							port::shared** pshared = index_;
							int min_id = 0x7fffffff;
							for (int i = 0; i < nodes_count_; ++i, ++pshared) {
								port::shared* shared_ = *pshared;
								if (shared_->out.state == PACKET_TRANSMIT) {
									if (shared_->out.send_count > 0) {
										shared_->out.send_count--;
										if (min_id > shared_->out.id) {
											min_id = shared_->out.id;
											winner_ = shared_;
										}
									}
									else {
										shared_->out.state = PACKET_REFUSE;
									}
								}
							}

							if (winner_) {
								port::shared** pshared = index_;
								for (int i = 0; i < nodes_count_; ++i, ++pshared) {
									(*pshared)->received = winner_->out;
								}
								pshared = index_;
								for (int i = 0; i < nodes_count_; ++i, ++pshared) {
									(*pshared)->received.state = PACKET_COMPLETE;
								}
								winner_->out.state = PACKET_COMPLETE;
								winner_ = nullptr;
								//SetEvent(master_connection_->hEvent_);
							}
						}
					}
				}
				virtual void do_priotitet_run(double _time) {
					for (int i = 0; i < test_ports_count_; i++) {
						test_ports_[i]->pool();
					}
					tick_count++;

					if (test_send_prescale_ > 0) {
						if (++test_send_tick_ >= test_send_prescale_) {
							test_send_tick_ = 0;
							for (int i = 0; i < test_ports_count_; i++) {
								static uint8_t buf[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
								if (!test_ports_[i]->send(system::env::rand(2047), buf, 8)) {
									refuse_count++;
								}
								send_count++;
							}
						}
					}
					else {
						for (int i = 0; i < test_ports_count_; i++) {
							random_t r = system::env::rand(100);
							if (r < test_send_freq_) {
								static uint8_t buf[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
								if (!test_ports_[i]->send(system::env::rand(2047), buf, 8)) {
									refuse_count++;
								}
								send_count++;
							}
						}
					}

					if (test_ports_count_ > 0) {
						if (_time >= showTime_) {
							showTime_ = _time + showPeriod_;
							robo_infolog("tick_count:\t%d\n\tsend_count:\t%d\n\tconfirm_count:\t%d(%4.3f kbod )\n\trefuse_count:\t%d (%2.2f%%)\n\tlost_count:\t%d\n\treceive_count:\t%d\n", tick_count, send_count, confirm_count, confirm_count * 75 / showPeriod_ / 1000, refuse_count, 100.0 * refuse_count / (send_count == 0 ? 1 : send_count), send_count - confirm_count - refuse_count, receive_count);
							tick_count = 0;
							send_count = 0;
							confirm_count = 0;
							refuse_count = 0;
							receive_count = 0;
						}
					}
				}

				virtual bool do_begin(void) {
					ROBO_LBREAKN(edev::agent::do_begin())
						ROBO_LBREAKN(ini::load(type, RT("BAUDRATE"), baudrate_));
					ROBO_LBREAKN(ini::load(type, RT("CHANNEL"), channel_));
					ROBO_LBREAKN(ini::load(type, RT("REPEAT_MAX_COUNT"), master_port_.repeat_max_count));
					ROBO_LBREAKN(ini::load(type, RT("TEST_CONNECTION_COUNT"), test_ports_count_));
					ROBO_LBREAKN(ini::load(type, RT("TEST_CONNECTION_SEND_PRESCALE"), test_send_prescale_));
					ROBO_LBREAKN(ini::load(type, RT("TEST_CONNECTION_SEND_FREQ_100"), test_send_freq_));
					ROBO_LBREAKN(ini::load(type, RT("TEST_CONNECTION_REPEAT_MAX_COUNT"), test_repeat_max_count_));
					ROBO_LBREAKN(ini::load(type, RT("TEST_CONNECTION_SHOW_PERIOD_SEC"), showPeriod_));


					master_port_.channel = channel_;
					master_port_.set_on_receive((ican::on_receive_f*)&on_receive_);
					master_port_.set_on_event((ican::on_event_f*)&on_event_);
					ROBO_LBREAKN(master_port_.open());

					if (test_ports_count_ > 0) {
						test_ports_ = new  port * [test_ports_count_];
						for (int i = 0; i < test_ports_count_; i++) {
							port* tmp = new port;
							tmp->channel = channel_;
							tmp->set_on_receive((ican::on_receive_f*)&on_receive_);
							tmp->set_on_event((ican::on_event_f*)&on_event_);
							tmp->repeat_max_count = test_repeat_max_count_;
							test_ports_[i] = tmp;
							ROBO_LBREAKN(tmp->open());
						}
					}

					busyPeriod = 100 / baudrate_;

					robo_infolog("can emulator channel '%s' was started", name.c_str());
					return  true;
				}
				virtual void do_reconfig(void) {};
				virtual void do_finish(void) {
					master_port_.close();
					if (test_ports_) {
						for (int i = 0; i < test_ports_count_; i++) {
							test_ports_[i]->close();
							delete test_ports_[i];
						}
						delete[] test_ports_;
					}
					test_ports_ = nullptr;
					robo_infolog("can emulator channel  '%s' was finished", name.c_str());
				}
			public:
				agent(void)
					: on_event_(this, &agent::test_on_event__)
					, on_receive_(this, &agent::test_on_receive__) {}
				~agent() {}
				virtual void set_local_ini(cstr _ini) { system::ini::begin(_ini); }
			} agent_;
		}
	}
}

extern "C" {
	ROBO_EXPORT_RUNTIME robo::edev::agent* ROBO_EXPORT_RUNTIME_DECL query_agent(void) {
		return & robo::net::emu_can::agent_;
	}
}

