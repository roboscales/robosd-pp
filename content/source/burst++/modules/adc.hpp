#ifndef burst_adc_hpp
#define burst_adc_hpp
#include "burst++/modules/actor.hpp"
#include "core/robosd_system.hpp"
#include <algorithm>

namespace burst {
	template< class number, class driver>  class adc_t : public actor {
		using R = typename driver::raw_t;
		enum {N = driver::channel_count};
		using discret_t = typename number::discret_t;
		using signal_t = typename number::signal_t;
		using long_discret_t = typename number::long_discret_t;

	public:
		enum { count = N};
		struct config_s {
			actor::config_s tag;
			unsigned init_count_bits;
		};
		struct present_s {
			actor::present_s tag;
			R raw[count];
			signal_t values[count];
			bool ready;
		};
		
		#define reg_arr(t,x,n) if(n < count) reg(t, x[n], RT(#n));
		#if ROBO_APP_BURST_VARTREE_ENABLED
		virtual void do_regvar_present(void) {
			using namespace burst::var;
			ACTOR_PRESENT_S(p);
			if (actual_mode >= mode::full) {
				reg(types::const_uint8, p.ready, RT("ready"));
				push(RT("raw"));
				auto t = (types)descriptor_enco(sizeof(R), false, false, false);				
				#define reg_raw_(i) reg_arr(t, p.raw, i);
				reg_raw_(0);
				reg_raw_(1);
				reg_raw_(2);
				reg_raw_(3);
				reg_raw_(4);
				reg_raw_(5);
				reg_raw_(6);
				reg_raw_(7);
				reg_raw_(8);
				reg_raw_(10);
				reg_raw_(11);
				reg_raw_(12);
				reg_raw_(13);
				reg_raw_(14);
				reg_raw_(15);
				
				pop();
				push(RT("val"));
				#define reg_value_(i) if(i<N) reg_arr(number::var::signal, p.values, i);
				reg_value_(0);
				reg_value_(1);
				reg_value_(2);
				reg_value_(3);
				reg_value_(4);
				reg_value_(5);
				reg_value_(6);
				reg_value_(7);
				reg_value_(8);
				reg_value_(10);
				reg_value_(11);
				reg_value_(12);
				reg_value_(13);
				reg_value_(14);
				reg_value_(15);
				
				pop();
			}
		}

		virtual void do_regvar_conf(void) {
			using namespace burst::var;
			ACTOR_CONFIG_S(c);
			if (actual_mode >= mode::config) {
				reg(types::uint8, c.init_count_bits, RT("icb"));
			}			
		}
		#endif

		R offset[N] = {};
		uint32_t acc[N] = {};
		int init_count = 0;

		#define ADC_CONFIG(a) ADC_CONFIG_(a)
		#define ADC_CONFIG_(a)\
		{\
			ACTOR_CONFIG(a)\
			, a##_INIT_COUNT_BITS\
		}

		virtual void begin(void) {

			actor::begin();
			ACTOR_CONFIG_S(cfg);
			ACTOR_PRESENT_S(p);
			uint32_t * a = acc;
			for (int i = 0; i < N; ++i, ++a) {
				*a = 0;
			}
			init_count = 1 << cfg.init_count_bits;
			p.ready = false;

		};

		virtual void run(void) {

			ACTOR_CONFIG_S(cfg);
			ACTOR_PRESENT_S(p);
			driver::query(p.raw);
			if (p.ready) {
				signal_t * v = p.values;
				R * n = p.raw;
				R * o = offset;
				for (int i = 0; i < N; ++i, ++n, ++o, ++v) {
					*v = (signal_t)((long_discret_t)(*n - *o));
				}
			}
			else {
				uint32_t * a = acc;
				R * n = p.raw;
				for (int i = 0; i < N; ++i, ++a, ++n) {
					*a += *n;
				}
				init_count--;
				if (init_count == 0) {
					signal_t * v = p.values;
					R * n = p.raw;
					R * o = offset;
					uint32_t * a = acc;
					int shift = cfg.init_count_bits;
					for (int i = 0; i < N; ++i, ++v, ++n, ++o, ++a) {
						*o = (R)((*a + (1 << (shift - 1))) >> shift) + 1;
						*v = (signal_t)((long_discret_t)(*n - *o)  );
					}
					p.ready = true;
				}
			}

		}
		
		
		adc_t(const config_s& _config, present_s& _present)
			: actor(_config.tag, _present.tag) {};
		#if ROBO_APP_ULTRACOMPACT == 0
		adc_t(const config_s& _config, present_s& _present, subsystem& _subsystem)
			: actor(_config.tag, _present.tag, _subsystem) {};
		#endif

	};	
	
