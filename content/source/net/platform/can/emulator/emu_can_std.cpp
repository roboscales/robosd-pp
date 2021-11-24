//---------------------------------------------------------------------------
#include "core/robosd_ini.hpp"
#include "core/robosd_log.hpp"
#include "core/robosd_string.hpp"
#include "im/edev/edev.hpp"
#include "net/platform/can/emulator/emu_can.hpp"
#include <core/robosd_list.hpp>
#include <queue>
namespace robo {
	namespace net {
		namespace emu_can {
			enum { SHARED_UNUSED = 0, SHARED_USED = 1 };
			enum { NORM = 0, CHANGED = 1 };
			enum { PACKET_EMPTY = 0, PACKET_TRANSMIT = 1, PACKET_REFUSE = 2, PACKET_COMPLETE = 3 };

			struct packet{
				union{
					struct{
						uint16_t id;
						uint8_t size;
						struct{
							uint8_t state:2;
							uint8_t send_count : 6;
						};
					};
					uint32_t header;
				};
				union{
					uint32_t data32[2];
					uint8_t data[8];
				};
			};

			struct shared{
				packet out;
				packet received;
				uint8_t used;
				uint8_t unused[2];
			};

			struct ether{
				shared shareds[ROBO_CAN_CONNECTION_COUNT];
				uint8_t changed;
			};

			//void emu_can_on_receive_dummy_(robo_can_p /*_can*/, robo_can_msg_id_t /*_msg*/, robo_byte_p /*_buf*/, robo_size_t /*_len*/){}

			//void emu_can_events_dummy_(robo_can_p /*_can*/, robo_can_event_t /*_ev*/){}
			int refuse_count = 0;
			int receive_count = 0;
			int fault_count = 0;
			int tick_count = 0;
			int send_count = 0;
			int confirm_count = 0;

			class connection{
			public:
			private:
			shared *  shared_ = nullptr;
			port * port_;

			shared_t shared_memo_;
	
	emu_can_ether_p ether_;
	//ref ref_;
	emuCanConnection(emu_can_port_p _port)
		: shared_(nullptr)
		, port_(_port)
		//, ref_(this)
		, ether_(nullptr)
	{
		port_->can.instance = this;
		if (port_->can.on_receive == 0) port_->can.on_receive = emu_can_on_receive_dummy_;
		if (port_->can.on_event == 0) port_->can.on_event = emu_can_events_dummy_;
		robo_os_shared_init(&shared_memo_, sizeof(emu_can_ether_t), 0, 0);
	}
	void close_(void);
	robo_result_t open_(emu_can_port_p _port);
		robo_result_t send_(robo_can_msg_id_t _id, robo_byte_p _buf, robo_size_t _len){
		if (shared_){
			if ( shared_->out.state != EMU_CAN_PACKET_EMPTY ){
				return ROBO_ERROR;
			}
			shared_->out.id = _id;
			shared_->out.size = _len;
			shared_->out.send_count = port_->repeat_max_count + 1;
			memcpy(shared_->out.data, _buf, _len);
			shared_->out.state = EMU_CAN_PACKET_TRANSMIT;
/*			if ( mode_ == SIMPLE ){
				queue_.push(shared_);
			}*/
		}
		return ROBO_SUCCESS;

	}
	robo_result_t ready_(){
		return shared_->out.state == EMU_CAN_PACKET_EMPTY ? ROBO_ANSW_YES : ROBO_ANSW_NO;
	}
	void  poll_(){
		switch (shared_->out.state){
		case EMU_CAN_PACKET_REFUSE:
			shared_->out.state = EMU_CAN_PACKET_EMPTY;
			port_->can.on_event(&(port_->can), ROBO_CAN_FAULT);
			break;
		case EMU_CAN_PACKET_COMPLETE:
			shared_->out.state = EMU_CAN_PACKET_EMPTY;
			confirm_count++;
			break;
		}
		switch (shared_->received.state){
		case EMU_CAN_PACKET_REFUSE:
			shared_->received.state = EMU_CAN_PACKET_EMPTY;
			port_->can.on_event(&(port_->can), ROBO_CAN_FAULT);
			break;
		case EMU_CAN_PACKET_COMPLETE:
			shared_->received.state = EMU_CAN_PACKET_EMPTY;
			port_->can.on_receive( &(port_->can) , shared_->received.id, shared_->received.data, shared_->received.size );
			break;
		}

	}
public:
	static robo_result_t ROBO_DECL open(emu_can_port_p _port){
		emuCanConnection *  node = new emuCanConnection(_port);
		if (node->open_(_port) != ROBO_SUCCESS){
			ROBO_RETNERR();
		}
		else {
			return ROBO_SUCCESS;
		}
	}

