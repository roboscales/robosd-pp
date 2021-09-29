#ifndef mexo_adc_hpp
#define mexo_adc_hpp
#include "mexo/mexo.hpp"
#include "mexo/math.hpp"
namespace mexo {
	template <typename q, typename D, unsigned C> class adc
		: public sence_handler<robo::array<typename q::signal_t, C> >, private D {
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
		};

		struct present_s {
			typename A::present_s	sb;
			discret_t native[C];
			discret_t offset[C];
			long_discret_t acc[C];
		};
	private:
		unsigned int init_count_ = 0;
		bool init_ = false;
		void reset_(void) {
			long_discret_t* a = handler::present_cast<present_s>().acc;
			for (int i = 0; i < C; ++i, ++a) {
				*a = 0;
			}
			init_count_ = 1 << handler::config_cast<config_s>().init_count_shift;
		}
	public:
		void reset(void) {
			bool init_ = false;
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
				*n = D::sence[*ix];
			}
			if (init_) {
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

					init_ = true;
				}
			}
			D::query();
		}

		virtual bool do_handler_reconfig(void) {
			reset();
			D::query();
			return true;
		}
	public:
		adc(const config_s& _config, present_s& _present)
			: A(_config.sb, _present.sb) {}
	};


	template <typename q, typename D>  class single_adc
		: public sence_handler< typename q::signal_t >, private D {
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
			present.native = D::sence[config.index];
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

		single_adc(const config_s& _config, present_s& _present)
			: A(_config.sb, _present.sb) {}
	};

	template < typename q, typename D  > class diff_adc : public adc<q, D, 2> {
		typedef adc<q, D, 2> B;
	public:
		typedef typename B::config_s config_s;
		struct present_s {
			typename B::present_s	adc;
			typename q::signal_t output;
		};


		diff_adc(const config_s& _config, present_s& _present)
			: B(_config, _present.adc) {}
	protected:
		void execute(void) {
			B::execute();
			present_s& present = handler::present_cast<present_s>();
			present.output = present.adc.sb.output.values[1] - present.adc.sb.output.values[0];
		}
		virtual void do_handler_adjust(void) {}

	};

}
#endif