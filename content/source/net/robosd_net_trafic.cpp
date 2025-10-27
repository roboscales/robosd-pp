#include "net/robosd_net_trafic.hpp"
namespace robo {
	namespace net {
		void trafic_s::agent::reset(void) {
			prev = 0;
			delta = 0;
			total = 0;
			rate = 0.f;
		}

		void trafic_s::agent::inc(size_t _sz) {
			total += _sz;
		}

		void trafic_s::agent::tick1sec(void) {
			delta = total - prev;
			prev = total;
			rate = (rate * 3.f + delta) * 0.25f;
		}

		void trafic_s::counter::reset(void) {
			bytes.reset();
			packets.reset();
			load = 0.f;
		}

		void trafic_s::counter::tick1sec(void) {
			bytes.tick1sec();
			packets.tick1sec();
			if (packets.rate > 0.5f) {
				load = 0.f;
			}
			else {
				load = bytes.rate / packets.rate;
			}
		}
		void trafic_s::counter::inc(size_t _sz) {
			bytes.inc(_sz);
			packets.inc(1);
		}


		void trafic_s::counters::reset(void) {
			success.reset();
			refuse.reset();
		}

		void trafic_s::counters::tick1sec(void) {
			success.tick1sec();
			refuse.tick1sec();
		}

		void trafic_s::tick1sec(void) {
			incom.tick1sec();
			outcom.tick1sec();
		}
		void trafic_s::reset(void) {
			incom.reset();
			outcom.reset();
		}
		trafic_s::trafic_s(void) {
			reset();
		}
	}
}