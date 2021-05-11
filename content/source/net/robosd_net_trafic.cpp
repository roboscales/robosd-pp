#include "net/robosd_net_trafic.hpp"
namespace robo {
	void itrafic::agent::reset(void) {
		prev = 0;
		delta = 0;
		total = 0;
		rate = 0.f;
	}

	void itrafic::agent::inc(size_t _sz) {
		total += _sz;
	}

	void itrafic::agent::tick1sec(void) {
		delta = total - prev;
		prev = total;
		rate = (rate * 3.f + delta) * 0.25f;
	}

	void itrafic::counter::reset(void) {
		bytes.reset();
		packets.reset();
		load = 0.f;
	}

	void itrafic::counter::tick1sec(void) {
		bytes.tick1sec();
		packets.tick1sec();
		if (packets.rate > 0.5f) {
			load = 0.f;
		}
		else {
			load = bytes.rate / packets.rate;
		}
	}
	void itrafic::counter::inc(size_t _sz) {
		bytes.inc(_sz);
		packets.inc(1);
	}


	void itrafic::counters::reset(void) {
		success.reset();
		refuse.reset();
	}

	void itrafic::counters::tick1sec(void) {
		success.tick1sec();
		refuse.tick1sec();
	}

	void itrafic::tick1sec(void) {
		incom.tick1sec();
		outcom.tick1sec();
	}
	void itrafic::reset(void) {
		incom.reset();
		outcom.reset();
	}
	itrafic::itrafic(void) {
		reset();
	}
}