	template< class number, class driver>  class analog_input_t : public actor, public driver {
		using R = typename driver::raw_t;
		enum {N = driver::channel_count};
		using signal_t = typename number::signal_t;
		using long_signal_t = typename number::long_signal_t;

	public:
		enum { count = N};
		struct config_s {
			actor::config_s tag;
			unsigned init_count_bits;
		};
		struct present_s {
			actor::present_s tag;
			R raw[count];
			signal_t values[count];
			struct{
				R raw[N] = {};
				signal_t values[count];
			} startup;
			bool ready;
		};
		
		#define reg_arr(t,x,n) if(n < count) reg(t, x[n], RT(#n));
		#if ROBO_APP_BURST_VARTREE_ENABLED
		virtual void do_regvar_present(void) {
			using namespace burst::var;
			ACTOR_PRESENT_S(p);
			if (actual_mode >= mode::full) {
				reg(types::const_uint8, p.ready, RT("ready"));
				push(RT("raw"));
				auto t = (types)descriptor_enco(sizeof(R), false, false, false);				
				#define reg_raw_(i) reg_arr(t, p.raw, i);
				reg_raw_(0);
				reg_raw_(1);
				reg_raw_(2);
				reg_raw_(3);
				reg_raw_(4);
				reg_raw_(5);
				reg_raw_(6);
				reg_raw_(7);
				reg_raw_(8);
				reg_raw_(10);
				reg_raw_(11);
				reg_raw_(12);
				reg_raw_(13);
				reg_raw_(14);
				reg_raw_(15);
				pop();
				push(RT("val"));
				#undef reg_value_
				#define reg_value_(i) if(i<N) reg_arr(number::var::signal, p.values, i);
				reg_value_(0);
				reg_value_(1);
				reg_value_(2);
				reg_value_(3);
				reg_value_(4);
				reg_value_(5);
				reg_value_(6);
				reg_value_(7);
				reg_value_(8);
				reg_value_(10);
				reg_value_(11);
				reg_value_(12);
				reg_value_(13);
				reg_value_(14);
				reg_value_(15);
				pop();
			}
		}

		virtual void do_regvar_conf(void) {
			using namespace burst::var;
			ACTOR_CONFIG_S(c);
			if (actual_mode >= mode::config) {
				reg(types::uint8, c.init_count_bits, RT("icb"));
				converter ** c = converters;
				for (int i = 0; i < N; ++i, ++c) {
					(*c)->do_regvar_conf();
				}
			}			
		}
		#endif

		R acc[N] = {};
		int init_count = 0;

		#define ANALOG_INPUT_CONFIG(a) ANALOG_INPUT_CONFIG_(a)
		#define ANALOG_INPUT_CONFIG_(a)\
		{\
			ACTOR_CONFIG(a)\
			, a##_INIT_COUNT_BITS\
		}
		
		struct converter{
			signal_t value;
			virtual void begin(void) = 0;
			virtual signal_t operator () (const R & _value) = 0;
			#if ROBO_APP_BURST_VARTREE_ENABLED
			virtual void do_regvar_conf(void) = 0;
			#endif
		} * converters[N];

		virtual void begin(void) {
			actor::begin();
			ACTOR_CONFIG_S(cfg);
			ACTOR_PRESENT_S(p);
			R * a = acc;
			for (int i = 0; i < N; ++i, ++a) {
				*a = 0;
			}
			init_count = 1 << cfg.init_count_bits;
			converter ** c = converters;
			for (int i = 0; i < N; ++i, ++c) {
				(*c)->begin();
			}
			p.ready = false;
		};
		

		
		struct line_converter: public converter{
			typename number::discret2signal scaler;
			typedef typename number::discret2signal::config_s config_s;
			virtual void begin(void){
				scaler.begin();
			}
			#if ROBO_APP_BURST_VARTREE_ENABLED
			virtual void do_regvar_conf(void) {
				using namespace burst::var;
				if (actual_mode >= mode::config) {
								/*const struct config_s {
				struct{
					range_s <signal_t> signal;
					range_s <discret_t> raw;
				} range;
			} & config;*/
					push(name);
					varreg(RT("sg"),number::var::signal, scaler.config.range.signal);
					varreg(RT("raw"),number::var::discret, scaler.config.range.raw );
					pop();
				}			
			}
			#endif
			virtual signal_t operator () (const R & _value){
				return scaler.run((typename number::discret_t)_value);
			}
			robo::cstr name;
			line_converter( robo::cstr _name, const config_s & _config) : scaler(_config),name(_name){}
		};
		
		virtual void convert(void) {
			ACTOR_PRESENT_S(p);
			signal_t * v = p.values;
			R tmp_raw[N]; 
			R * n = tmp_raw;
			converter ** c = converters;
			{
				#if ROBO_APP_ULTRACOMPACT == 0
				::robo::system::guard g__;
				#else
				//typename driver::guard g__;
				#endif
				std::copy_n(p.raw,N,tmp_raw);
			}
			for (int i = 0; i < N; ++i, ++n, ++v, ++c) {
				(*v) = (*c)->value =  (**c)(*n);
			}
		}

		virtual void run(void) {
			ACTOR_CONFIG_S(cfg);
			ACTOR_PRESENT_S(p);
			driver::query(p.raw);
			if (!p.ready) {
				R * a = acc;
				R * n = p.raw;
				for (int i = 0; i < N; ++i, ++a, ++n) {
					*a += *n;
				}
				init_count--;
				if (init_count == 0) {
					typename number::signal_t * v = p.startup.values;
					R * o = p.startup.raw;
					R* a = acc;
					converter ** c = converters;
					int shift = cfg.init_count_bits;
					for (int i = 0; i < N; ++i, ++v, ++o, ++a, ++c) {
						*o = (R)((*a + (1 << (shift - 1))) >> shift) + 1;
						*v = (**c)(*o);
					}
					p.ready = true;
				}
			}
		}

		analog_input_t(const config_s& _config, present_s& _present, converter * ( &_converters)  [N] )
			: actor(_config.tag, _present.tag) {
				std::copy_n(_converters,N,converters);
			};
			#if ROBO_APP_ULTRACOMPACT == 0
		analog_input_t(const config_s& _config, present_s& _present, subsystem& _subsystem, converter * ( &_converters)  [N])
			: actor(_config.tag, _present.tag, _subsystem){
				std::copy_n(_converters,N,converters);
			};
			#endif

	};	


