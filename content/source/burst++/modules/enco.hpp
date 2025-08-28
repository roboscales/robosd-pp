#ifndef burst_enco_hpp
#define burst_enco_hpp
#include "burst++/modules/actor.hpp"
namespace burst {
	#ifndef BURST_ENCO_ONLINE_RESTART
	#define BURST_ENCO_ONLINE_RESTART 1
	#endif
	template<class number> class enco_t : public actor {
	public:
		using signal_t = typename number::signal_t;
		using long_signal_t = typename number::long_signal_t;
		struct config_s {
			actor::config_s tag;
		};
		#define ENCO_CONFIG(a) ENCO_CONFIG_(a)
		#define ENCO_CONFIG_(a)\
		{\
			ACTOR_CONFIG(a)\
		}
		struct present_s {
			actor::present_s tag;
			struct {
				uint32_t fault;
				uint32_t total;
			} counter;
			signal_t delta_acc;
			long_signal_t position;
			bool ready;
		};
		
		#if ROBO_APP_BURST_VARTREE_ENABLED
		virtual void do_regvar_present(void) {
			using namespace burst::var;
			ACTOR_PRESENT_S(p);
			push(RT("counter"));
			reg(types::uint32, p.counter.fault, RT("fault"));
			reg(types::uint32, p.counter.total, RT("total"));
			pop();
			if (actual_mode >= mode::full) {
				reg(number::var::const_signal, p.delta_acc, RT("delta_acc"));
				reg(number::var::const_long_signal, p.position, RT("pos"));
				reg(types::const_uint8, p.ready, RT("ready"));
			}
		}
		virtual void do_regvar_conf(void) {}
		#endif	


		enco_t(const config_s& _config, present_s& _present)
			: actor(_config.tag, _present.tag) {};
		enco_t(const config_s& _config, present_s& _present, subsystem& _subsystem)
			: actor(_config.tag, _present.tag, _subsystem) {};

		virtual void begin(void) {
			ACTOR_PRESENT_S(p);
			p = {};			
		}
	};

