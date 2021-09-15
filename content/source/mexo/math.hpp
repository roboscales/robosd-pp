#ifndef mexo_math_hpp
#define mexo_math_hpp
#include "mexo/mexo.hpp" 
#include <limits>  
#include <math.h>
namespace mexo {
	enum class  signal_bits {
		i7 = 8
		, i15 = 16
		, i31 = 32
		, i63 = 64
		, real15 = 1 //float + float
		, real31 = 2 //float + float
		, real63 = 3 //float + double
	};
	template<signal_bits bits> struct dsigna {
		enum {
			max = (1 << ((int)bits - 1)) - 1
			, min = (-max)
			, half_pos = max >> 1
			, half_neg = -half_pos
			, uhalf_pos = (1LU << ((int)bits - 2) - 1)
			, uhalf_neg = ((1LU << (int)bits) - half_pos)
		};
	};

	struct int7 : public dsigna<signal_bits::i7> {
		typedef int8_t signal_t;
		typedef uint8_t usignal_t;
		typedef int8_t parameter_t;
		typedef int16_t long_signal_t;
	};

	struct int15 : public dsigna<signal_bits::i15> {
		typedef int16_t signal_t;
		typedef uint16_t usignal_t;
		typedef int16_t parameter_t;
		typedef int32_t long_signal_t;
	};

	struct int31 : public dsigna<signal_bits::i31> {
		typedef int32_t signal_t;
		typedef uint32_t usignal_t;
		typedef int32_t parameter_t;
		typedef int64_t long_signal_t;
	};

	struct real15 : public dsigna<signal_bits::i31> {
		typedef float signal_t;
		typedef float parameter_t;
		typedef float long_signal_t;
	};

	struct real31 : public dsigna<signal_bits::i31> {
		typedef float signal_t;
		typedef float parameter_t;
		typedef double long_signal_t;
	};

	struct real63 : public dsigna<signal_bits::i31> {
		typedef double signal_t;
		typedef double parameter_t;
		typedef double long_signal_t;
	};

	template< typename q > struct fixed_point {

		typedef typename  q::signal_t signal_t;
		typedef typename  q::usignal_t usignal_t;
		typedef typename  q::long_signal_t long_signal_t;

		union point {
			long_signal_t value;
			struct {
				usignal_t first;
				signal_t second;
			};
		};

		static void extract(const long_signal_t& _src, signal_t& _dst, unsigned int shift) {
			point v;
			if (_src == 0 || shift == 0) {
				v.value = _src;
			}
			else {
				int r = 1 << (shift - 1);
				if (_src > 0) {
					if (q::max - _src < r) {
						v.value = q::max >> shift;
					}
					else {
						v.value = (_src + r) >> shift;
					}
					if (v.second < q15::half_pos) {
						if (v.first > q15::uhalf_pos) {
							_dst = (v.second << 1) + 1;
						}
						else {
							_dst = (v.second << 1);
						}
					}
					else {
						_dst = q15::max;
					}
				}
				else {
					if (_src - q::min < r) {
						v.value = -((-q::min) >> shift);
					}
					else {
						v.value = -((r - _src) >> shift);
					}
					if (v.second > q15::half_neg) {
						if (v->first < q15::uhalf_neg) {
							_dst = -(((-v.second) << 1) + 1);
						}
						else {
							_dst = -(-(v.second) << 1);
						}
					}
					else {
						_dst = q15::min;
					}

				}
			}
		}

	};

	#ifndef MEXO_BASE_TYPE
	#define MEXO_BASE_TYPE real15
	#endif
	typedef MEXO_BASE_TYPE::signal_t signal_t;
	typedef MEXO_BASE_TYPE::long_signal_t long_signal_t;
	typedef MEXO_BASE_TYPE::parameter_t parameter_t;

	constexpr signal_t pi = robo::pi<signal_t>;
	constexpr signal_t one_div_sqrt3 = robo::one_div_sqrt3<signal_t>;
	constexpr signal_t sqrt3_div_2 = robo::sqrt3_div_2<signal_t>;

	template<typename L, typename S>	void extract(const L& _src, S& _dst, unsigned int shift);

