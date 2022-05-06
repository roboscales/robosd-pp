#ifndef mexo_adc_hpp
#define mexo_adc_hpp
#include "mexo/mexo.hpp"
#include "mexo/math.hpp"
namespace mexo {
	namespace adc {
		template <typename q, typename D> class machine
			: public handler, protected D {
		public:
			typedef typename q::signal_t signal_t;
			typedef typename q::long_signal_t long_signal_t;
			typedef typename q::parameter_t parameter_t;
			typedef typename q::discret_t discret_t;
			typedef typename q::long_discret_t long_discret_t;
			enum { channel_count = D::channel_count };

			struct config_s {
				handler::config_s	sb;
				const unsigned int index[channel_count];
				const parameter_t scale[channel_count];
				const unsigned init_count_shift;
			};

			struct present_s {
				handler::present_s	sb;
				discret_t native[channel_count];
				discret_t offset[channel_count];
				long_discret_t acc[channel_count];
				discret_t values[channel_count];
				bool ready;
			};
			//todo govnocod
			enum { vns = 7 };
			char_t varnames[vns * channel_count * 2];

		private:
			unsigned int init_count_ = 0;
			void reset_(void) {
				long_discret_t* a = present_cast<present_s>().acc;
				for (int i = 0; i < channel_count; ++i, ++a) {
					*a = 0;
				}
				init_count_ = 1 << config_cast<config_s>().init_count_shift;
			}
		public:
			void reset(void) {
				present_s& present = present_cast<present_s>();
				present.ready = false;
				reset_();
			}
		protected:
			virtual void do_handler_adjust(void) {}

			void execute(void) {
				present_s& present = present_cast<present_s>();
				const config_s& config = config_cast<config_s>();

				discret_t* n = present.native;
				const unsigned int* ix = config.index;
				for (int i = 0; i < channel_count; ++i, ++n, ++ix) {
					*n = D::raw[*ix];
				}
				if (present.ready) {
					signal_t* v = present.values;
					discret_t* n = present.native;
					const parameter_t* s = config.scale;
					discret_t* o = present.offset;
					for (int i = 0; i < channel_count; ++i, ++v, ++n, ++s, ++o) {
						*v = *s * ((parameter_t)*n - *o);
					}
				}
				else {
					long_discret_t* a = present.acc;
					discret_t* n = present.native;
					for (int i = 0; i < channel_count; ++i, ++a, ++n) {
						*a += *n;
					}
					init_count_--;
					if (init_count_ == 0) {
						signal_t* v = present.values;
						discret_t* n = present.native;
						const parameter_t* s = config.scale;
						discret_t* o = present.offset;
						long_discret_t* a = present.acc;

						for (int i = 0; i < channel_count; ++i, ++v, ++n, ++s, ++o, ++a) {
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
				handler::do_handler_create_vars(_vars, _master_key);
				present_s& present = present_cast<present_s>();
				if (var::machine::actual_mode() >= var::machine::mode::full) {
					string key;
					char_t* vn = varnames;
					for (int i = 0; i < channel_count; ++i) {
						size_t n = ::robo::system::sprintf(vn, vns, RT("nat%d"), i);
						//todo static string class!
						var::record::create(q::var::const_discret, present.native[i], vn, _master_key, _vars);
						vn += (n + 1);
						n = ::robo::system::sprintf(vn, vns, RT("ofs%d"), i);
						var::record::create(q::var::const_discret, present.offset[i], vn, _master_key, _vars);
						vn += (n + 1);
					}
					var::record::create(::mexo::var::uint8, present.ready, RT("ready"), _master_key, _vars);
					/*var::record::create(types::var::const_discret, config.duty.low, RT("duty.low"), _master_key, _vars);
					var::record::create(types::var::const_discret, config.duty.hi, RT("duty.hi"), _master_key, _vars);
					var::record::create(types::var::const_signal, config.voltage.low, RT("v.low"), _master_key, _vars);
					inverter.create_var(_master_key, _vars);*/
				}
			}
			#endif

		public:
			machine(const config_s& _config, present_s& _present)
				: handler(_config.sb, _present.sb) {}
			bool ready(void) { return present_cast<present_s>().ready; }
		};


		template <typename q, typename D>  class single_machine
			: public handler {
		public:
			typedef typename q::signal_t signal_t;
			typedef typename q::long_signal_t long_signal_t;
			typedef typename q::parameter_t parameter_t;
			typedef typename q::discret_t discret_t;
			typedef typename q::long_discret_t long_discret_t;

			struct config_s {
				handler::config_s	sb;
				unsigned int index;
				parameter_t scale;
				unsigned int  init_count_shift;
			};

			struct present_s {
				handler::present_s	sb;
				discret_t native;
				discret_t offset;
				long_discret_t acc;
				discret_t value;
			};

		private:
			unsigned int init_count_ = 0;
			bool init_ = false;
			void reset_(void) {
				present_cast<present_s>().acc = 0;
				init_count_ = 1 << config_cast<config_s>().init_count_shift;
			}
		public:
			void reset(void) {
				bool init_ = false;
				reset_();
			}
		protected:
			void execute(void) {
				present_s& present = present_cast<present_s>();
				const config_s& config = config_cast<config_s>();
				present.native = D::raw[config.index];
				if (init_) {
					present.value = config.scale * (present.native - present.offset);
				}
				else {
					present.acc += present.native;
					init_count_--;
					if (init_count_ == 0) {
						present.offset = (present.acc + (1 << (config.init_count_shift - 1))) >> config.init_count_shift;
						present.value = config.scale * (present.native - present.offset);
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
				: handler(_config.sb, _present.sb) {}

			bool ready(void) { return present_cast<present_s>().ready; }
		};

		template < typename q, typename D  > class current_sence : public machine<q, D> {
			typedef machine<q, D> BB;
		public:
			typedef typename BB::config_s config_s;
			struct present_s {
				typename BB::present_s	adc;
				typename q::signal_t current;
				typename q::signal_t delta;
			};

			current_sence(const config_s& _config, present_s& _present)
				: BB(_config, _present.adc) {}

			typename q::signal_t& current_ref(void) {
				return handler::present_cast<present_s>().current;
			};
			typename q::signal_t& current_delta_ref(void) {
				return handler::present_cast<present_s>().delta;
			};

		protected:
			void execute(void) {
				BB::execute();
				present_s& present = handler::present_cast<present_s>();
				typename q::signal_t tmp = present.current;
				present.current = present.adc.values[1] - present.adc.values[0];
				present.delta = present.current - tmp;
			}
			virtual void do_handler_adjust(void) {}
		};

		template < typename q, typename D  > class current_abc_sence : public machine<q, D> {
		public:
			typedef machine<q, D> BB;
			typedef typename q::signal_t signal_t;
		private:
			const cs_t<q> & cs_;
		public:
			typedef typename q::discret_t discret_t;
			typedef typename q::long_signal_t long_signal_t;
			typedef abc_t<q> abc_t;
			typedef ab_t<q> ab_t;
			typedef dq_t<q> dq_t;

			typedef typename BB::config_s config_s;
			struct present_s {
				typename BB::present_s	adc;
				abc_t abc;
				ab_t ab;
				dq_t current;
				dq_t delta;
			};

			current_abc_sence(const config_s& _config, present_s& _present, const  cs_t<q>& _cs)
				: BB(_config, _present.adc)
				, cs_(_cs) 
			{}
			typename q::signal_t& current_ref(void) { 
				present_s& present = handler::present_cast<present_s>(); 
				return present.current.cross; 
			}
			typename q::signal_t& current_delta_ref(void) { 
				present_s& present = handler::present_cast<present_s>(); 
				return present.delta.cross; 
			}
			typename q::signal_t& lat_current_ref(void) { 
				present_s& present = handler::present_cast<present_s>(); 
				return present.current.lateral; 
			}
			typename q::signal_t& lat_current_delta_ref(void) { 
				present_s& present = handler::present_cast<present_s>(); 
				return present.delta.lateral; 
			}

		protected:
			void execute(void) {
				BB::execute();
				present_s& present = handler::present_cast<present_s>();
				dq_t tmp = present.current;

				present.abc.A = present.adc.values[0];
				present.abc.B = present.adc.values[1];
				present.abc.C = present.adc.values[2];

				present.abc.transform(present.ab);
				present.ab.transform(present.current, cs_);

				present.delta.cross = present.current.cross - tmp.cross;
				present.delta.lateral = present.current.lateral - tmp.lateral;

			}
			virtual void do_handler_adjust(void) {}
			#if ROBO_APP_MEXO_VAR_ENABLED == 1
			virtual void do_handler_create_vars(var::record::list& _vars, int _master_key) {
				BB::do_handler_create_vars(_vars, _master_key);
				present_s& present = present_cast<present_s>();
				var::record::create(q::var::const_signal, present.abc.A, RT("A"), _master_key, _vars);
				var::record::create(q::var::const_signal, present.abc.B, RT("B"), _master_key, _vars);
				var::record::create(q::var::const_signal, present.abc.C, RT("C"), _master_key, _vars);
				var::record::create(q::var::const_signal, present.ab.alfa, RT("alfa"), _master_key, _vars);
				var::record::create(q::var::const_signal, present.ab.beta, RT("beta"), _master_key, _vars);
				var::record::create(q::var::const_signal, present.current.cross, RT("cross"), _master_key, _vars);
				var::record::create(q::var::const_signal, present.current.lateral, RT("lat"), _master_key, _vars);

			}
			#endif
		};		
	}

}
#endif