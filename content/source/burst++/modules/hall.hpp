#ifndef burst_hall_hpp
#define burst_hall_hpp
#include "burst++/modules/actor.hpp"
namespace burst {
	namespace hall {
		class extra : public actor {
		public:
			enum class modes { qubic = 1, regress = 2, fuzzy = 3, none = 0 };
			#if 0
			using signal_t = typename number::signal_t;
			using long_signal_t = typename number::long_signal_t;

			long_signal_t angle32;
			signal_t angle;
			long_signal_t delta32;
			long_signal_t speed32;
			struct {
				signal_t prev;
				signal_t* pactual;
			} hall;
			uint32_t ticks;
			uint32_t begin;
			uint32_t lost;
			uint32_t period;
			bool on_sign_reset_flag;
			bool lost_flag;
			virtual void aproxx(void);
			virtual void interp(void);
			long_signal_t* speeds_history;
			int rank;
			#endif
		};

		union pins_s {
			struct {
				unsigned int A : 1;
				unsigned int B : 1;
				unsigned int C : 1;
			};
			unsigned int index;
		};

		template < class number > class ref_t : public actor {
		public:
			using signal_t = typename number::signal_t;
			using long_signal_t = typename number::long_signal_t;

			struct config_s {
				actor::config_s tag;
				struct {
					signal_t native;
					signal_t dynamic;
				} offset;
				bool inv;
				extra::modes extra_mode;
			};

			#define HALL_CONFIG(h) HALL_CONFIG_(h)
			#define HALL_CONFIG_(h)\
			{\
				ACTOR_CONFIG(h)\
				, {\
					h##_OFFSET_NATIVE\
					,h##_OFFSET_DYNAMIC\
				}\
				,h##_INV\
				,h##_EXTRA_MODE\
			}

			struct present_s {
				actor::present_s tag;
				struct {
					unsigned fault;
					unsigned total;
				} counter;
				int sector;
				pins_s pins;
				signal_t raw;
				signal_t angle;
				signal_t extra_angle;
				signal_t delta;
				signal_t delta_acc;
			};
			ref_t(const config_s& _config, present_s& _present)
				: actor(_config.tag, _present.tag) {};
			ref_t(const config_s& _config, present_s& _present, subsystem& _subsystem)
				: actor(_config.tag, _present.tag, _subsystem) {};
		};
		template < class number, class driver > class machine_t : public ref_t<number> {
			using B = ref_t<number>;
		public:
			using signal_t = typename number::signal_t;
			using long_signal_t = typename number::long_signal_t;

			typedef typename B::config_s config_s;
			typedef typename B::present_s present_s;
			union pins_s {
				struct {
					unsigned int A : 1;
					unsigned int B : 1;
					unsigned int C : 1;
				};
				unsigned int index;
			};
			static inline const int sectors[8] = {
				// B C A
				-1 //{ 0, 0, 0 }
				, 0//{ 0, 0, 1 }
				, 2//{ 0, 1, 0 }
				, 1 //{ 0, 1, 1 }
				, 4 //{ 1, 0, 0 }
				, 5 //{ 1, 0, 1 }
				, 3 //{ 1, 1, 0 }
				, -1 //{1, 1, 1}
			};
			static inline const int sdiffs[6] = {
				0, 1, 2, 3, -2, -1
			};
			static inline const signal_t one_div_3 = number::frac(1. / 3);
			static inline const signal_t two_div_3 = number::frac(2. / 3);
			static inline const signal_t one = number::frac(1.);
			static inline signal_t angles[6] = {
				0
				, one_div_3
				, two_div_3
				, one
				, -two_div_3
				, -one_div_3
			};
			


			machine_t(const config_s & _config, present_s& _present)
				: B(_config, _present) {};
			machine_t(const config_s& _config, present_s& _present, subsystem& _subsystem)
				: B(_config, _present, _subsystem) {};

			int prev_index = -1;
			int true_diff = 0;
			int sector_prev = -1;

			virtual void begin(void) {
				B::begin();
				ACTOR_PRESENT_S(p);
				p = {};
				prev_index = -1;
				true_diff = 0;
				sector_prev = -1;
			}
			virtual void run(void) {
				//int index = _pins->A + (_pins->B <<1) + (_pins->C <<2);
				ACTOR_PRESENT_S(p);
				ACTOR_CONFIG_S(cfg);
				driver::query(p.pins);
				int index = p.pins.index;
				if (index != prev_index) {
					prev_index = index;
					p.counter.total++;
					if (index > 0) {
						int sector = sectors[index];
						if (sector >= 0) {
							int delta = sector - sector_prev;
							sector_prev = sector;
							if (delta >= 0) {
								delta = sdiffs[delta];
							}
							else {
								delta = sdiffs[6 + delta];
							}
							if (delta == 3) {
								if (true_diff < 0) {
									delta = -3;
								}
							}
							else {
								true_diff = delta;
							}
							int raw;
							if (cfg.inv) {
								delta = -delta;
								raw = -angles[sector];
							}
							else {
								raw = angles[sector];
							}
							signal_t angle = raw + cfg.offset.native;
							if (delta > 0) {
								angle += cfg.offset.dynamic;
							}
							else if (delta < 0) {
								angle -= cfg.offset.dynamic;
							}
							p.delta_acc += delta;
							p.delta = delta;
							p.angle = angle;
							p.raw = raw;
							p.sector = sector;
							return;
						}
					}
					p.counter.fault++;
					//to do так делать нельзя, та как накапливается ошибка!
					p.delta = 0;
					p.delta_acc = 0;
				}
				else {
					p.delta = 0;
				}
			}
		};
	}
}
#endif