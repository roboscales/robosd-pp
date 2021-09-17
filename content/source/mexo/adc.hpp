#ifndef mexo_adc_hpp
#define mexo_adc_hpp
#include "mexo/mexo.hpp"
#include "mexo/math.hpp"
namespace mexo {
	template <typename D, typename q, unsigned C> class adc_b : public sence_block_t<typename q::signal_t>, private D {
	public:
		typedef typename q::signal_t signal_t;
		typedef typename q::long_signal_t long_signal_t;
		typedef typename q::parameter_t parameter_t;
		typedef typename q::discret_t discret_t;
		typedef typename q::long_discret_t long_discret_t;

		struct config_s {
			typename sence_block_t<signal_t>::config_s	sb;
			unsigned int index[C];
			parameter_t scale[C];
			unsigned int  init_count_shift;
		};

		struct present_s {
			typename sence_block_t<signal_t>::present_s	sb;
			discret_t native[C];
			discret_t offset[C];
			long_discret_t acc[C];
			parameter_t values[C];
		};
	private:
		unsigned int init_count_ = 0;
		bool init_ = false;
		void reset_(void) {
			long_discret_t* a = iblock::present_cast<present_s>().acc;
			for (int i = 0; i < C; ++i, ++a) {
				*a = 0;
			}
			init_count_ = 1 << iblock::config_cast<config_s>().init_count_shift;
		}
	public:
		void reset(void) {
			bool init_ = false;
			reset_();
		}
		virtual void execute(void) {
			present_s& present = iblock::present_cast<present_s>();
			const config_s& config = iblock::config_cast<config_s>();

			discret_t* n = present.native;
			const unsigned int* ix = config.index;
			for (int i = 0; i < C; ++i, ++n, ++ix) {
				*n = D::sence[*ix];
			}
			if (init_) {
				parameter_t* v = present.values;
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
					parameter_t* v = present.values;
					discret_t* n = present.native;
					const parameter_t* s = config.scale;
					discret_t* o = present.offset;
					long_discret_t* a = present.acc;

					for (int i = 0; i < C; ++i, ++v, ++n, ++s, ++o, ++a) {
						*o = (discret_t)(((long_discret_t)*a + (1 << (config.init_count_shift - 1))) >> config.init_count_shift);
						*v = *s * (*n - *o);
					}

					init_ = true;
				}
			}
			D::query();
		}

		virtual bool reconfig(void) {
			ROBO_LBREAKN(sence_block_t<signal_t>::reconfig());
			reset();
			D::query();
			return true;
		}


		adc_b(isubsystem& _subsystem, cstr  _name, config_s& _config, present_s& _present)
			: sence_block_t<signal_t>(_subsystem, _name, _config.sb, _present.sb) {}
	};

	template <typename D, typename q> class single_adc_b : public sence_block_t<typename q::signal_t>, private D {
	public:
		typedef typename q::signal_t signal_t;
		typedef typename q::long_signal_t long_signal_t;
		typedef typename q::parameter_t parameter_t;
		typedef typename q::discret_t discret_t;
		typedef typename q::long_discret_t long_discret_t;

		struct config_s {
			typename sence_block_t<signal_t>::config_s	sb;
			unsigned int index;
			parameter_t scale;
			unsigned int  init_count_shift;
		};

		struct present_s {
			typename sence_block_t<signal_t>::present_s	sb;
			discret_t native;
			discret_t offset;
			long_discret_t acc;
			parameter_t value;
		};

	private:
		unsigned int init_count_ = 0;
		bool init_ = false;
		void reset_(void) {
			iblock::present_cast<present_s>().acc = 0;
			init_count_ = 1 << iblock::config_cast<config_s>().init_count_shift;
		}
	public:
		void reset(void) {
			bool init_ = false;
			reset_();
		}
		virtual void execute(void) {
			present_s& present = iblock::present_cast<present_s>();
			const config_s& config = iblock::config_cast<config_s>();
			present.native = D::sence[config.index];
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


		virtual bool reconfig(void) {
			ROBO_LBREAKN(sence_block_t<signal_t>::reconfig());
			reset();
			D::query();
			return true;
		}

		single_adc_b(isubsystem& _subsystem, cstr  _name, config_s& _config, present_s& _present)
			: sence_block_t<signal_t>(_subsystem, _name, _config.sb, _present.sb) {}

	};

	template <typename D, typename q  > class adc_diff_b : public adc_b<D, q, 2> {
		typedef adc_b<D, q, 2> B;
	public:
		typedef typename B::present_s present_s;
		typedef typename B::config_s config_s;
		virtual void execute(void) {
			B::execute();
			present_s& present = iblock::present_cast<present_s>();
			present.sb.output = present.values[1] - present.values[0];
		}
		adc_diff_b(isubsystem& _subsystem, cstr  _name, config_s& _config, present_s& _present)
			: B(_subsystem, _name, _config, _present) {}
	};


}
#endif