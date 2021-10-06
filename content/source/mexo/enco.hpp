#ifndef mexo_sence_hpp
#define mexo_sence_hpp
#include "mexo/mexo.hpp"
#include "mexo/math.hpp"
namespace mexo {
	namespace enco {
		template <typename q, typename D> class increment
			: public sence_handler< typename q::lobg_signal_t>, private D {
			typedef  sence_handler< typename q::lobg_signal_t> A;
		public:
			typedef typename q::signal_t signal_t;
			typedef typename q::long_signal_t long_signal_t;
			typedef typename q::parameter_t parameter_t;

			struct config_s {
				typename A::config_s	sb;
				parameter_t scale;
				unsigned shift;
			};

			struct present_s {
				typename A::present_s	sb;
				//long_signal_t native;
			};

		protected:
			virtual void do_handler_adjust(void) {}

			void execute(void) {
				present_s& present = handler::present_cast<present_s>();
				const config_s& config = handler::config_cast<config_s>();
				long_signal_t tmp = (long_signal_t)D::native() * config.scale;
				present.sb.output = q::round_s(tmp, config.shift);
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