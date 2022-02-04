#ifndef mexo_sence_hpp
#define mexo_sence_hpp
#include "mexo/mexo.hpp"
#include "mexo/math.hpp"
namespace mexo {
	namespace enco {
		template <
			typename q
			, typename D
			, uint8_t round_resolution
			, uint8_t raw_resolution
			, uint8_t actual_resolution
			, bool inverce
			, typename native_t
			, typename unative_t
			, typename doutput_t
			, typename output_t
		> class increment_machine
			: public handler, protected D {
		public:
			static  inline const  int shift = (round_resolution - raw_resolution);
			static  inline const int value_shift = (raw_resolution - actual_resolution);
			struct config_s {
				handler::config_s	sb;
				uint8_t init_count_shift;
			};
			uint32_t start_pause_tick;;

			struct present_s {
				handler::present_s	sb;
				struct {
					unative_t raw;
					unative_t ceiled;
					native_t delta;
				} native;
				struct{
					unsigned fault;
					unsigned total;
				} counter;
				doutput_t delta;
				output_t acc;
				output_t position;
			};
		private:
			::robo::delegat::base<void> * converter_ = nullptr;
		protected:
			virtual void do_handler_adjust(void) {
			}

			void execute(void) {
				present_s& present = handler::present_cast<present_s>();
				const config_s& config = handler::config_cast<config_s>();
				present.counter.total++;
				if (start_pause_tick == 0) {
					if (!D::error()) {
						present.native.raw = D::encode();
						D::query();
						unative_t tmp = present.native.raw << shift;
						native_t  tmp_delta = (native_t)(tmp - present.native.ceiled);
						present.native.ceiled = tmp;
						//todo проверить на всехли компиляторах shift будет арифметический
						present.native.delta = (((native_t)(tmp_delta)) >> shift);
						if (inverce) {
							present.delta = -(doutput_t)(present.native.delta);
						}
						else {
							present.delta = (doutput_t)(present.native.delta);
						}
					}
					else {
						present.counter.fault++;
						present.native.raw += present.native.delta;
						present.native.ceiled += (present.native.delta << shift);
					}
					present.acc += present.delta;
					present.position = present.acc >> value_shift;
					if ( converter_ != nullptr ) {
						(*converter_)();
					}
				}
				else {
					if (!D::error()) {
						present.native.raw = D::encode();
						present.native.ceiled = present.native.raw << shift;
						if (inverce) {
							present.acc = -(output_t)present.native.raw;
						}
						else {
							present.acc = (output_t)present.native.raw;
						}
						present.position = present.acc >> value_shift;
						start_pause_tick--;
					}
					else {
						present.counter.fault++;
					}

				}
			}
			virtual void do_handler_create_vars(var::record::list& _vars, int _master_key) {
				handler::do_handler_create_vars(_vars, _master_key);
				present_s& present = handler::present_cast<present_s>();
				if (var::machine::actual_mode() >= var::machine::mode::full) {
					var::record::create(::mexo::var::uint32, present.native.raw, RT("native"), _master_key, _vars);
					var::record::create(::mexo::var::uint32, present.counter.fault, RT("cnt.fault"), _master_key, _vars);
					var::record::create(::mexo::var::uint32, present.counter.total, RT("cnt.tot"), _master_key, _vars);
					var::record::create(q::var::signal, present.delta, RT("delta"), _master_key, _vars);
					var::record::create(q::var::long_signal, present.position, RT("po"), _master_key, _vars);
				}
			}

			virtual bool do_handler_reconfig(void) {
				present_s& present = handler::present_cast<present_s>();
				const config_s& config = handler::config_cast<config_s>();
				present.native = {};
				present.counter = {};
				start_pause_tick = 1 << config.init_count_shift;
				return true;
			}
		public:
			increment_machine(const config_s& _config, present_s& _present)
				: handler(_config.sb, _present.sb) {}
			void set_converte(::robo::delegat::base<void> * _converter) {
				converter_ = _converter;
			}
			const present_s& present(void) { return  handler::present_cast<present_s>() };
		};

		template <typename q, typename D, uint8_t raw_resolution, uint8_t actual_resolution, bool inverce > class increment32_t
			: public increment_machine <
					q
					, D
					, 32
					, raw_resolution
					, actual_resolution
					, inverce
					, int32_t
					, uint32_t
					, typename q::signal_t
					, typename q::long_signal_t
				>
			{
			typedef increment_machine <
				q
				, D
				, 32
				, raw_resolution
				, actual_resolution
				, inverce
				, int32_t
				, uint32_t
				, typename q::signal_t
				, typename q::long_signal_t
			> A;
			protected:

		public:
			increment32_t(const typename A::config_s& _config, typename A::present_s& _present)
				: A(_config, _present) {}
		};

		template <typename q, typename D, uint8_t raw_resolution, uint8_t actual_resolution, bool inverce > class increment16_t
			: public
			increment_machine <
			q
			, D
			, 16
			, raw_resolution
			, actual_resolution
			, inverce
			, int16_t
			, uint16_t
			, typename q::signal_t
			, typename q::long_signal_t
			>
		{
			typedef increment_machine <
				q
				, D
				, 16
				, raw_resolution
				, actual_resolution
				, inverce
				, int16_t
				, uint16_t
				, typename q::signal_t
				, typename q::long_signal_t
			> A;
		protected:

		public:
			increment16_t(const typename A::config_s& _config, typename A::present_s& _present)
				: A(_config, _present) {}
		};

		template <typename q, typename D, uint8_t raw_resolution, uint8_t actual_resolution, bool inverce > class increment64_t
			: public
			increment_machine <
			q
			, D
			, 32
			, raw_resolution
			, actual_resolution
			, inverce
			, int32_t
			, uint32_t
			, typename int32_t
			, typename int64_t
			>
		{
			typedef increment_machine <
				q
				, D
				, 32
				, raw_resolution
				, actual_resolution
				, inverce
				, int32_t
				, uint32_t
				, typename int32_t
				, typename int64_t
			> A;
		protected:

		public:
			increment64_t(const typename A::config_s& _config, typename A::present_s& _present)
				: increment_machine(_config, _present) {}
		};
	
	}
}
#endif