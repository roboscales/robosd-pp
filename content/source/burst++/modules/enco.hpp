#ifndef burst_enco_hpp
#define burst_enco_hpp
#include "burst++/modules/actor.hpp"
namespace burst {
	template<class number> class enco_t : public actor {
	public:
		using signal_t = typename number::signal_t;
		using long_signal_t = typename number::long_signal_t;
		struct config_s {
			actor::config_s tag;
		};
		#define ENCO_CONFIG(a) ENCO_CONFIG_(a)
		#define ENCO_CONFIG_(a)\
		{\
			ACTOR_CONFIG(a)\
		}
		struct present_s {
			actor::present_s tag;
			struct {
				unsigned fault;
				unsigned total;
			} counter;
			signal_t delta_acc;
			long_signal_t position;
			bool ready;
		};

		enco_t(const config_s& _config, present_s& _present)
			: actor(_config.tag, _present.tag) {};
		enco_t(const config_s& _config, present_s& _present, subsystem& _subsystem)
			: actor(_config.tag, _present.tag, _subsystem) {};

		virtual void begin(void) {
			ACTOR_PRESENT_S(p);
			p = {};			
		}
	};
	template<class number, class driver> class enco_abs32_t : public enco_t<number> {
		using B = enco_t<number>;
		using R = typename driver::native_t;
	public:
	
		using signal_t = typename number::signal_t;
		using long_signal_t = typename number::long_signal_t;
		struct  config_s {
			typename B::config_s ref;
			struct {
				uint8_t round;
				uint8_t raw;
				uint8_t actual;
			} resolution;
			uint8_t init_count_bits;
			struct {
				R native;
				long_signal_t position;
			} offset;
			bool inverce;
		};
		#define ENCO_ABS32_CONFIG(a) ENCO_ABS32_CONFIG_(a)
		#define ENCO_ABS32_CONFIG_(a)\
			{\
				ENCO_CONFIG(a)\
				,{\
					a##_RESOLUTION_ROUND\
					, a##_RESOLUTION_RAW\
					, a##_RESOLUTION_ACTUAL\
				}\
				, a##_INIT_COUNT_BITS\
				,{\
					a##_OFFSET_NATIVE\
					,a##_OFFSET_POSITION\
				}\
				,a##_INVERCE\
			}
		//burst_enco_t ref;
		//void (*query)(void);
		//uint32_t(*encode)(void);
		//burst_bool_t(*error)(void);
		uint32_t start_pause_tick = 0;
		struct present_s {
			typename B::present_s ref;
			struct {
				long_signal_t native;
				long_signal_t position;
			} offset;
			struct {
				int raw;
				int value;
			} shift;
			struct {
				R raw;
				uint32_t ceiled;
				int32_t delta;
			} native;
			signal_t delta;
			long_signal_t acc;
		};

		enco_abs32_t(const config_s& _config, present_s& _present)
			: B(_config.ref, _present.ref) {};
		enco_abs32_t(const config_s& _config, present_s& _present, subsystem& _subsystem)
			: B(_config.ref, _present.ref, _subsystem) {};

		virtual void begin(void) {
			B::begin();
			ACTOR_CONFIG_S(cfg);
			ACTOR_PRESENT_S(p);
			p.shift.raw = (cfg.resolution.round - cfg.resolution.raw);
			p.shift.value = (cfg.resolution.raw - cfg.resolution.actual);

			p.native = {};
			p.delta = 0;
			p.ref.position = cfg.offset.position;
			p.offset.native = cfg.offset.native;
			start_pause_tick = 1 << cfg.init_count_bits;
		}
		virtual void run(void) {
			ACTOR_CONFIG_S(conf);
			ACTOR_PRESENT_S(p);
			if (p.ref.ready && (conf.offset.native != p.offset.native || conf.offset.position != p.offset.position)) {
				begin();
				return;
			}
			p.ref.counter.total++;
			bool success = driver::query(p.native.raw);

			if (p.ref.ready) {
				if (success) {
					uint32_t tmp = fast::lsh(p.native.raw, p.shift.raw);
					int32_t  tmp_delta = (int32_t)(tmp - p.native.ceiled);
					p.native.ceiled = tmp;

					p.native.delta = fast::rsh(tmp_delta, p.shift.raw);// (((native_t)(tmp_delta)) >> shift);
					signal_t dtmp;
					if (conf.inverce) {
						dtmp = fast::rsh(-p.native.delta, p.shift.value);
					}
					else {
						dtmp = fast::rsh(p.native.delta, p.shift.value);
					}

					if (dtmp > number::max) {
						p.delta = p.ref.delta_acc = number::max;
					}
					else if (dtmp < number::min) {
						p.delta = p.ref.delta_acc = number::min;
					}
					else {
						p.delta = dtmp;
						long_signal_t adtmp = p.ref.delta_acc + p.delta;
						if (dtmp > number::max) {
							p.ref.delta_acc = number::max;
						}
						else if (dtmp < number::min) {
							p.ref.delta_acc = number::min;
						}
						else {
							p.ref.delta_acc = adtmp;
						}
					}
				}
				else {
					p.ref.counter.fault++;
					p.native.delta = p.ref.delta_acc = 0;
					//to do так делать нельзя, та как накапливается ошибка!
					/*present.native.raw += present.native.delta;
					present.native.ceiled += (present.native.delta << shift);
					present.delta_acc += present.delta;*/
				}
				if (conf.inverce) {
					p.acc -= p.native.delta;
				}
				else {
					p.acc += p.native.delta;
				}
				//todo round_l не катит
				p.ref.position = fast::rsh(p.acc, p.shift.value);
				p.ref.position += conf.offset.position;
			}
			else {
				if (success) {
					p.native.ceiled = fast::lsh(p.native.raw, p.shift.raw);
					uint32_t tmp = p.native.ceiled + conf.offset.native;
					if (conf.inverce) {
						tmp = 0xFFFFFFFF - tmp;
					}
					p.acc = fast::rsh(((long_signal_t)tmp), p.shift.raw);
					p.ref.position = fast::rsh(p.acc, p.shift.value);
					p.ref.position += conf.offset.position;
					start_pause_tick--;
					if (start_pause_tick == 0) {
						p.ref.ready = true;
					}
				}
				else {
					p.ref.counter.fault++;
				}
			}
		}
	};

}
#endif