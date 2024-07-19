#ifndef robosd_net_itrafic
#define robosd_net_itrafic
#include "core/robosd_common.hpp"
namespace robo {
	
	template <typename E>struct ROBO_EXPORT statistic_s{
		uint32_t request;
		uint32_t confirm;
		struct{
			uint32_t total;
			uint32_t detail[(unsigned)E::count];
		} refuse;
	};
	
	struct ROBO_EXPORT itrafic {
		struct agent {
			size_t prev = 0;
			size_t delta = 0;
			size_t total = 0;
			float rate = 0.f;
			void inc(size_t _sz);
			void reset(void);
			void tick1sec(void);
		};

		struct ROBO_EXPORT counter {
			agent bytes;
			agent packets;
			float load = 0.f;
			void inc(size_t _sz);
			void reset(void);
			void tick1sec(void);
		};

		struct ROBO_EXPORT counters {
			counter success;
			counter refuse;
			void reset(void);
			void tick1sec(void);
		};

		counters incom;
		counters outcom;

		void tick1sec(void);
		void reset(void);
		itrafic(void);
	};
}
#endif