	template<class number, class driver> class enco_abs32_t : public enco_t<number> {
		using B = enco_t<number>;
		using R = typename driver::native_t;
	public:
	
		using signal_t = typename number::signal_t;
		using long_signal_t = typename number::long_signal_t;
		struct  config_s {
			typename B::config_s ref;
			struct {
				uint8_t round;
				uint8_t raw;
				uint8_t actual;
			} resolution;
			uint8_t init_count_bits;
			struct {
				R native;
				long_signal_t position;
			} offset;
			bool inverce;
		};
		#define ENCO_ABS32_CONFIG(a) ENCO_ABS32_CONFIG_(a)
		#define ENCO_ABS32_CONFIG_(a)\
			{\
				ENCO_CONFIG(a)\
				,{\
					a##_RESOLUTION_ROUND\
					, a##_RESOLUTION_RAW\
					, a##_RESOLUTION_ACTUAL\
				}\
				, a##_INIT_COUNT_BITS\
				,{\
					a##_OFFSET_NATIVE\
					,a##_OFFSET_POSITION\
				}\
				,a##_INVERCE\
			}
		//burst_enco_t ref;
		//void (*query)(void);
		//uint32_t(*encode)(void);
		//burst_bool_t(*error)(void);
		uint32_t start_pause_tick = 0;
		struct {
			long_signal_t native;
			long_signal_t position;
		} offset;
		struct {
			uint8_t raw;
			uint8_t value;
		} shift;

		struct present_s {
			typename B::present_s ref;
			struct {
				R raw;
				uint32_t ceiled;
				int32_t delta;
			} native;
			signal_t delta;
			long_signal_t acc;
		};
		#if ROBO_APP_BURST_VARTREE_ENABLED
		virtual void do_regvar_present(void) {
			enco_t<number>::do_regvar_present();
			using namespace burst::var;
			if (actual_mode >= mode::full) {
				ACTOR_PRESENT_S(p);
				push(RT("native"));
				reg((types)descriptor_enco(sizeof(R), false, true, false), p.native.raw, RT("raw"));
				reg(types::const_uint32, p.native.ceiled, RT("ceiled"));
				reg(types::const_int32, p.native.delta, RT("delta"));
				pop();
				reg(number::var::const_signal, p.delta, RT("delta"));
				reg(number::var::const_long_signal, p.acc, RT("acc"));
			}
		}
		virtual void do_regvar_conf(void) {
			enco_t<number>::do_regvar_conf();
			using namespace burst::var;
			if (actual_mode >= mode::tuning) {
				ACTOR_CONFIG_S(c);
				push(RT("offset"));
				reg((types)descriptor_enco(sizeof(R), false, false, false), c.offset.native, RT("native"),true);
				reg(number::var::long_signal, c.offset.position, RT("pos"),true);
				pop();
				if (actual_mode >= mode::config) {
					push(RT("reso"));
					reg(types::uint8, c.resolution.round, RT("round"),true);
					reg(types::uint8, c.resolution.raw, RT("raw"),true);
					reg(types::uint8, c.resolution.actual, RT("actual"),true);
					pop();
					reg(types::uint8, c.init_count_bits, RT("icb"),true);
					reg(types::uint8, c.inverce, RT("inv"),true);
				}
			}
		}
		#endif	
		enco_abs32_t(const config_s& _config, present_s& _present)
			: B(_config.ref, _present.ref) {};
		enco_abs32_t(const config_s& _config, present_s& _present, subsystem& _subsystem)
			: B(_config.ref, _present.ref, _subsystem) {};

		virtual void begin(void) {
			B::begin();
			ACTOR_CONFIG_S(cfg);
			ACTOR_PRESENT_S(p);
			shift.raw = (cfg.resolution.round - cfg.resolution.raw);
			shift.value = (cfg.resolution.raw - cfg.resolution.actual);

			p.native = {};
			p.delta = 0;
			p.ref.position = cfg.offset.position;
			offset.native = cfg.offset.native;
			offset.position = cfg.offset.position;
			start_pause_tick = 1 << cfg.init_count_bits;
		}
		void movezero(long_signal_t _delta) {
			ACTOR_CONFIG_S(conf);
			ACTOR_PRESENT_S(p);
			p.ref.position -= conf.offset.position;
			*(const_cast<long_signal_t *> (&conf.offset.position)) += _delta;
			offset.position += _delta;
			p.ref.position += conf.offset.position;
		}
		virtual void run(void) {
			ACTOR_CONFIG_S(conf);
			ACTOR_PRESENT_S(p);

			p.ref.counter.total++;
			bool success = driver::query(p.native.raw);
			if (p.ref.ready) {
				if (success) {
					uint32_t tmp = robo::digit::lsh( (uint32_t)p.native.raw, shift.raw);
					int32_t  tmp_delta = (int32_t)(tmp - p.native.ceiled);

					p.native.ceiled = tmp;

					p.native.delta = robo::digit::rsh(tmp_delta, shift.raw);// (((native_t)(tmp_delta)) >> shift);
					long_signal_t dtmp;
					if (conf.inverce) {
						dtmp = robo::digit::rsh(-p.native.delta, shift.value);
					}
					else {
						dtmp = robo::digit::rsh(p.native.delta, shift.value);
					}

					if (dtmp > number::max) {
						p.delta = p.ref.delta_acc = number::max;
					}
					else if (dtmp < number::min) {
						p.delta = p.ref.delta_acc = number::min;
					}
					else {
						p.delta = dtmp;
						long_signal_t adtmp = p.ref.delta_acc + p.delta;
						if (dtmp > number::max) {
							p.ref.delta_acc = number::max;
						}
						else if (dtmp < number::min) {
							p.ref.delta_acc = number::min;
						}
						else {
							p.ref.delta_acc = adtmp;
						}
					}
					
					#if BURST_ENCO_ONLINE_RESTART
					if(!board::if_configure()){
						if (conf.offset.native != offset.native || conf.offset.position != offset.position) {
							begin();
							return;
						}
					}
					#endif

				}
				else {
					p.ref.counter.fault++;
					//p.native.delta = p.ref.delta_acc = 0;
					//to do так как закоментировано - так делать нельзя, та как накапливается ошибка!
					/*present.native.raw += present.native.delta;
					present.native.ceiled += (present.native.delta << shift);*/
					p.ref.delta_acc += p.delta;
				}
				if (success) {
					if (conf.inverce) {
						p.acc -= p.native.delta;
					}
					else {
						p.acc += p.native.delta;
					}
				//todo round_l не катит
					p.ref.position = robo::digit::rsh(p.acc, shift.value);
					p.ref.position += conf.offset.position;
				}
			}
			else {
				if (success) {
					p.native.ceiled = robo::digit::lsh((uint32_t)p.native.raw, shift.raw);
					uint32_t tmp = p.native.ceiled + robo::digit::lsh((uint32_t)conf.offset.native, shift.raw);
					if (conf.inverce) {
						tmp = 0xFFFFFFFF - tmp;
					}
					p.acc = robo::digit::rsh(((long_signal_t)tmp), shift.raw);
					p.ref.position = robo::digit::rsh(p.acc, shift.value);
					p.ref.position += conf.offset.position;
					start_pause_tick--;
					if (start_pause_tick == 0) {
						p.ref.ready = true;
					}
				}
				else {
					p.ref.counter.fault++;
				}
			}
		}
	};
class resolver_driver_s {
		public:
		enum class statuses { wait = 0,fault, ready};
		struct  present_s {
			uint32_t ceiled;			
			statuses status;
			bool restart;
		};
		private:
		present_s &  present_;
		public:

