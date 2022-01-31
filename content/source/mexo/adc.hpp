#ifndef mexo_adc_hpp
#define mexo_adc_hpp
#include "mexo/mexo.hpp"
#include "mexo/math.hpp"
namespace mexo {
	namespace adc {
		template <typename q, typename D, unsigned C> class machine
			: public sence_handler<robo::array<typename q::signal_t, C> >, protected D {
		public:
			typedef robo::array<typename q::signal_t, C> out_t;
			typedef sence_handler<out_t> A;
			typedef typename q::signal_t signal_t;
			typedef typename q::long_signal_t long_signal_t;
			typedef typename q::parameter_t parameter_t;
			typedef typename q::discret_t discret_t;
			typedef typename q::long_discret_t long_discret_t;

			struct config_s {
				typename A::config_s	sb;
				const unsigned int index[C];
				const parameter_t scale[C];
				const unsigned init_count_shift;
				bool ready;
			};

			struct present_s {
				typename A::present_s	sb;
				discret_t native[C];
				discret_t offset[C];
				long_discret_t acc[C];
			};
			//todo govnocod
			enum { vns = 7 };
			char varnames[vns * C * 2];

		private:
			unsigned int init_count_ = 0;
			void reset_(void) {
				long_discret_t* a = handler::present_cast<present_s>().acc;
				for (int i = 0; i < C; ++i, ++a) {
					*a = 0;
				}
				init_count_ = 1 << handler::config_cast<config_s>().init_count_shift;
			}
		public:
			void reset(void) {
				present_s& present = handler::present_cast<present_s>();
				present.ready = false;
				reset_();
			}
		protected:
			virtual void do_handler_adjust(void) {}

			void execute(void) {
				present_s& present = handler::present_cast<present_s>();
				const config_s& config = handler::config_cast<config_s>();

				discret_t* n = present.native;
				const unsigned int* ix = config.index;
				for (int i = 0; i < C; ++i, ++n, ++ix) {
					*n = D::raw[*ix];
				}
				if (present.ready) {
					signal_t* v = present.sb.output.values;
					discret_t* n = present.native;
					const parameter_t* s = config.scale;
					discret_t* o = present.offset;
					for (int i = 0; i < C; ++i, ++v, ++n, ++s, ++o) {
						*v = *s * ((parameter_t)*n - *o);
					}
				}
				else {
					long_discret_t* a = present.acc;
					discret_t* n = present.native;
					for (int i = 0; i < C; ++i, ++a, ++n) {
						*a += *n;
					}
					init_count_--;
					if (init_count_ == 0) {
						signal_t* v = present.sb.output.values;
						discret_t* n = present.native;
						const parameter_t* s = config.scale;
						discret_t* o = present.offset;
						long_discret_t* a = present.acc;

						for (int i = 0; i < C; ++i, ++v, ++n, ++s, ++o, ++a) {
							*o = (discret_t)(((long_discret_t)*a + (1 << (config.init_count_shift - 1))) >> config.init_count_shift);
							*v = *s * (*n - *o);
						}

						present.ready = true;
					}
				}
				D::query();
			}

			virtual bool do_handler_reconfig(void) {
				reset();
				D::query();
				return true;
			}
			#if ROBO_APP_MEXO_VAR_ENABLED == 1
			virtual void do_handler_create_vars(var::record::list& _vars, int _master_key) {
				A::do_handler_create_vars(_vars, _master_key);
				present_s& present = A::present_cast<present_s>();
				if (var::machine::actual_mode() >= var::machine::mode::full) {
					string key;
					char* vn = varnames;
					for (int i = 0; i < C; ++i) {
						size_t n = ::robo::system::sprintf(vn, vns, RT("nat%d"), i);
						//todo static string class!
						var::record::create(q::var::const_discret, present.native[i], vn, _master_key, _vars);
						vn += (n + 1);
						n = ::robo::system::sprintf(vn, vns, RT("ofs%d"), i);
						var::record::create(q::var::const_discret, present.offset[i], vn, _master_key, _vars);
						vn += (n + 1);
					}
					var::record::create(::mexo::var::uin8_t, present.ready, RT("ready"), _master_key, _vars);
					/*var::record::create(types::var::const_discret, config.duty.low, RT("duty.low"), _master_key, _vars);
					var::record::create(types::var::const_discret, config.duty.hi, RT("duty.hi"), _master_key, _vars);
					var::record::create(types::var::const_signal, config.voltage.low, RT("v.low"), _master_key, _vars);
					inverter.create_var(_master_key, _vars);*/
				}
			}
			#endif

		public:
			machine(const config_s& _config, present_s& _present)
				: A(_config.sb, _present.sb) {}
			bool ready(void) { return init_; }
		};