	// Ќовый шаблон с конвертером как параметром шаблона
	// Ќовый шаблон с однотипным конвертером
	template< class number, class driver, class converter>  class uniform_analog_input_t : public actor, public driver {
		using R = typename driver::raw_t;
		enum { N = driver::channel_count };
		using signal_t = typename number::signal_t;
		using long_signal_t = typename number::long_signal_t;

	public:
		enum { count = N };
		struct config_s {
			actor::config_s tag;
			unsigned init_count_bits;
			typename converter::config_s converters[N];
		};
		struct present_s {
			actor::present_s tag;
			R raw[count];
			signal_t values[count];
			struct {
				R raw[N] = {};
				signal_t values[count];
			} startup;
			bool ready;
		};

#define reg_arr(t,x,n) if(n < count) reg(t, x[n], RT(#n));
#if ROBO_APP_BURST_VARTREE_ENABLED
		virtual void do_regvar_present(void) {
			using namespace burst::var;
			ACTOR_PRESENT_S(p);
			if (actual_mode >= mode::full) {
				reg(types::const_uint8, p.ready, RT("ready"));
				push(RT("raw"));
				auto t = (types)descriptor_enco(sizeof(R), false, false, false);
#define reg_raw_(i) reg_arr(t, p.raw, i);
				reg_raw_(0);
				reg_raw_(1);
				reg_raw_(2);
				reg_raw_(3);
				reg_raw_(4);
				reg_raw_(5);
				reg_raw_(6);
				reg_raw_(7);
				reg_raw_(8);
				reg_raw_(10);
				reg_raw_(11);
				reg_raw_(12);
				reg_raw_(13);
				reg_raw_(14);
				reg_raw_(15);
				pop();
				push(RT("val"));
#define reg_value_(i) if(i<N) reg_arr(number::var::signal, p.values, i);
				reg_value_(0);
				reg_value_(1);
				reg_value_(2);
				reg_value_(3);
				reg_value_(4);
				reg_value_(5);
				reg_value_(6);
				reg_value_(7);
				reg_value_(8);
				reg_value_(10);
				reg_value_(11);
				reg_value_(12);
				reg_value_(13);
				reg_value_(14);
				reg_value_(15);
				pop();
			}
		}

		virtual void do_regvar_conf(void) {
			using namespace burst::var;
			ACTOR_CONFIG_S(c);
			if (actual_mode >= mode::config) {
				reg(types::uint8, c.init_count_bits, RT("icb"));
				for (int i = 0; i < N; ++i) {
					converters[i].do_regvar_conf();
				}
			}
		}
#endif

		R acc[N] = {};
		int init_count = 0;
		converter converters[N];

		#define UNIFORM_ANALOG_INPUT_CONFIG(actor_name, ...) \
			UNIFORM_ANALOG_INPUT_CONFIG_(actor_name,   __VA_ARGS__)

		#define UNIFORM_ANALOG_INPUT_CONFIG_(actor_name, ...) \
		{\
			ACTOR_CONFIG(actor_name)\
			, actor_name##_INIT_COUNT_BITS\
			, {\
			 __VA_ARGS__\
			}\
		}

		virtual void begin(void) {
			actor::begin();
			ACTOR_CONFIG_S(cfg);
			ACTOR_PRESENT_S(p);
			R* a = acc;
			for (int i = 0; i < N; ++i, ++a) {
				*a = 0;
			}
			init_count = 1 << cfg.init_count_bits;
			for (int i = 0; i < N; ++i) {
				converters[i].begin();
			}
			p.ready = false;
		};

		virtual void convert(void) {
			ACTOR_PRESENT_S(p);
			signal_t* v = p.values;
			R tmp_raw[N];
			R* n = tmp_raw;
			{
#if ROBO_APP_ULTRACOMPACT == 0
				::robo::system::guard g__;
#else
				//typename driver::guard g__;
#endif
				std::copy_n(p.raw, N, tmp_raw);
		}
			for (int i = 0; i < N; ++i, ++n, ++v) {
				*v = converters[i](*n);
			}
	}

		virtual void run(void) {
			ACTOR_CONFIG_S(cfg);
			ACTOR_PRESENT_S(p);
			driver::query(p.raw);
			if (!p.ready) {
				R* a = acc;
				R* n = p.raw;
				for (int i = 0; i < N; ++i, ++a, ++n) {
					*a += *n;
				}
				init_count--;
				if (init_count == 0) {
					typename number::signal_t* v = p.startup.values;
					R* o = p.startup.raw;
					R* a = acc;
					int shift = cfg.init_count_bits;
					for (int i = 0; i < N; ++i, ++v, ++o, ++a) {
						*o = (R)((*a + (1 << (shift - 1))) >> shift) + 1;
						*v = converters[i](*o);
					}
					p.ready = true;
				}
			}
		}

	private:
		// ¬спомогательный конструктор (без подсистемы)
		template <std::size_t... Is>
		uniform_analog_input_t(const config_s& _config, present_s& _present,
			std::index_sequence<Is...>)
			: actor(_config.tag, _present.tag),
			converters{ converter(_config.converters[Is])... }
		{}

	#if ROBO_APP_ULTRACOMPACT == 0
		// ¬спомогательный конструктор (с подсистемой)
		template <std::size_t... Is>
		uniform_analog_input_t(const config_s& _config, present_s& _present,
			subsystem& _subsystem,
			std::index_sequence<Is...>)
			: actor(_config.tag, _present.tag, _subsystem),
			converters{ converter(_config.converters[Is])... }
		{}
	#endif

public:
	uniform_analog_input_t(const config_s& _config, present_s& _present)
		: uniform_analog_input_t(_config, _present,
			std::make_index_sequence<N>{}) {}

#if ROBO_APP_ULTRACOMPACT == 0
	uniform_analog_input_t(const config_s& _config, present_s& _present,
		subsystem& _subsystem)
		: uniform_analog_input_t(_config, _present, _subsystem,
			std::make_index_sequence<N>{}) {}

#endif

	};



