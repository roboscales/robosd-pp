#ifndef mexo_sence_hpp
#define mexo_sence_hpp
#include "mexo/mexo.hpp"
#include "mexo/math.hpp"
namespace mexo {
	namespace enco {
		template <typename q, typename D > class increment
			: public sence_handler< typename q::long_signal_t, typename q::signal_t > {
			typedef  sence_handler< typename q::long_signal_t, typename q::signal_t > A;
		public:
			typedef typename D::unative_t unative_t;
			typedef typename D::native_t native_t;

			struct config_s {
				typename A::config_s	sb;
				typename q::parameter_t scale;
				unsigned native_shift;
			};

			struct present_s {
				typename A::present_s	sb;
				struct {
					unative_t actual;
					unative_t ceiled;
					native_t delta;
				} native;
				struct{
					unsigned fault;
					unsigned total;
				} counter;
			};
		private:
		protected:
			virtual void do_handler_adjust(void) {}

			void execute(void) {
				present_s& present = handler::present_cast<present_s>();
				const config_s& config = handler::config_cast<config_s>();
				present.counter.total++;
				if(!D::error()){
					present.native.actual = D::native();
				} else {
					present.counter.fault++;
					present.native.actual +=  present.native.delta;
					present.native.ceiled += (present.native.delta<<config.native_shift);
					present.sb.output += present.sb.delta;
				}
				
				native_t tmp = present.native.actual << config.native_shift;
				native_t  tmp_delta = tmp - present.native.ceiled;
				present.native.ceiled = tmp;
				present.native.delta = (((native_t)(tmp_delta)) >> config.native_shift);
				present.sb.delta = (typename q::signal_t)present.native.delta * config.scale;
				present.sb.output += present.sb.delta;
			}

			virtual bool do_handler_reconfig(void) {
				return true;
			}
		public:
			increment(const config_s& _config, present_s& _present)
				: A(_config.sb, _present.sb) {}
		};
	}
}
#endif