	template<>	void extract(const float& _src, float& _dst, unsigned int shift) {
		if (shift == 0) {
			_dst = _src;
		}
		else {
			int d = 1 << shift;
			int r = 1 << (shift - 1);
			_dst = (_src + r) / d;
		}
	}
	/*
	template<typename D>	void extract(const D& _src, D& _dst, unsigned int shift) {
		if (_src == 0 || shift == 0) {
			_dst = _src;
		}
		else {
			int r = 1 << (shift - 1);
			if (_src > 0) {
				if (std::numeric_limits<D>::max() - _src < r) {
					_dst = std::numeric_limits<D>::max() >> shift;
				}
				else {
					_dst = (_src + r) >> shift;
				}
			}
			else {
				if (_src - std::numeric_limits<D>::lowest() < r) {
					_dst = -(-std::numeric_limits<D>::lowest()) >> shift;
				}
				else {
					_dst = -((r - _src) >> shift);
				}
			}
		}
	}*/

	template<>	void extract(const int7::long_signal_t& _src, int7::signal_t& _dst, unsigned int _shift) {
		fixed_point<int7>::extract(_src, _dst, _shift);
	}

	template<>	void extract(const int15::long_signal_t& _src, int15::signal_t& _dst, unsigned int _shift) {
		fixed_point<int15>::extract(_src, _dst, _shift);
	}

	template<>	void extract(const int31::long_signal_t& _src, int31::signal_t& _dst, unsigned int _shift) {
		fixed_point<int31>::extract(_src, _dst, _shift);
	}

	/*
		template <typename F, typename D>  void round(const F& _f, D& _d) {
			F tmp = _f;
			if (tmp > ((F)0)) tmp += ((F)0.5);
			else
				if (tmp < ((F)0)) tmp -= ((F)0.5);
			_d = (D)tmp;
		}*/

	template <typename I, typename  O, typename  P>  void scale(const I& _x, const P& _scale, const O& _offset, O& _y) {
		_y = (O)(_scale * _x + _offset);
	}

	template <typename I, typename O, typename F> class scale_b : public function_block_t<I, O> {
		typedef function_block_t<I, O> B;
	public:
		struct config_s {
			typename B::config_s fb;
			F scale;
			O offset;
		};
		struct present_s {
			typename B::present_s fb;
		};

	protected:
		virtual void execute(void) {
			present_s& present = iblock::present_cast<present_s>();
			const config_s& config = iblock::config_cast<config_s>();
			scale(B::input.value(), config.scale, config.offset, present.fb.output);
		}
	public:
		scale_b(isubsystem& _subsystem, cstr  _name, config_s& _config, present_s& _present)
			: B(_subsystem, _name, _config, _present) {}
	};

	template <typename I, typename O, typename F> class control_scale_b : public controller_block_t<I, O> {
		typedef controller_block_t<I, O> B;
	public:
		struct config_s {
			typename B::config_s cb;
			F scale;
			O offset;
		};

		struct present_s {
			typename B::present_s cb;
		};
	protected:
		virtual void execute(void) {
			present_s& present = iblock::present_cast<present_s>();
			const config_s& config = iblock::config_cast<config_s>();
			scale(B::input.value(), config.scale, config.offset, present.cb.output);
			B::saturate();
			B::update_satstate();
		};
	public:
		control_scale_b(isubsystem& _subsystem, cstr  _name, config_s& _config, present_s& _present)
			: controller_block_t<I, O>(_subsystem, _name, _config.cb, _present.cb) {}
	};


	template <typename I, typename O, typename F> class to_parrot_scale_b : public controller_block_t<I, O> {
		typedef controller_block_t<I, O> B;
	public:
		struct config_s {
			typename B::config_s cb;
			F scale;
		};
		struct present_s {
			typename controller_block_t<I, O>::present_s cb;
		};
	protected:
		virtual void execute(void) {
			round(iblock::config_cast<config_s>().scale * B::input.value(), iblock::present_cast<present_s>().cb.output);
			B::saturate();
			B::update_satstate();
		}
	public:
		to_parrot_scale_b(isubsystem& _subsystem, cstr  _name, config_s& _config, present_s& _present)
			: B(_subsystem, _name, _config.cb, _present.cb) {}
	};

