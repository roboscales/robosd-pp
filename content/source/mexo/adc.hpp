#ifndef mexo_adc_hpp
#define mexo_adc_hpp
#include "mexo/mexo.hpp"
#include "mexo/math.hpp"
namespace mexo {
	template <typename D, typename F, size_t C> class adc_b : public sence_block_t<F>, private D {
	public:
		struct config_s {
			typename sence_block_t<F>::config_s	sb;
			unsigned int index[C];
			F scale[C];
			unsigned int  init_count_shift;
		};

		typedef typename D::native_t native_t;
		typedef typename D::acc_t acc_t;

		struct present_s {
			typename sence_block_t<F>::present_s	sb;
			native_t native[C];
			native_t offset[C];
			acc_t acc[C];
			F values[C];
		};
	private:
		unsigned int init_count_ = 0;
		bool init_ = false;
		void reset_(void) {
			acc_t* a = iblock::present_cast<present_s>().acc;
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

			native_t* n = present.native;
			const unsigned int* ix = config.index;
			for (int i = 0; i < C; ++i, ++n, ++ix) {
				*n = D::sence[*ix];
			}
			if (init_) {
				F* v = present.values;
				native_t* n = present.native;
				const F* s = config.scale;
				native_t* o = present.offset;
				for (int i = 0; i < C; ++i, ++v, ++n, ++s, ++o) {
					*v = *s * ((F)*n - *o);
				}
			}
			else {
				acc_t* a = present.acc;
				native_t* n = present.native;
				for (int i = 0; i < C; ++i, ++a, ++n) {
					*a += *n;
				}
				init_count_--;
				if (init_count_ == 0) {
					F* v = present.values;
					native_t* n = present.native;
					const F* s = config.scale;
					native_t* o = present.offset;
					acc_t* a = present.acc;

					for (int i = 0; i < C; ++i, ++v, ++n, ++s, ++o, ++a) {
						*o = (native_t)(*a + (1 << (config.init_count_shift - 1))) >> config.init_count_shift;
						*v = *s * ((F)*n - *o);
					}

					init_ = true;
				}
			}
			D::query();
		}

		virtual bool reconfig(void) {
			ROBO_LBREAKN(sence_block_t<F>::reconfig());
			reset();
			D::query();
			return true;
		}


		adc_b(isubsystem& _subsystem, cstr  _name, config_s& _config, present_s& _present)
			: sence_block_t<F>(_subsystem, _name, _config.sb, _present.sb) {}
	};

	template <typename D, typename F> class single_adc_b : public sence_block_t<F>, private D {
	public:
		typedef typename D::native_t native_t;
		typedef typename D::acc_t acc_t;
		struct config_s {
			typename sence_block_t<F>::config_s	sb;
			unsigned int index;
			F scale;
			unsigned int  init_count_shift;
		};

		struct present_s {
			typename sence_block_t<F>::present_s	sb;
			native_t native;
			native_t offset;
			acc_t acc;
			F value;
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
				present.value = config.scale * ((F)present.native - present.offset);
			}
			else {
				present.acc += present.native;
				init_count_--;
				if (init_count_ == 0) {
					present.offset = (present.acc + (1 << (config.init_count_shift - 1))) >> config.init_count_shift;
					present.value = config.scale * ((F)present.native - present.offset);
					init_ = true;

				}
			}
			D::query();
		}


		virtual bool reconfig(void) {
			ROBO_LBREAKN(sence_block_t<F>::reconfig());
			reset();
			D::query();
			return true;
		}

		single_adc_b(isubsystem& _subsystem, cstr  _name, config_s& _config, present_s& _present)
			: sence_block_t<F>(_subsystem, _name, _config.sb, _present.sb) {}

	};

	template <typename D, typename F  > class adc_diff_b : public adc_b<D, F, 2> {
		typedef adc_b<D, F, 2> B;
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