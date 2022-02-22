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
				unative_t native_offset;
				output_t position_offset;
				bool inverce;
			};
			unative_t native_offset_prev =(unative_t)0;
			output_t position_offset_prev = (output_t)0;
			uint32_t start_pause_tick;

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
				doutput_t delta_acc;
			};
		protected:
			virtual void do_handler_adjust(void) {
			}

			void execute(void) {
				present_s& present = present_cast<present_s>();
				const config_s& config = config_cast<config_s>();
				if (config.native_offset != native_offset_prev || config.position_offset != position_offset_prev) {
					do_handler_reconfig();
				}
				present.counter.total++;
				if (start_pause_tick == 0) {
					if (!D::error()) {
						present.native.raw = D::encode();
						D::query();
						unative_t tmp = present.native.raw << shift;
						native_t  tmp_delta = (native_t)(tmp - present.native.ceiled);
						present.native.ceiled = tmp;
						//todo проверить на всехли компиляторах shift будет арифметический
						present.native.delta = (native_t)(tmp_delta >> shift);// (((native_t)(tmp_delta)) >> shift);
						if (config.inverce) {
							present.delta = -(doutput_t)q::round_l(present.native.delta , value_shift);
						}
						else {
							present.delta = (doutput_t)q::round_l(present.native.delta, value_shift);
						}
						present.delta_acc += present.delta;
					}
					else {
						present.counter.fault++;
						present.native.raw += present.native.delta;
						present.native.ceiled += (present.native.delta << shift);
						present.delta_acc += present.delta;
					}
					present.acc += present.native.delta;
					//todo round_l не катит
					present.position = q::round_l(present.acc, value_shift);
					present.position -= config.position_offset;
				}
				else {
					if (!D::error()) {
						present.native.raw = D::encode();
						present.native.ceiled = present.native.raw << shift;
						unative_t tmp = present.native.ceiled - config.native_offset;
						if (config.inverce) {
							present.acc = (output_t)(std::numeric_limits<unative_t>::max() - tmp);
						}
						else {
							present.acc = (output_t)tmp;
						}
						present.position = q::round_l(present.acc, value_shift);
						present.position -= config.position_offset;
						start_pause_tick--;
					}
					else {
						present.counter.fault++;
					}

				}
			}
			virtual void do_handler_create_vars(var::record::list& _vars, int _master_key) {
				handler::do_handler_create_vars(_vars, _master_key);
				present_s& present = present_cast<present_s>();
				if (var::machine::actual_mode() >= var::machine::mode::full) {
					var::record::create(::mexo::var::uint32, present.native.raw, RT("native"), _master_key, _vars);
					var::record::create(::mexo::var::uint32, present.counter.fault, RT("cnt.fault"), _master_key, _vars);
					var::record::create(::mexo::var::uint32, present.counter.total, RT("cnt.tot"), _master_key, _vars);
					var::record::create(q::var::signal, present.delta, RT("delta"), _master_key, _vars);
					var::record::create(q::var::signal, present.delta_acc, RT("delta_acc"), _master_key, _vars);
					var::record::create(q::var::long_signal, present.position, RT("po"), _master_key, _vars);
				}
			}

			virtual bool do_handler_reconfig(void) {
				present_s& present = present_cast<present_s>();
				const config_s& config = config_cast<config_s>();
				present.native = {};
				present.counter = {};
				start_pause_tick = 1 << config.init_count_shift;
				position_offset_prev = config.position_offset;
				native_offset_prev = config.native_offset;
				return true;
			}
		public:
			increment_machine(const config_s& _config, present_s& _present)
				: handler(_config.sb, _present.sb) {}
			const doutput_t& delta_ref(void) { return  present_cast<present_s>().delta; }
			doutput_t& delta_acc_ref(void) { return  present_cast<present_s>().delta_acc; }
			const output_t& position_ref(void) { return  present_cast<present_s>().position; }
		};

		template <typename q, typename D, uint8_t raw_resolution, uint8_t actual_resolution > class increment32_t
			: public increment_machine <
					q
					, D
					, 32
					, raw_resolution
					, actual_resolution
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

		template <typename q, typename D, uint8_t raw_resolution, uint8_t actual_resolution> class increment16_t
			: public
			increment_machine <
			q
			, D
			, 16
			, raw_resolution
			, actual_resolution
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

		template <typename q, typename D, uint8_t raw_resolution, uint8_t actual_resolution > class increment64_t
			: public
			increment_machine <
			q
			, D
			, 32
			, raw_resolution
			, actual_resolution
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

		template<typename q, typename enco> class rotator_t
			: public function_handler< typename enco::unative_t, cs_t<q> > {
			typedef function_handler< typename enco::unative_t, cs_t<q> > A;
		public:
			typedef typename q::signal_t signal_t;
			typedef typename q::long_signal_t long_signal_t;
			typedef  typename enco::unative_t unative_t;
			struct config_s {
				typename A::config_s fb;
				unative_t offset;
				bool inverce;
				uint16_t pole_count;
			};
			struct present_s {
				typename A::present_s fb;
				bool active;
			};
		protected:

			#if ROBO_APP_MEXO_VAR_ENABLED == 1
			virtual void do_handler_create_vars(var::record::list& _vars, int _master_key) {
				A::do_handler_create_vars(_vars, _master_key);
				present_s& present = present_cast<present_s>();
				const config_s& config = config_cast<config_s>();
				if (var::machine::actual_mode() >= var::machine::mode::full) {
					var::record::create(var::const_uint8, present.active, RT("active"), _master_key, _vars);
					var::record::create(q::var::const_signal, present.fb.output.si, RT("ab.sin"), _master_key, _vars);
					var::record::create(q::var::const_signal, present.fb.output.co, RT("ab.cos"), _master_key, _vars);
					var::record::create(q::var::const_signal, present.fb.output.angle, RT("dq.angle"), _master_key, _vars);

				}
				if (var::machine::actual_mode() >= var::machine::mode::tuning) {
					var::record::create(::mexo::var::uint16, config.pole_count, RT("pole_count"), _master_key, _vars);
					var::record::create(::mexo::var::uint8, config.inverce, RT("inverce"), _master_key, _vars);
					if (sizeof(unative_t) == sizeof(signal_t)) {
						var::record::create(q::var::usignal, config.offset, RT("offset"), _master_key, _vars);
					}
					else {
						var::record::create(q::var::ulong_signal, config.offset, RT("offset"), _master_key, _vars);
					}
				}
			};
			#endif


			void execute(void) {
				present_s& present = present_cast<present_s>();
				const config_s& config = config_cast<config_s>();
				if (present.active) {
					unative_t tmp;
					if (config.inverce) {
						tmp = std::numeric_limits<unative_t>::max() - A::input;
					}
					else {
						tmp = A::input;
					}
					tmp *= config.pole_count;
					tmp -= config.offset;
					if (sizeof(unative_t) == sizeof(signal_t)) {
						present.fb.output.rotate((q::signal_t)tmp);
					}
					else {
						present.fb.output.rotate(q::scale_l((long_signal_t)tmp));
					}
				}
			}
			bool do_handler_reconfig(void) {
				present_cast<present_s>().fb.output.rotate((signal_t)0);
				on();
				return true;
			}
			virtual void do_handler_adjust(void) {
				present_cast<present_s>().fb.output.rotate(q::scale_l(A::input));
			}

		public:
			rotator_t(const config_s& _config
					  , present_s& _present
					  , unative_t& _input
			)
				: A(_config.fb, _present.fb, _input) {
			}
			void on() { present_cast<present_s>().active = true; };
			void off() { present_cast<present_s>().active = false; };
			void angle_set(signal_t _angle) {
				present_s& present = present_cast<present_s>();
				if (present.active == false) {
					present.fb.output.rotate(_angle);
				}
			}

		};

	
	}
}
#endif