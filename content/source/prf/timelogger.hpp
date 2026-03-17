#ifndef robosd_prf_timelogger_hpp
#define robosd_prf_timelogger_hpp
#include <stdint.h>

namespace robo {
	namespace prf {
		template <typename P, size_t N > class timelogger_t: public P {
		public:
			struct record_s {
				uint32_t ns;
				struct {
					uint32_t event:31;
					uint32_t dir : 1;
				};

			};
			struct records_s {
				int count;
				record_s* items;
			};
		private:

			records_s records[2] = {};
			record_s pool[2][N] = {};
			records_s* precords_ = records;
			int ix_ = 0;
			uint32_t begin_ = 0;;
			timelogger_t(void) {
				records[0].items = pool[0];
				records[1].items = pool[1];
			}
			records_s* get_(void) {
				typename P::guard g__;
				begin_ = P::time_ns();
				records_s* ret = precords_;
				ix_ = 1 - ix_;
				precords_ = records + ix_;
				precords_->count = 0;
				return ret;
			}
			static timelogger_t& instance_(void) {
				static timelogger_t instance__;
				return instance__;
			}
			void event_(uint32_t _event, uint32_t _dir) {
				int& cnt = precords_->count;
				if (cnt < N) {
					auto& p = precords_->items[cnt];
					p.dir = _dir;
					uint32_t now = P::time_ns();
					p.ns = now - begin_;
					begin_ = now;
					p.event = _event;
					cnt++;
				}
			}
			void raise_(uint32_t _event) {
				event_(_event, 1);
			}
			void fail_(uint32_t _event) {
				event_(_event, 0);
			}
		public:
			static void raise(uint32_t _event) {
				instance_().raise_(_event);
			}
			static void fail(uint32_t _event) {
				instance_().fail_(_event);
			}
			static const records_s* get(void) {
				return instance_().get_();
			}
		};

		template <typename D, typename P, size_t N > class datalogger_t : public P {
		public:
			struct record_s {
				uint32_t ns;
				D value;
			};
			struct records_s {
				int count;
				record_s* items;
			};
		private:

			records_s records[2] = {};
			record_s pool[2][N] = {};
			records_s* precords_ = records;
			int ix_ = 0;
			uint32_t begin_ = 0;;
			datalogger_t(void) {
				records[0].items = pool[0];
				records[1].items = pool[1];
			}
			records_s* get_(void) {
				typename P::guard g__;
				begin_ = P::time_ns();
				records_s* ret = precords_;
				ix_ = 1 - ix_;
				precords_ = records + ix_;
				precords_->count = 0;
				return ret;
			}
			static datalogger_t& instance_(void) {
				static datalogger_t instance__;
				return instance__;
			}
			void save_(const D & _value) {
				int& cnt = precords_->count;
				if (cnt < N) {
					auto& p = precords_->items[cnt];
					uint32_t now = P::time_ns();
					p.ns = now - begin_;
					begin_ = now;
					p.value = _value;
					cnt++;
				}
			}
		public:
			static void save(const D & _value) {
				instance_().save_(_value);
			}
			static const records_s* get(void) {
				return instance_().get_();
			}
		};

	}

}
#endif 