		template <typename T>  T & present(void) {
			return reinterpret_cast < T&>(present_);
		}
		#define RESOLVER_PRESENT_S(s) present_s& s= resolver_driver_s::template present<present_s>()
		
		resolver_driver_s(present_s& _present)
			: present_(_present) {};
	};
	template<class number>	struct  resolver_sincos_raw_t{
		typename number::signal_t sn;
		typename number::signal_t cs;
		uint16_t rphase;
		//bool 
	};
	template<class number, class driver,class generator> class resolver_sincos_t : public resolver_driver_s{
		using B =  resolver_driver_s;
		using statuses = B::statuses;
		enum class modes {wait_prf=0, ready};
	public:
		using raw_s =  resolver_sincos_raw_t<number>;

		struct  present_s {
			typename B::present_s ref;
//			raw_s raw;
			uint16_t rphase;
			uint32_t rsin;
			typename number::signal_t teta;
			bool enable;
		//	typename number::signal_t sn;
			burst::resolver_sincos_raw_t<number> raw;
			bool data_enable;
		};
		#if ROBO_APP_BURST_VARTREE_ENABLED
		void present_reg(void) {
			RESOLVER_PRESENT_S(p);

			using namespace burst::var;
			push(RT("sico"));
			reg(types::const_uint32, p.ref.ceiled, RT("ceiled"));
			reg(types::const_uint32, p.ref.status, RT("status"));
			reg(types::const_uint8,  p.ref.restart, RT("restart"));
			reg(types::const_uint16, p.rphase, RT("rphase"));
			reg(types::const_uint32, p.rsin, RT("rsin"));
			reg(number::var::signal, p.teta, RT("teta"));

		//	reg(number::var::signal, p.sn, RT("sn"));

			reg(number::var::signal, p.raw.sn, RT("raw.sn"));
			reg(number::var::signal, p.raw.cs, RT("raw.cs"));
			reg(number::var::signal, p.raw.rphase, RT("raw.rphase"));


			pop();
		}
		#endif
		/*
		bool data_enable(){
			RESOLVER_PRESENT_S(p);
			auto tmp = p.data_enable;
			p.data_enable = false;
			return tmp;
		}*/
		
		
		resolver_sincos_t(present_s& _present)
			: B(_present.ref){
				fill();
		}
		constexpr static inline const			uint32_t time_nsb =    (uint32_t)( 1024000000./generator::freq_hz/generator::table_size +0.5);

		void start(void){
			driver::start(generator::sin_array,generator::table_size,time_nsb);				
			RESOLVER_PRESENT_S(p);
			p.enable = true;
		}
		
		void  fill(void) {
			using long_signal_t=  typename number::long_signal_t;
			using signal_t=  typename number::signal_t;
			constexpr long_signal_t amp_pp =    (long_signal_t)(generator::amp*generator::table_zero +0.5);
			constexpr long_signal_t step_pp =    (long_signal_t)( (4294967346. + generator::table_size/2)/ generator::table_size);

			typename number::long_signal_t phase = 0;
			for (int i=0; i<generator::table_size;++i,phase+=step_pp) {
				typename number::long_signal_t sphase;
				if(phase==0){
					sphase=0;
				} else if(phase>0){
					sphase=(signal_t)((phase+32767)>>16);
				} else{
					sphase=(signal_t)((phase-32767)>>16);
				}
				typename number::long_signal_t y = amp_pp*number::sin( sphase);
				typename number::signal_t sy;
				if(y==0){
					sy=0;
				} else if(y>0){
					sy=(signal_t)((y+16383)>>15);
				} else{
					sy=(signal_t)((y-16383)>>15);
				}
				auto n= (i+generator::table_offset) %generator::table_size;
				generator::sin_array[i] = sy + generator::table_zero;
			}				
		}
		
		void run(void){
			RESOLVER_PRESENT_S(p);
			if(p.enable){
				if(p.data_enable){
					p.rphase = p.raw.rphase;
					p.rphase+=generator::table_mess_offset;
					while (p.rphase >= generator::table_size)
						p.rphase -= generator::table_size;
					p.rsin = generator::sin_array[p.rphase];
					
					if(p.rsin>generator::table_mess_th+generator::table_zero){
						p.teta = number::atan2(p.raw.sn, p.raw.cs);
						p.ref.ceiled = number::l_rad2ceil (p.teta);
						p.ref.status = statuses::ready;
					} else if(p.rsin<-generator::table_mess_th+generator::table_zero){
						p.teta = number::atan2(-p.raw.sn, -p.raw.cs);
						p.ref.ceiled = number::l_rad2ceil (p.teta);
						p.ref.status = statuses::ready;
					} else {
						p.ref.status = statuses::wait;
					}
					p.data_enable = false;
				} else {
						p.ref.status = statuses::fault;
				}
			} else {				
				p.ref.status = statuses::wait;
			}
		}
	};

			
	template<class driver> struct resolver_driver_t: public burst::resolver_driver_s, public driver {
		using B =  burst::resolver_driver_s;		
		using statuses = B::statuses;		

		struct  present_s {
			typename B::present_s ref;
			typename driver::native_t raw;
		};
		resolver_driver_t(present_s& _present)
			: B(_present.ref){
		}
		void run(void){
			RESOLVER_PRESENT_S(p);
			if(driver::query(p.raw)){
				p.ref.status = statuses::ready;
				p.ref.ceiled = p.raw<<(32-driver::resolution);
			}	else {
				p.ref.status = statuses::fault;
			}
		}
		#if ROBO_APP_BURST_VARTREE_ENABLED
		void present_reg(void) {
			RESOLVER_PRESENT_S(p);
			using namespace burst::var;
			push(RT("rsd"));
			reg(types::const_uint32, p.ref.ceiled, RT("ceiled"));
			reg(types::const_uint32, p.ref.status, RT("status"));
			reg(types::const_uint8, p.ref.restart, RT("restart"));
			reg(const_unsigned_type(p.raw), p.raw, RT("raw"));
			pop();
		}
		#endif
	};
	
