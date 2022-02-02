#ifndef mexo_sence_hpp
#define mexo_sence_hpp
#include "mexo/mexo.hpp"
#include "mexo/math.hpp"
namespace mexo {
	namespace enco {
		template <typename q, typename D > class increment
			: public handler, protected D {
		public:
			typedef typename D::unative_t unative_t;
			typedef typename D::native_t native_t;
			typedef typename q::long_signal_t long_signal_t;
			struct config_s {
				handler::config_s	sb;
				typename q::parameter_t scale;
				unsigned native_shift;
			};

			struct present_s {
				handler::present_s	sb;
				struct {
					unative_t actual;
					unative_t ceiled;
					native_t delta;
				} native;
				struct{
					unsigned fault;
					unsigned total;
				} counter;
				long_signal_t delta;
				long_signal_t position;
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
					native_t tmp = present.native.actual << config.native_shift;
					native_t  tmp_delta = tmp - present.native.ceiled;
					present.native.ceiled = tmp;
					present.native.delta = (((native_t)(tmp_delta)) >> config.native_shift);
					present.delta = (typename q::signal_t)present.native.delta * config.scale;
				} else {
					present.counter.fault++;
					present.native.actual +=  present.native.delta;
					present.native.ceiled += (present.native.delta<<config.native_shift);
				}
				
				present.position += present.delta;
			}

			virtual bool do_handler_reconfig(void) {
				return true;
			}
		public:
			increment(const config_s& _config, present_s& _present)
				: handler(_config.sb, _present.sb) {}
		};

		template <class q, class D> class increment_t : public ::mexo::handler_t <
			subsystem_handler
			, increment<q, D >
			, prioritet_subsystem
		> {
			typedef handler_t <
				subsystem_handler
				, increment<types, D>
				, prioritet_subsystem
			> A;
		public:
			;
			increment_t(
				cstr _name
				, prioritet_subsystem* _owner
				, const A::config_s& _config
				, A::present_s& _present
			)
				: A(_name, _owner, _config, _present) {}
		};		
	}
}
#endif