	static void ROBO_DECL close(emu_can_port_p _port){
		emuCanConnection * node = (emuCanConnection *)(_port->can.instance);
		if (node){
			node->close_();
			delete node;
		}

	}
	static robo_result_t send(emu_can_port_p _port, robo_can_msg_id_t _id, robo_byte_p _buf, robo_size_t _len){
		emuCanConnection * node = (emuCanConnection *)(_port->can.instance);
		if (node){
			return node->send_(_id, _buf, _len);
		}
		else{
			return ROBO_ERROR;
		}
	}

	static void ROBO_DECL poll(emu_can_port_p _port){
		emuCanConnection * node = (emuCanConnection *)(_port->can.instance);
		if (node){
			node->poll_();
		}

	}
	static robo_result_t ROBO_DECL ready(emu_can_port_p _port){
		emuCanConnection * node = (emuCanConnection *)(_port->can.instance);
		if (node){
			return node->ready_();
		}
		else {
			return ROBO_ERROR;
		}

	}
};




extern "C"{
	void emuCan_run(double _period, double _time);
	void emuCan_finish(void);
	ROBO_EXPORT robo_result_t ROBO_EXPORT_RUNTIME_DECL dev_startup(emu_dev_agent_p _agent);
	void emu_can_test_on_event(robo_can_p _can, robo_can_event_t _ev);
	void emu_can_test_on_receive(robo_can_p _port, robo_can_msg_id_t _id, robo_byte_p _data, robo_size_t _size);
}


class emuCan{	
	emu_dev_agent_p agent_;
	enum { SIMPLE = 0, EMULATOR = 1 } mode_ = SIMPLE;
	//	std::queue<emu_can_shared_p> queue_;