	template <typename I, typename O = I> class ramp_b : public controller_block_t<I, O> {
		typedef controller_block_t<I, O> B;
	public:
		struct config_s {
			typename B::config_s cb;
			O rampStep;
		};
		struct present_s {
			typename B::present_s cb;
		};
	protected:

		virtual void execute(void) {

			iblock::satstate remote = controller_block_t<I, O>::master_satstate.value();
			present_s& present = iblock::present_cast<present_s>();
			const config_s& config = iblock::config_cast<config_s>();
			if (remote == iblock::satstate::both) {
				present.cb.satstate = iblock::satstate::both;
				return;
			}

			I inp = B::input.value();
			const range_s<O>& r = B::range.value();
			iblock::satstate inp_sut;
			if (inp >= r.hi) {
				inp = r.hi;
				inp_sut = iblock::satstate::up;
			}
			else if (inp <= r.low) {
				inp = r.low;
				inp_sut = iblock::satstate::low;
			}
			else {
				inp_sut = iblock::satstate::none;
			}

			O out = present.cb.output;
			O delta = inp - out;

			if (delta > std::numeric_limits<O>::epsilon()) {
				if (remote != iblock::satstate::up) {
					if (delta < config.rampStep) {
						present.cb.output = inp;
						present.cb.local_satstate = inp_sut;
					}
					else {
						present.cb.output += config.rampStep;
						present.cb.local_satstate = iblock::satstate::none;
					}
				}
				else {
					present.cb.local_satstate = iblock::satstate::none;
				}
			}
			else {
				if (delta < -std::numeric_limits<O>::epsilon()) {
					if (remote != iblock::satstate::low) {
						if ((-delta) < config.rampStep) {
							present.cb.output = inp;
							present.cb.local_satstate = inp_sut;
						}
						else {
							present.cb.output -= config.rampStep;
							present.cb.local_satstate = iblock::satstate::none;
						}
					}
					else {
						present.cb.local_satstate = iblock::satstate::none;
					}
				}
				else {
					present.cb.local_satstate = inp_sut;
				}
			}
			B::update_satstate();
		}
	public:
		ramp_b(isubsystem& _subsystem, cstr  _name, config_s& _config, present_s& _present)
			: controller_block_t<I, O>(_subsystem, _name, _config.cb, _present.cb) {}
	};

	template< typename I, typename O, typename P>  class  filter_b : public function_block_t<I, O> {
		typedef function_block_t<I, O> B;
	public:
		struct config_s {
			typename B::config_s fb;
			P	gain;
		};
		struct present_s {
			typename B::present_s fb;
		};
		P gain1 = (P)0;
		P gain2 = (P)0;

	protected:
		virtual void execute(void) {
			present_s& present = iblock::present_cast<present_s>();
			present.fb.output = (O)(gain1 * present.fb.output + gain2 * B::input.value());
		}

		virtual bool reconfig(void) {
			const config_s& config = iblock::config_cast<config_s>();
			gain1 = config.gain;
			gain2 = (P)1 - config.gain;
			B::reconfig();
			return true;
		};

	public:
		filter_b(isubsystem& _subsystem, cstr  _name, config_s& _config, present_s& _present)
			: function_block_t<I, O>(_subsystem, _name, _config.fb, _present.fb) {}
	};


	/*
	struct signal2ph_s {
		signal_t cross;
		signal_t lateral;
	};

	struct signal3ph_s {
		signal_t A;
		signal_t B;
		signal_t C;
	};


	class transform {
	private:
		signal_t angle_;
		signal_t sin_;
		signal_t cos_;
	public:
		const signal_t& angle(void) { return angle_; };
		const signal_t& sin(void) { return sin_; };
		const signal_t& cos(void) { return cos_; };
		void set_angle(const signal_t& _angle) {
			angle_ = _angle;
			sin_ = (signal_t)::sin(_angle);
			cos_ = (signal_t)::cos(_angle);
		}
		void forward(const signal2ph_s& _src, signal2ph_s& _dst);
		void backward(const signal2ph_s& _src, signal2ph_s& _dst);
	};


	signal2ph_s& operator >> (const signal3ph_s& _s3, signal2ph_s& _s2);
	signal3ph_s& operator >> (const signal2ph_s& _s2, signal3ph_s& _s3);
	*/

}
#endif