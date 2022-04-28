#include "core/robosd_backend.hpp"
#include "core/robosd_ring_buf.hpp"
#include "net/robosd_net_master.hpp"

class can8_bus : public robo::backend::bus {
	enum { master_buf_size = 10, idle_id = 0xFF};
	uint8_t in_buffer_[master_buf_size] = {};
	uint8_t out_buffer_[master_buf_size] = {};
	robo::net::master& channel_;
	msg message_;
protected:
	virtual bool do_load(void) {
		ROBO_LBREAKN(::robo::backend::bus::do_load());
		return true;
	}

	::robo::delegat::smember<can8_bus, void, bool > confirm_delegat;

	uint16_t wait_id_ = idle_id;
	void exchange_confirm(bool _result) {
		if(wait_id_!= idle_id){
			wait_id_ = idle_id;
			if (_result) {
				std::copy_n(in_buffer_ + 2, 8, message_.tran.data);
				bus::confirm(ROBO_TRAN_COMPLETE);
			}
			else {
				bus::confirm(ROBO_TRAN_REFUSE);
			}
		}
	}


	virtual bool post(msg* /*_msg*/ ) {
		uint16_t id = ((message_.address & 0xF) << 4) + ((message_.suba) & 0xF);
		*((uint16_t*)in_buffer_) = wait_id_ = 0x400 + id;
		if (message_.tran.request == ROBO_TRAN_EXCANGE) {
			*((uint16_t*)out_buffer_) = 0x200 + id;
			std::copy_n(message_.tran.data, message_.tran.size_actual, out_buffer_ + 2);
			channel_.exchange(out_buffer_, message_.tran.size_actual + 2, in_buffer_, message_.tran.size_actual + 2, &confirm_delegat);
		}
		else if (message_.tran.request == ROBO_TRAN_REQUEST_PUT) {
			*((uint16_t*)out_buffer_) = 0x200 + id;
			std::copy_n(message_.tran.data, message_.tran.size_actual, out_buffer_ + 2);
			channel_.exchange(out_buffer_, message_.tran.size_actual + 2, in_buffer_, 0, &confirm_delegat);
		}
		else if (message_.tran.request == ROBO_TRAN_REQUEST_GET) {
			*((uint16_t*)out_buffer_) = 0x00 + id;
			out_buffer_[2] = 6;
			channel_.exchange(out_buffer_, 3, in_buffer_, message_.tran.size_actual + 2, &confirm_delegat);
		}
		return true;
	}

	virtual void cancel(void) {
		channel_.cancel();
		if (wait_id_ != idle_id) {
			wait_id_ = idle_id;
			bus::confirm(ROBO_TRAN_REFUSE);
		}
	}
	virtual bool ready(void) {
		return channel_.ready();
	}
	//msg
	virtual msg* get_msg(void) {
		if (wait_id_ == idle_id) {
			return &message_;
		}
		else {
			return nullptr;
		}
	}
	virtual void  release_msg(msg* _msg) {
	}
public:
	can8_bus(robo::cstr _name, robo::app::module* _owner, robo::net::master& _channel)
		: robo::backend::bus(_name, _owner)
			, confirm_delegat(this, &can8_bus::exchange_confirm)
			, channel_(_channel) {
	}
};