		template <typename q, typename D>  class single_machine
			: public sence_handler< typename q::signal_t > {
		public:
			typedef sence_handler< typename q::signal_t > A;
			typedef typename q::signal_t signal_t;
			typedef typename q::long_signal_t long_signal_t;
			typedef typename q::parameter_t parameter_t;
			typedef typename q::discret_t discret_t;
			typedef typename q::long_discret_t long_discret_t;

			struct config_s {
				typename A::config_s	sb;
				unsigned int index;
				parameter_t scale;
				unsigned int  init_count_shift;
			};

			struct present_s {
				typename A::present_s	sb;
				discret_t native;
				discret_t offset;
				long_discret_t acc;
			};

		private:
			unsigned int init_count_ = 0;
			bool init_ = false;
			void reset_(void) {
				handler::present_cast<present_s>().acc = 0;
				init_count_ = 1 << handler::config_cast<config_s>().init_count_shift;
			}
		public:
			void reset(void) {
				bool init_ = false;
				reset_();
			}
		protected:
			void execute(void) {
				present_s& present = handler::present_cast<present_s>();
				const config_s& config = handler::config_cast<config_s>();
				present.native = D::raw[config.index];
				if (init_) {
					present.sb.output = config.scale * (present.native - present.offset);
				}
				else {
					present.acc += present.native;
					init_count_--;
					if (init_count_ == 0) {
						present.offset = (present.acc + (1 << (config.init_count_shift - 1))) >> config.init_count_shift;
						present.sb.output = config.scale * (present.native - present.offset);
						init_ = true;

					}
				}
				D::query();
			}
			virtual void do_handler_adjust(void) {}

			virtual bool do_handler_reconfig(void) {
				reset();
				D::query();
				return true;
			}

			single_machine(const config_s& _config, present_s& _present)
				: A(_config.sb, _present.sb) {}
			const typename q::signal_t& output(void) { return handler::present_cast<present_s>().sb.output; }
			bool ready(void) { return handler::present_cast<present_s>().ready; }
		};

		/*template < typename q, typename D  > class diff_adc : public adc<q, D, 2> {
			typedef adc<q, D, 2> B;
		public:
			typedef typename B::config_s config_s;
			struct present_s {
				typename B::present_s	adc;
				typename q::signal_t output;
			};


			diff_adc(const config_s& _config, present_s& _present)
				: B(_config, _present.adc) {}
			const typename q::signal_t& output(void) { return handler::present_cast<present_s>().output; }

		protected:
			void execute(void) {
				B::execute();
				present_s& present = handler::present_cast<present_s>();
				present.output = present.adc.sb.output.values[1] - present.adc.sb.output.values[0];
			}
			virtual void do_handler_adjust(void) {}
		};*/


		template<typename q, typename D> class sensor : public machine<q, D, D::converter::count >
			, prioritet_subsystem
					> {
				typedef  adc<q, D, D::converter::count>	A;
		public:
			typedef typename D::adapter_t converter_t;
			struct present_s {
				typename A::present_s	adc;
				typename converter_t::present_s converter;
			};
			struct config_s {
				typename A::config_s	adc;
				typename converter_t::config_s converter;
			};
			converter_t  converter;
			adc_sensor(const adc::config_s& _config, adc::present_s& _present)
				: A(_config, _present), converter(_config.converter, _present.converter){}
			void execute(void) {
				A::execute();
				if (A::ready()) {
					present_s& present = handler::present_cast<present_s>();
					converter.update(present.adc.sb.output);
				}
			}
			virtual bool do_handler_reconfig(void) {
				A::reset();
				converter::reset();
				return true;
			}
			/*types::signal_t& current_ref(void) {
				adc::present_s& present = adc::present_cast<present_s>();
				return present.sb.output.values[0];
			};
			types::signal_t& current_delta_ref(void) {
				adc::present_s& present = adc::present_cast<present_s>();
				return present.sb.delta.values[0];
			};*/
		};

		template<typename q> struct current_dc_converter {
			struct present_s {
				typename q::signal_t current;
			};
			struct config_s {
			};
			present_s & present;
			void update(typename q::discret_t * _values) {
				present.current = _values[1] - _values[0];
			}
			current_dc_converter(config_s /*& _config*/, present_s& _present) : present(_present) {}
			typename q::signal_t& current_ref(void) { return present.current;  }
		};

		template<typename q> struct current_abc_converter {
			typedef typename q::signal_t signal_t;
			typedef typename q::discret_t discret_t;
			typedef typename q::long_signal_t long_signal_t;
			typedef abc_t<q> abc_t;
			typedef ab_t<q> ab_t;
			typedef dq_t<q> dq_t;

			struct present_s {
				typename q::signal_t output;
				abc_t abc;
				ab_t ab;
				dq_t dq;
			};
			struct config_s {
			};
			present_s& present;
			void update(typename q::discret_t* _values) {
				present.output = _values[1] - _values[0];
			}
			current_dc_converter(present_s& _present) : present(_present) {}
			typename q::signal_t& current_ref(void) { return present.dq.cross; }
			typename q::signal_t& lat_current_ref(void) { return present.dq.lateral; }
			void rotate(typename q::signal_t _angle) { present.dq.angle = _angle;  }
		};
	}

}
#endif