	float baudrate_;
	float busyPeriod;
	double busyTime_;
	double showTime_;
	float showPeriod_;
	emu_can_port_t master_port_;
	emuCanConnection * master_connection_;
	int test_ports_count_;
	int test_send_prescale_;
	int test_send_tick_;
	emu_can_port_p * test_ports_;
	int test_ports_show_period_us_;
	int test_send_freq_;
	int test_repeat_max_count_;
	emu_can_shared_p winner;
	emuCan()
	: baudrate_(1000000.0f)
	, busyPeriod(0.0f)
	, busyTime_(0.0)
	, showTime_(0.0)
    , showPeriod_(1.0)
    , master_connection_(0)
	, test_ports_count_(0)
	, test_send_prescale_(0)
	, test_send_tick_(0)
	, test_ports_(0)
	, test_ports_show_period_us_(0)
    , test_send_freq_(0)
	, test_repeat_max_count_(0)
	{
		memset(&master_port_, 0, sizeof(master_port_));
	}
	~emuCan(){
	}
	//emuCanConnection::list nodes_;
	int nodes_count_;
	emu_can_shared_p index_[ROBO_CAN_CONNECTION_COUNT];
public:
    void run(double /*_period*/, double _time){

		if (_time >= busyTime_){
			busyTime_ = _time + busyPeriod;

			/*if (mode_ == SIMPLE){

			}
			else */
			{
				emu_can_ether_p ether = master_connection_->ether_;
				if (ether->changed == EMU_CAN_CHANGED){
					emu_can_shared_p shared = master_connection_->ether_->shareds;
					nodes_count_ = 0;
					for (int i = 0; i < ROBO_CAN_CONNECTION_COUNT; i++, shared++){
						if (shared->used == EMU_CAN_SHARED_USED){
							if (shared->out.state != EMU_CAN_PACKET_EMPTY){
								shared->out.state = EMU_CAN_PACKET_REFUSE;
							}
							index_[nodes_count_++] = shared;
						}
					}
					winner = 0;
					ether->changed = EMU_CAN_NORM;
				}
				else{

					emu_can_shared_p * pshared = index_;
					int min_id = 0x7fffffff;
					for (int i = 0; i < nodes_count_; i++, pshared++){
						emu_can_shared_p shared = *pshared;
						if (shared->out.state == EMU_CAN_PACKET_TRANSMIT){
							if (shared->out.send_count > 0){
								shared->out.send_count--;
								if (min_id > shared->out.id){
									min_id = shared->out.id;
									winner = shared;
								}
							}
							else{
								shared->out.state = EMU_CAN_PACKET_REFUSE;
							}

						}
					}

					if (winner){
						//ether->received = winner->out;
						emu_can_shared_p * pshared = index_;
						for (int i = 0; i < nodes_count_; i++, pshared++){
							(*pshared)->received = winner->out;
						}
						pshared = index_;
						for (int i = 0; i < nodes_count_; i++, pshared++){
							(*pshared)->received.state = EMU_CAN_PACKET_COMPLETE;
						}
						winner->out.state = EMU_CAN_PACKET_COMPLETE;
						winner = 0;
						//SetEvent(master_connection_->hEvent_);
					}
				}
			}

			for (int i = 0; i < test_ports_count_; i++){
				emu_can_port_poll(test_ports_[i]);
			}
			emu_can_port_poll(&master_port_);
			tick_count++;

			if (test_send_prescale_ > 0){
				if (++test_send_tick_ >= test_send_prescale_){
					test_send_tick_ = 0;
					for (int i = 0; i < test_ports_count_; i++){
						static uint8_t buf[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
						if (robo_can_send(&(test_ports_[i]->can), ROBO_STD_RAND(2047), buf, 8) != ROBO_SUCCESS) {
							refuse_count++;
						}
						send_count++;
					}
				}
			}
			else {
				for (int i = 0; i < test_ports_count_; i++){
					robo_random_t r = ROBO_STD_RAND(100);
					if (r < test_send_freq_){
						static uint8_t buf[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
						if (robo_can_send(&(test_ports_[i]->can), ROBO_STD_RAND(2047), buf, 8) != ROBO_SUCCESS) {
							refuse_count++;
						}
						send_count++;
					}
				}
			}
		}
		if (test_ports_count_>0){
			if (_time >= showTime_){
				showTime_ = _time + showPeriod_;
				robo_log(0, 0, "tick_count:\t%d\n\tsend_count:\t%d\n\tconfirm_count:\t%d(%4.3f)\n\trefuse_count:\t%d (%2.2f%%)\n\tlost_count:\t%d\n\treceive_count:\t%d\n", tick_count, send_count, confirm_count, confirm_count * 75 / showPeriod_ / 1000, refuse_count, 100.0*refuse_count / (send_count == 0 ? 1 : send_count), send_count - confirm_count - refuse_count, receive_count);
				tick_count = 0;
				send_count = 0;
				confirm_count = 0;
				refuse_count = 0;
				receive_count = 0;
			}
		}
	}


	void finish(void){
		robo_can_close(&(master_port_.can));

		if (test_ports_){
			for (int i = 0; i < test_ports_count_; i++){
				robo_can_close( &(test_ports_[i]->can));
				delete test_ports_[i];
			}
			delete[] test_ports_;
		}
		test_ports_ = nullptr;
		robo_infolog("can emulato  channel with id %d was finished", agent_->ref.id);
	}

	robo_result_t connect_(){
		ROBO_CHECKRET( robo_can_open( &(master_port_.can) ) );
		master_connection_ = (emuCanConnection *) (master_port_.can.instance);
		for (int i = 0; i < test_ports_count_; i++){
			ROBO_CHECKRET(robo_can_open( &(test_ports_[i]->can) ) );
		}
		return ROBO_SUCCESS;
	}
	robo_result_t startup(emu_dev_agent_p _agent){
		agent_ = _agent;
		char sec[ROBO_INI_SECTION_MAX_LEN + 1];
		//Агент- это наш представитель на стороне эмулятора. Агент может существовать и без реализации. Например, в момент загрузки из ini
		_agent->instance = this;
		_agent->run = &emuCan_run;
		_agent->finish = &emuCan_finish;
        robo_sprintf(sec, ROBO_INI_SECTION_MAX_LEN, RS("DEVICE_TYPE_%s"), _agent->type);
		baudrate_ = 1000;
		robo_ini_load_float(sec, RS("BAUDRATE"), 1000, &(baudrate_));
		robo_ini_load_int(sec, RS("TEST_CONNECTION_COUNT"), 0, &(test_ports_count_));
		robo_ini_load_int(sec, RS("TEST_CONNECTION_SEND_PRESCALE"), 0, &(test_send_prescale_));
		robo_ini_load_int(sec, RS("TEST_CONNECTION_SEND_FREQ"), 0, &(test_send_freq_));
		robo_ini_load_int(sec, RS("TEST_CONNECTION_REPEAT_MAX_COUNT"), 0, &(test_repeat_max_count_));
		robo_ini_load_float(sec, RS("TEST_CONNECTION_SHOW_PERIOD_SEC"), 1.0, &(showPeriod_));
		
		if (test_ports_count_ > 0){
			test_ports_ = new  emu_can_port_p[test_ports_count_];
			for (int i = 0; i < test_ports_count_;i++){
				emu_can_port_p tmp = new emu_can_port_t;
				tmp->can.channel = agent_->ref.id;
				tmp->can.on_receive = emu_can_test_on_receive;
				tmp->can.on_event = emu_can_test_on_event;
				tmp->repeat_max_count = test_repeat_max_count_;
				test_ports_[i] = tmp;
			}
		}

		busyPeriod = 100 / baudrate_;
		master_port_.can.channel = agent_->ref.id;
		
		if (connect_() < ROBO_SUCCESS){
			finish();
			ROBO_RETNERR();
		}
		robo_infolog("can emulator channel  was started with id %d", agent_->ref.id);
		return ROBO_SUCCESS;
	}
	static emuCan & instance(){
		static emuCan instance_;
		return instance_;
	}

} ;

void emuCan_run(double _period, double _time){
	emuCan::instance().run(_period, _time);
}

//завершение работы сети
void emuCan_finish(void){
	emuCan::instance().finish();
}


#ifndef EMU_CAN_STARTAUP_ENABLED
#define EMU_CAN_STARTAUP_ENABLED 1
#endif
#if EMU_CAN_STARTAUP_ENABLED ==1

//инициализация работы сети
robo_result_t ROBO_EXPORT_RUNTIME_DECL dev_startup(emu_dev_agent_p _agent){
	return emuCan::instance().startup(_agent);
}
#endif

void emuCanConnection::close_(void){
	if (ether_){
		if (shared_){
			shared_->used = EMU_CAN_SHARED_UNUSED;
			shared_ = 0;
		}
		ether_->changed = EMU_CAN_CHANGED;
		ether_ = nullptr;
	}

	robo_os_shared_release(&shared_memo_);

}


void emu_can_test_on_event(robo_can_p /* _can*/, robo_can_event_t _ev){
	switch (_ev){
	case ROBO_CAN_FAULT:
		refuse_count++;
    default:;
	}
}
void emu_can_test_on_receive(robo_can_p /*_can*/, robo_can_msg_id_t /*_id*/, robo_byte_p /*_data*/, robo_size_t /*_size*/){
	receive_count++;

}

robo_result_t emuCanConnection::open_(emu_can_port_p _port){
	
	char buf[ROBO_CAN_CHANNEL_SHARED_FILE_NAME_LEN + 1];
	robo_sprintf(buf, ROBO_CAN_CHANNEL_SHARED_FILE_NAME_LEN, RS(ROBO_CAN_CHANNEL_SHARED_FILE_NAME), _port->can.channel);
	
	ROBO_CHECKRET(robo_os_shared_create(buf, &shared_memo_))
	ether_ = (emu_can_ether_p)(shared_memo_.memo);
	robo_os_shared_lock(&shared_memo_);
	emu_can_shared_p shared = ether_->shareds;
	for (int i = 0; i < ROBO_CAN_CONNECTION_COUNT; i++, shared++) {
		if (shared->used == EMU_CAN_SHARED_UNUSED) {
			shared_ = shared;
			shared->used = EMU_CAN_SHARED_USED;
			ether_->changed = EMU_CAN_CHANGED;
			robo_os_shared_unlock(&shared_memo_);
			return ROBO_SUCCESS;
		}
	}
	robo_os_shared_unlock(&shared_memo_);
	ROBO_RETERR("Any empty slot is not found");
		
	return ROBO_SUCCESS;
}


robo_result_t ROBO_DECL  robo_can_open(robo_can_p _can){
	ROBO_RETEX( emuCanConnection::open((emu_can_port_p)_can) );
}
void ROBO_DECL  robo_can_close(robo_can_p _can){
	emuCanConnection::close((emu_can_port_p)_can);
}
robo_result_t ROBO_DECL  robo_can_send(robo_can_p _can, robo_can_msg_id_t _id, robo_byte_p _buf, robo_size_t _len){
	return emuCanConnection::send((emu_can_port_p)_can, _id, _buf, _len);
}
robo_result_t ROBO_DECL  robo_can_lock(robo_can_p /*_can*/){
	return ROBO_SUCCESS;
}
robo_result_t ROBO_DECL  robo_can_unlock(robo_can_p /*_can*/){
	return ROBO_SUCCESS;
}
robo_result_t ROBO_DECL  robo_can_ready(robo_can_p _can){
	ROBO_RETEX(emuCanConnection::ready((emu_can_port_p)_can));
}
void ROBO_DECL  robo_can_reset(robo_can_p _can){
	emuCanConnection::close((emu_can_port_p)_can);
	ROBO_LOGERR(emuCanConnection::open((emu_can_port_p)_can));
}
void ROBO_DECL emu_can_port_poll(emu_can_port_p _port){
	emuCanConnection::poll(_port);
}