	// line_converter дл€ uniform_analog_input_t
	template<class number>
	struct line_analog_input_t {
		using signal_t = typename number::signal_t;
		using discret_t = typename number::discret_t;

		typename number::discret2signal scaler;

		struct config_s {
			typename number::discret2signal::config_s scaler;
			robo::cstr name;
		};

#define LINE_ANALOG_INPUT_CONFIG(pref)  LINE_ANALOG_INPUT_CONFIG_(pref)
#define LINE_ANALOG_INPUT_CONFIG_(pref) \
    {                                       \
        DISCRET2SIGNAL_CONFIG(pref)         \
        , RT(#pref)                         \
    }

		const config_s & config;   // <-- замен€ем ссылку на указатель

		void begin(void) {
			scaler.begin();
		}

#if ROBO_APP_BURST_VARTREE_ENABLED
		void do_regvar_conf(void) {
			using namespace burst::var;
			if (actual_mode >= mode::config) {
				push(config.name); // используем указатель
				varreg(RT("sg"), number::var::signal, scaler.config.range.signal);
				varreg(RT("raw"), number::var::discret, scaler.config.range.raw);
				pop();
			}
		}
#endif

		signal_t operator() (const discret_t& _value) {
			return scaler.run(_value);
		}


		line_analog_input_t(const config_s& _config)
			: scaler(_config.scaler)
			, config(_config) {}


	};

}
#endif