#ifndef mexo_adc_hpp
#define mexo_adc_hpp
#include "mexo/mexo.hpp"
#include "mexo/math.hpp"
namespace mexo {
	template <typename D, typename F, size_t C> class adc : private D {
		unsigned int init_count_ = 0;
		bool init_ = false;
		void reset_(void) {
			native_t* a = acc;
			for (int i = 0; i < C; ++i, ++a) {
				*a = 0;
			}
			init_count_ = 1 << config.init_count_shift;
		}
	public:
		typedef  F output_t;
		struct config_s {
			iblock::config_s	block;
			unsigned int index[C];
			F scale[C];
			unsigned int  init_count_shift;
		};
		config_s  config = {};

		typedef typename D::native_t native_t;
		native_t native[C];
		native_t offset[C];
		native_t acc[C];

		F values[C];
		void reset(void) {
			bool init_ = false;
			reset_();
		}
		void update_output(void) {
			native_t* n = native;
			unsigned int* ix = config.index;
			for (int i = 0; i < C; ++i, ++n, ++ix) {
				*n = D::sence[*ix];
			}
			if (init_) {
				F* v = values;
				native_t* n = native;
				F* s = config.scale;
				native_t* o = offset;
				for (int i = 0; i < C; ++i, ++v, ++n, ++s, ++o) {
					*v = *s * ((F)*n - *o);
				}
			}
			else {
				native_t* a = acc;
				native_t* n = native;
				for (int i = 0; i < C; ++i, ++a, ++n) {
					*a += *n;
				}
				init_count_--;
				if (init_count_ == 0) {
					F* v = values;
					native_t* n = native;
					F* s = config.scale;
					native_t* o = offset;
					native_t* a = acc;

					for (int i = 0; i < C; ++i, ++v, ++n, ++s, ++o, ++a) {
						*o = (*a + (1 << (config.init_count_shift - 1))) >> config.init_count_shift;
						*v = *s * ((F)*n - *o);
					}

					reset_();

					init_ = true;

				}
			}
		}

		void query(void) {
			D::query();
		}

		bool applay(const config_s& _config) {
			config = _config;
			reset();
			return true;
		}
	};

	template <typename D, typename F> class adc<D, F, 1> : private D {
		unsigned int init_count_ = 0;
		bool init_ = false;
		void reset_(void) {
			acc = 0;
			init_count_ = 1 << config.init_count_shift;
		}
	public:
		typedef  F output_t;
		struct config_s {
			unsigned int index;
			F scale;
			unsigned int  init_count_shift;
		};
		config_s  config = {};

		typedef typename D::native_t native_t;
		native_t native;
		native_t offset;
		native_t acc;

		F value;
		void reset(void) {
			bool init_ = false;
			reset_();
		}
		void update_output(void) {
			native = D::sence[config.index];
			if (init_) {
				value = config.scale * ((F)native - offset);
			}
			else {
				acc += native;
				init_count_--;
				if (init_count_ == 0) {
					offset = (acc + (1 << (config.init_count_shift - 1))) >> config.init_count_shift;
					value = config.scale * ((F)native - offset);
					reset_();
					init_ = true;

				}
			}
		}

		void query(void) {
			D::query();
		}

		bool applay(const config_s& _config) {
			config = _config;
			reset();
			return true;
		}
	};

	template <typename D, typename F> class adc_single : private D {
		unsigned int init_count_ = 0;
		bool init_ = false;
		void reset_(void) {
			acc = 0;
			init_count_ = 1 << config.init_count_shift;
		}
	public:
		struct config_s {
			unsigned int index;
			F scale;
			unsigned int  init_count_shift;
		};
		config_s  config = {};

		typedef typename D::native_t native_t;
		native_t native;
		native_t offset;
		native_t acc;

		F value = (F)0;
		void reset(void) {
			bool init_ = false;
			reset_();
		}
		void update_output(void) {
			native = D::sence[config.index];
			if (init_) {
				value = config.scale * (native - offset);
			}
			else {
				acc += native;
				init_count_--;
				if (init_count_ == 0) {
					offset = (acc + (1 << (config.init_count_shift - 1))) >> config.init_count_shift;
					value = config.scale * (native - offset);
					reset_();
					init_ = true;

				}
			}
		}

		void query(void) {
			D::query();
		}

		bool applay(const config_s& _config) {
			config = _config;
			reset();
			return true;
		}
		F& output_value(void) {
			return value;
		}
	};

	template <typename D, typename F  > class adc_diff : public adc<D, F, 2> {
	public:
		F diff_value = (F)0;
		F& output_value(void) {
			return diff_value;
		}
		void update_output(void) {
			adc<D, F, 2>::update_output();
			diff_value = adc<D, F, 2>::values[1] - adc<D, F, 2>::values[0];
		}
	};


}
#endif