	template<class number, class hidriver_s,class lowdriver_s> class resolver_x2_t : public enco_t<number>,public hidriver_s, public lowdriver_s {
		using B = enco_t<number>;
		//using R = typename driver::native_t;
		//using H = typename driver::hi_resolution_t;
		//using L = typename driver::lo_resolution_t;
	public:
	
		using signal_t = typename number::signal_t;
		using long_signal_t = typename number::long_signal_t;
		using statuses = resolver_driver_s::statuses;
		struct  config_s {
			typename B::config_s ref;
			uint8_t resolution;
			uint8_t sence_hi_segment_bits;
			uint8_t init_count_bits;
			struct {
				uint32_t sence_hi;
				uint32_t native;
				long_signal_t position;
			} offset;
			bool inverce;
			bool allwaice_splice;
		};
		#define RESOLVER_X2_CONFIG(a) RESOLVER_X2_CONFIG_(a)
		#define RESOLVER_X2_CONFIG_(a)\
			{\
				ENCO_CONFIG(a)\
				, a##_RESOLUTION\
				, a##_SENCE_HI_SEGMENT_BITS\
				, a##_INIT_COUNT_BITS\
				,{\
					a##_OFFSET_SENCE_HI\
					,a##_OFFSET_NATIVE\
					,a##_OFFSET_POSITION\
				}\
				,a##_INVERCE\
				,a##_ALLWAIS_SPLICE\
			}

		uint32_t start_pause_tick = 0;
		struct {
			uint32_t native;
			long_signal_t position;
		} offset;
		struct {
			uint8_t raw;
//			uint8_t value;
		} shift;

		struct present_s {
			typename B::present_s ref;
			struct {
				uint32_t actual;
				uint32_t prev;
				uint64_t total;
				uint64_t accum;
				uint32_t begin;
				uint32_t fault;
				int32_t delta;
			} splice;
			struct {
				uint32_t prev;
				int32_t delta;				
			} hires;
			uint32_t upper_reference;
			uint32_t upper_bits;
			uint32_t low_bits;
			struct {
				uint32_t ceiled;
				int32_t delta;
			} native;
			signal_t delta;
			long_signal_t acc;
			typename hidriver_s::present_s hidrv;
			typename lowdriver_s::present_s lowdrv;
			uint32_t svt0[3];
		};
		
		
		#if ROBO_APP_BURST_VARTREE_ENABLED
		virtual void do_regvar_present(void) {
			enco_t<number>::do_regvar_present();
			using namespace burst::var;
			ACTOR_PRESENT_S(p);
			if (actual_mode >= mode::full) {
				push(RT("splice"));
				reg(types::const_uint32, p.splice.actual, RT("actual"));
				reg(types::const_uint32, p.splice.prev, RT("prev"));
				reg(types::const_uint64, p.splice.total, RT("total"));
				reg(types::const_uint64, p.splice.accum, RT("accum"));
				reg(types::const_uint32, p.splice.begin, RT("begin"));
				reg(types::const_int32, p.splice.delta, RT("delta"));
				pop();
				push(RT("native"));
				reg(types::const_uint32, p.native.ceiled, RT("ceiled"));
				reg(types::const_int32, p.native.delta, RT("delta"));
				pop();
				reg(number::var::const_signal, p.delta, RT("delta"));
				reg(number::var::const_long_signal, p.acc, RT("acc"));
				push(RT("srvt"));
				reg(types::const_uint32, p.svt0[0], RT("0"));
				reg(types::const_uint32, p.svt0[1], RT("1"));
				reg(types::const_uint32, p.svt0[2], RT("2"));
				pop();
			}
			reg(types::uint32, p.splice.fault, RT("splice_fault"));

			if (actual_mode >= mode::full) {
				lowdriver_s::present_reg();
				hidriver_s::present_reg();
			}
		}
		
		virtual void do_regvar_conf(void) {
			enco_t<number>::do_regvar_conf();
			using namespace burst::var;
			if (actual_mode >= mode::tuning) {
				ACTOR_CONFIG_S(c);
				push(RT("offset"));
				reg(types::uint32, c.offset.sence_hi, RT("sence_hi"),true);
				reg(types::uint32, c.offset.native, RT("native"),true);
				reg(number::var::long_signal, c.offset.position, RT("pos"),true);
				pop();
				if (actual_mode >= mode::config) {
					reg(types::uint8, c.resolution, RT("reso"),true);
					reg(types::uint8, c.sence_hi_segment_bits, RT("hsb"),true);
					reg(types::uint8, c.init_count_bits, RT("icb"),true);
					reg(types::uint8, c.inverce, RT("inv"),true);
					reg(types::uint8, c.allwaice_splice, RT("alw"),true);
				}
			}
		}
		#endif	
		resolver_x2_t(const config_s& _config, present_s& _present)
			: B(_config.ref, _present.ref), hidriver_s(_present. hidrv), lowdriver_s(_present.lowdrv) {};
		resolver_x2_t(const config_s& _config, present_s& _present, subsystem& _subsystem)
			: B(_config.ref, _present.ref, _subsystem), hidriver_s(_present. hidrv), lowdriver_s(_present.lowdrv) {}; 
		bool first_tact = true;
		virtual void begin(void) {
			B::begin();
			ACTOR_CONFIG_S(cfg);
			ACTOR_PRESENT_S(p);
			shift.raw = (cfg.sence_hi_segment_bits+ hidriver_s::resolution - cfg.resolution);
	
			p = {};
			//p.abs = {};
			p.ref.position = cfg.offset.position;
			offset.native = cfg.offset.native;
			offset.position = cfg.offset.position;
			start_pause_tick = 1 << cfg.init_count_bits;
			first_tact = true;
		}
		void movezero(long_signal_t _delta) {
			ACTOR_CONFIG_S(conf);
			ACTOR_PRESENT_S(p);
			p.ref.position -= conf.offset.position;
			*(const_cast<long_signal_t *> (&conf.offset.position)) += _delta;
			offset.position += _delta;
			p.ref.position += conf.offset.position;
		}
		virtual void run(void) {
			
			ACTOR_CONFIG_S(conf);
			ACTOR_PRESENT_S(p);
			if (p.ref.ready) {
				#if BURST_ENCO_ONLINE_RESTART
				//todo govnocod
				if(!board::if_configure()){
					if (conf.offset.native != offset.native || conf.offset.position != offset.position || p.lowdrv.ref.restart || p.hidrv.ref.restart ) {
						begin();
						return;
					}
				}
				#else
				if (p.lowdrv.ref.restart || p.hidrv.ref.restart ) {
					begin();
					return;
				}
				#endif

				p.ref.counter.total++;
				hidriver_s::run();
				if(conf.allwaice_splice){
					lowdriver_s::run();
					splice();
				}
				bool success = p.hidrv.ref.status == statuses::ready;
				if (success) {
					p.hires.delta = robo::digit::rsh( (int32_t) (p.hidrv.ref.ceiled - p.hires.prev),/* conf.sence_hi_segment_bits*/(32 - hidriver_s::resolution));
					//21 bit
					p.hires.prev = p.hidrv.ref.ceiled;

					p.native.ceiled += p.hires.delta;
					
					p.native.delta = p.hires.delta;
					//p.native.delta = robo::digit::rsh(p.hires.delta, shift.raw);
					long_signal_t dtmp;
					if (conf.inverce) {
						dtmp = -p.native.delta;
					}
					else {
						dtmp = p.native.delta;
					}

					if (dtmp > number::max) {
						p.delta = p.ref.delta_acc = number::max;
					}
					else if (dtmp < number::min) {
						p.delta = p.ref.delta_acc = number::min;
					}
					else {
						p.delta = dtmp;
						long_signal_t adtmp = p.ref.delta_acc + p.delta;
						if (dtmp > number::max) {
							p.ref.delta_acc = number::max;
						}
						else if (dtmp < number::min) {
							p.ref.delta_acc = number::min;
						}
						else {
							p.ref.delta_acc = adtmp;
						}
					}
					if (conf.inverce) {
						p.acc -= p.native.delta;
					}
					else {
						p.acc += p.native.delta;
					}
					//todo round_l не катит
					p.ref.position = robo::digit::rsh(p.acc, shift.raw) ;
					p.ref.position += conf.offset.position;
				} else {
					p.ref.counter.fault++;
					//p.native.delta = p.ref.delta_acc = 0;
					//to do так как закоментировано - так делать нельзя, та как накапливается ошибка!
					/*present.native.raw += present.native.delta;
					present.native.ceiled += (present.native.delta << shift);*/
					p.ref.delta_acc += p.delta;
				}
			}
			else 
			{
				
				hidriver_s::run();
				lowdriver_s::run();
				if( p.lowdrv.ref.status != statuses::fault &&  p.hidrv.ref.status == statuses::ready){
					//bool success = driver::query_hi(p.native.raw.hires);
					p.hidrv.ref.status = statuses::wait;
					if ( p.lowdrv.ref.status == statuses::ready) {
						p.ref.counter.total++;
						p.lowdrv.ref.status = statuses::wait;
						if( ! splice() ){
							p.ref.counter.fault++;
							
						} else {
							start_pause_tick--;
							p.splice.delta =  p.splice.actual - p.splice.prev;
							p.splice.prev = p.splice.actual;
							p.splice.total += p.splice.delta;
							p.splice.accum += p.splice.total;
														
							if  (start_pause_tick==0){
								p.splice.begin = (uint32_t) robo::digit::round(p.splice.accum,conf.init_count_bits);
								p.native.ceiled = p.splice.begin;
								p.hires.prev = p.hidrv.ref.ceiled;
							} else{
								p.native.ceiled = p.splice.actual;
							}
							
							uint32_t tmp = p.native.ceiled + conf.offset.native;
							if (conf.inverce) {
								tmp = 0xFFFFFFFF - tmp;
							}
							
							p.acc = robo::digit::rsh( (long_signal_t)tmp, 32- (hidriver_s::resolution + conf.sence_hi_segment_bits) );

							p.ref.position = robo::digit::rsh(p.acc, shift.raw);
							p.ref.position += conf.offset.position;

							if (start_pause_tick == 0) {
								p.ref.ready = true;
							}
						}
						if(first_tact){
							first_tact = false;
						}
					}
				} else {
						p.ref.counter.total++;
						p.ref.counter.fault++;
				}				
			}
		}
		uint32_t upper_bits_prev= 0;;
		
		bool splice(void){
			ACTOR_CONFIG_S(conf);
			ACTOR_PRESENT_S(p);
			
			p.upper_reference =  p.lowdrv.ref.ceiled; // Показания грубого датчика - 32 бит обязательно
			static volatile uint32_t mask = ((( 1U<<conf.sence_hi_segment_bits)-1)<<(32-conf.sence_hi_segment_bits));
			p.upper_bits =  p.upper_reference & mask; //берем старшие биты грубого датчика
			
			if(!first_tact ){
				int32_t upper_reference_delta = (p.upper_bits-upper_bits_prev);
				upper_bits_prev = p.upper_bits;
				if (upper_reference_delta> (1L<<( 32-conf.sence_hi_segment_bits)) || upper_reference_delta < - (1L<<( 32-conf.sence_hi_segment_bits)) ){
					p.splice.fault++;
					return false;
				}
			} else  {
				upper_bits_prev = p.upper_bits;
			}
			p.low_bits = p.hidrv.ref.ceiled;// Показания точного датчика к  32 bit - обязательно
			p.low_bits += conf.offset.sence_hi; // Смещаем 32 bit  показания точного датчика на требуемый угол ( пи)
			p.low_bits >>= conf.sence_hi_segment_bits ; //уступаем старшие биты грубому датчику
			
			//uint32_t vt1 = (uint16_t)resolve_machine_tetta;

			
			p.svt0[0]	= p.upper_bits + p.low_bits ; 
			p.svt0[1] = p.svt0[0] - (1L<<( 32-conf.sence_hi_segment_bits));
			p.svt0[2] = p.svt0[0] + (1L<<( 32-conf.sence_hi_segment_bits));

			int32_t resolver_delta = std::numeric_limits<int32_t>::max();
			auto result = p.upper_reference ;

			for (int n  = 0; n < 3; n++) { 
				int32_t tmp_delta = (int32_t)(p.svt0[n] - p.upper_reference);
				if (abs(tmp_delta) < abs(resolver_delta)) {
					resolver_delta = tmp_delta;
					result = p.svt0[n];
				}
			}
			p.splice.actual = result;
			return true;
		}
			
	};

	
}
#endif